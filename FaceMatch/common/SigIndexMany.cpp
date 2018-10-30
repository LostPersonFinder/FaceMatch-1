
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

#include "common.h" // 2012-2017 (C) 
#include "SigIndexMany.h"
#include "ImageMatcher.h"
#include <fstream>

namespace FaceMatch
{

#define pForAllSubsInit ParallelErrorStream PES; const int DscCnt=mDscIndexPack.size();
#define ForAllSubs for (int i=0; i<DscCnt; ++i) try{ auto & ndx = *mDscIndexPack[i]->DscNdx;
#define ForAllSubsBegin pForAllSubsInit ForAllSubs

#define pForAllSubsBegin pForAllSubsInit \
CV_DO_PRAGMA(omp parallel for shared(PES)) \
	ForAllSubs

#define pForAllSubsEnd }PCATCH(PES, format("SigIndexMany [%d] descriptor error", i)) PES.report(format("%s parallel index update errors", __FUNCTION__));

SigIndexMany::SigIndexMany(unsigned dim, unsigned flags): TBase(dim, flags)
{
	mDscIndexPack.push_back(new TDscIndex("HAAR", "", .1, mFlags, mLabelDelimiter));
	mDscIndexPack.push_back(new TDscIndex("LBPH", "", .3, mFlags, mLabelDelimiter));
	mDscIndexPack.push_back(new TDscIndex("ORB", "", .6, mFlags, mLabelDelimiter));
	mDscIndexPack.push_back(new TDscIndex("SURF", "", .9, mFlags, mLabelDelimiter));
	mDscIndexPack.push_back(new TDscIndex("SIFT", "", 1, mFlags, mLabelDelimiter));
}
const string & SigIndexMany::LabelDelimiter(const string & dlm)const
{	TIMELOG("SigIndexMany::LabelDelimiter")
	if (!mOK2syncSubIndexes) return mLabelDelimiter=dlm;
	TBase::LabelDelimiter(dlm);
	pForAllSubsBegin
		ndx.LabelDelimiter(dlm);
	pForAllSubsEnd
	return mLabelDelimiter;
}
PImgDscBase SigIndexMany::newDescriptor(const Image * pImg, unsigned dim)const
{	FTIMELOG
	TLocker lkr(mLock); // fully formed index with multiple descriptors is required
	PDSC pdsc = new ImgDscMany(pImg, DefaultFacePatchDim, mDscIndexPack.size());
	auto & dsc = *pdsc;
	try
	{
	pForAllSubsBegin
		PImgDscBase pd;
		const string & DT=mDscIndexPack[i]->DscType;
		try
		{
			TIMELOG(DT+"::newDescriptor");
			pd=ndx.newDescriptor(pImg, dim);
		}
		catch(const exception & e)
		{
			PES.append(DT, e.what());
			pd=ndx.newDescriptor();
		}
		dsc[i]=pd;
	pForAllSubsEnd
	}
	catch(const exception & e)
	{
		if (getVerbLevel()) clog<<"exception: "<<e.what()<<endl;
	}
	return (PImgDscBase&)pdsc;
}
void SigIndexMany::insert(const string & ID, PDSC pdsc, bool align)
{	TIMELOG("SigIndexMany::insert")
	const ImgDscMany & rdsc = *pdsc;
	CHECK(rdsc.size()==mDscIndexPack.size(), "SigIndexMany::insert detected a mis-aligned ID="+ID);
	TLocker locker(mLock);
	pForAllSubsBegin // assume descriptor ordering is preserved
		const PImgDscBase & rdi = rdsc[i];
		if (!rdi->empty())
		{
			if (align)
			{
				auto it = ndx.find(ID);
				if (it==ndx.end()) ndx.insert(ID, rdi);
			}
			else ndx.insert(ID, rdi);
		}
	pForAllSubsEnd
	TBase::insert(ID, pdsc);
}
void SigIndexMany::insert(const string & ID, PImgDscBase pdsc)
{	TIMELOG("SigIndexMany::insert")
	CHECK(pdsc, "SigIndexMany::insert got a NULL descriptor for "+ID);
	CHECK(pdsc->getType()==ImgDscMany::Type(), "SigIndexMany::insert got a descriptor of a wrong type "+pdsc->getType());
	insert(ID, pdsc, false);
}
SigIndexMany::size_type SigIndexMany::erase(const SigIndexMany::key_type & k)
{
	pForAllSubsBegin
		ndx.remove(k);
	pForAllSubsEnd
	return TBase::erase(k);
}
void SigIndexMany::absorb()const
{	TIMELOG("SigIndexMany::absorb")
	TBase::absorb();
	pForAllSubsBegin
		ndx.absorb();
	pForAllSubsEnd
	syncSubIndexes();
}
REALNUM SigIndexMany::getWeight(const string & DscID)const
{	FTIMELOG
	REALNUM w = 1;
	pForAllSubsBegin
		if (mDscIndexPack[i]->DscType == DscID)
			w=mDscIndexPack[i]->DscWeight;
	pForAllSubsEnd
	return w;
}
void SigIndexMany::scan(istream & s)
{
	if (getVerbLevel()>3) clog<<"SigIndexMany::scan"<<endl;
	TLocker locker(mLock);
	mDscIndexPack.clear();
	for (; !s.eof(); ) // parse super-index
	{
		string ln; getline(s, ln); trim(ln); if (ln.empty()) continue;
		stringstream StrmLn(ln);
		if (startsWith(ln, "# LabelDelimiter"))
		{
			string tmp;
			StrmLn>>tmp>>ws>>tmp>>ws>>mLabelDelimiter;
			continue;
		}
		else if (ln[0]=='#') continue;
		string DscType, DscIndexFN;
		REALNUM fWeight=1;
		if (!StrmLn.eof()) StrmLn>>DscType;
		if (!StrmLn.eof()) StrmLn>>DscIndexFN;
		if (DscIndexFN=="NULL") DscIndexFN.erase();
		if (!StrmLn.eof()) StrmLn>>fWeight;
		mDscIndexPack.push_back(new TDscIndex(DscType, DscIndexFN, fWeight, mFlags, mLabelDelimiter));
	}
	for (int i=0; i<mDscIndexPack.size(); ++i) // NOTE: parallel execution appears to conflict with insert
	{ // assume sequence of constituent indexes is fixed
		const auto & pdi = mDscIndexPack[i]->DscNdx;
		for (const auto & d : *pdi)
		{
			const string & ID=d.first;
			PImgDscBase pdb = d.second;
			auto it = find(ID);
			PDSC pdsc = it==end() ? newDescriptor() : it->second;
			(*pdsc)[i] = pdb;
			if (it==end()) insert(ID, pdsc, true);
		}
	}
}
void SigIndexMany::syncSubIndexes()const
{	FTIMELOG
	if (!mOK2syncSubIndexes || !mOutOfSync) return;
	const int DscCnt=mDscIndexPack.size();
	for (const auto p : *this)
	{
		const string & ID = p.first;
		const ImgDscMany & DM = (const ImgDscMany&)*p.second;
		pForAllSubsBegin
			auto pDsc=DM[i];
			const string
				&typeDPi=ndx.getDscType(),
				&typeDMi=pDsc->getType();
			CHECK(typeDMi==typeDPi, typeDMi+"!="+typeDPi);
			ndx.insert(ID, pDsc);
		pForAllSubsEnd
	}
	LabelDelimiter(mLabelDelimiter);
	mOutOfSync = false;
}
SigIndexMany::TDscIndex::TDscIndex(const string & aDscType, const string & aDscIndexFN, REALNUM weight, unsigned flags, const string & LblDlm):
	DscType(aDscType), DscIndexFN(aDscIndexFN), DscWeight(weight)
{
	if (DscType=="HAAR") DscNdx = new SigIndexHaarFace(DscIndexFN, flags); else
	if (DscType=="LBPH") DscNdx = new SigIndexLBPH(DscIndexFN, flags); else
	if (DscType=="ORB") DscNdx = new SigIndexORB(DscIndexFN, flags); else
	if (DscType=="SURF") DscNdx = new SigIndexSURF(DscIndexFN, flags); else
	if (DscType=="SIFT") DscNdx = new SigIndexSIFT(DscIndexFN, flags);
	CHECK(DscNdx, "unable to instantiate index for "+DscType+" using "+DscIndexFN);
	DscNdx->LabelDelimiter(LblDlm);
}
void SigIndexMany::TDscIndex::write(ostream & s, bool withData)const
{
	std::write(s, DscType);
	writeSimple(s, DscWeight);
	if (withData)
	{
		std::write(s, DscIndexFN);
		DscNdx->write(s);
	}
}
PImgDscNdxBase newSigIndex(const string & DscType, istream & s)
{	FTIMELOG
	PImgDscNdxBase DscNdx;
	if (DscType=="HAAR") DscNdx = new SigIndexHaarFace(s); else
	if (DscType=="LBPH") DscNdx = new SigIndexLBPH(s); else
	if (DscType=="ORB") DscNdx = new SigIndexORB(s); else
	if (DscType=="SURF") DscNdx = new SigIndexSURF(s); else
	if (DscType=="SIFT") DscNdx = new SigIndexSIFT(s);
	CHECK(DscNdx, "unable to instantiate '"+DscType+"' index from a binary stream");
	return DscNdx;
}
void SigIndexMany::TDscIndex::read(istream & s, bool withData)
{	TIMELOG("SigIndexMany::TDscIndex::read")
	std::read(s, DscType);
	readSimple(s, DscWeight);
	if (withData)
	{
		std::read(s, DscIndexFN);
		DscNdx=newSigIndex(DscType, s);
	}
}
void SigIndexMany::input(FileStorage & fs)
{	TIMELOG("SigIndexMany::input")
	fs["LabelDelimiter"]>>mLabelDelimiter;
	mDscIndexPack.clear();
	for (auto nd : fs["DescriptorTypesWeights"])
		mDscIndexPack.push_back(new TDscIndex(nd.name(), "", (REALNUM)nd, mFlags, mLabelDelimiter));
	TBase::input(fs);
	mOutOfSync = true;
	syncSubIndexes();
}
void SigIndexMany::output(FileStorage & fs)const
{
	fs<<"LabelDelimiter"<<mLabelDelimiter;
	fs<<"DescriptorTypesWeights"<<"{";
	for (const auto & pDsc : mDscIndexPack)
		fs<<pDsc->DscType<<pDsc->DscWeight;
	fs<<"}";
	TBase::output(fs);
}
void SigIndexMany::write(ostream & s)const
{
	std::write(s, mLabelDelimiter);
	for (const auto & pDsc : mDscIndexPack)
		pDsc->write(s, false);
	TBase::write(s);
}
bool SigIndexMany::Need2Absorb() const
{
	return mOutOfSync && mOK2syncSubIndexes;
}
void SigIndexMany::read(istream & s)
{	TIMELOG("SigIndexMany::read")
	std::read(s, mLabelDelimiter);
	for (auto & pDsc : mDscIndexPack)
		pDsc->read(s, false);
	TBase::read(s);
	mOutOfSync = true;
	syncSubIndexes();
}
void SigIndexMany::save(const string & IndexFN)const
{
	const string
		SubNdxPath = getFilePath(IndexFN),
		SubNdxBaseName = getFileName(IndexFN),
		ext = toLC(getFileExt(IndexFN)),
		prext = toLC(getFileExt(SubNdxBaseName)),
		SubNdxExt = isMLE(prext) ? prext : ".ndx";
	if (ext==".txt" || ext == ".tsv")
	{
		TLocker locker(mLock);
		vector<string> SuperIndexLines(mDscIndexPack.size());
		pForAllSubsBegin // assume consistent descriptor ordering
			auto pdx = mDscIndexPack[i];
			string DscType = pdx->DscType,
				DscIndexFN = SubNdxBaseName+"."+DscType+SubNdxExt;
			pdx->DscNdx->save(DscIndexFN);
			stringstream strm; strm<<DscType<<'\t'<<DscIndexFN<<'\t'<<pdx->DscWeight;
			SuperIndexLines[i] = strm.str();
		pForAllSubsEnd
		ofstream ofs(IndexFN);
		if (mLabelDelimiter!="/")
			ofs<<"# LabelDelimiter\t"<<mLabelDelimiter<<endl;
		for (const auto & ln: SuperIndexLines)
			ofs<<ln<<endl;
	}
	else if (ext==".mdx") // per-face-region multi-file descriptor index
	{
		ofstream ofs(IndexFN);
		CHECK(ofs.is_open(), "unable to write to "+IndexFN);
		if (mLabelDelimiter!="/")
			ofs<<"# LabelDelimiter\t"<<mLabelDelimiter<<endl;
		for (const auto & pdx: mDscIndexPack)
			ofs<<pdx->DscType<<'\t'<<pdx->DscWeight<<endl;
		ofs<<"# Descriptors"<<endl;
		for (const auto p : *this)
		{
			const string & ID = p.first;
			const ImgDscMany & DM = (const ImgDscMany&)*p.second;
			string DscFN = SubNdxPath+"dsc.MANY."+toFN(ID)+".mdx";
			ofstream dfs(DscFN, ios::binary);
			CHECK(dfs.is_open(), "unable to open descriptor file "+DscFN);
			DM.write(dfs);
			ofs<<ID<<":\t"<<DscFN<<endl;
		}
	}
	else if (isMLE(ext))
	{
		FileStorage fs(IndexFN, FileStorage::WRITE);
		output(fs);
	}
	else
	{
		ofstream ofs(IndexFN, ios::binary);
		write(ofs);
	}
}
unsigned SigIndexMany::load(const string & IndexFN, bool bClear)
{	TIMELOG("SigIndexMany::load")
	TLocker lkr(mLock);
	if (bClear) clear();
	auto oldSize = size();
	if (IndexFN.find('\n') != IndexFN.npos) // multi-index ingest
	{	TIMELOG("SigIndexMany::load::MultiIndex")
		mOK2syncSubIndexes=false;
		for (stringstream strm(IndexFN); !strm.eof(); )
		{
			string FN; getline(strm, FN); if (FN.empty()) continue;
			load(FN, false);
		}
		mOK2syncSubIndexes=true;
	}
	else if (toLC(getFileExt(IndexFN)) == ".mdx")  // per-face-region multi-file descriptor index
	{
		mDscIndexPack.clear();
		bool header = true;
		for (ListReader lr(IndexFN); !lr.end(); )
		{
			const string & ln = lr.fetch(); if (ln.empty()) continue;
			stringstream StrmLn(ln);
			if (startsWith(ln, "# LabelDelimiter"))
			{
				string tmp;
				StrmLn >> tmp >> ws >> tmp >> ws >> mLabelDelimiter;
				continue;
			}
			else if (startsWith(ln, "# Descriptors")) { header = false; continue; }
			else if (ln[0] == '#') continue;
			if (header)
			{
				string DscType, DscIndexFN;
				REALNUM weight = 1;
				if (!StrmLn.eof()) StrmLn >> DscType >> weight;
				mDscIndexPack.push_back(new TDscIndex(DscType, DscIndexFN, weight, mFlags, mLabelDelimiter));
			}
			else // body
			{
				auto pos = ln.find(":\t");
				if (pos == string::npos) continue;
				string ID = ln.substr(0, pos),
					DscFN = ln.substr(pos + 2);
				ifstream ifd(DscFN, ios::binary);
				CHECK(ifd.is_open(), "unable to read from " + DscFN);
				insert(ID, TBase::newDescriptor(ifd));
			}
		}
	}
	else return TBase::load(IndexFN, bClear);
	digest();
	return size() - oldSize;
}
void SigIndexMany::clear()
{
	pForAllSubsBegin
		ndx.clear();
	pForAllSubsEnd
	TBase::clear();
	mOutOfSync = false;
}
PScoredLines SigIndexMany::query0(const string & RgnID, const ImgDscBase & RgnDsc, const REALNUM MatchT, bool skipSelfMatch)const
{
	FTIMELOG
	const static unsigned VerbT=2;
	const int size=mDscIndexPack.size();
	WeightedQueryResults<> QryResByDscr;
	bool GroupCombine=mFlags&(dm|dmOM|dmRank);
	if (GroupCombine || getVerbLevel()>VerbT)
	{
		const REALNUM maxT = MatchT < 0 ? MatchT : getMaxQueryT(MatchT);
		const ImgDscMany & rdsc = dynamic_cast<const ImgDscMany&>(RgnDsc);
		ParallelErrorStream PES;
		{ TLocker locker(mLock);
			auto Query = [&](int i)
			{
				try
				{
					const TDscIndex & dnx=*mDscIndexPack[i];
					const ImageDescriptorIndexBase & ri=*dnx.DscNdx;
					// assume original descriptor ordering is preserved
					PScoredLines pSL = ri.query0(RgnID, *rdsc[i], maxT, skipSelfMatch); // NOTE: getting all with maxT=-1 causes ORB to stagnate
					QryResByDscr[dnx.DscType] = new WeightedScoredLines<>(pSL, dnx.DscWeight);
				}
				PCATCH(PES, format("mDscIndexPack[%d] problem", i))
			};
			if (mFlags&dmRank)
			#pragma omp parallel for shared(PES, QryResByDscr, RgnID, rdsc)
				for (int i=0; i<size; ++i) Query(i);
			else // NOTE: for some reason with OMP this runs slower on win7 8-core
				for (int i=0; i<size; ++i) Query(i);
		}
		PES.report("SigIndexMany::query0::Query errors");
	}
	PScoredLines pSL;
	if (GroupCombine) // need to combine and threshold?
	{
		if (QryResByDscr.empty()) return pSL;
		pSL = combine(QryResByDscr);
		pSL->trim(MatchT);
	}
	else pSL = TBase::query0(RgnID, RgnDsc, MatchT, skipSelfMatch);
	if (getVerbLevel()>VerbT) // show diagnostics
	{
		if (getVerbLevel()>VerbT+1) clog<<"Q: "<<RgnID<<"="<<RgnDsc<<endl;
		QryResByDscr[getComboMethod()] = new WeightedScoredLines<>(pSL);
		QryResByDscr.trimShow(getRepoPath(), RgnID, pSL->size());
	}
	return pSL;
}

static atomic<REALNUM> // optimized using CalTech.tiny
	sDistScaleDist(4.46968),
	sDistScaleDistOM(1.6473);
REALNUM SigIndexManyDist::getDistScale()const
{
	return mFlags&dm ? sDistScaleDistOM : sDistScaleDist;
}
void SigIndexManyDist::setDistScale(REALNUM s)const
{
	if (mFlags&dm)
		sDistScaleDistOM=s;
	else sDistScaleDist=s;
}
REALNUM SigIndexManyDist::getDist(const ImgDscBase & lhs, const ImgDscBase & rhs)const
{
	const ImgDscMany
		&LHS = dynamic_cast<const ImgDscMany &>(lhs),
		&RHS = dynamic_cast<const ImgDscMany &>(rhs);
	if (LHS.size()!=RHS.size())
		throw Exception(format("SigIndexManyDist::getDist LHS.size=%d != RHS.size=%d", LHS.size(), RHS.size()));
	if (LHS.size()!=mDscIndexPack.size())
		throw Exception(format("SigIndexManyDist::getDist LHS.size=%d != mDscIndexPack.size=%d", LHS.size(), mDscIndexPack.size()));
	REALNUM d=1;
	pForAllSubsBegin
		REALNUM dst = pow(LHS[i]->dist(*RHS[i]), mDscIndexPack[i]->DscWeight);
	#pragma omp critical (dist)
		d*=dst;
	pForAllSubsEnd
	d=R2Unit(d*getDistScale());
	return d;
}
MatDouble SigIndexMany::getWeights()const
{
	MatDouble W(1, mDscIndexPack.size());
	pForAllSubsBegin
		W(0, i) = mDscIndexPack[i]->DscWeight;
	pForAllSubsEnd
	return W;
}
void SigIndexMany::setWeights(const MatDouble & W)
{
	pForAllSubsBegin
		mDscIndexPack[i]->DscWeight = W(0, i);
	pForAllSubsEnd
}
/// a solver for descriptor index weight optimizer
class FEvalW: public Solver::Function
{
	SigIndexMany & mNdx;
	REALNUM mMatchT;
	size_t mLen;

