
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

#pragma once // 2011-2017 (C) 

#include "ImgDscMatcher.h"
#include "AccuracyEvaluation.h"
#include "Query.h"
#include "FaceRegions.h"
#include "Image.h"
#include "ColorFaceLandmarks.h"
#include <unordered_set>

using namespace std;
using namespace cv;

namespace FaceMatch
{

/// image matcher optimization options
enum EOptimization
{
	optParams=1, ///< optimize parameters
	optScale=1<<1, ///< optimize scale
	optThreshold=1<<2 ///< optimize threshold
};

/// real-valued accuracy evaluation base
typedef AccuracyEvaluation<REALNUM> TQryAccEvalBase;
/// real-valued accuracy evaluation class
struct TQryAccEval: TQryAccEvalBase
{
	unsigned TotalHitsPossible; ///< count total hits possible
	REALNUM HitCount; ///< hit count
	/// Instantiate.
	TQryAccEval(unsigned aTotalHitsPossible=0, ///< count total hits possible
		REALNUM aHitCount=0 ///< hit count
	): TQryAccEvalBase(),
		TotalHitsPossible(aTotalHitsPossible),
		HitCount(aHitCount) {}
	/// \return hit rate
	REALNUM HitRate()const{ return TotalHitsPossible ? (HitCount/TotalHitsPossible) : 0.0; }
};

/**
 * \brief image descriptor index base class
 *
 * Define a base for all FaceMatch basic (single descriptor kind) visual index classes.
 * The class declares and defines several virtual indexing functions (e.g. ingest and query), which can be overridden in the descendants.
 * Both, distance radius and nearest-K queries are supported.
 * All query results distances are designed to be in [0,1], with 0 being the perfect match, 1 serving as infinity,
 * and 0.5 being a close-to-optimal radius-query threshold, which needs to be ensured by the respective descriptor distance functions.
 *
 * \see SigIndexMany for more complex ensemble-based index visual classes.
 */
class LIBDCL ImageDescriptorIndexBase: protected unordered_map<string, PImgDscBase>
{
	typedef unordered_map<string, PImgDscBase> TBase;

	string mDscType;
	mutable PImgDscBase mDescriptor;

protected:
	/// read/write lock
	class TLock: public OMPNestedLock
	{
		atomic<bool> mWriting;
	public:
		/// Instantiate.
		TLock(bool writing=false /**< writing lock? */): mWriting(writing){}
		/// \return writing lock?
		bool writing()const{return mWriting;}
		/// Set writing lock status
		void writing(bool writing /**< writing lock? */){mWriting=writing;}
	}mLock; ///< instance lock
	/// read/write locker
	class TLocker: OMPLocker
	{
		TLock & mNLock;
		atomic<bool> mWasWriting;
	public:
		/// Instantiate.
		TLocker(const TLock & lock /**< a lock reference */): OMPLocker((TLock&)lock, lock.writing()),
			mNLock((TLock&)lock), mWasWriting(lock.writing())
		{
			if (getVerbLevel()>3) clog<<omp_get_thread_num()<<" set read lock"<<endl;
			mNLock.writing(mWasWriting);
		}
		/// Instantiate a writing lock.
		TLocker(TLock & lock /**< a lock reference */): OMPLocker(lock, true), mNLock(lock), mWasWriting(lock.writing())
		{
			if (getVerbLevel()>3) clog<<omp_get_thread_num()<<" set write lock"<<endl;
			mNLock.writing(true);
		}
		virtual ~TLocker()
		{
			if (getVerbLevel()>3) clog<<omp_get_thread_num()<<" clear lock"<<endl;
			mNLock.writing(mWasWriting);
		}
	};

#define FORALL { TLocker lock(mLock); for (auto it=begin(); it!=end(); ++it) {
#define ENDALL } }

	/// map image/region IDs to their label/group
	typedef unordered_map<string, string> MapImgRgnID2Label;
	/// ID to label map
	mutable MapImgRgnID2Label mImgRgnID2Label;

	typedef unordered_set<string> StringSet;
	/// map label/group to a key set
	typedef unordered_map<string, StringSet> Label2StringSet;
	/// count labels/groups
	mutable Label2StringSet mLabel2Keys, mLabel2FileName, mLabel2FilePath;

