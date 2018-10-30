
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

#include "SigIndexHaar.h"
#include "ImgDscHaar.h"
#include "ImageMatcher.h"

namespace FaceMatch
{

/**
 * \brief Define whole-image matching functionality interface to be called from web services or command line interfaces (CLI), taking descriptor type as a template parameter.
 *
 * The whole-image matching functionality is similar to that of the face region matching as defined by ImageMatcherFaceRegions, but without the face regions specified.
 * Descriptors are assumed to be descendants of ImgDscBase. Both basic (single descriptor) and ensemble (multiple descriptor) functionality is supported for the whole-image matching.
 */
template<class TSigNdx>
class LIBDCL ImageMatcherWhole: public ImageMatcherIndexed<TSigNdx>
{
	unsigned mFlags;
public:
	/// base type synonym
	typedef ImageMatcherIndexed<TSigNdx> TBase;
	/// Instantiate.
	ImageMatcherWhole(const string & NdxFN="", ///< index file name
		unsigned ImgNormDim=DefaultWholeImgDim, ///< normalized image diameter in pixels
		unsigned flags=0 ///< flags/options
	): TBase(NdxFN, ImgNormDim, flags), mFlags(flags) {}
	virtual ~ImageMatcherWhole(){}
	/// \return number of ingested records for the given image
	virtual unsigned ingest(const string & ImgFNLine, ///< input image file name and attributes
		const string & ID="", ///< optional image ID; if omitted, use image file name
		const unsigned ImgVar=0 ///< optionally ingest image variations, e.g. crop, rotation, scale
	) override
	{	FTIMELOG
		istringstream req(ImgFNLine); string ImgFN; getline(req, ImgFN, '\t');
		string ImgID = ID.empty()? getFileName(ImgFNLine, false, true) : trimc(ID);
		this->mSigIndex.remove(ImgID);
		return this->mSigIndex.ingest(ImgFN+"\td["+ImgID+"]", Image(ImgFN), ImgVar);
	}
	/**
	 * Query the image index, output results in the similarity order.
	 * \return number of matching records
	 * \sa ImageMatcher::query
	 */
	virtual unsigned query(string & result, ///< output relevant newline separated records with tab separated values, e.g: Dist2Query ImgID
		const string & ImgFNLine, ///< input image file name and attributes
		REALNUM tolerance=0.5, ///< tolerance	distance to query threshold in [0,1] range; 0 = perfect match
		unsigned flags=0 ///< query options, e.g. skip self-match, use pi/2 image/region rotation phases, etc.
	)const override
	{	FTIMELOG
		istringstream req(ImgFNLine); string ImgFN; getline(req, ImgFN, '\t');
		Image img(ImgFN);
		if (mFlags&moHistEQ) img.eqHist();
		const auto & NDX=this->mSigIndex;
		PScoredLines pSL=NDX.query(ImgFNLine, *NDX.newDescriptor(&img), tolerance, flags);
		unsigned cnt=pSL->size();
		ostringstream res; res<<ImgFNLine<<": found "<<cnt<<'\n'<<*pSL;
		result = res.str();
		return cnt;
	}
};

}