	double f(const MatDouble & W)const
	{
		mNdx.setWeights(W);
		return mNdx.evaluate(mMatchT ? mMatchT : W(0, mLen), weSelfExclude);
	}
public:
	/// Instantiate.
	FEvalW(SigIndexMany & ndx /**< image descriptors index */, REALNUM MatchT=0 /**< matching threshold */): mNdx(ndx), mMatchT(MatchT), mLen(ndx.mDscIndexPack.size()){}
	virtual double calc(const double* x)const override
	{
		double m = *min_element(x, x+mLen), M = *max_element(x, x+mLen);
		if (m<0 || M>1) return 0; // keep it in the range
		MatDouble W(1, mMatchT ? mLen : mLen+1, (double*)x);
		return -f(W); // max Fscore is at min -Fscore
	}
};
REALNUM SigIndexMany::optimize(REALNUM & MatchT, unsigned flags)
{
	REALNUM F=0; // TODO: prefer recall and hitrate over precision
	if ((flags&optThreshold) && (flags&optParams))
	{
		MatDouble W=getWeights();
		vector<double> X=W; X.push_back(MatchT); // [W,T]
		vector<double> step(X.size(), cv::norm(X, NORM_INF)); // max
		Ptr<DownhillSolver> pDHS=createDownhillSolver(new FEvalW(*this), step);
		F = -pDHS->minimize(X);
		setWeights(MatDouble(X));
		MatchT=X.back();
	}
	else if (flags&optParams)
	{
		MatDouble W=getWeights(), step(W.size(), cv::norm(W, NORM_INF));
		Ptr<DownhillSolver> pDHS=createDownhillSolver(new FEvalW(*this, MatchT), step);
		F = -pDHS->minimize(W);
		setWeights(W);
	}
	else if (flags&(optThreshold|optScale))
		return TBase::optimize(MatchT, flags);
	return F;
}
PScoredLines SigIndexMany::combine(WeightedQueryResults<> & QryResByDscr)const
{
	return QryResByDscr.combineDecreasingRadicals();
}
PScoredLines SigIndexManyDist::combine(WeightedQueryResults<> & QryResByDscr)const
{
	return QryResByDscr.combineWeightedDistances(true, getDistScale());
}
PScoredLines SigIndexManyRank::combine(WeightedQueryResults<> & QryResByDscr)const
{
	return QryResByDscr.combineBordaCounts();
}
bool operator==(const SigIndexMany & lhs, const SigIndexMany & rhs)
{
	const unsigned len=lhs.mDscIndexPack.size();
	if (rhs.mDscIndexPack.size()!=len) return false;
	ParallelErrorStream PES;
#pragma omp parallel for shared (PES)
	for (int i=0; i<len; ++i)
		try
		{
			 CHECK(*lhs.mDscIndexPack[i]==*rhs.mDscIndexPack[i], "unequal sub-indexes");
		}
		PCATCH(PES, format("%d-th descriptor pack", i))
	if (PES.report(format("%s index verification error", __FUNCTION__), &clog)) return false;
	return (const ImageDescriptorIndexBase &)lhs==(const ImageDescriptorIndexBase &)rhs;
}
} // namespace FaceMatch
