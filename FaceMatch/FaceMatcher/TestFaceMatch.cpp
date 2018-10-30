
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

#include "stdafx.h" // 2015-2017 (C) FaceMatch@NIH.gov
#include "TestFaceMatch.h"
#include <ImageMatcherFaceRegions.h>

TestFaceMatch::TestFaceMatch(unsigned FaceFinderFlags): mFaceFinderFlags(FaceFinderFlags)
{
	SFaceRegionDetector::init(); // single face region detector
	mTestMap["FaceDetection"] = Test(&TestFaceMatch::testFaceDetection);
	mTestMap["FaceMatching"] = Test(&TestFaceMatch::testFaceMatching);
	mTestMap["Singletons"] = Test(&TestFaceMatch::testSingletons);
}

struct Object
{
	typedef string type;
	Object(type val) : mValue(val) {}
	type & value(){ return mValue; }
private:
	type mValue;
};
static Object & getSingleObject(int n = -1)
{
	StaticLkdCtor Object o(toString(n));
	return o;
};
void TestFaceMatch::testSingletons()
{
	const int N = 32;
	vector<Object::type> SV(N);
	ParallelErrorStream PES;
#pragma omp parallel for shared (SV) // test simultaneous singleton access
	for (int i = 0; i < N; ++i)
		try
		{
			SV[i] = getSingleObject(i).value();
		}
		PCATCH(PES, format("parallel get singleton %d",i))
	PES.report("parallel get singleton errors");
	Object::type val = getSingleObject().value();
	if (getVerbLevel()) clog << "val=" << val << ", SV="<< SV;
	for (int i = 0; i < N; ++i)
		CHECK(SV[i] == val, format("value at %d differs from the common singleton value", i));
}
void TestFaceMatch::testFaceDetection()
{
	const string
		base = "../Images/", // base path makes sense for multiple files from the same folder
		list = ("12326_gladyscartyjpeg-1191520_md.jpg\tf[88,49;62,62]\n" // input images with face regions
				"3099739__86846_thumb.jpg\tf[193,157;64,64]\tf[225,109;80,80]\tf[121,144;83,83]");
	if (getVerbLevel()) clog<<"processing...";
	for (stringstream strmInput(list); !strmInput.eof(); )
	{
		string line; getline(strmInput, line); if (line.empty()) continue; // process line-by-line
		FaceFinder ff(SFaceRegionDetector::get(), base, line, // construct a FaceFinder instance per image
			mFaceFinderFlags|FaceFinder::discard|FaceFinder::detection|FaceFinder::HistEQ); // re-detect face regions
		stringstream strmFF; strmFF<<ff; // stream the resulting regions
		string resFF=strmFF.str();
		if (getVerbLevel()>1) clog<<"in: "<<line<<endl<<"out: "<<resFF<<endl;
		CHECK(resFF==line, "input and output face regions should be the same");
		CHECK(ff.gotFaces(), "some faces should be detected");
		auto pfr = ff.getPrimaryFaceRegion(); // primary (most central) face
		CHECK(pfr, "primary face region should not be null");
		CHECK(pfr->area(), "primary face region area should not be empty");
		for (auto c: ff.getFaces()) // iterate through the face regions
		{
			CHECK(c->Kind=="f" || c->Kind=="p", "face/profile region kind should be 'f' or 'p' respectively");
			const auto & fr = dynamic_cast<const FaceRegion&>(*c);
			CHECK(fr.diameter()>=cRegDimMin, "primary face regions should be large enough");
		}
	}
}
void TestFaceMatch::testFaceMatching()
{
	const string
		base = "../Images/", // base path makes sense for multiple files from the same folder
		list = ("12326_gladyscartyjpeg-1191520_md.jpg\tf[88,49;62,62]\n" // input images with face regions
				"3099739__86846_thumb.jpg\tf[193,157;64,64]\tf[225,109;80,80]\tf[121,144;83,83]");
	ImageMatcherFaceRegions im("", SFaceRegionDetector::get());
	if (getVerbLevel()) clog<<"compute a visual index...";
	for (stringstream strmInput(list); !strmInput.eof(); )
	{
		string line; getline(strmInput, line); if (line.empty()) continue; // process line-by-line
		if (getVerbLevel()>1) clog<<line<<endl;
		unsigned cnt=im.ingest(base+line, "");
		CHECK(cnt, "ingest count should be positive");
	}
	if (getVerbLevel()) clog<<"query visual index...";
	for (stringstream strmInput(list); !strmInput.eof(); ) // query visual index
	{
		string line; getline(strmInput, line); if (line.empty()) continue; // process line-by-line
		string res;
		unsigned cnt=im.query(res, base+line);
		CHECK(cnt, "query count should be positive");
		if (getVerbLevel()>1) clog<<res;
	}
	if (getVerbLevel()) clog<<"manipulate index...";
	im.save("ImageMatcherFaceRegions.txt");
	im.clear();
	CHECK(!im.count(), "record count should be 0");
	unsigned cnt=im.load("ImageMatcherFaceRegions.txt");
	CHECK(cnt>0, "load count should be positive");
	string res; cnt=im.list(res);
	if (getVerbLevel()>1) clog<<"list("<<cnt<<"):"<<res;
	CHECK(!res.empty(), "list results should not be empty");
	CHECK(cnt>0, "list count should be positive");
	cnt=im.remove(base+"3099739__86846_thumb.jpg");
	CHECK(cnt, "remove count should be 3");
}
