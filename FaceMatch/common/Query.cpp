
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

#include "common.h"
#include "Query.h"
#include "ImageCollage.h"

namespace FaceMatch
{
template<typename TID>
TPScoredLines<TID> TScoredLines<TID>::trimmed(REALNUM th, bool skipSelfMatch)const
{
	int TopN = th; // floor
	REALNUM tol = th-TopN; // in [0,1]

	PScoredLines pTrimmed; // initialized pointer
	TScoredLines & trimmed = *pTrimmed; // initialized reference
	int k=1;
	for (auto it=mScore2ID.begin(); it!=mScore2ID.end(); ++it) // ascend by score
	{
		const REALNUM score = it->first;
		if (skipSelfMatch && !score) continue; // skip self, if set
		const TID & ID = it->second;
		if (th<0) { trimmed.insert(ID, score); continue; } // keep all
		if (tol>0 && score>tol) break;
		if (TopN>0 && k++>TopN) break;
		trimmed.insert(ID, score);
	}
	return pTrimmed;
}
template<typename TID>
void TScoredLines<TID>::trim(REALNUM T,	bool skipSelf)
{
	TPScoredLines<TID> pSL=trimmed(T, skipSelf);
	OMPLocker lkr(mLock);
	clear(); if (!pSL) return;
	(*this)=*pSL;
}
template<typename TID>
void TScoredLines<TID>::insert(const TID & ID, REALNUM score, bool minScore)
{
	OMPLocker lkr(mLock);
	auto it = mID2Score.find(ID);
	bool bNew = it==mID2Score.end();
	if (bNew)
	{
		mID2Score[ID]=score;
		mScore2ID.emplace(score, ID);
	}
	else // update the score
	{
		if (minScore && score > it->second) return; // insert only a min score
		if (!minScore && score < it->second) return; // insert only a max score
		it->second = score;
		auto rng = mScore2ID.equal_range(score);
		for (auto jt=rng.first; jt!=rng.second; ++jt)
			if (jt->second==ID)
			{
				mScore2ID.erase(jt);
				mScore2ID.emplace(score, ID);
				break;
			}
	}
}
template<typename TID>
void TQueryResults<TID>::display(const string & RepoPath)const
{
	ImageCollage IC("QueryResults", RepoPath);
	ImageCollage::Row * R = 0;
	for (const auto & it: *this)
	{
		R=IC.add(it.first); // start a new row
		const auto & SSL = *it.second;
		const auto & S2D = SSL.getMapScore2ID();
		for (const auto & jt: S2D)
			R->add(RepoPath, jt.second, jt.first);
	}
	IC.show();
	int k=waitKeyThrow();
	if (strchr("oO", k)) IC.out();
}
inline bool isML(string fmt)
{
	if (checkPrefix(fmt, "%YAML") || checkPrefix(fmt, "<?xml")) return true;
	toUpper(fmt);
	return fmt=="XML" || fmt=="YML" || fmt=="YAML";
}
template<typename TID>
FileStorage & TScoredLines<TID>::output(FileStorage & os)const
{
	os<< "results" << "[";
	for (const auto & S2D : mScore2ID)
		os << "{" << "score" << S2D.first << "RegID" << S2D.second << "}";
	os << "]";
	return os;
}
template<typename TID>
ostream & TScoredLines<TID>::print(ostream & os)const
{
	for (const auto & p : mScore2ID)
		os << p.first << '\t' << p.second << endl;
	return os;
}
template<typename TID>
unsigned TQueryResults<TID>::output(FileStorage & os)const
{
	os << "QueryResults" << "[";
	unsigned cnt=0;
	for (const auto & e: *this)
	{
		os <<"{"<< "query" << e.first << *e.second << "}";
		cnt += e.second->size();
	}
	os << "]";
	return cnt;
}
template<typename TID>
unsigned TQueryResults<TID>::print(ostream & os)const
{
	unsigned cnt=0;
	for (const auto & e: *this)
	{
		os<<e.first<<" found "<<e.second->size()<<endl<<*e.second;
		cnt += e.second->size();
	}
	return cnt;
}
template<typename TID>
unsigned TQueryResults<TID>::output(string & results)const
{
	unsigned cnt=0;
	if (isML(results))
	{
		FileStorage os("", FileStorage::WRITE | FileStorage::MEMORY | (results == "XML" ? FileStorage::FORMAT_XML : FileStorage::FORMAT_YAML));
		cnt=output(os);
		results = os.releaseAndGetString();
	}
	else
	{
		stringstream os;
		cnt=print(os);
		results = os.str();
	}
	return cnt;
}
template<typename TID>
TScoredLines<TID>::TScoredLines(const FileNode & results)
{
	for (const auto & nRes : results)
		insert(string(nRes["RegID"]), REALNUM(nRes["score"]));
}
template TScoredLines<>::TScoredLines(const FileNode & fn);
template<typename TID>
TScoredLines<TID>::TScoredLines(istream & is, int cnt)
{
	for (unsigned i=0; i<cnt; ++i)
	{
		REALNUM score; is>>score>>ws;
		string RegID; getline(is, RegID);
		insert(RegID, score);
	}
}
template<typename TID>
TQueryResults<TID>::TQueryResults(const string & input)
{
	if (isML(input))
	{
		FileStorage ifs(input, FileStorage::READ | FileStorage::MEMORY);
		const auto & QueryResults = ifs["QueryResults"];
		for (const auto & nd : QueryResults)
			(*this)[nd["query"]] = TPScoredLines<TID>(nd["results"]);
	}
	else // legacy plain text format
	{
		for (stringstream StrmQR(input); !StrmQR.eof(); )
		{
			string ln; getline(StrmQR, ln); if (ln.empty()) continue;
			auto pos = ln.find(" found "); if (pos==ln.npos) continue;
			const string & key = ln.substr(0, pos);
			int cnt = atoi(ln.substr(pos+7).c_str());
			(*this)[key] = TPScoredLines<TID>(StrmQR, cnt);
		}
	}
}
template TQueryResults<>::TQueryResults(const string & input);
/// @return b^e
template<typename NUM>
inline NUM pwr(const NUM & b /**< base*/ , const NUM & e /**< exponent */) { return e==1 ? b : pow(b,e); }
template<typename key, typename TID>
TPScoredLines<TID> WeightedQueryResults<key, TID>::combineWeightedDistances(bool useProd, REALNUM DistScale)const
{
	TPScoredLines<TID> res;
	for (auto it=this->begin(); it!=this->end(); ++it)
	{
		const auto & wSL = *it->second;
		const REALNUM W = wSL.getWeight();
		const auto & SL = *wSL.getScoredLines();
		const auto & S2D = SL.getMapScore2ID();
	//--- compute product/average of weighted distances
		for (const auto & sli: S2D)
		{
			const TID & ID = sli.second;
			if (res->contains(ID)) continue; // already computed
			const REALNUM score = sli.first;
			if (useProd && score==0) { res->insert(ID, 0); continue; }
			// double dist = pwr(score,W); // distance accumulator
			double dist = W * (useProd ? log(score) : score); // distance accumulator
			unsigned N=1;
			for (auto jt=it; ++jt!=this->end(); ) // traverse the remaining lines
			{
				const auto & wsl = *jt->second;
				const REALNUM w = wsl.getWeight();
				const auto & sl = *wsl.getScoredLines();
				const auto & d2s = sl.getMapID2Score();
				auto kt=d2s.find(ID); if (kt==d2s.end()) continue;
				// double dw = pwr(kt->second, w);
				REALNUM scr=kt->second;
				if (useProd && scr==0) { dist=0; break; }
				double dw = w*(useProd ? log(scr) : scr);
				// if (useProd) dist *= dw; else
				dist += dw;
				++N;
			}
			dist = useProd ? exp(dist) : dist/N;
			res->insert(ID, DistScale ? R2Unit(DistScale*dist) : dist); // weighted/scaled product/average
		}
	}
	return res;
}
/// Utility: define scores and points for Borda count.
template<typename NUM=REALNUM>
struct BordaScores
{
	NUM Distance, ///< distance in [0,1]
		BordaPoints; ///< Borda points
	/// Instantiate.
	BordaScores(NUM d=1 /**< distance */, NUM p=0 /**< points */): Distance(d), BordaPoints(p) {}
};
template<typename key, typename TID>
TPScoredLines<TID> WeightedQueryResults<key, TID>::combineBordaCounts()const
{
	FTIMELOG
	const unsigned QryResCnt=this->size();
	if (!QryResCnt) return TPScoredLines<TID>();
	size_t MaxLen=0;
	for (const auto & qri: *this)
		MaxLen=max(MaxLen, qri.second->getScoredLines()->size());
//--- compute Borda points
	typedef BordaScores<> TBordaScores;
	typedef map<TID, TBordaScores> MapRgnID2BordaScores;
	MapRgnID2BordaScores RgnPts; // Borda points
	REALNUM MaxPt=1;
	for (const auto & qri: *this)
	{
		const auto & wsl = *qri.second;
		const auto & sl = *wsl.getScoredLines();
		const auto & S2D = sl.getMapScore2ID(); // lines by ascending distance
		unsigned rank=0; // populate Borda points according to rank in the distance-ascending list
		const REALNUM W=wsl.getWeight();
		for (const auto & S2Di: S2D)
		{
			TBordaScores & BS = RgnPts[S2Di.second]; // TODO: ensure the top rank
			if (S2Di.first<BS.Distance) BS.Distance=S2Di.first;
			REALNUM pts = BS.BordaPoints += W*(MaxLen-rank);
			if (pts>MaxPt) MaxPt=pts;
			++rank;
		}
	}
//--- assign distances
	MaxPt+=1; // making sure 0 is assigned only to the self-matches
	TPScoredLines<TID> pSL;
	for (const auto & rpi: RgnPts)
	{
		const auto & bs=rpi.second;
		REALNUM d = bs.Distance ? (MaxPt - bs.BordaPoints)/MaxPt : 0;
		pSL->insert(rpi.first, d);
	}
	return pSL;
}

//=== instantiations

template PScoredLines WeightedQueryResults<int>::combineBordaCounts()const;
template TPScoredLines<int> WeightedQueryResults<int,int>::combineBordaCounts()const;
template TPScoredLines<int> WeightedQueryResults<int,int>::combineWeightedDistances(bool useProd, REALNUM DistScale)const;

template TPScoredLines<string> TScoredLines<string>::trimmed(REALNUM th, bool skipSelfMatch)const;
template void TScoredLines<string>::insert(const string & ID, REALNUM score, bool minScore);
template void TScoredLines<int>::insert(const int & ID, REALNUM score, bool minScore);
template ostream & ScoredLines::print(ostream & os)const;
template void ScoredLines::trim(REALNUM T, bool skipSelf);

template void QueryResults::display(const string & RepoPath)const;
template unsigned QueryResults::output(string & s)const;
template unsigned QueryResults::print(ostream & os)const;

template struct WeightedQueryResults<string>;
template<>
void WeightedQueryResults<string>::trimShow(const string & RepoPath, const string & QryRgnID, REALNUM MatchT)
{
	ImageCollage IC(QryRgnID, RepoPath);
	for (const auto & e: *this)
	{
		const string & DscID=e.first;
		PWeightedScoredLines pWSL=e.second;
		PScoredLines
            pSL = pWSL->getScoredLines(),
            pSSL = pSL->trimmed(MatchT);
		if (!pSSL) return;
		ImageCollage::Row * R=IC.add();
		for (const auto & jt: pSSL->getMapScore2ID())
		{
			REALNUM score=jt.first;
			const string & ResRegID=jt.second;
			R->addMatch(RepoPath, DscID, QryRgnID, ResRegID, score);
		}
	}
	IC.show();
	waitKey();
}
template<>
PScoredLines WeightedQueryResults<string>::combineDecreasingRadicals()const
{
	ScoredLines WeightedResults;
	for (auto it=begin(); it!=end(); ++it)
	{
		const string & DscID = it->first;
		const WeightedScoredLines<> & wsl = *it->second;
		WeightedResults.insert(DscID, wsl.getWeight());
	}
	PScoredLines pSL;
	const auto & MapScore2ID = WeightedResults.getMapScore2ID();
	for (auto wit=MapScore2ID.begin(); wit!=MapScore2ID.end(); ++wit)
	{
		const string & DscID = wit->second;
		auto it=find(DscID);
		const WeightedScoredLines<> & wsl = *it->second;
		const ScoredLines & SL = *wsl.getScoredLines();
	//--- compute the decreasing radicals distance as sqrt(d1*sqrt(d2*sqrt(d3)...)
		for (auto sli: SL.getMapScore2ID())
		{
			const string & RegID = sli.second;
			if (pSL->contains(RegID)) continue; // already computed
			REALNUM dist=sli.first; // distance accumulator
			auto wjt=wit; ++wjt; // traverse the remaining lines
			for (; wjt!=MapScore2ID.end(); ++wjt)
			{
				const string & DscID = wjt->second;
				auto it=find(DscID);
				const WeightedScoredLines<> & wsl = *it->second;
				const ScoredLines & SL = *wsl.getScoredLines();
				const auto & MapID2Score = SL.getMapID2Score();
				auto kt=MapID2Score.find(RegID);
				if (kt==MapID2Score.end()) continue;
				dist=kt->second*sqrt(dist); // assume all dist in [0,1]
			}
			pSL->insert(RegID, dist);
		}
	}
	return pSL;
}

}
