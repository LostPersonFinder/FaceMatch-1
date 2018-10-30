
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
#include "TestFaceMatch.h"
#include "FaceMatcher.h"
#include <ImgDscHaar.h>
#include <ImgDscKeyPt.h>
#include <string>

enum
{
	errNone = 0,
	errArgument,
	errException,
	errUnknown
};

using namespace std;

int help(const string & ProgName, int ErrorCode)
{
	ostream & os = ErrorCode ? cerr : cout;
	os<<"CLI for face/image matching utility\n\
2011-2017 (C) FaceMatch@NIH.gov\n\
Synopsis:\n"
	<<std::getFileName(ProgName, false)<<" [options] [first.arg[ second.arg]]\n\
where\n\
  first.arg -- input image or image list file\n\
  second.arg -- query image or list file to match against the image(s) in first.arg\n\
options are\n\
  -c -- cascade to sub-regions in face detection and matching\n\
  -d {HAAR|SURF|SIFT|ORB|RSILC|MANY} -- image descriptor, default is SURF\n\
  -e -- evaluate the accuracy of matching using leave-one-out approach\n\
  -f -- use FLANN indexing\n\
  -GPU -- prefer GPU over CPU, when possible\n\
  -m -- mirror-flip second image\n\
  -o -- output index file name\n\
  -p RepoPath -- image repository path\n\
  -r A -- rotate the face region by A degrees; default is 0: no change\n\
  -x M -- crop/expand the face region by M; default is 1: no change\n\
  -s -- save visual matches\n\
  -t T -- matching threshold in [0,1]; default is 0.5\n\
  -test -- run unit tests\n\
  -V -- display visual diagnostics\n\
  -v -- display matched images\n\
  -w -- whole image match, no face detection\n\
output is written to STDOUT, each line in the format:\n\
  score\timage.ext\n\
in distance score ascending order for scores below the threshold."<<endl;
	return ErrorCode;
}

using namespace FaceMatch;

int main(int ac, char * av[])
{
	string ImgRepoPath, SpecificTest;
	REALNUM
		MatchT = 0.5,
		ImgXfmParam = 1;
	unsigned
		flags=fmNone,
		FaceFinderFlags = FaceFinder::ProcFlagsDefault,
		RunMode = rmExec;
	const char *ImgDsc="SURF", *aFN = 0, *bFN = 0, *OutNdxFN=0;
	bool prefGPU=false;
	auto VisualVerbose = [&](unsigned lvl)
	{
		FaceFinderFlags |= FaceFinder::visual | FaceFinder::verbose;
		flags |= fmShow;
		setVisVerbLevel(lvl);
	};
	try
	{
		for (int a=1; a<ac; ++a)
		{
			if (!strcmp(av[a], "-h")) return help(*av, 0);
			else if (!strcmp(av[a], "-c")) FaceFinderFlags |= FaceFinder::cascade;
			else if (!strcmp(av[a], "-d")) ImgDsc = av[++a];
			else if (!strcmp(av[a], "-e")) RunMode = rmEval;
			else if (!strcmp(av[a], "-f")) flags |= fmFLANN;
			else if (!strcmp(av[a], "-GPU")) prefGPU = true;
			else if (!strcmp(av[a], "-m")) flags |= fmMirror;
			else if (!strcmp(av[a], "-o")) OutNdxFN = av[++a];
			else if (!strcmp(av[a], "-p")) ImgRepoPath = av[++a];
			else if (!strcmp(av[a], "-r")) { flags |= fmRotate; ImgXfmParam = atof(av[++a]); }
			else if (!strcmp(av[a], "-x")) { flags |= fmScale; ImgXfmParam = atof(av[++a]); }
			else if (!strcmp(av[a], "-s")) flags |= fmSave;
			else if (!strcmp(av[a], "-t")) MatchT = atof(av[++a]);
			else if (!strcmp(av[a], "-test")) RunMode = rmTest;
			else if (startsWith(av[a], "-test:"))
			{
				RunMode = rmTest;
				const unsigned len = strlen("-test:");
				SpecificTest = av[a] + len;
			}
			else if (!strcmp(av[a], "-V")) VisualVerbose(1);
			else if (!strcmp(av[a], "-v")) flags |= fmShow;
			else if (checkPrefix(toLC(av[a]), "-v="))
			{
				const unsigned len=strlen("-v=");
				int VerbLevel=atoi(av[a]+len);
				flags |= fmShow;
				if (checkPrefix(av[a], "-V")) VisualVerbose(VerbLevel);
				setVerbLevel(VerbLevel);
			}
			else if (!strcmp(av[a], "-w")) ImgXfmParam = 0;
			else if (av[a][0] == '-') throw FaceMatch::Exception(format("invalid option '%s', use -h for help", av[a]));
			else if (!aFN) aFN = av[a];
			else if (!bFN) bFN = av[a];
		}
		preferGPU(prefGPU);
		if (RunMode==rmTest)
		{
			TestFaceMatch test(FaceFinderFlags);
			return test.run(SpecificTest);
		}
		if (!aFN) throw FaceMatch::Exception("missing first.arg");
		if (RunMode==rmEval)
		{
			FaceMatcher fm(ImgDsc, aFN, ImgRepoPath, MatchT, ImgXfmParam, FaceFinderFlags, flags);
			fm.eval(cout);
			return 0;
		}
		if (!bFN) throw FaceMatch::Exception("missing second.arg");
		string aExt = getFileExt(aFN);
		if (aExt==".lst" || aExt==".yml" || aExt==".ndx")
		{
			FaceMatcher fm(ImgDsc, aFN, ImgRepoPath, MatchT, ImgXfmParam, FaceFinderFlags, flags);
			const QueryResults & QR = fm.match(bFN, FaceFinderFlags, flags);
			cout<<QR;
			if (flags&fmShow) QR.display(ImgRepoPath);
			if (OutNdxFN) fm.save(OutNdxFN);
		}
		else // assume individual matching
		{
			FaceMatcher fm(ImgDsc);
			REALNUM d=fm.distance(aFN, bFN, ImgXfmParam, FaceFinderFlags, flags);
			cout<<d<<"\t"<<aFN<<"\t"<<bFN<<endl;
		}
	}
	catch(const exception & e)
	{
		cerr<<e.what()<<endl;
		return errException;
	}
	catch(const SpecialChar c)
	{
		cerr<<"special char("<<char(c)<<"): 0x"<<hex<<c<<endl;
		return errException;
	}
	catch(...)
	{
		cerr<<"unknown exception"<<endl;
		return errUnknown;
	}
	return errNone;
}
