
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

#pragma once // 2012-2017 (C) FaceMatch@NIH.gov

#include "ListStreamer.h"
#include <string>

using namespace std;

namespace FaceMatch
{

/**
 * \brief This is an abstract image matching interface (e.g. for utilization in web services).
 *
 * The caller would typically instantiate one of its descendants (e.g. whole image or face image matcher),
 * which must define the major image/face matching operations (e.g. ingest, query and remove)
 * and a few minor ones (e.g. load, save, list, count, eval, optimize), as well as some auxiliary functions (e.g. set/get LabelDelimiter).
 */
struct LIBDCL ImageMatcher
{
	/// Destroy.
	virtual ~ImageMatcher() {}
	/**
	 * Load an image matcher object from a signature index file.
	 * @param	IndexFN image signature index file name
	 * @return	number of loaded records
	 */
	virtual unsigned load(const string & IndexFN)=0;
	/**
	 * Save image signature index to a file.
	 * @param IndexFN	output file name
	 */
	virtual void save(const string & IndexFN)const=0;
	/// @return number of ingested image/region records
	virtual unsigned ingest(
		const string & ImgFN, ///< image file name optionally followed by attributes, e.g. ID
		const string & ID="", ///< image/region ID(s) that will be referenced in query output and used in remove and list functions; if omitted, ImgFN is assumed to be the ID
		const unsigned ImgVar=0 ///< image variations to ingest, e.g. crop, rotation, scale
		)=0;
	/// @return number of ingested image/region records
	virtual unsigned ingestList(const string & RepoPath, ///< image repository path ending with /
		const string & ImgLstFN, ///< image list file name
		const unsigned ImgVar=0, ///< insert variants @see ingest
		ostream * perr=0 ///< if given, exceptions will be logged there, not thrown
	);
	/// @return number of matching records
	virtual unsigned query(string & result, ///< [out] relevant newline separated records (in visual distance ascending order) with tab separated values, e.g: Dist2Query<tab>ImgID<tab>RoI etc.
		const string & request, /**< input query request, e.g: ImgFN[\\tRoI...], where optional RoIs are a set of face regions k[x,y;w,h],
			with k={p|f}, f=face, p=profile, as used by FaceFinder; regions smaller than 32 pixels across are typically not considered for matching due to the lack of visual detail */
		REALNUM tolerance=0.5, /**< real-valued threshold (t) on query distance/top-N count;
			t<0: get everything, t=0: get perfect match, t in (0,1): radius-t query; t>=1: get top-n records within distance d,
			where n is the integer part and d is the fractional part of t;
			for example, 1000.5 would retrieve at most 1000 records within the matching distance of 0.5 */
		unsigned flags=0 ///< query options, e.g. skip self-match, use rotation pi/2 phases, as defined by FaceMatch::eQueryOptions
		)const=0;
	/**
	 * Remove the image/region descriptor for the given face/image ID.
	 * Specifying "SomeID" removes all face/image regions whose keys include "SomeID" as a substring.
	 * @param ID	input image/region ID(s) to be removed by ID context
	 * @return	number of records removed
	 */
	virtual unsigned remove(const string & ID)=0;
	/// Clear the contents.
	virtual void clear()=0;
	/**
	 * List index entries.
	 * @param res [out] results list, new line separated
	 * @param req [in] request particular prefix pattern; if empty or *, return everything
	 * @return number of index entries
	 */
	virtual unsigned list(string & res, const string & req="")const=0;
	/// @return number of records
	virtual unsigned count()const=0;
	/// @return is object empty?
	virtual bool empty()const{return !count();}
	/**
	 * Evaluate the matcher using the image file list (+ params)
	 * @param RepoPath	optional image repository path
	 * @param ImgLstFN	optional image list file name
	 * @param tol	matching tolerance
	 * @param ImgVar	specify image variations to use
	 * @param we specify weighted evaluation
	 * @see EWeighedEvaluation
	 * @return	f-score in [0,1]
	 */
	virtual REALNUM eval(const string & RepoPath, const string & ImgLstFN, const REALNUM tol=0.5, const unsigned ImgVar=0, const unsigned we=weNonWeighed)=0;
	/**
	 * Optimize the matcher's parameters and/or threshold(s).
	 * @param MatchT input an initial threshold, output the optimal one
	 * @param flags indicate which parameters to optimize
	 * @param RepoPath	optional image repository path
	 * @param ImgLstFN	optional image list file name
	 * @param ImgVar	optionally use image variations too, e.g. crop, rotation, scale
	 * @return optimal F score
	 */
	virtual REALNUM optimize(REALNUM & MatchT, unsigned flags, const string & RepoPath, const string & ImgLstFN, const unsigned ImgVar=0)=0;
	/// @return label delimiter that was just set
	virtual const string & LabelDelimiter(const string & dlm /**< a label delimiter, e.g. "_" or "/" */) = 0;
	/// @return ID/label delimiter
	virtual const string & LabelDelimiter()const=0;
	/// @return text stream, where image matcher instance has been serialized
	friend LIBDCL ostream & operator<<(ostream & s, ///< output text stream
		const ImageMatcher & im ///< image matcher instance
	);
};

/**
 * \brief Define an indexed image matcher template, taking index image type as a parameter.
 *
 * Instantiate the respective visual index and provide the necessary functional specializations (overriding the base)
 * and utilities, e.g. load/save index.
 * \note Using the template ImageMatcherIndexed directly is not recommended, as it is fairly abstract and is defined mostly
 * for research purposes, and some of the production level methods (ingest and query) are actually overridden in its
 * descendant ImageMatcherFaceRegionsBase, which can in turn utilize ImageDescriptorIndex and be used with
 * single (e.g. ImgDscSIFT) or composite (e.g. ImgDscMany) descriptors.
 */
template<class TSigNdx>
class ImageMatcherIndexed: public ImageMatcher
{
protected:
	/// image descriptor signature index
	TSigNdx mSigIndex;
public:
	/// Instantiate an image matcher object by loading image signature index. @see load()
	ImageMatcherIndexed(const string & IndexFN, ///< image signature index file name; if empty, an empty index is created
		unsigned ImgNormDim, ///< normalized image/patch diameter in pixels
		unsigned flags ///< image matcher flags/options
	): mSigIndex(ImgNormDim, flags)
	{
		if (!IndexFN.empty())
			load(IndexFN);
	}
	virtual const string & LabelDelimiter(const string & dlm) override {return mSigIndex.LabelDelimiter(dlm);}
	virtual const string & LabelDelimiter()const override {return mSigIndex.LabelDelimiter();}
	unsigned ingestList(const string & RepoPath, const string & ImgLstFN, const unsigned ImgVar=0, ostream * perr=0)override
	{
		unsigned res=ImageMatcher::ingestList(RepoPath, ImgLstFN, ImgVar, perr);
		mSigIndex.digest();
		return res;
	}
	/**
	 * Load an image matcher object from a signature index file.
	 * The following modes are supported depending on the file extension:<br/>
	 * .txt: load from a text file as serialized by print(),<br/>
	 * {.xml|.yml|.yaml}[.gz]: load from an XML or YAML file as serialized by output(),<br/>
	 *   note: one could use file compression by specifying .xml.gz or .yml.gz,<br/>
	 * otherwise load from a binary file as serialized by write().
	 * @param	IndexFN image signature index file name
	 * @return	number of loaded records
	 */
	virtual unsigned load(const string & IndexFN) override
	{
		return mSigIndex.load(IndexFN);
	}
	/**
	 * Save image signature index to a file.
	 * If the file name has extension .txt, the output will be human readable text, otherwise it will be binary.
	 * @param IndexFN	output file name
	 */
	virtual void save(const string & IndexFN)const override
	{
		mSigIndex.save(IndexFN);
	}
	virtual unsigned remove(const string & ID) override
	{
		FTIMELOG
		return mSigIndex.remove(ID);
	}
	virtual void clear() override
	{	FTIMELOG
		mSigIndex.clear();
	}
	virtual unsigned list(string & res, const string & req="")const override
	{	FTIMELOG
		return mSigIndex.list(res, req);
	}
	/**
	 * Return the number of entries in the index.
	 * @return the number of index entries
	 */
	virtual unsigned count()const override
	{	FTIMELOG
		return mSigIndex.size();
	}
	/**
	 * Evaluate the matcher using the image file list (+ params)
	 * @param RepoPath	image repository path
	 * @param ImgLstFN	image list file name
	 * @param MatchT	match threshold tolerance in [0,1]; default is 0.5
	 * @param ImgVar	image variations for ingesting a list
	 * @param we	weighted evaluation kind; default is weNonWeighed
	 * @see EWeighedEvaluation
	 * @return	f-score in [0,1]
	 */
	virtual REALNUM eval(const string & RepoPath, const string & ImgLstFN, const REALNUM MatchT=0.5,
		const unsigned ImgVar=0, const unsigned we=weNonWeighed) override
	{
		if (!ImgLstFN.empty()) ingestList(RepoPath, ImgLstFN, ImgVar, &cerr);
		return mSigIndex.evaluate(MatchT, we);
	}
	virtual REALNUM optimize(REALNUM & MatchT, unsigned flags,
		const string & RepoPath, const string & ImgLstFN, const unsigned ImgVar=0) override
	{
		if (!ImgLstFN.empty()) ingestList(RepoPath, ImgLstFN, ImgVar, &cerr);
		return mSigIndex.optimize(MatchT, flags);
	}
	/**
	 * Get signature index reference.
	 * @return signature index reference
	 */
	const TSigNdx& getSigIndex()const{return mSigIndex;}
	/**
	 * Test the equality of two indexes.
	 * @param lhs	input index
	 * @param rhs	input index
	 * @return are they equal?
	 */
	friend bool operator==(const ImageMatcherIndexed & lhs, const ImageMatcherIndexed & rhs){return lhs.mSigIndex==rhs.mSigIndex;}
};

} // namespace FaceMatch
