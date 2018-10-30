
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

#include "stdafx.h"
#include "TestFaceFinder.h"
#include <string>
#include <sstream>
#include <numeric>
#include <thread>
#include <chrono>

TestFaceFinder::TestFaceFinder(FaceRegionDetector & FRD, unsigned flags, const string & RepoPath, const string & ImgLstFN,	int VrbLvl):
	UnitTest<TestFaceFinder>(RepoPath, ImgLstFN, VrbLvl), mFRD(FRD), mFlags(flags)
{
	mTestMap["FRD"]=Test(&TestFaceFinder::testFRD);
	mTestMap["EvalStats"]=Test(&TestFaceFinder::testEvalStats);
	mTestMap["Landmarks"]=Test(&TestFaceFinder::testLandmarks);
	mTestMap["Multithread"]=Test(&TestFaceFinder::testMultithread);
	mTestMap["Standard"]=Test(&TestFaceFinder::testStandard);
	mTestMap["Skin"]=Test(&TestFaceFinder::testSkin);
	mTestMap["SkinANN"]=Test(&TestFaceFinder::testSkinANN);
	mTestMap["SkinHist"]=Test(&TestFaceFinder::testSkinHist);
	mTestMap["SkinStatBGR"]=Test(&TestFaceFinder::testSkinStatBGR, false);
	mTestMap["SkinStatHSV"]=Test(&TestFaceFinder::testSkinStatHSV, false);
	mTestMap["SkinStatLab"]=Test(&TestFaceFinder::testSkinStatLab, false);
	if (usingGPU())
	{
		mTestMap["GPUlocks"] = Test(&TestFaceFinder::testGPUlocks);
		mTestMap["GPUvsCPU"] = Test(&TestFaceFinder::testGPUvsCPU);
	}
}

//==================================================================================== helpers

struct Lines: public vector<string>
{
	Lines(const string & FN)
	{
		for (ListReader lr(FN); !lr.end(); )
		{
			string FNLine; trim(lr.fetch(FNLine)); if (FNLine.empty() || FNLine[0]=='#') continue;
			push_back(FNLine);
		}
	}
};

