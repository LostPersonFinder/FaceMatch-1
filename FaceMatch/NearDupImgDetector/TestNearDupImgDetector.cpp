
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
#include "TestNearDupImgDetector.h"
#include "NearDupImgDetector.h"
#include <sstream>
#include <fstream>

using namespace std;

bool TestNearDupImgDetector::getMatch(const string & ImgFN, const string & line)const
{
	stringstream sline(line);
	NearDupImgDetector ndid(mRepoPath+ImgFN, sline, mRepoPath);
	stringstream os; ndid.printNearDups(os);
	string match; getline(os, match);
	return match==ImgFN;
}
void TestNearDupImgDetector::testSelfMatch()
{
	ifstream src(mImgLstFN.c_str());
	CHECK(src.is_open(), "unable to open "+mImgLstFN);
	if (mVerbose) clog<<"dedup -lst:in "<<mImgLstFN<<endl;

	string line; getline(src, line);
	while(line.length())
	{
		if (mVerbose) clog<<line<<endl;
		bool match = getMatch(line, line);
		CHECK(match, "Self match failed for " + line);
		string ImgFN="9999_P1130049jpg-1182496_md.jpg"; // self non-match
		match = getMatch(ImgFN, line);
		CHECK(!match, "Self non-match failed for " + ImgFN);
		getline(src, line);
	}
}
void TestNearDupImgDetector::testNearDups()
{
	ifstream src(mImgLstFN.c_str());
	CHECK(src.is_open(), "unable to open "+mImgLstFN);
if (mVerbose) clog<<"dedup -lst:in "<<mImgLstFN<<endl;
	NearDupImgDetector ndid("", src, mRepoPath);
	src.close();
//--- generate near-dup output
	stringstream strmNearDups;
	ndid.printNearDups(strmNearDups);
if (mVerbose>3) clog<<"strmNearDups="<<strmNearDups.str()<<endl;
//--- load expected output
	string ExpOutFN = getFileName(mImgLstFN)+".dup"+getFileExt(mImgLstFN);
if (mVerbose) clog<<"compare with "<<ExpOutFN<<endl;
	ifstream strmExpOut(ExpOutFN.c_str()); CHECK(strmExpOut.is_open(), "unable to open "+ExpOutFN);
	compare(strmExpOut, strmNearDups);
}
void TestNearDupImgDetector::testPrintAndGet()
{
if (mVerbose) clog<<"dedup -lst:in "<<mImgLstFN<<endl;
	NearDupImgDetector ndid("", mImgLstFN, mRepoPath);
	stringstream strm; ndid.printNearDups(strm);
	string ndup=ndid.getNearDups();
if (mVerbose)
	clog<<"print: "<<strm.str()<<endl
	<<"get: "<<ndup<<endl;
	CHECK(strm.str()==ndup, "print and get produced different results");
}
void TestNearDupImgDetector::testSyntheticNearDups()
{
	ifstream src(mImgLstFN.c_str());
	CHECK(src.is_open(), "unable to open "+mImgLstFN);
if (mVerbose) clog<<"dedup -lst:in "<<mImgLstFN<<endl;
	NearDupImgDetector ndid("", src, mRepoPath, genScale); // generate scaled near dups and verify proper similarity grouping
}
