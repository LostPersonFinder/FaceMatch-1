
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

#include "stdafx.h" // 2011-2017 (C) FaceMatch@NIH.gov
#include "TestImageMatch.h"

using namespace std;
using namespace FaceMatch;
/// \return a static reference to a single face region detector object
FaceRegionDetector & TestImageMatch::getFRD(bool preferGPU /**< prefer GPU, when available? */)
{
	StaticLkdCtor FaceRegionDetector FRD(
		// const string & XMLBasePath =
		"FFModels/",
		// const string & FaceModelFN =
		"haarcascade_frontalface_alt2.xml",
		// const string & ProfileModelFN =
		"haarcascade_profileface.xml",
		// const string & SkinColorMapperKind =
		"",
		// const string & SkinColorParmFN =
		"",
		// unsigned aFaceDiameterMin=
		cRegDimMin,
		// unsigned aFaceDiameterMax=
		cRegDimMax,
		// REALNUM SkinMassT=
		0.25,
		// REALNUM SkinLikelihoodT=
		0.5,
		// REALNUM aFaceAspectLimit=
		0.5,
		// bool preferGPU=
		preferGPU);
	return FRD;
}

#define FORLIST(LFN) \
for (ifstream is(LFN.c_str()); !is.eof(); ){\
	CHECK(is.is_open(), string("unable to open ")+LFN);\
	string FN; getline(is, FN);	trim(FN); if (FN.empty() || FN[0]=='#') continue;
#define ENDLIST }
void checkScoresCounts(const string & QR, REALNUM maxT)
{
	const unsigned maxC = maxT < 0 ? -1 : maxT;
	maxT = maxT < 0 ? 1 : max<REALNUM>(0, min<REALNUM>(maxT, 1));
	if (checkPrefix(QR, "%YAML") || checkPrefix(QR, "<?xml")) // TODO: use de-serialization query results constructor
	{
		FileStorage ifs(QR, FileStorage::READ | FileStorage::MEMORY);
		const auto & QueryResults = ifs["QueryResults"];
		for (const auto & nd : QueryResults)
		{
			unsigned cnt = 0;
			const string QRegID(nd["query"]);
			for (const auto & nRes : nd["results"])
			{
				REALNUM score(nRes["score"]);
				const string RID(nRes["RegID"]);
				CHECK(score >= 0 && score <= maxT,
					format("score=%g falls out of [0,%f] on '%s' compared with '%s'",
					score, maxT, QRegID.c_str(), RID.c_str()));
			}
			CHECK(cnt <= maxC,
				format("results count %d for '%s' should not exceed max count %d",
					cnt, QRegID.c_str(), maxC));
		}
	}
	else // assume plain text by default
	{
		unsigned i = 0;
		string src;
		for (stringstream strm(QR); !strm.eof(); ++i)
		{
			string ln; getline(strm, ln); if (ln.empty()) continue;
			stringstream strmLn(ln);
			string score; getline(strmLn, score, '\t');
			if (!isdigit(score[0]))
			{
				CHECK(i <= maxC,
					format("results count %d for '%s' should not exceed max count %d",
					i, src.c_str(), maxC));
				src = ln; i = 0; continue;
			}
			string region; getline(strmLn, region);
			REALNUM dist = atof(score.c_str());
			CHECK(dist >= 0 && dist <= maxT,
				format("dist=%s falls out of [0,%f] on '%s' compared with '%s'",
					score.c_str(), maxT, region.c_str(), src.c_str()));
		}
	}
}
/// \return slightly perturbed face region
string perturb(const string & sFR)
{
	stringstream in(sFR);
	string FN; getline(in, FN, '\t');
	stringstream out; out << FN;
	for (; !in.eof(); in >> ws)
	{
		FaceRegion fr(in);
		static const int a = 4;
		fr.x -= a; fr.width += 2 * a;
		out << '\t' << fr;
	}
	return out.str();
}
/// Test image matcher sequentially
template<class ImgMch /**< ImageMatcher descendant */>
void testSeq(const string & ImgNdxFN,
	const string & RepoPath,
	const string & ImgLstFN,
	unsigned verbose=0)
{
if (verbose) clog<<"testSeq "<<ImgNdxFN<<": ";
	const string ManyNdxFN="many.dsc.txt";
	ImgMch im(ManyNdxFN); // assuming descriptor ensemble indexing
	string res, FN="../Images/Steve.jpg";
	int TotalCnt=0, cnt=0;
if (verbose) clog<<"empty...";
	cnt=im.query(res, FN);
	if (verbose>1) clog<<"query empty index cnt="<<cnt<<" res="<<res<<endl;
	CHECK(cnt==0, "query empty index should bring trivial results");
if (verbose) clog<<"ingest...";
	const double maxIngestTime=4; // TODO: config/param
	TotalCnt=0, cnt=0;
	FORLIST(ImgLstFN)
		CVTiming tmg;
		try{ cnt=im.ingest(RepoPath+FN); }
		catch(const exception & e) { throw FaceMatch::Exception(format("%s on ", e.what())+FN); }
		double d=tmg.diff();
		CHECK(cnt, "ingest zero count on "+FN);
		if (verbose>1) clog<<FN<<" ingest time="<<d<<endl;
#ifndef _DEBUG
		if (verbose<2) CHECK(d<maxIngestTime, format("on %s, ingest duration of %.2f sec exceeded expected max of %.2f sec", FN.c_str(), d, maxIngestTime));
#endif
		TotalCnt+=cnt;
	ENDLIST
if (verbose) clog<<"query...";
	const double maxQueryTime=4; // TODO: config/param
	REALNUM MatchT=-0.5;
	setRepoPath(RepoPath);
	const vector<string> OutFormats = {"", "XML", "YML", "YAML"};
	unsigned fdx = 0, fct = OutFormats.size();
	FORLIST(ImgLstFN)
		string res = OutFormats[fdx++%fct];
		bool usingGPU=TestImageMatch::getFRD().usingGPU();
		CVTiming tmg;
		cnt = im.query(res, RepoPath+perturb(FN), MatchT);
		double d=tmg.diff();
		if (verbose>1) clog << "use GPU: " << usingGPU << endl << res << endl;
		CHECK(usingGPU==TestImageMatch::getFRD().usingGPU(), format("inconsistent GPU usage before query %d and after %d", usingGPU, TestImageMatch::getFRD().usingGPU()));
		CHECK(cnt, "query results count should not be 0 on "+FN);
		CHECK(!res.empty(), "query results should not be empty on "+FN);
		CHECK(!res.substr(res.find('\n')).empty(), "query results should not be trivial: "+res);
		checkScoresCounts(res, MatchT);
		if (MatchT<0)
			CHECK(cnt==TotalCnt, format("on %s, query results count=%d should equal total count=%d for a negative tolerance", FN.c_str(), cnt, TotalCnt));
		if (verbose>1) clog<<FN<<" query time="<<d<<endl;
#ifndef _DEBUG
		CHECK(d<maxQueryTime, format("on %s, query duration of %.2f sec exceeded expected max of %.2f sec", FN.c_str(), d, maxQueryTime));
#endif
		MatchT+=1;
	ENDLIST
if (verbose) clog<<"replace...";
	string ID="aPerson",
		FP="../Images/",
		fn1="12326_gladyscartyjpeg-1191520_md.jpg",
		FN1=FP+fn1,
		fn2="Steve.jpg",
		FN2=FP+fn2;
	unsigned len=im.count();
	if (verbose>1) clog<<"ingest "<<FN1<<" under ID="<<ID<<endl;
	cnt=im.ingest(FN1, ID); CHECK(cnt, "ingest zero count on "+FN1);
	cnt=im.query(res, FN1); if (verbose>1) clog<<"Qres: "<<res<<endl;
	CHECK(cnt && res.find("0\t"+FN1)!=string::npos, "query should retrieve a perfect match for ID="+ID+" on FN1="+FN1);
	if (verbose>1) clog<<"ingest "<<FN2<<" under ID="<<ID<<endl;
	cnt=im.ingest(FN2, ID); CHECK(cnt, "ingest zero count on "+FN2);
	cnt=im.query(res, FN2); if (verbose>1) clog<<"Qres: "<<res<<endl;
	CHECK(cnt && res.find("0\t"+FN2)!=string::npos, "query should retrieve a perfect match for ID="+ID+" on FN2="+FN2);
	cnt=im.query(res, FN1); if (verbose>1) clog<<"Qres: "<<res<<endl;
	CHECK(res.find("0\t"+FN1)==string::npos, "query should not retrieve a perfect match for ID="+ID+" on FN1="+FN1);
	cnt=im.remove(ID); if (verbose>2) { im.list(res); clog<<"list: "<<res<<endl; }
	CHECK(cnt==1, "remove("+ID+") should remove 1 record");
if (verbose) clog<<"count...";
	cnt = im.count();
	CHECK(len==cnt, "intermediate add/replace/remove an ID should leave the index unchanged");
if (verbose) clog<<"list...";
	cnt = im.list(res);
	CHECK(cnt, "list zero count on "+ImgNdxFN);
	cnt = im.list(res,"@"); if (verbose>1) clog<<"list groups: "<<res<<endl;
	CHECK(cnt, "list zero count on '"+ImgNdxFN+"' using @ groups");
	ID=RepoPath+"100"; // prefix
	cnt = im.list(res, ID);
	CHECK(cnt, "list zero count on '"+ImgNdxFN+"' using '"+ID+"'for ID/prefix");
if (verbose) clog<<"save...";
	im.save(ImgNdxFN);
	im.save(ImgNdxFN+".ndx");
if (verbose) clog<<"reload...";
	ImgMch imEcho(ImgNdxFN);
	CHECK(im==imEcho, "image matcher saved/reloaded differs from the ingested");
	ImgMch imBin(ImgNdxFN+".ndx");
	CHECK(im==imBin, "image matcher binary saved/reloaded differs from the ingested");
	imEcho.save(ImgNdxFN+".echo.txt");
	imBin.save(ImgNdxFN+".ndx.txt");
if (verbose) clog<<"remove...";
	FORLIST(ImgLstFN)
		stringstream strm(FN); getline(strm, FN, '\t');
		cnt = imBin.remove(FN);
		CHECK(cnt, "remove zero count on "+FN);
	ENDLIST
	cnt=imBin.count();
	CHECK(cnt==0, "cleaned index should be empty");
	cnt=im.ingest(FN1, ID); CHECK(cnt, "ingest zero count on "+FN1+" ID="+ID);
	cnt=im.remove(FN1); CHECK(cnt, "remove zero count on "+FN1);
	cnt=im.ingest(FN2, ID); CHECK(cnt, "ingest zero count on "+FN2+" ID="+ID);
	cnt=im.remove(fn2); CHECK(cnt, "remove zero count on "+FN2);
	cnt=im.ingest(FN1, ID); CHECK(cnt, "ingest zero count on "+FN1+" ID="+ID);
	cnt=im.remove(ID); CHECK(cnt, "remove zero count on "+ID);
if (verbose) clog<<"clear...";
	imEcho.clear();
	CHECK(imEcho.empty(), "non-empty image matcher after clearing");
	string ClrNdxFN=ImgNdxFN+".clear";
	imEcho.save(ClrNdxFN);
	ImgMch imClear(ClrNdxFN);
	CHECK(!imClear.count(), "non-empty image matcher after clearing, saving, loading of "+ClrNdxFN);
	cnt = imClear.list(res);
	CHECK(!cnt, "list non-zero count on "+ClrNdxFN);
}

void LogThreadCount(unsigned verbose)
{
	if (verbose>2 && omp_get_thread_num()==0)
#pragma omp critical
	clog<<"omp_get_num_threads="<<omp_get_num_threads()<<endl;
}

template<class ImgMch> // an ImageMatcher descendant
void testPar(const string & ImgNdxFN,
	const string & RepoPath,
	const string & ImgLstFN,
	unsigned verbose=0)
{
if (verbose) clog<<"testPar "<<ImgNdxFN<<": ";
	const short MchCnt=4;
	ImgMch matchers[MchCnt];
	ImgMch &im = *matchers;

	vector<string> ListFN, ListAttr;
	FORLIST(ImgLstFN)
		stringstream strm(FN);
		string key; getline(strm, key, '\t'); ListFN.push_back(key);
		string attr; getline(strm, attr); ListAttr.push_back(attr);
	ENDLIST
	const int ListSize=ListFN.size();

#define FORLINES for (int i=0; i<ListSize; ++i) { LogThreadCount(verbose); const string FN = ListFN[i], attr=ListAttr[i], Line = attr.empty() ? FN : FN+"\t"+attr;
#define ENDFOR }

if (verbose) clog<<"ingest...";
	ParallelErrorStream PES;
#pragma omp parallel for shared(PES)
	FORLINES
		try
		{
			#pragma omp critical
			if (verbose>1) clog<<Line<<endl;
		#pragma omp parallel for shared(PES)
			for (int j=0; j<MchCnt; ++j)
				try
				{
					unsigned cnt = matchers[j].ingest(RepoPath+FN, Line);
					CHECK(cnt, "ingest zero count on "+FN);
				}
				PCATCH(PES, format("matchers[%d]", j))
		}
		PCATCH(PES, "parallel ingest "+Line)
	ENDFOR
	PES.report("parallel ingest errors");

#define QUERYALL FORLINES try{string res; im.query(res, RepoPath+Line);} PCATCH(PES, "parallel query "+Line) ENDFOR

if (verbose) clog<<"time-par-query...";
	double TimeParQry = 0;
	{
		CVTiming tmg;
	#pragma omp parallel for schedule(dynamic) shared(PES)
		QUERYALL
		PES.report("parallel query errors");
		TimeParQry=tmg.diff();
		if (verbose>1) clog<<" duration="<<TimeParQry<<endl;
	}
if (verbose) clog<<"time-seq-query...";
	double TimeSeqQry = 0;
	{
		CVTiming tmg;
		QUERYALL
		TimeSeqQry=tmg.diff();
		if (verbose>1) clog<<" duration="<<TimeSeqQry<<endl;
	}
	const double speedup = TimeSeqQry/TimeParQry;
	if (verbose>1) clog<<" speedup="<<speedup;
	PES.check(speedup>=1, format("parallel query speedup %f was expected to exceed 1", speedup));
	if (verbose>1) clog<<endl;
if (verbose) clog<<MchCnt<<"-bucket-query...";
	double TimeBktQry = 0;
	{
		CVTiming tmg;
		#pragma omp parallel for shared(PES)
		for (int j=0; j<MchCnt; ++j)
			try
			{
				#pragma omp parallel for shared(PES)
				for (int i=0; i<ListSize; ++i)
					try
					{
						const string & FN = ListFN[i], &attr=ListAttr[i], &Line = FN + (attr.empty() ? "" : "\t"+attr);
						string res; matchers[j].query(res, RepoPath+Line);
					}
					PCATCH(PES, format("parallel bucket query matcher on %s", ListFN[i].c_str()))
			}
			PCATCH(PES, format("parallel bucket query matcher %d", j))
		// PES.report("parallel bucket query");
		TimeBktQry=tmg.diff();
		if (verbose>1) clog<<" duration="<<TimeBktQry;
		const double speedup=MchCnt*TimeSeqQry/TimeBktQry;
		if (verbose>1) clog<<" speedup="<<speedup;
		PES.check(speedup>1, format("bucket parallel query speedup %f was expected to exceed 1", speedup));
		if (verbose>1) clog<<endl;
	}
	if (!ImgNdxFN.empty())
	{
		im.save(ImgNdxFN);
		unsigned cnt=im.load(ImgNdxFN);
		CHECK(cnt && cnt==im.count(), "# of loaded records does not match the index size");
	}
if (verbose) clog << "load||query...";
	#pragma omp parallel sections shared(PES) // load, query
	{
		#pragma omp section // load
		try
		{
		#pragma omp critical
			if (verbose>1) clog<<"load in thread "<<omp_get_thread_num()<<endl;
			if (!ImgNdxFN.empty()) im.load(ImgNdxFN);
		#pragma omp critical
			if (verbose>2) clog<<"loaded "<<ImgNdxFN<<endl;
		}
		PCATCH(PES, "load||query")
		#pragma omp section // query
		try
		{
			#pragma omp critical
			if (verbose>1) clog<<"query in thread "<<omp_get_thread_num()<<endl;
			FORLINES
				try
				{
					string res; unsigned cnt=im.query(res, RepoPath+Line);
					PES.check(cnt, "query results zero count on "+FN);
					#pragma omp critical
					if (verbose>2) clog<<"query "<<FN<<" found "<<cnt<<endl;
				}
				PCATCH(PES, "query||load - FORLINES")
			ENDFOR
		}
		PCATCH(PES, "query||load")
	}
if (verbose) clog << "save||query...";
	#pragma omp parallel sections shared(PES) // save, query
	{
		#pragma omp section // save
		try
		{
			#pragma omp critical
			if (verbose>1) clog<<"save in thread "<<omp_get_thread_num()<<endl;
			if (!ImgNdxFN.empty()) im.save(ImgNdxFN);
		}
		PCATCH(PES, "save||query")
		#pragma omp section // query
		try
		{
			#pragma omp critical
			if (verbose>1) clog<<"query in thread "<<omp_get_thread_num()<<endl;
			FORLINES
				try
				{
					string res; unsigned cnt=im.query(res, RepoPath+Line);
					PES.check(cnt, "query results zero count on "+FN);
					#pragma omp critical
					if (verbose>2) clog<<"query "<<FN<<" found "<<cnt<<endl;
				}
				PCATCH(PES, "query||save - FORLINES")
			ENDFOR
		}
		PCATCH(PES, "query||save")
	}
if (verbose) clog << "query||remove...";
	#pragma omp parallel sections shared(PES) // query, remove
	{
		#pragma omp section // query
		try
		{
			#pragma omp critical
			if (verbose>1) clog<<"query in thread "<<omp_get_thread_num()<<endl;
			FORLINES
				try
				{
					string res; unsigned cnt=im.query(res, RepoPath+Line);
					#pragma omp critical
					if (verbose>2) clog<<"query "<<FN<<" found "<<cnt<<endl;
				}
				PCATCH(PES, "query||remove - FORLINES")
			ENDFOR
		}
		PCATCH(PES, "query||remove")
		#pragma omp section // remove
		try
		{
			#pragma omp critical
			if (verbose>1) clog<<"remove in thread "<<omp_get_thread_num()<<endl;
			FORLINES
				try
				{
					unsigned cnt = im.remove(RepoPath+FN);
					PES.check(cnt, "remove zero count on "+FN);
				}
				PCATCH(PES, "remove||query - FORLINES")
			ENDFOR
		}
		PCATCH(PES, "remove||query")
	}
	PES.report("parTest errors");
}

template<class ImgMch> // an ImageMatcher descendant
void test(const string & ImgNdxFN,
	const string & RepoPath,
	const string & ImgLstFN,
	unsigned verbose=0)
{
	testSeq<ImgMch>(ImgNdxFN, RepoPath, ImgLstFN, verbose);
	testPar<ImgMch>(ImgNdxFN, RepoPath, ImgLstFN, verbose);
}

typedef KeyPoints::const_iterator KPI;
bool compareKeyPointVectors(KPI ikpi, KPI ikpe, KPI ckpi, KPI ckpe)
{
	static const double tol=0.000001;
	for(;ikpi != ikpe && ckpi != ckpe; ikpi++, ckpi++)
	{
		if(
			(fabs((double)ikpi->pt.x - (double)ckpi->pt.x) > tol) ||
			(fabs((double)ikpi->pt.y - (double)ckpi->pt.y) > tol) ||
			(fabs((double)ikpi->size - (double)ckpi->size) > tol) ||
			(fabs((double)ikpi->angle - (double)ckpi->angle) > tol) ||
			(fabs((double)ikpi->response - (double)ckpi->response) > tol) ||
			(fabs((double)ikpi->octave - (double)ckpi->octave) > tol) ||
			(fabs((double)ikpi->class_id - (double)ckpi->class_id) > tol)
			)
			return false;
	}
	return true;
}
template<class NDX, class DSC>
void TestImageMatch::runSerialization()
{
	typedef ImageMatcherFaceRegionsBase<NDX> ImgMch;

	if (mVerbose) clog<<DSC::Type()<<"...";

	struct IM: public ImgMch
	{
		IM(const string & NdxFN=""): ImgMch(NdxFN, TestImageMatch::getFRD()) {}
	};

	const string &inFN=mImgLstFN, &RepoPath=mRepoPath;

	enum kind
	{
		base = 0,
		yml,
		ndx,
		kindCount
	};
	Ptr<IM> pimFRor[kindCount];

	pimFRor[base] = new IM();

	if (mVerbose>1) clog<<"ingest images from list "+inFN+"...";
	ifstream s(inFN.c_str());
	CHECK(s.is_open(), DSC::Type()+"unable to open "+inFN);
	int cnt=0;
	while(!s.eof())
	{
		string FN; getline(s, FN); trim(FN); if (FN.empty()) continue;
		try{ cnt += pimFRor[base]->ingest(RepoPath+FN, FN); }
		catch(const exception & e)
		{
			if (mVerbose) clog<<e.what()<<endl;
		}
	}
	CHECK(cnt, "ingested zero descriptors on "+inFN);
	s.close();
	if (mVerbose>1) clog<<"done"<<endl;

	string lsBase; pimFRor[base]->list(lsBase);
	if (mVerbose>1) clog<<"pimFRor[base]->list"<<endl<<lsBase<<endl;

	vector<string> FileName;
	FileName.push_back("test.yml.gz");
	FileName.push_back("test.ndx");

	for (int i=0, ndx=1, cnt=FileName.size(); i<cnt; ++i, ++ndx)
	{
		remove(FileName[i].c_str());
		pimFRor[base]->save(FileName[i]);
		pimFRor[ndx] = new IM(FileName[i]); // load saved
		string ls; pimFRor[ndx]->list(ls);
		if (mVerbose>1) clog<<"pimFRor["<<FileName[i]<<"]->list"<<endl<<ls<<endl;
		CHECK(lsBase==ls, "base index listing differs from loaded index listing on "+FileName[i]);
		CHECK((*pimFRor[base]==*pimFRor[ndx]), "base index differs from loaded index on "+FileName[i]);
	}

	if (mVerbose>1) clog<<"done"<<endl;
}
void TestImageMatch::testMatchWholeImageSeq()
{
	testSeq< ImageMatcherWhole<SigIndexHaarWhole> >("testWholeImage.ndx", mRepoPath, mImgLstFN, mVerbose);
}
void TestImageMatch::testMatchWholeImagePar()
{
	testPar< ImageMatcherWhole<SigIndexHaarWhole> >("testWholeImage.ndx", mRepoPath, mImgLstFN, mVerbose);
}
struct ImageMatcherFaceRegions4Test: public ImageMatcherFaceRegions
{
	ImageMatcherFaceRegions4Test(const string & NdxFN=""):
		ImageMatcherFaceRegions(NdxFN, TestImageMatch::getFRD()) {}
};
void TestImageMatch::testMatchFaceRegionsSeq()
{
	testSeq<ImageMatcherFaceRegions4Test>("testFaceRegions.txt", mRepoPath, getFileName(mImgLstFN)+".GT.lst", mVerbose);
}
struct ImageMatcherFaceRegionsOM4Test: public ImageMatcherFaceRegions
{
	ImageMatcherFaceRegionsOM4Test(const string & NdxFN=""):
		ImageMatcherFaceRegions(NdxFN, TestImageMatch::getFRD(),
		ImageMatcherFaceRegions::sFaceFinderFlags | (getVisVerbLevel() ? FaceFinder::visual : 0),
		DefaultFacePatchDim, dmOM) {}
};
void TestImageMatch::testMatchFaceRegionsSeqOM()
{
	testSeq<ImageMatcherFaceRegionsOM4Test>("testFaceRegions.txt", mRepoPath, getFileName(mImgLstFN)+".GT.lst", mVerbose);
}
void TestImageMatch::testMatchFaceRegionsParOM()
{
	testPar<ImageMatcherFaceRegionsOM4Test>("testFaceRegions.txt", mRepoPath, getFileName(mImgLstFN)+".GT.par.lst", mVerbose);
}
void TestImageMatch::testMatchFaceRegionsPar()
{
	testPar<ImageMatcherFaceRegions4Test>("testFaceRegions.txt", mRepoPath, getFileName(mImgLstFN)+".GT.par.lst", mVerbose);
}
struct IMSerializeIndividualDescriptors : public ImageMatcherFaceRegions
{
	const static unsigned FFF = FaceFinder::cascade & FaceFinder::detectOff;
	IMSerializeIndividualDescriptors(const string & NdxFN = "") :
		ImageMatcherFaceRegions(NdxFN, TestImageMatch::getFRD(), FFF) {}
	string ingestListSaveMultiIndex(const string & RepoPath, const string & ImgLstFN, const string TestDN, const unsigned ImgVar = 0, ostream * perr = 0)
	{
		string indexes;
		for (ListReader lr(ImgLstFN); !lr.end(); )
		{
			const string & ln = lr.fetch();	if (ln.empty() || ln[0] == '#') continue;
			try
			{
				clear();
				unsigned cnt = ingest(RepoPath + ln, "", ImgVar);
				for (unsigned i = 0; i<cnt; ++i)
				{
					string NdxFN = format("%s%s-%d.ndx", TestDN.c_str(), getFileName(ln, false).c_str(), i);
					save(NdxFN);
					indexes += NdxFN + "\n";
				}
			}
			catch (const FaceMatch::Exception & e)
			{
				if (perr) *perr << "exception: " << e.what() << " ingesting " << ln << endl;
				else throw e;
			}
		}
		return indexes;
	}
};
void TestImageMatch::testSerializeIndividualDescriptors()
{
	const string
		LFN = getFileName(mImgLstFN),
		GTLFN = contains(LFN, "GT") ? mImgLstFN : LFN + ".GT.lst",
		TestDN = "Test/",
		IndexFNBase = "testFaceRegions",
		SingleIndexFN = TestDN + IndexFNBase + ".ndx",
		MultiIndexFN = TestDN + IndexFNBase + ".mdx";

	makeDirectory(TestDN); // to output index files

	IMSerializeIndividualDescriptors FM;
	CVTiming tmg; // time ingest list
	FM.ingestList(mRepoPath, GTLFN, 0, &cerr);
	if (mVerbose) clog << "FM.ingestList time=" << tmg.diff() << endl;
	if (mVerbose>1) clog << "FM: " << FM;

	tmg.init();
	FM.save(SingleIndexFN);
	double tmgSingleFile = tmg.diff();
	if (mVerbose) clog << "FM.save single file index time=" << tmgSingleFile << endl;

	tmg.init();
	FM.save(MultiIndexFN);
	double tmgMultiFile = tmg.diff();
	if (mVerbose) clog << "FM.save multi-file index time=" << tmgMultiFile << endl;

	auto checkSingleMulti = [&](const string & label, const REALNUM tol)
	{
		REALNUM val = fabs(tmgSingleFile - tmgMultiFile) / max(tmgSingleFile, tmgMultiFile);
		CHECK(val < tol,
			format("%s: single file time %g differs too much relative to multi-file time %g; val=%g, tol=%g",
				label.c_str(), tmgSingleFile, tmgMultiFile, val, tol));
	};
	checkSingleMulti("save", .8); // TODO: address the large difference in saving time

	tmg.init(); // time single-file serialization of descriptors
	IMSerializeIndividualDescriptors FMS(SingleIndexFN);
	tmgSingleFile = tmg.diff();
	if (mVerbose) clog << "FM.load single file index time=" << tmgSingleFile << endl;

	tmg.init(); // time multi-file serialization of indexes: per face region, as in FM2
	IMSerializeIndividualDescriptors FMD;
	string indexes = FMD.ingestListSaveMultiIndex(mRepoPath, GTLFN, TestDN, 0, &cerr);
	tmgMultiFile = tmg.diff();
	if (mVerbose) clog << "FM.ingestListSaveMultiIndex time=" << tmgMultiFile << endl;

	tmg.init(); // time multi-file descriptor loading
	FMD.clear();
	FMD.load(indexes);
	tmgMultiFile = tmg.diff();
	CHECK(FMS == FMD, "single and multi-index loading should not produce different face matchers");
	if (mVerbose) clog << "FM.load multi-index time=" << tmgMultiFile << endl;
	checkSingleMulti("multi-index-load", .75);

	tmg.init(); // time multi-file descriptor loading
	IMSerializeIndividualDescriptors FMM(MultiIndexFN);
	tmgMultiFile = tmg.diff();
	CHECK(FMS == FMM, "single and multi-file loading should not produce different face matchers");
	if (mVerbose) clog << "FM.load multi-file index time=" << tmgMultiFile << endl;
	if (mVerbose>1) clog << "FMS: " << FMS << "FMM:" << FMM;
	checkSingleMulti("load", .75);
}
void TestImageMatch::testSerialization()
{
	runSerialization<SigIndexHaarWhole, ImgDscHaarWhole>();
	runSerialization<SigIndexHaarFace, ImgDscHaarFace>();
	runSerialization<SigIndexORB, ImgDscORB>();
	runSerialization<SigIndexSIFT, ImgDscSIFT>();
	runSerialization<SigIndexSURF, ImgDscSURF>();
	runSerialization<SigIndexLBPH, ImgDscLBPH>();
	runSerialization<SigIndexRSILC, ImgDscRSILC>();
	runSerialization<SigIndexMany, ImgDscMany>();
}
void testMatch(const ImgDscBase & aDsc, const ImgDscBase & bDsc)
{
	const string &aType=aDsc.getType(), &bType=bDsc.getType();
	CHECK(aType==bType, aType+" is different from "+bType);
	static const REALNUM eps=1e-2,
			tol=0.6; // TODO: reduce when RSILC is properly normalized/scaled
	const REALNUM
		dAB=aDsc.dist(bDsc),
		dBA=bDsc.dist(aDsc),
		diff=fabs(dAB-dBA);
	if (getVerbLevel()>1) clog<<"dAB="<<dAB<<", dBA="<<dBA<<", diff="<<diff<<endl;
	CHECK(diff<eps, format("%s fabs(dAB-dBA)=%f >= eps=%f", aType.c_str(), diff, eps));
	CHECK(dAB<tol, format("%s dAB=%f >= tol=%f", aType.c_str(), dAB, tol));
	CHECK(dBA<tol, format("%s dBA=%f >= tol=%f", aType.c_str(), dBA, tol));
}
void testMatchDescriptor(const string & type)
{
#define newDsc(DSC, IMG) const PImgDscBase p##DSC=newDescriptor(type, IMG); const ImgDscBase &DSC=*p##DSC;
if (getVerbLevel()) clog<<type<<":...sanity"; if (getVerbLevel()>1) clog<<endl;
	Image aImg("../Images/10047_mysmileypng.jpg");
	newDsc(aDsc, aImg)
	CV_Assert(aDsc==aDsc);
	CV_Assert(aDsc.dist(aDsc)==0);
if (getVerbLevel()) clog<<"...xform"; if (getVerbLevel()>1) clog<<endl;
	Image bImg(aImg, 1.5*aImg.dim()); bImg.rotateR(2*PI/3);
	newDsc(bDsc, bImg)
	testMatch(aDsc, bDsc);
if (getVerbLevel()) clog<<"...regress"; if (getVerbLevel()>1) clog<<endl;
	const string FN=format("testDescriptor.%s.yml.gz", type.c_str());
	if (!FileExists(FN) && getVerbLevel())
	{
		clog<<"...create("<<FN<<")";
		FileStorage ofs(FN, FileStorage::WRITE);
		aDsc.write(ofs);
	}
	FileStorage ifs(FN, FileStorage::READ);
	CHECK(ifs.isOpened(), "expected '"+FN+"' to exist, use a higher verbosity level to create it");
	newDsc(cDsc, ifs.root())
	CHECK(aDsc==cDsc, format("aDsc.dist(cDsc)=%f", aDsc.dist(cDsc)));
	testMatch(bDsc, cDsc);
if (getVerbLevel()) clog<<"...flip"; if (getVerbLevel()>1) clog<<endl;
	Image dImg=bImg; dImg.mirror();
	newDsc(dDsc, dImg)
	testMatch(bDsc, dDsc);
if (getVerbLevel()) clog<<"...binIO"; if (getVerbLevel()>1) clog<<endl;
{
	ofstream os("a."+type+".dsc", ios::binary); aDsc.write(os); os.close();
	ifstream is("a."+type+".dsc", ios::binary);
	newDsc(bDsc, is)
	testMatch(aDsc, bDsc);
}
if (getVerbLevel()) clog<<"...";
}
void TestImageMatch::testMatchDescriptorRSILC()
{
	testMatchDescriptor("RSILC");
}
void TestImageMatch::testMatchDescriptorSURF()
{
	testMatchDescriptor("SURF");
}
void TestImageMatch::testMatchDescriptors()
{
	static const char * DscNames[] = {"HAAR", "ORB", "SURF", "SIFT", "RSILC", "RSILCS"};
	for (const auto e: DscNames) testMatchDescriptor(e);
}
void TestImageMatch::testFRDusingGPUinQuery()
{
	FaceRegionDetector FRD(
			// const string & XMLBasePath =
			"FFModels/",
			// const string & FaceModelFN =
			"haarcascade_frontalface_alt2.xml",
			// const string & ProfileModelFN =
			"haarcascade_profileface.xml",
			// const string & SkinColorMapperKind =
			"",
			// const string & SkinColorParmFN =
			"",
			// unsigned aFaceDiameterMin=
			cRegDimMin,
			// unsigned aFaceDiameterMax=
			cRegDimMax,
			// REALNUM SkinMassT=
			0.25,
			// REALNUM SkinLikelihoodT=
			0.5,
			// REALNUM aFaceAspectLimit=
			0.5,
			// bool preferGPU=
			true);
	ImageMatcherFaceRegions im("", FRD);
	CHECK(FRD.usingGPU(), "should use GPU for face detection");
	int cnt=0;
if (mVerbose) clog<<"ingest...";
	FORLIST(mImgLstFN)
		CVTiming tmg;
		try{ cnt+=im.ingest(mRepoPath+FN); }
		catch(const exception & e) { if (mVerbose) clog<<format("%s on ", e.what())+FN<<endl; }
	ENDLIST
	CHECK(cnt, "ingest count should not be 0 for "+mImgLstFN);
	CHECK(FRD.usingGPU(), "FRD should use GPU for face detection in query");
	CHECK(im.usingGPU(), "IM should use GPU for face detection in query");
if (mVerbose) clog<<"query...";
	cnt=0;
	FORLIST(mImgLstFN)
		string res;
		try{ cnt += im.query(res, mRepoPath+perturb(FN)); }
		catch(const exception & e) { if (mVerbose) clog<<format("%s on ", e.what())+FN<<endl; }
	ENDLIST
	CHECK(cnt, "query results count should not be 0 on "+mImgLstFN);
	CHECK(FRD.usingGPU(), "FRD should still prefer GPU after query");
	CHECK(im.usingGPU(), "IM should still prefer GPU after query");
}
TestImageMatch::TestImageMatch(const string & RepoPath, const string & ImgLstFN, unsigned verbose, bool preferGPU):
	UnitTest<TestImageMatch>(RepoPath, ImgLstFN, verbose)
{
	mTestMap["MatchDescriptors"]=Test(&TestImageMatch::testMatchDescriptors);
	mTestMap["MatchDescriptor:RSILC"]=Test(&TestImageMatch::testMatchDescriptorRSILC, false);
	mTestMap["MatchDescriptor:SURF"]=Test(&TestImageMatch::testMatchDescriptorSURF, false);
	mTestMap["MatchWholeImageSeq"]=Test(&TestImageMatch::testMatchWholeImageSeq);
	mTestMap["MatchWholeImagePar"]=Test(&TestImageMatch::testMatchWholeImagePar);
	mTestMap["MatchFaceRegionsSeq"]=Test(&TestImageMatch::testMatchFaceRegionsSeq);
	mTestMap["MatchFaceRegionsSeqOM"]=Test(&TestImageMatch::testMatchFaceRegionsSeqOM);
	mTestMap["MatchFaceRegionsParOM"]=Test(&TestImageMatch::testMatchFaceRegionsParOM);
	mTestMap["MatchFaceRegionsPar"]=Test(&TestImageMatch::testMatchFaceRegionsPar);
	mTestMap["Serialization"]=Test(&TestImageMatch::testSerialization);
	mTestMap["SerializeIndividualDescriptors"]=Test(&TestImageMatch::testSerializeIndividualDescriptors, false);
	mTestMap["FRDusingGPUinQuery"]=Test(&TestImageMatch::testFRDusingGPUinQuery, false);
	getFRD(preferGPU); // initialize
}