void TestFaceFinder::evalStatsMin(const string & label, const FaceFinder::EvalStats & es, REALNUM MinF)
{
	if (mVerbose) clog<<label<<" accuracy: "<<es<<endl;
	CHECK(es.FScore()>=MinF, label+format(" F-score=%f is below the expected min %f", es.FScore(), MinF));
}
FaceFinder::EvalStats TestFaceFinder::evalLine(const string & FNLine)
{
	FaceFinder
		inFF(mFRD, mRepoPath, FNLine, mFlags&FaceFinder::detectOff),
		outFF(mFRD, mRepoPath, FNLine, mFlags|FaceFinder::discard|FaceFinder::detection);
#pragma omp critical
	if (mVerbose>2)
		clog<<"inFF: "<<inFF.getFaces()<<endl
		<<"outFF: "<<outFF.getFaces()<<endl;
	FaceFinder::EvalStats es(inFF.getFaces(), outFF.getFaces(), cOverlapSlackTDefault, 0, mFlags&FaceFinder::cascade);
	if (mVerbose>2) clog<<"GT:FF accuracy: "<<es<<endl;
	return es;
}
void TestFaceFinder::evalSeq(const string & GroundTruthFN, REALNUM FscoreGoal)
{
	if (mVerbose) clog<<"TestFaceFinder::evalSeq "<<GroundTruthFN<<", FscoreGoal="<<FscoreGoal<<endl;
	FaceFinder::EvalStats ES;
	for (ListReader lr(GroundTruthFN); !lr.end(); )
	{
		string FNLine; trim(lr.fetch(FNLine)); if (FNLine.empty()) continue;
		if (mVerbose>2) clog<<FNLine<<endl;
		ES+=evalLine(FNLine);
	}
	evalStatsMin("GT:FF", ES, FscoreGoal);
}
void TestFaceFinder::evalPar(const string & GroundTruthFN, REALNUM FscoreGoal)
{
	if (mVerbose) clog<<"TestFaceFinder::evalPar "<<GroundTruthFN<<", FscoreGoal="<<FscoreGoal<<endl;
	Lines GroundTruth(GroundTruthFN);
	const unsigned N=GroundTruth.size();
	stringstream strmErrors;
	vector<FaceFinder::EvalStats> es(N);
	ParallelErrorStream PES;
#pragma omp parallel for shared(PES, GroundTruth, strmErrors)
	for (int i=0; i<N; ++i)
		try
		{
			const string & FNLine=GroundTruth[i];
		#pragma omp critical
			if (mVerbose>2)
				clog<<"thread "<<omp_get_thread_num()<<" of "<<omp_get_num_threads()<<": "<<FNLine<<endl;
			es[i]=evalLine(FNLine);
		}
		PCATCH(PES, string(format("GroundTruth[%d]=%s", i, GroundTruth[i].c_str())))
	PES.report("parallel TestFaceFinder::evalPar errors");
	FaceFinder::EvalStats ES; for (int i=0; i<N; ++i) ES+=es[i];
	evalStatsMin("GT:FF", ES, FscoreGoal);
}
void TestFaceFinder::eval(const string & GroundTruthFN, REALNUM FscoreGoal, bool parallel)
{
	if (parallel) evalPar(GroundTruthFN, FscoreGoal);
	else evalSeq(GroundTruthFN, FscoreGoal);
}
void TestFaceFinder::testLine(const string & FNLine)
{
	FaceFinder
		inFF(mFRD, mRepoPath, FNLine, mFlags&FaceFinder::detectOff),
		outFF(mFRD, mRepoPath, FNLine, mFlags|FaceFinder::discard|FaceFinder::detection);
#pragma omp critical
	if (mVerbose>2)
		clog<<"inFF: "<<inFF.getFaces()<<endl
		<<"outFF: "<<outFF.getFaces()<<endl;
	FaceFinder::EvalStats es(inFF.getFaces(), outFF.getFaces(),
		cOverlapSlackTDefault, 0, mFlags&FaceFinder::cascade);
	evalStats("GT:FF", es, 1);
}
void TestFaceFinder::testSeq(const string & ExpectedOutputFN)
{
	if (mVerbose) clog<<"TestFaceFinder::testSeq "<<ExpectedOutputFN<<endl;
	for (ListReader lr(ExpectedOutputFN); !lr.end(); )
	{
		string FNLine; trim(lr.fetch(FNLine)); if (FNLine.empty()) continue;
		if (mVerbose>2) clog<<FNLine<<endl;
		testLine(FNLine);
	}
}
void TestFaceFinder::testPar(const string & ExpectedOutputFN)
{
	if (mVerbose) clog<<"TestFaceFinder::testPar "<<ExpectedOutputFN<<endl;
	Lines ExpectedOutput(ExpectedOutputFN);
	const unsigned N=ExpectedOutput.size();
	ParallelErrorStream PES;
#pragma omp parallel for shared(ExpectedOutput, PES)
	for (int i=0; i<N; ++i)
		try
		{
			const string & FNLine=ExpectedOutput[i];
			int TN=omp_get_thread_num(), TC=omp_get_num_threads();
		#pragma omp critical
			if (mVerbose>2) clog<<TN<<" of "<<TC<<": "<<FNLine<<endl;
			testLine(FNLine);
		}
		PCATCH(PES, string(format("ExpectedOutput[%d]=%s", i, ExpectedOutput[i].c_str())))
	PES.report("parallel TestFaceFinder::testPar errors");
}
void TestFaceFinder::test(const string & ExpectedOutputFN, bool parallel)
{
	if (parallel) testPar(ExpectedOutputFN);
	else testSeq(ExpectedOutputFN);
}
void TestFaceFinder::testSpecific(const string & kind, bool parallel)
{
	REALNUM FscoreGoal=0.75;
	if (kind=="LM")
	{
		mFRD.SkinToneMapper.clear();
		FscoreGoal = 0.37;
	}
	else if (kind=="NUL")
	{
		mFRD.SkinToneMapper.clear();
		FscoreGoal = 0.45;
	}
	else if (kind=="ANN")
	{
		mFRD.SkinToneMapper.load("ANN", "NET_PL_9_15_50_90.18.ann.yml", 0.25, 0.5, usingGPU());
		FscoreGoal = 0.8;
	}
	else if (kind=="HST")
	{
		mFRD.SkinToneMapper.load("Hist","Hist.Lab.yml");
		FscoreGoal = 0.8;
	}
	else if (kind=="Pack")
	{
		mFRD.SkinToneMapper.load("Pack","SkinMappers.txt");
	}
	else if (kind=="BGR")
	{
		mFRD.SkinToneMapper.load("Stat", "SkinStat.BGR.yml");
		FscoreGoal = usingGPU() ? 0.38 : FscoreGoal; // TODO: higher GPU goal
	}
	else if (kind=="HSV")
	{
		mFRD.SkinToneMapper.load("Stat", "SkinStat.HSV.yml");
		FscoreGoal = usingGPU() ? 0.29 : FscoreGoal; // TODO: higher GPU goal
	}
	else if (kind=="Lab")
	{
		mFRD.SkinToneMapper.load("Stat", "SkinStat.Lab.yml");
		FscoreGoal = usingGPU() ? 0.29 : 0.69; // TODO: higher goals
	}
	else if (mFRD.SkinToneMapper.empty()) mFRD.SkinToneMapper.load("Stat", "SkinStat."+kind+".yml");
	string FN=getFileName(mImgLstFN), ext=getFileExt(mImgLstFN);
	test(FN+".FF."+kind+xGPU()+ext, parallel);
	eval(FN+".GT"+ext, FscoreGoal, parallel);
	mFRD.SkinToneMapper.clear();
}
void TestFaceFinder::evalStats(const string & label, const FaceFinder::EvalStats & es, REALNUM ExpectedF)
{
	const REALNUM eps=1e-6;
#pragma omp critical
	if (mVerbose) clog<<label<<" accuracy: "<<es<<endl;
	stringstream ss;
	CHECK(fabs(es.FScore()-ExpectedF)<=eps, 
		dynamic_cast<stringstream&>(ss<<label<<" F-score="<<es.FScore()<<" is different from the expected "<<ExpectedF).str()
	);
}

