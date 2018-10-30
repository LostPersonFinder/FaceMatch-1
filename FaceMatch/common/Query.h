
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

#include "dcl.h"
#include "math_supp.h"
#include "omp_supp.h"
#include "Timing.h"

#include <map>
#include <unordered_map>
#include <string>

using namespace std;

namespace FaceMatch
{

/// default type ID
typedef string DefaultTID;
/// default scored lines smart pointer
template<typename TID=DefaultTID> struct TPScoredLines;
/// default scored lines smart pointer synonym
typedef TPScoredLines<> PScoredLines;

/// scored query result lines ascended by their scores
template<typename TID=DefaultTID>
class LIBDCL TScoredLines
{
public:
	/// map scores to IDs
	typedef multimap<REALNUM, TID> TMapScore2ID;
	/// map IDs to scores
	typedef unordered_map<TID, REALNUM> TMapID2Score;
private:
	TMapScore2ID mScore2ID;
	TMapID2Score mID2Score;
	mutable OMPNestedLock mLock; // copying lock would lead to invalid locking
public:
	/// Instantiate.
	TScoredLines() {}
	/// Instantiate.
	TScoredLines(const FileNode & fn);
	/// Instantiate from flat text stream.
	TScoredLines(istream & is, ///< input text stream
		int cnt=-1 /// line count to input; -1 = all
	);
	/// Copy instance.
	TScoredLines(const TScoredLines & src /**< source */) { *this=src; }
	/// Move instance.
	TScoredLines(TScoredLines && src /**< source */) { *this=move(src); }
	/// Copy-assign.
	const TScoredLines & operator=(const TScoredLines & src /**< source */)
	{
		mScore2ID=src.mScore2ID;
		mID2Score=src.mID2Score;
		return *this;
	}
	/// Move-assign.
	void operator=(TScoredLines && src /**< source */)
	{
		mScore2ID=move(src.mScore2ID);
		mID2Score=move(src.mID2Score);
	}
	/// @return map of line IDs to their scores
	const TMapID2Score & getMapID2Score() const { return mID2Score; }
	/// @return map of scores to their lines
	const TMapScore2ID & getMapScore2ID() const { return mScore2ID; }
	/// @return ID/name of the best matching line
	const TID & best()const
	{
		static const TID none;
		return mScore2ID.size() ? mScore2ID.begin()->second : none;
	}
	/// @return contains the given ID?
	bool contains(const TID & ID /**< query line ID */)const
	{
		OMPLocker lkr(mLock);
		auto it = mID2Score.find(ID);
		return it != mID2Score.end();
	}
	/// Clear the instance.
	void clear()
	{
		mScore2ID.clear();
		mID2Score.clear();
	}
	/// @return record count
	size_t size()const{ return mScore2ID.size(); }
	/// Insert a query results record, if the score is lower than already recorded
	void insert(const TID & ID, ///< results ID, typically an image ID/name
		REALNUM score, ///< real-valued score/distance, typically in [0,1]
		bool minScore=true ///< insert only the minimal score?
		);
	/**
	 * Truncate the scored line list using a threshold.
	 * When 0<T<1, remove lines scoring above T. When T>=1, output top T scoring lines.
	 * If T=N.dddd, then trim results to distance 0.dddd, then take top N, if any.
	 * @return scored lines in the ascending score order
	 */
	TPScoredLines<TID> trimmed(REALNUM T, ///< threshold value
		bool skipSelf=false ///< skip self-match?
		)const;
	/// Truncate this scored line list using a threshold and sort in the score-ascending order, @see trim
	void trim(REALNUM T, ///< threshold value
		bool skipSelf=false ///< skip self-match?
		);
	/// @return output stream, where object has been serialized
	FileStorage & output(FileStorage & os /**< output stream */)const;
	/// @return output stream, where object has been serialized
	ostream & print(ostream & os /**< output stream */)const;
	/// @return	output stream, where object has been serialized
	friend ostream & operator<<(ostream & os, ///< output stream
		const TScoredLines &qr ///< instance to serialize
	)
	{
		return qr.print(os);
	}
	/// @return	output stream, where object has been serialized
	LIBDCL friend FileStorage & operator<<(FileStorage & os, ///< output stream
		const TScoredLines &qr ///< instance to serialize
	)
	{
		return qr.output(os);
	}
};
/// specialized template synonym for scored lines
typedef TScoredLines<> ScoredLines;

/// safe pointer to a scored line
template<typename TID>
struct TPScoredLines: Ptr<TScoredLines<TID>>
{
	/// base class
	typedef Ptr<TScoredLines<TID>> TBase;
	/// Instantiate with a default pointer allocation, ensuring there's always something to point to.
	TPScoredLines(): TBase(new TScoredLines<TID>) {}
	/// Instantiate from structured storage.
	TPScoredLines(const FileNode & fn /**< OpenCV file storage node */): TBase(new TScoredLines<TID>(fn)) {}
	/// Instantiate from flat text stream.
	TPScoredLines(istream & is, ///< input text stream
		int cnt=-1 /// line count to input; -1=all
	): TBase(new TScoredLines<TID>(is, cnt)) {}
};

/// a map of query results, typically keyed by a group name/ID
template<typename TID=DefaultTID>
struct LIBDCL TQueryResults: map<string, TPScoredLines<TID>>
{
	/// base class
	typedef map<string, TPScoredLines<TID>> TBase;
	/// Instantiate.
	TQueryResults(const string & input="" /**< [in] string to de-serialize query results from */);
	/**
	 * Access a collection entry at the specified group key.
	 * If the key does not exist, initialize an entry.
	 * @param k	key of the group
	 * @return	corresponding query results group
	 */
	const TPScoredLines<TID> & at(const string & k)const{ return TBase::at(k); }
	/**
	 * Access a collection entry at the specified group key.
	 * If the key does not exist, initialize an entry.
	 * @param k	key of the group
	 * @return	corresponding query results group
	 */
	TPScoredLines<TID> & operator[](const string & k){ return TBase::operator[](k); }
	/// Display query results.
	void display(const string & RepoPath="" /**< path to get images from */)const;
	/// @return count of the results serialized to a plain text output stream
	unsigned print(ostream & os /**< output stream */)const;
	/// @return count of the results serialized to a structured output stream
	unsigned output(FileStorage & fs /**< structured output stream */)const;
	/// @return number of records in the output serialized results
	unsigned output(string & results /**< [in] results output format, e.g. XML, YML; default is plain text; [out] serialized results */)const;
	/**
	 * Output the query results to a text stream.
	 * @param os output text stream
	 * @param r query results to output
	 * @return output stream
	 */
	LIBDCL friend ostream & operator<<(ostream & os, const TQueryResults & r)
	{
		r.print(os);
		return os;
	}
};
/// specialized synonym for query results
typedef TQueryResults<> QueryResults;

/// associate a weight with scored lines
template<typename TID=DefaultTID>
class WeightedScoredLines
{
	const TPScoredLines<TID> mScoredLines;
	REALNUM mWeight;
public:
	/// Instantiate.
	WeightedScoredLines(const TPScoredLines<TID> & TScoredLines, ///< source scored lines
		REALNUM Weight=1 ///< weight for the scored lines
		): mScoredLines(TScoredLines), mWeight(Weight) {}
	/// @return scored lines weight
	REALNUM getWeight()const{return mWeight;}
	/// @return a smart pointer to the scored lines
	const TPScoredLines<TID> & getScoredLines()const{return mScoredLines;}
};

/// encapsulate weighted query results
template<typename key=string, typename TID=DefaultTID>
struct LIBDCL WeightedQueryResults: public unordered_map<key, Ptr<WeightedScoredLines<TID>>>
{
	/// smart pointer to weighted scored lines
	typedef Ptr<WeightedScoredLines<TID>> PWeightedScoredLines;
	/// Trim the results according to the match threshold and display the trimmed result.
	void trimShow(const string & RepoPath, const string & QryRgnID, REALNUM MatchT);
	/// Combine the multiple query results into a single scored list using weighted Borda count.
	TPScoredLines<TID> combineBordaCounts()const;
	/// Combine the multiple query results into a single scored list using the weighted distances method.
	TPScoredLines<TID> combineWeightedDistances(bool useProd=true, REALNUM DistScale=0)const;
	/// Combine the multiple query results into a single scored list using the decreasing radicals method.
	TPScoredLines<TID> combineDecreasingRadicals()const;
};

}
