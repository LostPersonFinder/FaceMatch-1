
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

#pragma once // 2015-2017 (C) 
#include "Query.h"
#include "ImgDscHaar.h"
#include "ImgDscKeyPt.h"
#include "ImgDscLBPH.h"
#include "ImgDscRSILC.h"
#include "ImgDscMany.h"

namespace FaceMatch
{

/// image descriptor index options
enum EImgDscNdxOptions
{
// matching options
	moNone=0, ///< no option, blank
	moHistEQ=1, ///< equalize histogram
	moGroupLabels=1<<1, ///< group labels/IDs in query output
// individual metric descriptors (re)grouping
	dmBF=1<<3, ///< descriptors (re)grouping via brute force
	dmFLANN=1<<4, ///< descriptors (re)grouping Fast Library for Approximate Nearest Neighbors (FLANN)
	dm=dmBF|dmFLANN, ///< descriptors (re)grouping via BF or FLANN
	dmOM=1<<5, ///< speed-optimal descriptor-dependent grouping/matching
	dmRank=1<<6, ///< descriptor re-ranking combination option for multi-descriptor indexes
// for saliency map computation
	smmLandMarks=1<<7, ///< saliency map computation via landmarks (eyes, nose, mouth)
	smmMachineLearning=1<<8, ///< saliency map computation via machine learning (not currently supported)
	smmAll=smmLandMarks|smmMachineLearning, ///< saliency map computation via landmarks and machine learning
};

/// \return smart pointer to a new descriptor of a known type, e.g. SIFT, SURF, ORB, etc.
template<typename T> inline
PImgDscBase newDescriptor(const string & type, ///< descriptor type, e.g. "SURF"
	T && t, ///< source to construct a descriptor from, e.g. istream
	ostream * pstrm=nullptr ///< pointer to error stream to output exceptions; re-throw errors, if NULL
)
{
	try
	{
		if (type=="HAAR" || type=="ImgDscHaarFace") return new ImgDscHaarFace(t);
		if (type=="LBPH" || type=="ImgDscLBPH") return new ImgDscLBPH(t);
		if (type=="SURF" || type=="ImgDscSURF") return new ImgDscSURF(t);
		if (type=="SIFT" || type=="ImgDscSIFT") return new ImgDscSIFT(t);
		if (type=="ORB"  || type=="ImgDscORB") return new ImgDscORB(t);
		if (type=="RSILC"  || type=="ImgDscRSILC") return new ImgDscRSILC(t);
		if (type=="RSILCS"  || type=="ImgDscRSILCS") return new ImgDscRSILCS(t);
		if (type == "MANY" || type == "ImgDscMANY") return new ImgDscMany(t);
		throw Exception("unsupported descriptor type "+type);
	}
	catch(const exception & e)
	{
		if (pstrm)
		{
			*pstrm<<"exception: "<<e.what()<<endl;
			return newDescriptor(type, nullptr);
		}
		else throw; // re-throwing the original exception
	}
	return 0;
}

/// image descriptor matcher for a known descriptor type
class LIBDCL ImgDscMatcher
{
	const string mImgDscType;
	atomic<bool> mModified;
	mutable OMPNestedLock mLock;
	/**
	 * Query the index by individual (clustered) descriptors.
	 * @param query	rows of query descriptors
	 * @param t	query threshold in the format K.D with the whole part (K) denoting Knn, and with decimal part (D) setting a radius search in [0,1)
	 * return results in the distance ascending order
	 */
	PScoredLines queryDsc(const ImgDscBase & q, REALNUM t, bool skipSelf);
	void queryRadius(const Mat & query, TMatches & matches, REALNUM t);
	PScoredLines combineMatches(const TMatches & matches, bool useDist=false)const;
	const string & getImgID(unsigned n)const;
	REALNUM getDistScale()const;
protected:
	/// associate ID with image descriptor
	struct ImgDsc
	{
		string ID; ///< descriptor ID
		PImgDscBase Dsc; ///< image descriptor
		/// Instantiate.
		ImgDsc(const string & id="", ///< descriptor ID
			const PImgDscBase & pdsc=nullptr ///< image descriptor
			): ID(id), Dsc(pdsc){}
		/// Instantiate.
		ImgDsc(const string & type, ///< descriptor type
			const FileNode & fn ///< OpenCV file storage node to read instance from
			)
		{
			fn["ID"]>>ID;
			Dsc = newDescriptor(type, fn["Dsc"]);
		}
		/// Instantiate.
		ImgDsc(const string & type, ///< descriptor type
			istream & s ///< binary stream to read the instance from
			)
		{
			std::read(s, ID);
			Dsc = newDescriptor(type, s);
		}
		/// Write the instance.
		void write(FileStorage & fs /** output OpenCV file storage */)const
		{
			fs<<"{"
				<<"ID"<<ID
				<<"Dsc"<<"{";
					Dsc->write(fs);
				fs<<"}";
			fs<<"}";
		}
		/// Write the instance.
		void write(ostream & s /** output binary stream */)const
		{
			std::write(s, ID);
			Dsc->write(s);
		}
	};
	/// image descriptor array
	struct ImgDscNdx: public vector<ImgDsc>
	{
		/// Read the instance.
		void read(const string & type, ///< descriptor type
			const FileNode & fn ///< OpenCV file storage node to read instance from
		)
		{
			for (auto it=fn.begin(); it!=fn.end(); ++it)
				push_back(ImgDsc(type, *it));
		}
		/// Write the instance.
		void write(ostream & s /** output binary stream */)const
		{
			const unsigned len=size(); writeSimple(s, len);
			for (unsigned i=0; i<len; ++i) at(i).write(s);
		}
		/// Read the instance.
		void read(const string & type, ///< descriptor type
			istream & s ///< binary stream to read the instance from
		)
		{
			unsigned len=0; readSimple(s, len);
			for (unsigned i=0; i<len; ++i)
				push_back(ImgDsc(type, s));
		}
		void remove(const string & ID)
		{
			for (auto it = begin(); it != end();)
				if (it->ID == ID) it = erase(it); else ++it;
		}
	}mImgDscNdx; ///< image descriptor instance

