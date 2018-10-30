
//Informational Notice:
//
//This software was developed under contract funded by the National Library of Medicine, which is part of the National Institutes of Health, an agency of the Department of Health and Human Services, United States Government.
//
//The license of this software is an open-source BSD license.  It allows use in both commercial and non-commercial products.
//
//The license does not supersede any applicable United States law.
//
//The license does not indemnify you from any claims brought by third parties whose proprietary rights may be infringed by your usage of this software.
//
//Government usage rights for this software are established by Federal law, which includes, but may not be limited to, Federal Acquisition Regulation (FAR) 48 C.F.R. Part52.227-14, Rights in Data?General.
//The license for this software is intended to be expansive, rather than restrictive, in encouraging the use of this software in both commercial and non-commercial products.
//
//LICENSE:
//
//Government Usage Rights Notice:  The U.S. Government retains unlimited, royalty-free usage rights to this software, but not ownership, as provided by Federal law.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//?	Redistributions of source code must retain the above Government Usage Rights Notice, this list of conditions and the following disclaimer.
//
//?	Redistributions in binary form must reproduce the above Government Usage Rights Notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
//?	The names,trademarks, and service marks of the National Library of Medicine, the National Cancer Institute, the National Institutes of Health, and the names of any of the software developers shall not be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE U.S. GOVERNMENT AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITEDTO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE U.S. GOVERNMENT
//OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once // 2012-2017 (C) 

#include "ImageMatcher.h"
#include "SigIndexMany.h"
#include "FaceFinder.h"

