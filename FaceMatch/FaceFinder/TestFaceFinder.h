
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

#pragma once // 2012-2017 (C) FaceMatch@NIH.gov

#include <UnitTest.h>

using namespace FaceMatch;

/// test face finder
class TestFaceFinder: public UnitTest<TestFaceFinder>
{
	FaceRegionDetector & mFRD;
	unsigned mFlags;
//=== helpers
	/**
	 * For each image file in mImgLstFN, get faces via face detection, and compare the results to the expected output.
	 */
	void test(const string & ExpectedOutputFN, bool parallel=false);
	void testSeq(const string & ExpectedOutputFN);
	void testPar(const string & ExpectedOutputFN);
	void testLine(const string & FNLine);
	/**
	 * For each image file in GroundTruthFN, evaluate current face detector and verify that it meets specified accuracy goal.
	 */
	void eval(const string & GroundTruthFN, REALNUM FscoreGoal=0.75, bool parallel=false);
	void evalSeq(const string & GroundTruthFN, REALNUM FscoreGoal=0.75);
	void evalPar(const string & GroundTruthFN, REALNUM FscoreGoal=0.75);
	FaceFinder::EvalStats evalLine(const string & FNLine);
	/**
	 * For each image file in mImgLstFN, get faces via face detection + SkinStat in a given color space,
	 * and compare the results to the expected output.
	 */
	void testSpecific(const string & kind, bool parallel=false);
	void evalStats(const string & label, const FaceFinder::EvalStats & es, REALNUM ExpectedF);
	void evalStatsMin(const string & label, const FaceFinder::EvalStats & es, REALNUM MinF);
	bool usingGPU()const{return mFRD.usingGPU();}
	string xGPU()const{return usingGPU() ? ".GPU" : "";}
//=== tests
	/**
	 * Test EvalStats response for complete and partial matches.
	 */
	void testEvalStats();
	/**
	 * For each image file in mImgLstFN, get faces via standard detection, and compare the results to the expected output.
	 */
	void testStandard();
	/**
	 * For each image file in mImgLstFN, get faces via standard detection + SkinStatBGR, and compare the results to the expected output.
	 */
	void testSkinStatBGR();
	/**
	 * For each image file in mImgLstFN, get faces via standard detection + SkinStatHSV, and compare the results to the expected output.
	 */
	void testSkinStatHSV();
	/**
	 * For each image file in mImgLstFN, get faces via standard detection + SkinStatLab, and compare the results to the expected output.
	 */
	void testSkinStatLab();
	/// For each image file in mImgLstFN, get faces via standard detection + SkinANN, and compare the results to the expected output.
	void testSkinANN();
	/// For each image file in mImgLstFN, get faces via standard detection + SkinHist, and compare the results to the expected output.
	void testSkinHist();
	/// For each image file in mImgLstFN, get faces via standard detection + Skin ensemble, and compare the results to the expected output.
	void testSkin();
	/// For each image file in mImgLstFN, run standard+SkinANN face detection in parallel, and compare the results to the expected output.
	void testMultithread();
	/// For each image file in mImgLstFN, run standard face detection with sub-regions, and compare the results to the expected output.
	void testLandmarks();
	/// Load Face Region Detector (FRD) multiple times with and without GPU.
	void testFRD();
	/// Test GPU locks while multi-threading.
	void testGPUlocks();
	/// Test GPU/CPU accuracy and speedup.
	void testGPUvsCPU();
public:
	/**
	 * Instantiate.
	 * @param FRD	face region detector reference
	 * @param flags	face finder flags
	 * @param RepoPath	image repository path
	 * @param ImgLstFN	image list file name
	 * @param VrbLvl	verbosity level; 0 = no diagnostics
	 */
	TestFaceFinder(FaceRegionDetector & FRD, unsigned flags,
		const string & RepoPath=TestRepoPath,
		const string & ImgLstFN=TestImgLstFN,
		int VrbLvl=0);
};