	/// Output index.
	friend FileStorage & operator<<(FileStorage & fs, ///< output OpenCV file storage
		const ImgDscNdx & ndx ///< index to output
		)
	{
		fs<<"[";
		for (auto it=ndx.begin(); it!=ndx.end(); ++it)
			it->write(fs);
		fs<<"]";
		return fs;
	}
	/// smart pointer to DescriptorMatcher
	typedef Ptr<DescriptorMatcher> PDescriptorMatcher;
	PDescriptorMatcher mPNdx; ///< instance of DescriptorMatcher
	unsigned mMaxK; ///< maximum K for kNN search

	/// descriptors collection type
	typedef Mat TDscVec;

	/// @return radius for the normalized threshold to be used in non-normalized radius-queries
	virtual REALNUM getRadius(REALNUM t /**< real-valued normalized threshold in [0,1] */)const
	{
		REALNUM s=getDistScale(), r=Unit2R(t);
		return s ? r/s : r;
	}

public:
	/// Instantiate.
	ImgDscMatcher(const string & ImgDscType, ///< known/OpenCV image descriptor type
		unsigned & flags ///< descriptor grouping flags, e.g. enable FLANN indexing, as defined by FaceMatch::EImgDscNdxOptions
		);
	virtual ~ImgDscMatcher(){}
	bool modified()const{ return mModified; }
	/// Absorb all current descriptors into the respective indexes.
	virtual void absorb();
	/// Cluster the descriptors, if necessary.
	virtual void train();
	/// Clear the index.
	void clear()
	{
		OMPLocker lkr(mLock);
		mImgDscNdx.clear();
		if (mPNdx) mPNdx->clear();
		mModified = true;
	}
	/// Insert an index entry.
	void insert(const string & ID, ///< image/region ID
		PImgDscBase pDsc ///< image/region descriptor
		)
	{
		OMPLocker lkr(mLock);
		mImgDscNdx.push_back(ImgDsc(ID, pDsc));
		mModified = true;
	}
	/// Remove an index entry.
	void remove(const string & ID /**< image/region ID */)
	{
		OMPLocker lkr(mLock);
		mImgDscNdx.remove(ID);
		mModified = true;
	}
	/**
	 * Query the index, pre-clustering it, if necessary.
	 * @param RgnDsc	image/face region descriptor
	 * @param MatchT	query threshold in the format K.D with the whole part (K) denoting Knn, and with decimal part (D) setting a radius search in [0,1)
	 * @param skipSelfMatch	skip self-match?
	 * return results in the distance ascending order
	 */
	PScoredLines query(const ImgDscBase & RgnDsc, const REALNUM MatchT=0.5, bool skipSelfMatch=false);
};

} // namespace FaceMatch
