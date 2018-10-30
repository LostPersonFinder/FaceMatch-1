
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

#include "common.h" // 2011-2017 (C) 
#include "ImageDescriptorIndexBase.h"
#include <fstream>

namespace FaceMatch
{
ostream & operator<<(ostream & s, const TQryAccEval & cqr)
{
	s<<cqr.HitCount<<'\t'<<cqr.TotalHitsPossible<<'\t'<<cqr.HitRate()<<'\t'<<(const TQryAccEvalBase&)cqr;
	return s;
}
const string & ImageDescriptorIndexBase::LabelDelimiter(const string & dlm)const
{	FTIMELOG
	mLabelDelimiter=dlm;
	regroup();
	return dlm;
}
bool ImageDescriptorIndexBase::Need2Absorb()const
{
	return (mFlags&dm) && mIDM.modified();
}
void ImageDescriptorIndexBase::absorb()const
{
	TLocker locker(mLock);
	mIDM.absorb();
}
void ImageDescriptorIndexBase::digest(bool force)const
{
	if (force || Need2Absorb()) absorb();
}
void ImageDescriptorIndexBase::read(istream & s)
{	FTIMELOG
	CHECK(s.good(), "invalid stream for loading binary ImageDescriptorIndex");
	std::read(s, mLabelDelimiter); LabelDelimiter(mLabelDelimiter);
	size_type totalPairs=0;	readSimple(s, totalPairs);
	for (int pair=0; pair<totalPairs; ++pair)
	{
		string key; std::read(s, key);
		PImgDscBase pDsc(newDescriptor(s));
		CHECK(!pDsc->empty(), "an empty descriptor for "+key);
		insert(key, pDsc);
	}
	CHECK(size()>=totalPairs, format("expected %d entries, but got %d", totalPairs, size()));
}
void ImageDescriptorIndexBase::input(FileStorage & fs)
{
	const string & type = getDscType();
	FileNode fnImgDscNdx = fs["ImageDescriptorIndex"];
	fnImgDscNdx["LabelDelimiter"]>>mLabelDelimiter;
	for (auto pn : fnImgDscNdx)
	{
		if (pn.name().find("pair")==string::npos) continue;
		string key(pn["key"]);
		try
		{
			const FileNode & dn = pn[type];
			if (dn.empty())
				throw Exception("missing index node type "+type);
			PImgDscBase pDsc(newDescriptor(dn));
			if (pDsc->empty())
				throw Exception("an empty descriptor for "+key);
			insert(key, pDsc);
		}
		catch(const exception & e)
		{
			clog<<"exception: "<<e.what()<<" while loading "<<key<<" of type "<<type<<endl;
			continue;
		}
		catch(...)
		{
			cerr<<"unknown exception while loading "<<key<<" of type "<<type<<endl;
			throw;
		}
	}
}
void ImageDescriptorIndexBase::insertCropped(const string & ID, const Image & img, const REALNUM a)
{
	Size size; Point ofs;
	img.mx().locateROI(size, ofs);
	Rect o(ofs, img.size()), r=o;
	stretch(r, a);
	Point dTL=o.tl()-r.tl(), dBR=r.br()-o.br();
	Mat mx = img.mx();
	Image dst(mx.adjustROI(dTL.y, dBR.y, dTL.x, dBR.x));
	insertVariant(dst, ID, "CROP", a);
}
void ImageDescriptorIndexBase::insertScaled(const string & ID, const Image & src, const REALNUM a)
{
	Image dst(src, src.width()*a, src.height()*a);
	insertVariant(dst, ID, "SCALE", a);
}
void ImageDescriptorIndexBase::insertRotated(const string & ID, const Image & img, const REALNUM a)
{
	Image dst(img); dst.rotateR(a);
	insertVariant(dst, ID, "ROTATE", a);
}
void ImageDescriptorIndexBase::insertVariant(const Image & img, const string & ID, const string & var, const REALNUM a)
{
	stringstream StrmRgnID;	StrmRgnID<<ID<<"/"<<var<<"\tv["<<a<<"]";
	const string & RegID = StrmRgnID.str();
	if (getVisVerbLevel()>1)
	{
		string FN=toFN(RegID)+".jpg";
		clog<<"VarImgFN="<<FN<<endl;
		img.store(FN);
	}
	insert(RegID, newDescriptor(&img));
}
unsigned ImageDescriptorIndexBase::insertVariations(const string & ImgID, Image img, const unsigned ImgVar)
{	FTIMELOG
	if (mFlags&moHistEQ) img.eqHist();
	unsigned cnt=0;
	if (ImgVar) insertVariant(img, ImgID, "SELF", 0);
	else insert(ImgID, newDescriptor(&img));
	++cnt;
	if (ImgVar & ivCrop)
	{
		TIMELOG("insertCropped"); // TODO: param/config
		insertCropped(ImgID, img, 0.75);
		insertCropped(ImgID, img, 0.9);
		cnt+=2;
	}
	if (ImgVar & ivRotate)
	{
		TIMELOG("insertRotated");
		const static REALNUM a=PI/6; // TODO: param/config, e.g. 2*PI/3
		insertRotated(ImgID, img, a);
		insertRotated(ImgID, img, -a);
		cnt+=2;
	}
	if (ImgVar & ivScale)
	{
		TIMELOG("insertScaled"); // TODO: param/config
		insertScaled(ImgID, img, 0.5);
		insertScaled(ImgID, img, 2.0);
		cnt+=2;
	}
	return cnt;
}
void ImageDescriptorIndexBase::getAcc(TQryAccEval & ae, const QueryResults & qr, REALNUM MatchT, unsigned we)const
{
	for (const auto & it: qr)
	{
		const string & RecID = it.first;
		PScoredLines pSL = it.second->trimmed(MatchT, we&weSelfExclude);
		incAccEval(ae, RecID, *pSL, we);
	}
}
void ImageDescriptorIndexBase::printAt(ostream & s, const QueryResults & qr, REALNUM MatchT, unsigned we)const
{
	TQryAccEval ae; getAcc(ae, qr, MatchT, we);
	s<<MatchT<<'\t'<<ae<<endl;
}
void ImageDescriptorIndexBase::print(ostream & s, const QueryResults & qr, unsigned we)const
{
	for (REALNUM t=0.1; t<1; t+=0.1) // distance based
		printAt(s, qr, t, we);
	for (REALNUM t=1.25; t<=40; t*=2) // rank based
		printAt(s, qr, round(t), we);
	// TODO: compute print top-N percent results
}
const string & ImageDescriptorIndexBase::getLabel(const string & ID)const
{
	auto it = mImgRgnID2Label.find(ID);
	return it==mImgRgnID2Label.end() ? nemo : it->second;
}
unsigned ImageDescriptorIndexBase::getLabelCnt(const string & label)const
{
	auto it = mLabel2Keys.find(label);
	return it==mLabel2Keys.end() ? 0 : it->second.size();
}
void ImageDescriptorIndexBase::incAccEval(TQryAccEval & ae,	const string & RecID, const ScoredLines & SL, unsigned we)const
{
	bool bSkipSelfMatch = we&weSelfExclude;
	we&=weEvalKindMask; // trim the flags

	const string & RecLbl=getLabel(RecID);
	unsigned LblCnt=getLabelCnt(RecLbl);
	REALNUM M=0, FP=0,
		FN = LblCnt-int(bSkipSelfMatch);
	if (bSkipSelfMatch) { if (FN>0) ++ae.TotalHitsPossible; }
	else ++ae.TotalHitsPossible;
	bool hit=false;
	unsigned n=SL.size(), rank=0, selfCnt=0;
	const ScoredLines::TMapScore2ID & S2D = SL.getMapScore2ID();
	for (auto it = S2D.begin(); it != S2D.end(); ++it, ++rank)
	{
		REALNUM
			dist = it->first,
			score = 1; // non-weighted default
		const string & RegID = it->second;
		if (bSkipSelfMatch && RegID == RecID) { ++selfCnt;  continue; }
		switch (we)
		{
		case weAntiDist: score = 1-dist; break;
		case weAntiRank: score = 1-rank/(REALNUM)n; break;
		case weInvRank: score = 1./(rank+1); break;
		}
		string lbl = getLabel(RegID);
		if (RecLbl==lbl)
		{
			M+=score; FN-=score;
			if (!hit) { ae.HitCount+=score; hit=true; }
		}
		else
		{
			if (rank == selfCnt && getVerbLevel() > 1) // TODO: remove
				clog << "warning: top rank="<<rank
				<<", RecID=" << RecID << ", RegID=" << RegID
				<< ", RecLbl=" << RecLbl << ", lbl=" << lbl << endl;
			FP += score;
		}
	}
	ae.mMatchCnt += M;
	ae.mFoundTotal += M+FP;
	ae.mGivenTotal += M+FN;
}
unsigned ImageDescriptorIndexBase::ingest(const string & ImgID, const Mat & img, const FaceRegions & rgns, const unsigned ImgVar)
{
	unsigned cnt=0;
	for (const PFaceRegion & rt: rgns) // TODO: consider parallelization
	{
		stringstream StrmRgnID;
		if (ImgVar && ImgID.rfind('/')==string::npos)
			StrmRgnID<<ImgID<<"/"; // ensure the variants have the same label
		StrmRgnID<<ImgID<<'\t'<<*rt;
		cnt+=insertVariations(StrmRgnID.str(), img, *rt, ImgVar);
	}
	return cnt;
}
void ImageDescriptorIndexBase::print(ostream & s, const_iterator it, const string & fmt)const
{
    if (it==end()) return;
	if (fmt=="ACRD")
	{
		const string FN=getRepoPath()+it->first+"."+it->second->getType()+"."+fmt;
		ofstream os(FN);
		it->second->print(os, fmt);
	}
	s<<it->first<<"\t"<<*(it->second);
	s<<endl;
}
void ImageDescriptorIndexBase::matchInfix(StringSet & matches, const string & IDnfx, bool exhaustive)const
{
	FTIMELOG
	auto ix=find(IDnfx); // exact match
	if (ix!=end()) { matches.insert(ix->first); return; }

	auto LU = [&](const Label2StringSet & map)
	{
		auto it=map.find(IDnfx);
		if (it!=map.end())
			matches.insert(it->second.begin(), it->second.end());
	};
	LU(mLabel2Keys);
	LU(mLabel2FileName);
	LU(mLabel2FilePath);
	if (!exhaustive) return;
	FORALL // linear search
		const string & key = it->first;
		auto pos = key.rfind(IDnfx); // reverse infix match
		if (pos==string::npos) continue;
		if (pos==0 || strchr("[/\\.", key[pos-1])) // prefix match // TODO: more accurate matching
			matches.insert(key);
	ENDALL
}
unsigned ImageDescriptorIndexBase::print(ostream & s, const string IDPrefix, const string & fmt)const
{
	s<<"ImageDescriptorIndex: "<<size()<<" entries in "<<mLabel2Keys.size()<<" groups"<<endl;
	if (IDPrefix.empty()) return size();
	bool justCount = checkPrefix(IDPrefix, "#") || checkPrefix(IDPrefix, "!") || checkPrefix(IDPrefix, "cnt") || checkPrefix(IDPrefix, "count");
	string IDpfx = justCount ? IDPrefix.substr(IDPrefix.find(' ')+1) : IDPrefix;
	if (IDpfx=="*")
	{
		if (!justCount)
			FORALL
				print(s, it, fmt);
			ENDALL
		return size();
	}
	if (IDpfx=="@" || IDpfx=="groups" || IDpfx=="labels")
	{
		for (const auto & e: mLabel2Keys)
			s<<e.first<<": "<<e.second.size()<<endl;
		return mLabel2Keys.size();
	}
	StringSet matches;
	matchInfix(matches, IDpfx, true);
	if (!justCount)
		for (const auto & e: matches) print(s, find(e), fmt);
	return matches.size();
}
void ImageDescriptorIndexBase::evaluate(TQryAccEval & ae, QueryResults & QryRes, REALNUM MatchT, unsigned we)const
{
	bool // extract the flags
		bSkipSelfMatch = we&weSelfExclude,
		bQueryNonVariant = we&weQueryNonVariant; // query only by non-variant "SELF" pictures
	unsigned VL=getVerbLevel();
	const REALNUM t = MatchT<0 ? (VL>1 ? MatchT : 40.99) : MatchT; // TODO: param/config
	for (auto it=begin(); it!=end(); ++it) // TODO: parallel?
	{
		const string & RecID = it->first;
		if (bQueryNonVariant && RecID.find("SELF")==string::npos) continue;
		const ImgDscBase & RegDsc = *(it->second);
		PScoredLines pSL = query(RecID, RegDsc, t, bSkipSelfMatch);
		if (MatchT<0) // record query results per query region for extended accuracy report
			QryRes[RecID] = pSL;
		incAccEval(ae, RecID, *pSL, we);
	}
	if (VL>1) clog<<QryRes<<endl;
}
void ImageDescriptorIndexBase::evaluate(TQryAccEval & ae, REALNUM MatchT, unsigned we)const
{
	QueryResults QryRes; evaluate(ae, QryRes, MatchT, we);
	if (getVerbLevel())
	{
		clog<<"Tol\tHitCnt\tAllCnt\tHitRate\t"<<aeDefaultTitles<<endl
			<<MatchT<<'\t'<<ae<<endl;
		if (MatchT<0) print(clog, QryRes, we);
	}
}
REALNUM ImageDescriptorIndexBase::evaluate(REALNUM MatchT, unsigned we)const
{
	TQryAccEval ae; evaluate(ae, MatchT, we);
	return ae.FScore();
}

REALNUM ImageDescriptorIndexBase::getDistScale()const
{
	if (!mDescriptor) mDescriptor=newDescriptor();
	return mDescriptor->getDistScale(mFlags&dm);
}
void ImageDescriptorIndexBase::setDistScale(REALNUM s)const
{
	if (!mDescriptor) mDescriptor=newDescriptor();
	mDescriptor->setDistScale(s, mFlags&dm);
}

/// objective for distance scale optimization
class FEvalDistScale: public Solver::Function
{
	const ImageDescriptorIndexBase & mNdx;
	REALNUM mT;
	double f(double x)const
	{
		if (x<=0) return -1;
		mNdx.setDistScale(x);
		if (getVerbLevel()) clog<<"DistScale="<<mNdx.getDistScale()<<endl;
		return mNdx.evaluate(mT, weSelfExclude);
	}
public:
	/// Instantiate.
	FEvalDistScale(const ImageDescriptorIndexBase & ndx /**< input index */, REALNUM T /**< matching threshold */): mNdx(ndx), mT(T) {}
	virtual double calc(const double * x)const override
	{
		return -f(*x); // max Fscore is at min -Fscore
	}
};
/// objective for matching threshold optimization
class FEvalMatchT: public Solver::Function
{
	const ImageDescriptorIndexBase & mNdx;
	double f(double t)const
	{
		TQryAccEval ae; mNdx.evaluate(ae, t, weSelfExclude);
		REALNUM P=ae.Precision(), R=ae.Recall(), F=ae.FScore();
		return P>P ? F-(P-R)/2 : F; // prefer recall
	}
public:
	/// Instantiate.
	FEvalMatchT(const ImageDescriptorIndexBase & ndx /** input index */): mNdx(ndx){}
	virtual double calc(const double * x)const override
	{
		return -f(*x); // max Fscore is at min -Fscore
	}
};
/// map matching threshold to evaluation accuracy statistics
struct MapQryAccEval: public map<REALNUM, TQryAccEval>
{
	/// @return optimal top-N and the matching threshold
	REALNUM findOptimalTopN(REALNUM & MatchT /**< [in/out] matching threshold */)const
	{
		REALNUM maxF=0;
		for (auto it=begin(); it!=end(); ++it)
		{
			REALNUM F=it->second.FScore();
			if (F<maxF) continue;
			maxF=F;
			MatchT=it->first;
		}
		return maxF;
	}
};
REALNUM ImageDescriptorIndexBase::optimize(REALNUM & MatchT, unsigned flags)
{
	REALNUM F=0;
	if (flags&optThreshold)
	{
		if (MatchT>=1) // optimal integer top-N F-score
		{
			unsigned we=weSelfExclude;
			QueryResults QryRes;
			TQryAccEval ae; evaluate(ae, QryRes, -1, we);
			MapQryAccEval mqae;
			for (REALNUM t=1; t<=40; ++t) // rank based
				getAcc(mqae[t], QryRes, t, we);
			F=mqae.findOptimalTopN(MatchT);
		}
		else if (MatchT>=0) // optimize in [0,1)
		{
			double step = MatchT;
			Ptr<DownhillSolver> pDHS = createDownhillSolver(new FEvalMatchT(*this), MatDouble(1, 1, step));
			MatDouble T(1, 1, MatchT);
			F = -pDHS->minimize(T);
			MatchT = T(0,0);
		}
		else // negative: optimize for top-N and [0,1] simultaneously
		{
			REALNUM
				T01=0.5, F01=0, // optimize(flags, T01),
				TTN=5, FTN=0; // optimize(flags, TTN);
			ParallelErrorStream PES;
			#pragma omp parallel sections shared(PES)
			{
				#pragma omp section
					try {F01=optimize(T01, flags);}
					PCATCH(PES, "F01=optimize(T01, flags)")
				#pragma omp section
					try{FTN=optimize(TTN, flags);}
					PCATCH(PES, "FTN=optimize(TTN, flags)")
			}
			PES.report("parallel ImageDescriptorIndexBase::optimize errors");
			if (FTN>F01) { MatchT=TTN; F=FTN; }
			else { MatchT=T01; F=F01; }
		}
	}
	else if (flags&optScale)
	{
		if (getVerbLevel()) clog<<"initial DistScale="<<getDistScale()<<endl;
		double step = getDistScale();
		Ptr<DownhillSolver> pDHS = createDownhillSolver(new FEvalDistScale(*this, MatchT), MatDouble(1, 1, step));
		MatDouble S(1, 1, getDistScale());
		F = -pDHS->minimize(S);
		clog<<"optimal DistScale="<<S(0,0)<<endl;
	}
	return F;
}
void ImageDescriptorIndexBase::save(const string & IndexFN)const
{
	string ext = getFileExt(IndexFN); toUpper(ext);
	if (ext==".TXT")
	{
		ofstream os(IndexFN.c_str());
		dump(os);
	}
	else if (isMLE(ext)) // mark-up language?
	{
		FileStorage fs(IndexFN, FileStorage::WRITE);
		output(fs);
	}
	else if (ext==".ACRD") // Affine Covariant Region Detectors, http://www.robots.ox.ac.uk/~vgg/research/affine/detectors.html
	{
		ofstream os(IndexFN.c_str());
		print(os, "*", "ACRD");
	}
	else // assume binary output
	{
		ofstream os(IndexFN.c_str(), ios::binary);
		write(os);
	}
	// TODO: save mIDM, if needed
}
bool operator==(const ImageDescriptorIndexBase & lhs, const ImageDescriptorIndexBase & rhs)
{
	if (lhs.size()!=rhs.size())
		return false;
	const unsigned VerbLevel=getVerbLevel();
	for (auto lit=lhs.begin(); lit!=lhs.end(); ++lit)
	{
		auto rit=rhs.find(lit->first);
		if (rit==rhs.end()) return false;
		if (VerbLevel>2)
		{
			clog<<"lit->first="<<lit->first<<endl<<"rit->first="<<lit->first<<endl;
			clog<<"*lit->second="<<*lit->second<<endl<<"*rit->second="<<*rit->second<<endl;
		}
		if (*lit->second!=*rit->second)
			return false;
	}
	return true;
}
string getRegionKind(const string & RegID)
{
	int pos=RegID.find('[');
	if (pos==string::npos || RegID[pos-1]=='d') return "w"; // whole image
	stringstream StreamRegID(RegID);
	string ID; getline(StreamRegID, ID, '\t'); // ID
	FaceRegion fr(StreamRegID);
	return fr.Kind;
}
MatReal computeSaliencyMapLM(const Mat & src, const FaceRegions & landmarks)
{
	MatReal dst = Mat::zeros(src.size(), src.depth()); //dst = SaliencyMap
	for (auto it=landmarks.begin(); it!=landmarks.end(); ++it)
		if (isFRRectKind((*it)->Kind)) GaussianBell(dst, dynamic_cast<const FaceRegion &>(**it));
	if (getVerbLevel()>2) imshow("bells", dst);
	return dst;
}
MatReal computeSaliencyMapNN(const Mat & src, const FaceRegions & landmarks)
{
	MatReal dst=Mat::ones(src.size(), src.depth());
	// TODO: compute [SV]
	return dst;
}
Image ImageDescriptorIndexBase::cropAddSaliency(const Mat & src, const FaceRegion & fr)const
{
	Image img(src, fr);
	Mat & dst=img.mx();
	if ((mFlags&smmAll)==0) return img;
	byte CC = dst.channels();
	if (CC==1 || CC==3)
	{
		vector<Mat> chn; split(dst, chn);
		Mat SM=computeSaliencyMap(dst, fr.mSubregions);
		chn.push_back(SM);
		merge(chn, dst);
	}
	return img;
}
MatUC ImageDescriptorIndexBase::computeSaliencyMap(const Mat & src, const FaceRegions & landmarks)const
{
	MatReal dst=Mat::ones(src.size(), src.depth()), SM;
	if (mFlags&smmLandMarks) SM=computeSaliencyMapLM(src, landmarks);
	if (mFlags&smmMachineLearning) SM=computeSaliencyMapNN(src, landmarks);
	return 0xFF*dst.mul(SM);
}
REALNUM ImageDescriptorIndexBase::getDist(const ImgDscBase & lhs, const ImgDscBase & rhs)const
{
	REALNUM d=lhs.dist(rhs);
#ifdef _DEBUG
	if (d<0 || d>1)
		throw Exception(format("dist=%f falls out of [0,1] for %s", d, lhs.getType().c_str()));
#endif
	return d;
}
PScoredLines ImageDescriptorIndexBase::queryDist(const string & RgnID, const ImgDscBase & RgnDsc, const REALNUM MatchT, bool skipSelfMatch)const
{	FTIMELOG
	if (getVerbLevel()>3) clog<<"ImageDescriptorIndexBase::queryDist"<<' '<<RgnID<<" t="<<MatchT<<endl;
	PScoredLines SL;
	string QRegKind=getRegionKind(RgnID);
	FORALL // linear search
		const string & ResID=it->first;
	// if (ResID.find("SELF")!=string::npos) continue; // retrieve only variant descriptors
		string RK=getRegionKind(it->first); // TODO: parse/index region kinds during insertion?
		const ImgDscBase & idb = *(it->second);
		REALNUM d=getDist(RgnDsc, idb);
		if (d==0 && skipSelfMatch) continue;
		if (RK!=QRegKind) d=sqrt(d); // punish region kind mismatch
		if (MatchT>=0 && d>MatchT) continue;
		SL->insert(ResID, d);
	ENDALL
	return SL;
}
ImageRegionIDSplit::ImageRegionIDSplit(const string & ImgRgnID, const string & dlm)
{
	auto pos=ImgRgnID.find('\t');
	FilePath = pos==string::npos ? ImgRgnID : ImgRgnID.substr(0, pos);
	pos=ImgRgnID.find("d[", pos); // ID
	if (pos==string::npos)
	{
		if (dlm=="/") // e.g. CalTechFaces
			Label=ImgRgnID.substr(0, ImgRgnID.rfind("/"));
		else
			Label=ImgRgnID.substr(0, ImgRgnID.find(dlm));
	}
	else
	{
		pos+=2;
		Label=ImgRgnID.substr(pos, ImgRgnID.find("]", pos)-pos);
	}
}
void ImageDescriptorIndexBase::group(const string & ImgRgnID)const
{
	ImageRegionIDSplit split(ImgRgnID, mLabelDelimiter);
	if (split.Label.empty()) return;
	mImgRgnID2Label[ImgRgnID]=split.Label;
	mLabel2Keys[split.Label].insert(ImgRgnID);
	mLabel2FilePath[split.FilePath].insert(ImgRgnID);
	mLabel2FileName[getFileName(split.FilePath, false, true)].insert(ImgRgnID);
}
void ImageDescriptorIndexBase::regroup()const
{
	mImgRgnID2Label.clear();
	mLabel2Keys.clear();
	FORALL
		group(it->first);
	ENDALL
}
void ImageDescriptorIndexBase::insert(const string & ImgRgnID, PImgDscBase dsc)
{	FTIMELOG
	if (getVerbLevel() > 2) clog << "insert ImgRgnID="<< ImgRgnID << endl << "dsc=" << *dsc << endl;
	if (dsc->empty())
		clog<<"warning: got an empty "+dsc->getType()+" descriptor for "+ImgRgnID<<endl;
	TLocker locker(mLock);
	iterator it = find(ImgRgnID);
	bool bNew = it==end();
	if (bNew)
	{
		TIMELOG("ImageDescriptorIndex::insert::new")
		group(ImgRgnID);
		(*this)[ImgRgnID]=dsc;
	}
	else it->second = dsc;
	mIDM.insert(ImgRgnID, dsc);
}
unsigned ImageDescriptorIndexBase::eraseInfix(const string & IDnfx)
{	FTIMELOG
	TLocker locker(mLock);
	StringSet matches; matchInfix(matches, IDnfx);
	for (const auto & key: matches)
	{
		const string label = getLabel(key);
		if (!label.empty())
		{
			mImgRgnID2Label.erase(key);
			mLabel2Keys[label].erase(key);
			if (!mLabel2Keys[label].size())
				mLabel2Keys.erase(label);
		}
		mIDM.remove(key);
		erase(key);
	}
	return matches.size();
}
unsigned ImageDescriptorIndexBase::remove(const string & Regs)
{
	unsigned cnt=0;
	for (stringstream StrmRegs(Regs); !StrmRegs.eof(); )
	{
		string ID; getline(StrmRegs, ID); if (ID.empty()) continue;
		cnt+=eraseInfix(ID); // locks it there
	}
	return cnt;
}
PScoredLines ImageDescriptorIndexBase::query0(const string & RgnID, const ImgDscBase & RgnDsc, const REALNUM MatchT, bool skipSelfMatch)const
{
	if (getVerbLevel() > 2) clog << "query0 RgnID=" << RgnID << endl <<"RgnDsc=" << RgnDsc << endl;
	PScoredLines pSL;
	if (mFlags&dm) // approximate query
		pSL=mIDM.query(RgnDsc, MatchT, skipSelfMatch);
	else // legacy linear face group search
	{
		const int TopN=MatchT; // floor for top-N
		const REALNUM tol = MatchT==0 ? 0 : (MatchT>TopN ? MatchT-TopN : getMaxQueryT(MatchT));
		pSL=queryDist(RgnID, RgnDsc, tol, skipSelfMatch);
		if (MatchT>=1) pSL->trim(TopN);
	}
	return pSL;
}
PScoredLines ImageDescriptorIndexBase::query(const string & RgnID, const ImgDscBase & RgnDsc, const REALNUM MatchT, bool skipSelfMatch)const
{
	PScoredLines pSL=query0(RgnID, RgnDsc, MatchT, skipSelfMatch);
	if (mFlags&moGroupLabels)
	{
		QueryResults IDGrps; // group results by label/ID
		for (const auto & e: pSL->getMapID2Score())
		{
			const string & RegID=e.first, &ID=getLabel(RegID);
			IDGrps[ID]->insert(RegID, e.second);
		}
		if (getVerbLevel()>1) clog<<"IDGrps: "<<IDGrps;
		PScoredLines pMSL; // order by median distance
		bool med=true; // TODO: param/config
		for (const auto & e: IDGrps)
		{
			const auto & S2D = e.second->getMapScore2ID();
			auto it=S2D.begin(); // get an average
			if (med)
			{
				for (int i=0, m=S2D.size()/2; i<m; ++i) ++it;
				pMSL->insert(it->second, it->first);
			}
			else //mean
			{
				REALNUM sum=0; for (const auto & d: S2D) sum+=d.first;
				it=S2D.lower_bound(sum/S2D.size());
			}
			pMSL->insert(it->second, it->first);
		}
		pSL=PScoredLines(); // output the closest from the median-nearest group
		bool min=true; // TODO: config/param
		for (const auto & e: pMSL->getMapScore2ID())
		{
			const auto & d = min ? *IDGrps[getLabel(e.second)]->getMapScore2ID().begin() : e;
			pSL->insert(d.second, d.first);
		}
	}
	return pSL;
}
void ImageDescriptorIndexBase::query(QueryResults & QryRes, const string & RgnID, const ImgDscBase & RgnDsc, const REALNUM MatchT, bool skipSelfMatch)const
{
	QryRes[RgnID]=query(RgnID, RgnDsc, MatchT, skipSelfMatch);
}
unsigned ImageDescriptorIndexBase::query(string & results, const string & ImgID, const Mat & img, const FaceRegions & rgns, const REALNUM MatchT, unsigned flags)const
{
	digest();
	QueryResults QryRes; // results accumulator
	for (const auto & re: rgns)
	{
		if (!isFRRectKind(re->Kind)) continue;
		const FaceRegion & FR = dynamic_cast<const FaceRegion &>(*re);
		stringstream StrmRgnID;	StrmRgnID<<ImgID<<'\t'<<FR;
		const string RgnID = StrmRgnID.str();
		Image RgnImg(cropAddSaliency(img, FR));
		if (mFlags&moHistEQ) RgnImg.eqHist();
		if (flags&qoRotationPhases)
		{
			ParallelErrorStream PES;
		#pragma omp parallel for shared(QryRes, PES) // TODO: drop phases for rotation invariant descriptors, e.g. SURF, SIFT, ORB
			for (int rp=Image::tphZero; rp<Image::tphCount; ++rp)
				try
				{
					Image tmp(RgnImg, (Image::ETurnPhase)rp);
					const PImgDscBase & pRgnDsc = newDescriptor(&tmp);
					query(QryRes, RgnID, *pRgnDsc, MatchT, flags&qoSkipSelf);
				}
				PCATCH(PES, format("query phase %d", rp))
			PES.report("parallel query phases errors");
		}
		else
		{
			const PImgDscBase & pRgnDsc = newDescriptor(&RgnImg);
			if (getVerbLevel()>3) imshow("Q:RgnImg", RgnImg.mx());
			query(QryRes, RgnID, *pRgnDsc, MatchT, flags&qoSkipSelf);
		}
	}
	// serialize results
	return QryRes.output(results);
}
void ImageDescriptorIndexBase::output(FileStorage & fs)const
{
	fs<<"ImageDescriptorIndex"<<"{";
	fs<<"LabelDelimiter"<<mLabelDelimiter;
	unsigned k=0;
	FORALL
		stringstream strm; strm<<"pair"<<k++; // unique tag as required by import
		fs<<strm.str()<<"{";
			fs<<"key"<<it->first;
			const PImgDscBase & pdsc=it->second;
			fs<<pdsc->getType()<<"{";
				pdsc->write(fs);
			fs<<"}";
		fs<<"}";
	ENDALL
	fs<<"}";
}
void ImageDescriptorIndexBase::write(ostream & s)const
{
	std::write(s, mLabelDelimiter);
	writeSimple(s, size());
	FORALL
		std::write(s, it->first);
		it->second->write(s);
	ENDALL
}
unsigned ImageDescriptorIndexBase::load(const string & IndexFN, bool bClear)
{	FTIMELOG
	if (bClear) clear();
	unsigned OldCnt=size();
	if (getVerbLevel()>3) clog<<"load "<<IndexFN<<endl;
	TLocker lkr(mLock);
	string ext = getFileExt(IndexFN); toLower(ext);
	if (ext==".txt" || ext==".tsv")
	{
		ifstream s(IndexFN.c_str());
		CHECK(s.is_open(), "unable to open index '" + IndexFN + "'");
		scan(s);
	}
	else if (ext==".xml" || ext==".yml" || ext==".yaml" || ext==".gz")
	{
		FileStorage fs(IndexFN, FileStorage::READ);
		CHECK(fs.isOpened(), "unable to open index '" + IndexFN + "'");
		input(fs);
	}
	else // assume binary stream
	{
		ifstream s(IndexFN.c_str(), ios::binary);
		CHECK(s.is_open(), "unable to open index '" + IndexFN + "'");
		read(s);
	}
	digest();
	return size();
}
} // namespace FaceMatch