//==================================================================================== tests
void TestFaceFinder::testEvalStats()
{
	unsigned RegMinDim=12; REALNUM RegMatchTol=cOverlapSlackTDefault;
	FaceRegions GT(RegMinDim), FF(RegMinDim);
	GT.add(new FaceRegion("f", Rect(0,0,16,16)), false, 0);	FF.add(new FaceRegion("f", Rect(1,1,16,16)), false, 0);	// M
	GT.add(new FaceRegion("p", Rect(20,20,16,16)), false, 0);	FF.add(new FaceRegion("f", Rect(21,21,16,16)), false, 0); // W
	GT.add(new FaceRegion("p", Rect(40,40,16,16)), false, 0);													// FN
													FF.add(new FaceRegion("p", Rect(61,61,16,16)), false, 0);	// FP
	if (mVerbose) clog<<"GT="<<GT<<endl;
	CHECK(GT.size()==3,"GT size is different from expected 3");

	if (mVerbose) clog<<"FF="<<FF<<endl;
	CHECK(FF.size()==3, "FF size is different from expected 3");

	evalStats("GT:GT", FaceFinder::EvalStats(GT, GT, RegMatchTol, 0.0), 1.0);
	evalStats("GT:FF sw=0.0", FaceFinder::EvalStats(GT, FF, RegMatchTol, 0.0), 1./3);
	evalStats("GT:FF sw=0.5", FaceFinder::EvalStats(GT, FF, RegMatchTol, 0.5), 0.5);
	evalStats("GT:FF sw=1.0", FaceFinder::EvalStats(GT, FF, RegMatchTol, 1.0), 2./3);
}
void TestFaceFinder::testLandmarks()
{
	unsigned FFopts=mFlags;
	PFaceFinder pFF=new FaceFinder(mFRD, "../Images/", "face2.rot90.png", mFlags);
	CHECK(!pFF->gotFaces(), "faces were found when none should be");
	pFF=new FaceFinder(mFRD, "../Images/", "face2.rot90.png", mFlags|FaceFinder::rotation);
	CHECK(pFF->gotFaces(), "check for non-cascade faces failed");
	mFlags|=FaceFinder::cascade;
	pFF=new FaceFinder(mFRD, "../Images/", "face2.rot90.png", mFlags|FaceFinder::rotation|FaceFinder::seekLandmarks|FaceFinder::keepCascaded);
	CHECK(pFF->gotFaces(true), "relaxed check for cascaded faces failed");
	CHECK(pFF->gotFaces(), "strict check for cascaded faces failed");
	testSpecific("LM");
	mFlags=FFopts;
}
void TestFaceFinder::testMultithread()
{
	testSpecific("HST", true);
}
void TestFaceFinder::testStandard()
{
	unsigned FFopts=mFlags;
	mFlags|=FaceFinder::rotation;
	testSpecific("NUL");
	mFlags=FFopts;
}
void TestFaceFinder::testSkinStatBGR()
{
	testSpecific("BGR");
}
void TestFaceFinder::testSkinStatHSV()
{
	testSpecific("HSV");
}
void TestFaceFinder::testSkinStatLab()
{
	testSpecific("Lab");
}
void TestFaceFinder::testSkinANN()
{
	testSpecific("ANN");
}
void TestFaceFinder::testSkinHist()
{
	testSpecific("HST");
}
void TestFaceFinder::testSkin()
{
	testSpecific("Pack");
}

