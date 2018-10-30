
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

#pragma once // 2012-2016 (C) 

#include "ImgDscMany.h"
#include "SigIndexKeyPt.h"
#include "SigIndexHaar.h"
#include "ImgDscLBPH.h"
#include "ImgDscRSILC.h"
#include "ImageDescriptorIndex.h"
#include "ImgDscMatcher.h"

namespace FaceMatch
{
/// index for Local Binary Pattern Histogram (LBPH) descriptor ImgDscLBPH
typedef ImageDescriptorIndex<ImgDscLBPH> SigIndexLBPH;
/// index for Rotation and Scale Invariant Line Color (RSILC) descriptor ImgDscRSILC
typedef ImageDescriptorIndex<ImgDscRSILC> SigIndexRSILC;
/// index for Rotation and Scale Invariant Line Color (RSILC) descriptor ImgDscRSILC
typedef ImageDescriptorIndex<ImgDscRSILCS> SigIndexRSILCS;

/// OpenCV smart pointer to the index base class
typedef Ptr<ImageDescriptorIndexBase> PImgDscNdxBase;

/**
 * \brief Define a visual index ensemble for query results combination.
 *
 * This is a <a href="https://docs.google.com/document/d/12p_cwvmAy3WSo9XoaDfK2ovSMfDFtFiBEEorRc371FI/edit?pli=1">basic ensemble</a>
 * of elementary index instances typically derived from ImageDescriptorIndexBase.
 * The query results are combined using a decreasing confidence radical formula
 * \f$ D = \sqrt{d_1 \sqrt{d_2 \sqrt{d_3 ...}}} \f$
 * where the importance of the index instances is determined by their confidence order, starting with 1 being most confident.
 * The combined distance is clearly guaranteed to be in [0,1], with 0 being the perfect match, and 1 serving as infinity.\
 *
 * \note <a href="https://docs.google.com/document/d/12p_cwvmAy3WSo9XoaDfK2ovSMfDFtFiBEEorRc371FI/edit?usp=sharing">Serialization</a>
 * of this macro-index is done in two stages: binary serialization of the elementary index instances (in parallel),
 * and the text file that records the constituent index file names along with their types (HAAR, SIFT, SURF, ...) and optional weights,
 * used in the more sophisticated class descendants. Single image descriptor index files are typically stored in binary (.ndx)
 * files when running a release version of the library, or in text (.yml) files, when running a debug version for easier
 * troubleshooting. The composite (ensemble) index files are tab-delimited text files listing the weighted index single
 * descriptor (binary) indexes.
 */
class LIBDCL SigIndexMany: public ImageDescriptorIndex<ImgDscMany>
{
	mutable bool mOK2syncSubIndexes=true, mOutOfSync=false;
	MatDouble getWeights()const;
	void setWeights(const MatDouble & W);
	friend class FEvalW;
	void insert(const string & ID, PDSC rdsc, bool align);
protected:
	/// descriptor index record
	struct TDscIndex
	{
		string
			/// descriptor type
			DscType,
			/// descriptor index file name
			DscIndexFN;
		/// pointer to an image descriptor instance
		PImgDscNdxBase DscNdx;
		/// descriptor index weight; default is 1
		REALNUM DscWeight;
		TDscIndex(): DscWeight(1) {}
		/**
		 * Instantiate as a copy.
		 * \param rhs	right-hand-side to copy form
		 */
		TDscIndex(const TDscIndex& rhs){*this=rhs;}
		/**
		 * Assign as a copy.
		 * \param rhs	right-hand-side to copy form
		 * \return this const reference
		 */
		const TDscIndex& operator=(const TDscIndex& rhs)
		{
			DscType=rhs.DscType;
			DscIndexFN=rhs.DscIndexFN;
			DscNdx=rhs.DscNdx;
			DscWeight=rhs.DscWeight;
			return *this;
		}
		/**
		 * Instantiate.
		 * \param aDscType	descriptor type (HAAR, SURF, ORB, SIFT)
		 * \param aDscIndexFN	descriptor index file name
		 * \param weight	real-valued descriptor weight
		 * \param flags	options
		 * \param LblDlm	label delimiter, e.g. "_" or "/"
		 * \throw FaceMatch::Exception in case of an error
		 */
		TDscIndex(const string & aDscType, const string & aDscIndexFN, REALNUM weight, unsigned flags, const string & LblDlm);
		void write(ostream & s, bool withData=false)const;
		void read(istream & s, bool withData=false);
		virtual ~TDscIndex(){}
		friend bool operator==(const TDscIndex & lhs, const TDscIndex & rhs)
		{
			return lhs.DscType==rhs.DscType && lhs.DscWeight==rhs.DscWeight && *lhs.DscNdx==*rhs.DscNdx;
		}
	};
	/// smart pointer to DscIndex
	typedef Ptr<TDscIndex> PDscIndex;
	/// pack of smart pointers to descriptor indexes
	typedef vector<PDscIndex> TDscIndexPack;
	/// collection of descriptor indexes
	mutable TDscIndexPack mDscIndexPack;
	/// base class type definition
	typedef ImageDescriptorIndex<ImgDscMany> TBase;
	/**
	 * Insert a descriptor.
	 * \param ID	descriptor ID
	 * \param dsc	smart pointer to a descriptor of the compatible type
	 * \return reference the smart pointer of the inserted descriptor
	 */
	virtual void insert(const string & ID, PImgDscBase dsc) override;
	/**
	 * Get weight of the descriptor index by ID.
	 * \param DscID descriptor ID
	 * \return real-valued weight of the descriptor type
	 */
	REALNUM getWeight(const string & DscID)const;
	/**
	 * Combine weighted query results.
	 * \param QryResByDscr	weighted query results by descriptor
	 * \return scored lines with the combined results
	 */
	virtual PScoredLines combine(WeightedQueryResults<> & QryResByDscr)const;
	/// \return descriptor combination method
	virtual string getComboMethod()const{return "MANY";}
	/**
	 * Query the index.
	 * \param RgnID	image region ID
	 * \param RgnDsc	image region base reference
	 * \param MatchT	face match tolerance
	 * \param skipSelfMatch	skip self-match?
	 * \return query results
	 */
	virtual PScoredLines query0(const string & RgnID, const ImgDscBase & RgnDsc, const REALNUM MatchT, bool skipSelfMatch)const override;
	virtual size_type erase(const key_type & k) override;
	void syncSubIndexes()const;
public:
	/**
	 * Instantiate.
	 * Constituent index order matters for future computations.
	 * \param dim	face patch dimension
	 * \param flags	face matcher options
	 */
	SigIndexMany(unsigned dim, unsigned flags);
	using TBase::query;
	virtual const string & LabelDelimiter(const string & dlm)const override;
	virtual const string & LabelDelimiter()const override {return TBase::LabelDelimiter();}
	/// In addition to the base functionality, support .mdx per face region individual descriptor index files.
	virtual void save(const string & IndexFN)const override;
	/// In addition to the base functionality, support .mdx and .ndx per-face region individual descriptor index files.
	virtual unsigned load(const string & IndexFN /**< input index file name(s) delimited by new lines for batch ingest */, bool bClear = false)override;
	virtual void output(FileStorage & fs)const override;
	virtual void write(ostream & s)const override;
	virtual bool Need2Absorb()const override;
	virtual void absorb()const override;
	/**
	 * Allocate a new descriptor.
	 * \param img	a constant image pointer
	 * \param dim	patch diameter to normalize to; 0 = no normalization
	 */
	virtual PImgDscBase newDescriptor(const Image * img=0, unsigned dim=0)const override;
	virtual void clear()override;
	virtual void scan(istream & s)override;
	virtual void input(FileStorage & fs)override;
	virtual void read(istream & s)override;
	virtual REALNUM optimize(REALNUM & MatchT, unsigned flags) override;
	friend LIBDCL bool operator==(const SigIndexMany & lhs, const SigIndexMany & rhs);
};

/**
 * \brief Define a visual <a href="https://docs.google.com/document/d/12p_cwvmAy3WSo9XoaDfK2ovSMfDFtFiBEEorRc371FI/edit?pli=1">index</a> for distance based query results combination.
 * \image HTML FaceMatching.png "Face image matching. Image descriptors: HAAR, SIFT, SURF, ORB, LBPH."
 * As its base, this macro-index maintains an ensemble of simple index instances (that typically descend from ImageDescriptorIndexBase),
 * querying them in parallel, and combining query results based on weighted distance scores as \f$d = \prod_i d_i^{w_i}\f$,
 * where \f$d_i \in [0,1]\f$ is the distance measured by i-th descriptor index, and \f$w_i \in [0,1]\f$ is its weight in the descriptor ensemble.
 * Individual descriptor index weights are determined empirically, e.g. using their matching accuracy on some representative data-set.
 * \note The weights later can be optimized with respect to a changed image data-set, which evidently needs to be annotated.
 */
class LIBDCL SigIndexManyDist: public SigIndexMany
{
protected:
	/**
	 * Compute distance between two ensemble descriptors as a product of constituent weighted distances:
	 * \f$d = \prod_i d_i^{w_i}\f$,
	 * where \f$d_i \in [0,1]\f$ is the distance measured by i-th descriptor index,
	 * and \f$w_i \in [0,1]\f$ is its weight in the descriptor ensemble.
	 */
	virtual REALNUM getDist(const ImgDscBase & lhs, const ImgDscBase & rhs)const override;

	virtual PScoredLines combine(WeightedQueryResults<> & QryResByDscr)const override;
	virtual string getComboMethod()const override{return "DIST";}
public:
	/// Instantiate.
	SigIndexManyDist(unsigned dim, ///< face/image diameter in pixels
		unsigned flags ///< flags/options
		): SigIndexMany(dim, flags) {}
	virtual REALNUM getDistScale()const override;
	virtual void setDistScale(REALNUM s)const override;
};

/**
 * Define an index for re-ranking based index query combination.
 */
class LIBDCL SigIndexManyRank: public SigIndexMany
{
protected:
	virtual PScoredLines combine(WeightedQueryResults<> & QryResByDscr)const override;
	virtual string getComboMethod()const override{return "RANK";}
public:
	/// Instantiate.
	SigIndexManyRank(unsigned dim, ///< face/image diameter in pixels
		unsigned flags ///< flags/options
		): SigIndexMany(dim, flags)
	{
		mFlags=flags|dmRank; // needed for parallel queries of the constituents
	}
};

}