	unsigned mFlags; ///< flags/options
	mutable string mLabelDelimiter; ///< a label delimiter

	mutable ImgDscMatcher mIDM; ///< image descriptor matcher (e.g. BF or FLANN)

//=== helpers
	/// \return label/group by image/region ID
	virtual const string & getLabel(const string & ID /**< image/region ID */)const;
	 /// \return number of items in the group
	virtual unsigned getLabelCnt(const string & label /**< label group name */)const;
	/**
	 * Insert the descriptor of a distorted image/region.
	 * \param img	input image/region
	 * \param ID	image/region ID
	 * \param var	distortion variant
	 * \param a	distortion magnitude
	 */
	void insertVariant(const Image & img, const string & ID, const string & var, const REALNUM a);
	/**
	 * Insert the descriptor a cropped variant of an image/region.
	 * \param ID	image/region ID
	 * \param img	input image
	 * \param a	crop scale in [0,1]
	 */
	void insertCropped(const string & ID, const Image & img, const REALNUM a);
	/**
	 * Insert the descriptor a rotated variant of an image/region.
	 * \param ID	image/region ID
	 * \param img	input image
	 * \param a	rotation angle in radians
	 */
	void insertRotated(const string & ID, const Image & img, const REALNUM a);
	/**
	 * Insert the descriptor a scaled variant of an image/region.
	 * \param ID	image/region ID
	 * \param src	input image
	 * \param a real-valued scale factor
	 */
	void insertScaled(const string & ID, const Image & src, const REALNUM a);
	/**
	 * Insert descriptors of distorted variants of an image.
	 * \param ImgID	image ID
	 * \param img	input image
	 * \param ImgVar	distortion variants: ivCrop|ivRotate|ivScale
	 */
	unsigned insertVariations(const string & ImgID, Image img, const unsigned ImgVar);
	/**
	 * Insert descriptors of distorted variants of a region.
	 * \param RegID	region ID
	 * \param img	input image
	 * \param fr	input region
	 * \param ImgVar	distortion variants: ivCrop|ivRotate|ivScale
	 */
	unsigned insertVariations(const string & RegID, const Mat & img, const FaceRegion & fr, const unsigned ImgVar)
	{
		const Image ImgFR(cropAddSaliency(img, fr));
		return insertVariations(RegID, ImgFR, ImgVar);
	}
	/**
	 * Output the index to an OpenCV file storage.
	 * \param fs	output file storage
	 */
	virtual void output(FileStorage & fs)const;
	/**
	 * Compute a distance between two image descriptors.
	 * \param lhs	input descriptor
	 * \param rhs	input descriptor
	 * \return real-valued distance in [0,1]
	 */
	virtual REALNUM getDist(const ImgDscBase & lhs, const ImgDscBase & rhs)const;
	/// Query the index.
	void query(QueryResults & QryRes, ///< [out] query results
		const string & RgnID, ///< region ID
		const ImgDscBase & RgnDsc, ///< query region descriptor
		const REALNUM MatchT, ///< matching threshold
		bool skipSelfMatch=false ///< skip self match?
	)const;
	/// \return need (re)absorb indexes?
	virtual bool Need2Absorb()const;
	/// Insert a descriptor into image descriptor matcher.
	void insertIDM(PImgDscBase dsc /**< input descriptor */);
	/// Query image descriptor matcher.
	PScoredLines queryIDM(const ImgDscBase & RgnDsc, ///< region descriptor
		const REALNUM MatchT=0.5, ///< matching threshold
		bool skipSelfMatch=false ///< skip self-match?
		)const;
	/// \return number of entries erased, whose IDs match (or contain) the given string
	unsigned eraseInfix(const string & IDsub /**< ID or sub-string to match */);
	/// Increment accuracy evaluator.
	void incAccEval(TQryAccEval & ae, ///< accuracy evaluator to be incremented
		const string & RecID, ///< record ID
		const ScoredLines & SL, ///< scored lines to work with
		unsigned we ///< weighted evaluation options
		)const;
	/// Print query results to a text stream.
	void print(ostream & s, ///< output text stream
		const QueryResults & cqr, ///< query results to output
		unsigned we ///< weighted evaluation options
		)const;
	/// Print index contents to a text stream.
	virtual void dump(ostream & s ///< output text stream
		)const { print(s); }
	/// Print query results at a certain threshold value.
	void printAt(ostream & s, ///< output text stream
		const QueryResults & cqr, ///< query results to output
		REALNUM MatchT, ///< matching threshold
		unsigned we ///< weighted evaluation options
		)const;
	/// Get accuracy evaluation status.
	void getAcc(TQryAccEval & ae, ///< output accuracy evaluator
		const QueryResults & qr, ///< query results
		REALNUM MatchT, ///< matching threshold
		unsigned we ///< weighted evaluation options
		)const;
	/// Evaluate retrieval accuracy.
	void evaluate(TQryAccEval & ae, ///< output accuracy evaluator
		QueryResults & QryRes, ///< query results
		REALNUM MatchT, ///< matching threshold
		unsigned we ///< weighted evaluation options
		)const;
	/// \return number of elements removed
	virtual size_type erase(const key_type & k /**< key to be removed */){ TLocker locker(mLock); return TBase::erase(k); }
	/// Regroup image/region descriptors w.r.t. to label delimiter.
	void regroup()const;
	/// Update internal groups.
	void group(const string & ImgRgnID /**< image/region ID */ )const;
	/// Look for specified infix in the index keys/ids/labels.
	void matchInfix(StringSet & matches, ///< matching keys
		const string & IDpfx, ///< infix to match, e.g. ID/label or image file name
		bool exhaustive=false ///< use linear search, when infix is not found in smart look-ups
	)const;
public:
	using TBase::find;
	using TBase::size;
	using TBase::begin;
	using TBase::end;
	/// Instantiate.
	ImageDescriptorIndexBase(const string & ImgDscType, ///< descriptor type
		unsigned flags=0, ///< flags/options
		const string & LblDlm="/" ///< ID delimiter
		): mDscType(ImgDscType), mFlags(flags), mLabelDelimiter(LblDlm), mIDM(ImgDscType, mFlags)
	{}
	/// Destroy.
	virtual ~ImageDescriptorIndexBase(){}
	/// \return a smart pointer to a new descriptor
	virtual PImgDscBase newDescriptor(const Image * pImg=0, ///< source image pointer; if 0, create an empty descriptor
		unsigned NormDim=0 ///< normalized dimension to resize the source image to
		)const=0;
	/// \return a smart pointer to a new descriptor
	virtual PImgDscBase newDescriptor(istream & s /**< binary input stream */)const=0;
	/// \return a smart pointer to a new descriptor
	virtual PImgDscBase newDescriptor(const FileNode & dn /**< file storage input node */)const=0;
	/// \return image descriptor type
	virtual const string & getDscType()const{return mDscType;}
	/**
	 * \brief Load an index from a signature index file.
	 *
	 * The following modes are supported depending on the file extension:<br/>
	 * .txt: load from a text file as serialized by print(),<br/>
	 * {.xml|.yml|.yaml}[.gz]: load from an XML or YAML file as serialized by output(),<br/>
	 *   note: one could use file compression by specifying .xml.gz or .yml.gz,<br/>
	 * otherwise load from a binary file as serialized by write().
	 * \param	IndexFN image signature index file name
	 * \param	bClear clear the index before loading?
	 * \return	size of the loaded index
	 */
	virtual unsigned load(const string & IndexFN, bool bClear=false);
	/// Clear the descriptor collection and indexes.
	virtual void clear()
	{
		TBase::clear();
		mIDM.clear();
	}
	/// (re)absorb descriptors into respective (sub)indexes
	virtual void absorb()const;
	/// Initialize the object instance from a text stream.
	virtual void scan(istream & s /**< input text stream */){}
	/// Initialize the object instance from an XML/YML stream.
	virtual void input(FileStorage & fs /**< input XML/YML stream */);
	/// Initialize the object instance from a binary stream.
	virtual void read(istream & s /**< input binary stream */);
	/**
	 * Save image signature index to a file.
	 * If the file name has extension .txt, the output will be human readable text,
	 * otherwise it will be binary or in some special format, e.g. XML or YML (optionally g-zipped).
	 * \param IndexFN	output file name
	 */
	virtual void save(const string & IndexFN)const;
	/**
	 * Insert a descriptor.
	 * \param ID	descriptor's ID
	 * \param dsc	image descriptor to insert
	 */
	virtual void insert(const string & ID, PImgDscBase dsc);
	/**
	 * Ingest an image with its regions of interest, e.g. faces, profiles, etc.
	 * \param ImgID	image ID
	 * \param img	input image
	 * \param ImgVar	which image variations to ingest, e.g. rotations, crops, scales
	 */
	virtual unsigned ingest(const string & ImgID, const Image & img, const unsigned ImgVar=0)
	{
		return insertVariations(ImgID, img, ImgVar);
	}
	/**
	 * Ingest an image with its regions of interest, e.g. faces, profiles, etc.
	 * \param ImgID	image ID
	 * \param img	image matrix
	 * \param rgns	regions of interest
	 * \param ImgVar	which image variations to ingest, e.g. rotations, crops, scales
	 */
	virtual unsigned ingest(const string & ImgID, const Mat & img, const FaceRegions & rgns, const unsigned ImgVar=0);
	/**
	 * Remove regions specified as a new-line separated list.
	 * Each entry, whose key prefix matches regions in the list are removed.
	 * \param Regs	a new-line separated list of regions, e.g. ID<TAB>f[x,y;w,h]
	 * \return number of entries erased
	 */
	virtual unsigned remove(const string & Regs);
	/// \return scored lines for a query region
	virtual PScoredLines query0(const string & RgnID, ///< query region ID
		const ImgDscBase & RgnDsc, ///< query region descriptor
		const REALNUM MatchT, ///< matching threshold
		bool skipSelfMatch=false /// skip self match?
		)const;
	/// \return scored lines for a query region
	PScoredLines query(const string & RgnID, ///< query region ID
		const ImgDscBase & RgnDsc, ///< query region descriptor
		const REALNUM MatchT, ///< matching threshold
		bool skipSelfMatch=false /// skip self match?
		)const;
	/**
	 * Print index entries by their prefix.
	 * \param s	output text stream
	 * \param IDpfx	ID prefix
	 * \param fmt format for specialize output, e.g. <a href="http://www.robots.ox.ac.uk/~vgg/research/affine/index.html">ACRD</a>
	 * \return number of matching entries
	 */
	unsigned print(ostream & s, const string IDpfx="", const string & fmt="")const;
	/**
	 * Print the contents of the iterator.
	 * \param s	output stream
	 * \param it	index iterator to print
	 * \param fmt format for specialize output, e.g. <a href="http://www.robots.ox.ac.uk/~vgg/research/affine/index.html">ACRD</a>
	 */
	void print(ostream & s, const_iterator it, const string & fmt="")const;
	/**
	 * List index entries.
	 * \param NdxEntries	newline separated list of index entries
	 * \param request	prefix pattern for the entries to be listed
	 * \return index size
	 */
	virtual unsigned list(string & NdxEntries, const string & request="")const
	{
		stringstream s;
		unsigned cnt=print(s, request);
		NdxEntries = s.str();
		return cnt;
	}
	/**
	 * Perform distance radius query of the index.
	 * \param RgnID	region ID
	 * \param RgnDsc	region descriptor
	 * \param MatchT	distance matching tolerance in [0,1]; -1=inf, return all
	 * \param skipSelfMatch	skip self match?
	 * \return scored lines
	 */
	virtual PScoredLines queryDist(const string & RgnID, const ImgDscBase & RgnDsc, const REALNUM MatchT, bool skipSelfMatch=false)const;
	/**
	 * Test equality of two indexes.
	 * \param lhs	input index
	 * \param rhs	input index
	 * \return	are they equal?
	 */
	friend LIBDCL bool operator==(const ImageDescriptorIndexBase & lhs, const ImageDescriptorIndexBase & rhs);
	/**
	 * Evaluate the matcher using the ingested descriptors and their IDs.
	 * Log the accuracy figures in a verbose mode.
	 * \param MatchT	matching threshold; default is 0.5; at -1, evaluate at all threshold levels
	 * \param we	weighted matching; default is 0 = weNonWeighed
	 * \see query, EWeighedEvaluation
	 * \return	accuracy as f-score in [0,1]
	 */
	REALNUM evaluate(REALNUM MatchT=0.5, unsigned we=weNonWeighed)const;
	/// Evaluate the accuracy at a given threshold.
	void evaluate(TQryAccEval & ae, ///< [out] accuracy evaluator
		REALNUM MatchT, ///< matching threshold
		unsigned we ///< weighted matching options
		)const;
	/// Digest all existing descriptors into respective (sub)indexes, if forced or when necessary.
	void digest(bool force=false /**< (re)absorb everything? */)const;
	/// Set label/group delimiter.
	virtual const string & LabelDelimiter(const string & dlm /**< label delimiter, e.g. "_" or default "/" */)const;
	/// \return label/group delimiter.
	virtual const string & LabelDelimiter()const{return mLabelDelimiter;}
	/// \return saliency map channel for the source image
	MatUC computeSaliencyMap(const Mat & src, ///< input image
		const FaceRegions & landmarks ///< face landmark regions
	)const;
	/// \return image of the face region with a saliency map, if any.
	Image cropAddSaliency(const Mat & src, ///< source image
		const FaceRegion & fr ///< face region
	)const;
	/**
	 * Query the index given an image, and a set of face/profile regions
	 * \param results	[in] output format, e.g. "XML" or "YAML", default is plain text; [out] new-line separated query results
	 * \param ImgID	image ID or name
	 * \param img	image raster
	 * \param rgns	face/profile regions
	 * \param MatchT	optional matching threshold in [0,1]; default is 0.5
	 * \param flags	query options, e.g. skip self-match, use pi/2 rotation phases? default is false, etc.
	 */
	unsigned query(string & results, const string & ImgID, const Mat & img, const FaceRegions & rgns, const REALNUM MatchT=0.5, unsigned flags=0)const;
	/// Optimize the image matcher.
	virtual REALNUM optimize(REALNUM & MatchT, ///< [in] initial threshold value, [out] optimized threshold value
		unsigned flags ///< optimization flags/options
		);
	/// \return image descriptor distance scale factor. \see ImgDscBase::getDistScale()
	virtual REALNUM getDistScale()const;
	/// Set image descriptor distance scale factor. \see ImgDscBase::setDistScale().
	virtual void setDistScale(REALNUM s)const;
	/// Write the index to a binary stream.
	virtual void write(ostream & s /**< output binary stream */)const;
};

MatReal LIBDCL computeSaliencyMapLM(const Mat & src, const FaceRegions & landmarks);
MatReal LIBDCL computeSaliencyMapNN(const Mat & src, const FaceRegions & landmarks);

/**
 * Get image/face region kind from its ID string that is typically in the format RegID<tab>k[x,y;w,h], with k being the kind, e.g. [f]ace or [p]rofile.
 * \param  RegID	input region ID
 * \return	region kind
 */
string LIBDCL getRegionKind(const string & RegID);

/**
 * \brief Get image/face label from region ID string.
 *
 * ID string is typically in the format Label/RegID\<tab\>k[x,y;w,h], with k being the face region kind, e.g. [f]ace or [p]rofile.
 * There may be a label prefix after the last '/', e.g. Path/Label_ID, in which case the delimiter should be specified.
 * In case of a complex face region the ID may be encoded as d[label] with in it, e.g. k{[x,y;w,h]\<tab\>d[label]...}
 * There also may be no label present, which is handled gracefully by assuming the whole input ID may be the label.
 */
struct LIBDCL ImageRegionIDSplit
{
	string
	/// path/to/image/file.ext
		FilePath,
	/// image region label/ID
		Label;
	/// Instantiate.
	ImageRegionIDSplit(const string & ImgRgnID, ///< input face image region possibly with a path
		const string & dlm ///< ID delimiter; usually '/', but can also be '_' or any other character
	);
};

}