#define FRDCommonArgs "FFModels/", "haarcascade_frontalface_alt2.xml", "haarcascade_profileface.xml", "", "", cRegDimMin, cRegDimMax, 0.25, 0.5, 0.5
/// \return a static reference to a single face region detector object preferring GPU
static FaceRegionDetector & getFRDGPU()
{
	StaticLkdCtor FaceRegionDetector FRD(FRDCommonArgs, true);
	return FRD;
}

/// \return a static reference to a single face region detector object preferring CPU
static FaceRegionDetector & getFRDCPU()
{
	StaticLkdCtor FaceRegionDetector FRD(FRDCommonArgs, false);
	return FRD;
}

/// \return a static reference to a single face region detector object
static FaceRegionDetector & getFRD(bool preferGPU /**< prefer GPU, when available */)
{
	return preferGPU ? getFRDGPU() : getFRDCPU();
}
int testGPUCnt(bool VL=0)
{
	int cnt = getGPUCount();
	if (VL) clog << "GPU count=" << cnt << endl;
	return cnt;
}
void TestFaceFinder::testGPUvsCPU()
{
	CHECK(usingGPU(), "expecting GPU(s) to be preferred and used");
	CHECK(testGPUCnt(mVerbose), "expecting GPU(s) to be available");
	const string & RepoPath = "../Images/";
	const double MinSpeedup = 2;
	FaceRegionDetector
		&FRDGPU = getFRDGPU(),
		&FRDCPU = getFRDCPU();
	for (const string & ImgFN : { "Glen.Croc.jpg", "Steve.jpg", "Lena.jpg", "kimjongeun.jpg" })
	{
		CVTiming tmg;
		FaceFinder ffCPU(FRDCPU, RepoPath, ImgFN, mFlags);
		double durCPU = tmg.diff();

		tmg.init();
		FaceFinder ffGPU(FRDGPU, RepoPath, ImgFN, mFlags);
		double durGPU = tmg.diff();

		evalStats(ImgFN + " GPU:CPU", FaceFinder::EvalStats(ffGPU.getFaces(), ffCPU.getFaces()), 1.f);
		double GPUspeedup = durCPU / durGPU;
		if (mVerbose) clog << ImgFN << " GPU:CPU speedup = " << GPUspeedup << endl;
		CHECK(GPUspeedup > MinSpeedup, format("%s GPU/CPU speedup %g is lower than minimum %g", ImgFN.c_str(), GPUspeedup, MinSpeedup));
	}
}
void TestFaceFinder::testGPUlocks()
{
	CHECK(usingGPU(), "expecting GPU(s) to be preferred and used");
	CHECK(testGPUCnt(mVerbose), "expecting GPU(s) to be available");
	ParallelErrorStream PES;
	auto LK = [&](int ID)
	{
		try
		{
			Ptr<GPULocker> plkr = ID < 0 ? new GPULocker() : new GPULocker(ID);
			if (!plkr) throw FaceMatch::Exception(format ("unable to allocate GPULocker for %d", ID));
			GPULocker & lkr = *plkr;
			if (mVerbose) clog<<"thread "<<omp_get_thread_num()<<" locked GPU "<<lkr.getID()<<endl;
			this_thread::sleep_for(chrono::seconds(1));
			if (mVerbose) clog<<"thread "<<omp_get_thread_num()<<" unlocked GPU "<<lkr.getID()<<endl;
		}
		catch (const FaceMatch::Exception & e)
		{
			string msg=e.what(), lbl="locking invalid GPU:";
			if (ID>=0 && ID<getGPUCount() && msg.find("CHECK(ID>=0 && ID<getCount())")==string::npos)
				PES.append(lbl, msg);
			else if (mVerbose)
				clog<<lbl<<" correct response: "<<msg<<endl;
		}
		PCATCH(PES, "locking a GPU")
	};
	auto FF = [&](const string imgFN)
	{
		try
		{
			if (mVerbose) clog<<" process "<<imgFN<<" in thread "<<omp_get_thread_num()<<endl;
			FaceFinder ff(getFRD(true), "", imgFN, mFlags);
			if (mVerbose) clog<<"FF="<<ff<<endl;
		}
		catch (const FaceMatch::Exception & e)
		{
			string msg=e.what(), lbl="processing gif:";
			if (getFileExt(imgFN)==".gif" && msg.find(".gif: empty image")==string::npos)
				PES.append(lbl, msg);
			else if (mVerbose)
				clog<<lbl<<" correct response: "<<msg<<endl;
		}
		PCATCH(PES, "processing "+imgFN)
	};
	#pragma omp parallel sections
	{
		#pragma omp section
			LK(-1);
		#pragma omp section
			LK(-1);
		#pragma omp section
			LK(0);
		#pragma omp section
			LK(1);
		#pragma omp section
			LK(100);
		#pragma omp section
			FF("../Images/Glen.Croc.jpg");
		#pragma omp section
			FF("../Images/Lena.jpg");
		#pragma omp section
			FF("../Images/Steve.jpg");
		#pragma omp section
			FF("../Images/Mike.png");
		#pragma omp section
			FF("../Images/Mike.gif");
	}
	PES.report("parallel TestFaceFinder::testPar errors");
}
void TestFaceFinder::testFRD()
{
	try { FaceRegionDetector(); } // default instantiation/destruction
	catch(const exception & e) { throw FaceMatch::Exception(format("FRD default instantiation failed: %s", e.what())); }
	catch(...) { throw FaceMatch::Exception("FRD default instantiation failed with an unknown exception"); }

	const string
		& XMLBasePath = "FFModels/",
		& FaceModelFN = "haarcascade_frontalface_alt2.xml",
		& ProfileModelFN = "haarcascade_profileface.xml",
		& SkinColorMapperKind = "",
		& SkinColorParmFN = "";
	const unsigned
		aFaceDiameterMin=cRegDimMin,
		aFaceDiameterMax=cRegDimMax;
	const REALNUM
		SkinMassT=0.25,
		SkinLikelihoodT=0.5,
		aFaceAspectLimit=0.5;
	const int GPUCnt = getGPUCount();

	ErrorDumpSuppressor cvErrDmpSupp(!mVerbose);

	const bool PrefGPU[]={false, true};
#define tryFRD for (bool p : PrefGPU){ try {
#define CorrectCatch \
	catch(const exception & e) { if (mVerbose) clog<<"correct behavior: "<<e.what()<<endl; }\
	catch(...) { if (mVerbose) clog<<"correct behavior with an unknown exception"<<endl; }
#define catchFRD throw FaceMatch::Exception("FRD instantiation happened with an invalid argument"); } CorrectCatch }

	tryFRD
		FaceRegionDetector("??", FaceModelFN, ProfileModelFN, SkinColorMapperKind, SkinColorParmFN, aFaceDiameterMin, aFaceDiameterMax, SkinMassT, SkinLikelihoodT, aFaceAspectLimit, p);
	catchFRD

	tryFRD
		FaceRegionDetector(XMLBasePath, "", ProfileModelFN, SkinColorMapperKind, SkinColorParmFN, aFaceDiameterMin, aFaceDiameterMax, SkinMassT, SkinLikelihoodT, aFaceAspectLimit, p);
	catchFRD

	tryFRD
		FaceRegionDetector(XMLBasePath, FaceModelFN, "", SkinColorMapperKind, SkinColorParmFN, aFaceDiameterMin, aFaceDiameterMax, SkinMassT, SkinLikelihoodT, aFaceAspectLimit, p);
	catchFRD

	cvErrDmpSupp.off();

	const unsigned N=16; // TODO: config/param
	Ptr<FaceRegionDetector> gpuFRD, cpuFRD;
	for (int i=1; i<=N; ++i)
	{
		gpuFRD = new FaceRegionDetector(XMLBasePath, FaceModelFN, ProfileModelFN, SkinColorMapperKind, SkinColorParmFN,
				aFaceDiameterMin, aFaceDiameterMax,
				SkinMassT, SkinLikelihoodT, aFaceAspectLimit,
				true);
		if (GPUCnt) CHECK(gpuFRD->usingGPU(), format("FRD not using %d GPUs", GPUCnt));
		cpuFRD = new FaceRegionDetector(XMLBasePath, FaceModelFN, ProfileModelFN, SkinColorMapperKind, SkinColorParmFN,
				aFaceDiameterMin, aFaceDiameterMax,
				SkinMassT, SkinLikelihoodT, aFaceAspectLimit, false);
		CHECK(!cpuFRD->usingGPU(), "CPU FRD using GPU");
		if (mVerbose) clog<<i<<" ";
	}
}