namespace FaceMatch
{

/**
 * \brief Face region descriptor matching template taking a descriptor index type (typically derived from ImageDescriptorIndexBase) as a parameter.
 *
 * Define a template for all visual index classes in FaceMatch library, making use of the ImageDescriptorIndexBase descendants for visual indexing.
 * A visual index is implemented as a map of face/image IDs to their descriptors, typically derived from ImgDescBase.
 * Ordered keys (ID) are typically strings that uniquely identify face/image regions in a data-set, e.g. ImageURL<tab>FaceRegion.
 * Values are typically <a href="http://docs.opencv.org/modules/core/doc/basic_structures.html#ptr">smart pointers</a>
 * to ImgDscBase descendants, e.g. ImgDscRSILC for key-line based matching or ImgDscHaar for color-based wavelet matching.
 * A visual index can be saved to or loaded from a binary or a text file in the key-value sequence,
 * hence each image descriptor needs to implement its own binary and text serialization.
 *
 * \note Binary serialization is much quicker compared to text, but the latter being human-readable can be used for troubleshooting.
 */
template<class TSigNdx>
class LIBDCL ImageMatcherFaceRegionsBase: public ImageMatcherIndexed<TSigNdx>
{
	typedef ImageMatcherIndexed<TSigNdx> TBase;
	FaceRegionDetector & mFRD;
	unsigned mFaceFinderFlags;
	static FaceRegionDetector & singletonFRD()
	{
		StaticLkdCtor FaceRegionDetector FRD;
		return FRD;
	}
public:
	static const unsigned sFaceFinderFlags = FaceFinder::selective | FaceFinder::rotation | FaceFinder::generateID; // TODO: | FaceFinder::HistEQ; causes no key-point warnings in unit tests
	/// Instantiate.
	ImageMatcherFaceRegionsBase(const string & NdxFN="", ///< index file name appropriate for TSigNdx
		FaceRegionDetector & FRD=singletonFRD(), ///< face region detector instance to refer to during face localization
		unsigned FaceFinderFlags=sFaceFinderFlags, ///< face finder flags as defined by FaceMatch::FaceFinder::ProcFlags
		unsigned ImgNormDim=DefaultFacePatchDim, ///< normalized face region diameter=max(width,height) in pixels
		unsigned flags=0 ///< face match flags/options as defined by FaceMatch::EImgDscNdxOptions
	): TBase(NdxFN, ImgNormDim, flags),
		mFRD(FRD), mFaceFinderFlags(FaceFinderFlags)
	{}
	virtual ~ImageMatcherFaceRegionsBase(){}
	/**
	 * Ingest the given image regions into the descriptor index.
	 * \param ImgPathFNExt		input image path/file name optionally followed by tab delimited face regions with the tab delimited attributes, e.g. ImgPathFN	f{[x,y;w,h]	d[ID1]}	f{[x,y;w,h]	d[ID2]}...
	 * \param Regs	optional person/face IDs and their regions (new-line delimited) that will be referenced in query output:<br/>
	 * 		ID1<tab>RoI<br/>
	 * 		ID2<tab>RoI<br/>
	 * 		...<br/>
	 * 	where IDi is the person ID/label and the region of interest (RoI) is defined as a rectangle {f|p}[x,y;w,h], where f=face and p=profile.
	 * 	For a single ID, the RoI is optional and will be located as the most-central face/profile.
	 * 	For multiple IDs, RoIs are required.
	 * \param ImgVar	ingest region variations (e.g. crop, rotation, scale), typically for evaluation purposes
	 * \return number of ingested records
	 */
	virtual unsigned ingest(const string & ImgPathFNExt, const string & Regs="", const unsigned ImgVar=0) override
	{	FTIMELOG
	//--- remove potential duplicates 
		string ImgPathFN = getFileName(ImgPathFNExt, true, true);
		stringstream strm(Regs.empty() ? ImgPathFN : Regs); // TODO: remove ID-RoI pairs syntax; rely on the newer d[attribute] syntax instead
		string ImgID; getline(strm, ImgID, '\t'); ImgID=getFileName(ImgID, false, true); 
		if (!ImgID.empty() && ImgID==Regs)
			this->remove(ImgID); // remove potential duplicates
	//--- parse face regions, when provided; or detect them, if not
		FaceFinder ff(mFRD, "", ImgPathFNExt, Regs.empty() ? ImgID : Regs, mFaceFinderFlags);
		unsigned VL = getVerbLevel();
		if (ff.getFaces().empty())
		{
			if (VL) clog << "no face regions to ingest in '" + ImgPathFN + "', Regs=" + Regs << endl;
			return 0;
		}
		if (VL>1) clog<<ImgPathFN<<ff.getFaces()<<endl;
	//--- ingest
		TimeLog(SigIndexIngest);
		unsigned cnt = this->mSigIndex.ingest(ImgPathFN,
			ff.getOriginalImage().empty() ? Image(ImgPathFN).mx() : ff.getOriginalImage(),
			ff.getFaces(), ImgVar);
		return cnt;
	}
	/**
	 * Query the image index, output results in the ascending distance-to-query order.
	 * \param result	output relevant, newline separated records with tab separated values in each line, e.g: Dist2Query<tab>ID<tab>RoI<nl> etc.
	 * \param request	input query request, e.g: ImgFN<tab>RoI1<tab>RoI2..., with tab-delimited regions
	 * \param tolerance	threshold on query distance score in [0,1] range, or top-N records, as defined in ImageMatcher::query
	 * \param flags	query options, e.g. skip self-match, use pi/2 image/region rotation phases, etc; as defined by FaceMatch::eQueryOptions
	 * \return number of matching records
	 */
	virtual unsigned query(string & result, const string & request, REALNUM tolerance=0.5, unsigned flags=0)const override
	{	FTIMELOG
		FaceFinder ff(mFRD, "", request, mFaceFinderFlags|FaceFinder::selective);
		if (ff.getImageFN().empty()) throw Exception("unable to query using empty image file name");
		if (ff.getOriginalImage().empty()) throw Exception("unable to query using empty image "+ff.getImageFN());
		if (ff.getFaces().empty()) throw Exception("no detected face regions to query in "+ff.getImageFN());
		TimeLog(SigIndexQuery);
		unsigned cnt = this->mSigIndex.query(result, ff.getImageFN(), ff.getOriginalImage(), ff.getFaces(), tolerance, flags);
		return ff.getFaces().size() ? cnt : 0;
	}
	virtual bool usingGPU()const{return mFRD.usingGPU();}
};

/// default image <a href="https://docs.google.com/document/d/12p_cwvmAy3WSo9XoaDfK2ovSMfDFtFiBEEorRc371FI/edit?usp=sharing">descriptor ensemble</a> matcher for face/profile regions to be used in web services (WS) or command-line interface (CLI)
typedef ImageMatcherFaceRegionsBase<SigIndexManyDist> ImageMatcherFaceRegions;

} // namespace FaceMatch
