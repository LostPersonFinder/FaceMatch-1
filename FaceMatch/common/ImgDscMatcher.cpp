
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

#include "common.h" // 2014-2017 (C) 
#include "ImgDscMatcher.h"

namespace FaceMatch
{
ImgDscMatcher::ImgDscMatcher(const string & ImgDscType, unsigned & flags): mImgDscType(ImgDscType), mModified(false), mMaxK(0)
{
	if (checkPrefix(ImgDscType, "ImgDscHaar"))
	{
		flags &= ~(dm|dmOM); return;
	}
	if (flags&dmOM) // descriptor-dependent optimal grouping/search
		flags|=dmFLANN;
	if (flags&dmFLANN) // use FLANN, when possible
	{
		if (ImgDscType=="ImgDscORB") mPNdx = new BFMatcher(NORM_HAMMING); // NOTE: FlannBasedMatcher(new flann::LshIndexParams(12,20,2)) is not supported in OpenCV-2.4.9
		else mPNdx=new FlannBasedMatcher;
	}
	else if (flags&dmBF) // use exhaustive search
	{
		if (ImgDscType=="ImgDscORB") mPNdx=new BFMatcher(NORM_HAMMING);
		else if (ImgDscType=="ImgDscLBPH") mPNdx=new FlannBasedMatcher(new flann::LinearIndexParams);
		else mPNdx=new BFMatcher(NORM_L1);
	}
}
unsigned maxK(const DescriptorMatcher & dm)
{
	const auto & TrnDsc=dm.getTrainDescriptors();
	unsigned m=numeric_limits<unsigned>::max(), len=TrnDsc.size();
	for (int i=0; i<len; ++i)
	{
		unsigned sz=TrnDsc[i].rows;
		if (m>sz) m=sz;
	}
	return m;
}
void ImgDscMatcher::absorb()
{	FTIMELOG
	OMPLocker lkr(mLock);
	if (mImgDscType=="ImgDscMany" || !mPNdx || !mModified) return;
	vector<Mat> dscs;
	const unsigned icnt = mImgDscNdx.size();
	for (unsigned i = 0; i<icnt; ++i)
	{
		Mat d = mImgDscNdx[i].Dsc->getVectors();
		dscs.push_back(d);
	}
	mPNdx->clear();
	mPNdx->add(dscs);
	train();
	mMaxK=maxK(*mPNdx);
	mModified = false;
}
REALNUM ImgDscMatcher::getDistScale()const
{
	OMPLocker lkr(mLock);
	return mImgDscNdx.size() ? mImgDscNdx.front().Dsc->getDistScale(true) : 1;
}
const string & ImgDscMatcher::getImgID(unsigned n)const
{
	OMPLocker lkr(mLock);
	const int sz=mImgDscNdx.size();
	if(n>=sz)
		throw Exception(format("ImgDscMatcher::getImgID: n=%d index exceeds mImgDscNdx size=%d", n, sz));
	return mImgDscNdx[n].ID;
}
struct SplitThreshold
{
	int TopN;
	REALNUM DisT;
	SplitThreshold(REALNUM T): TopN(T), DisT(T-TopN) {}
};
PScoredLines ImgDscMatcher::combineMatches(const TMatches & matches, bool useDist)const
{
	FTIMELOG
	OMPLocker lkr(mLock);
	const REALNUM DistScale=getDistScale();
	const int rcnt=matches.size();
	ParallelErrorStream PES;
	WeightedQueryResults<int, int> WQR;
#pragma omp parallel for shared (matches, WQR, PES)
	for (int r=0; r<rcnt; ++r)
		try
		{
			const TMatch & match = matches[r];
			const int ccnt=match.size();
			TPScoredLines<int> pSL;
			int k=0;
			for (int c=0; c<ccnt; ++c) // sequence matters because of distance ascending order
				try
				{
					const auto & mc = match[c];
					REALNUM d = R2Unit(DistScale*mc.distance);
					pSL->insert(mc.imgIdx, d);
				}
				PCATCH(PES, format("combine match col %d", c))
		#pragma omp critical
			WQR[r] = new WeightedScoredLines<int>(pSL);
		}
		PCATCH(PES, format("combine match row %d", r))
	PES.report("ImgDscMatcher::combineMatches errors");
	const TPScoredLines<int> pCombSL = useDist ?
		WQR.combineWeightedDistances(false) :
		WQR.combineBordaCounts();
	PScoredLines res; // TODO: better dist normalization
	if (!pCombSL) return res;
	for (const auto & e: pCombSL->getMapID2Score())
		res->insert(getImgID(e.first), e.second);
	return res;
}
void ImgDscMatcher::train()
{
	OMPLocker lkr(mLock);
	if (mPNdx && !mPNdx->getTrainDescriptors().empty())  mPNdx->train();
}
void ImgDscMatcher::queryRadius(const Mat & query, TMatches & matches, REALNUM t)
{
	FTIMELOG
	if (!mPNdx) return;
	OMPLocker lkr(mLock);
	DescriptorMatcher& ndx=*mPNdx;
	train(); // make sure there is no training in the loop
	const REALNUM R = t<0 ? numeric_limits<REALNUM>::max() : getRadius(t); // transform threshold to a descriptor radius
	TMatchesPack RowMatches(query.rows);
	ParallelErrorStream PES;
#pragma omp parallel for shared (ndx, query, RowMatches, PES)
	for (int r=0; r<query.rows; ++r) // perform radius match for each row-descriptor
		try { ndx.radiusMatch(query.row(r), RowMatches[r], R); }
		PCATCH(PES, format("radius query row %d", r))
	PES.report("ImgDscMatcher::queryRadius errors");
	for (int r=0, len=RowMatches.size(); r<len; ++r)
	{
		const TMatches & m = RowMatches[r];
		matches.insert(matches.end(), m.begin(), m.end());
	}
}
void trim(TMatches & matches, unsigned k)
{
	for (auto & m: matches) if (m.size()>k) m.resize(k);
}
PScoredLines ImgDscMatcher::queryDsc(const ImgDscBase & q, REALNUM t, bool skipSelf)
{
	FTIMELOG
	OMPLocker lkr(mLock);
	absorb();
	const Mat query=q.getVectors();
	TMatches matches;
	bool useDist=true;
	if (t<1) // threshold normalized distance in [0,1)
		queryRadius(query, matches, t);
	else // t>=1: top-t
	{
		SplitThreshold sth(t);
		if (sth.DisT>0)
		{
			queryRadius(query, matches, sth.DisT);
			if (sth.TopN) trim(matches, 2*sth.TopN); // tradeoff between speed and accuracy
		}
		else // kNN
		{
			if (mPNdx && mMaxK)
			{
				mPNdx->knnMatch(query, matches, min<int>(mMaxK, skipSelf ? sth.TopN+1 : sth.TopN));
				useDist=false;
			}
			else return PScoredLines();
		}
	}
	return combineMatches(matches, useDist);
}
PScoredLines ImgDscMatcher::query(const ImgDscBase & q, const REALNUM t, bool skipSelf)
{
	FTIMELOG
	const string & type = q.getType();
	if (type!=mImgDscType) throw Exception("incompatible query descriptor type "+type+" with dataset type "+mImgDscType);
	PScoredLines res = queryDsc(q, t, skipSelf);
	res->trim(t, skipSelf);
	return res;
}
} // namespace FaceMatch
