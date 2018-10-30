
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
#include "NearDupImgDetector.h"
#include "TestNearDupImgDetector.h"

using namespace std;
using namespace FaceMatch;

/**
 * Print NearDupImgDetector CLI help.
 * @param ProgName program executable name
 * @param ErrorCode error code to return; default is 0
 * @return error code
 */
int help(const char * ProgName, int ErrorCode=0)
{
	ostream & os = ErrorCode ? cerr : cout;
	os<<"A near-duplicate image detection/generation utility. 2011-2016 (C) \n\
Synopsis:\n\
  "<<ProgName<<" [options]\n\
where options are\n\
  -? or -h -- display help\n\
  -a -- keep image tab-delimited attributes\n\
  -d -- use near-duplicates stated after the image file name\n\
  -g[:cprs] -- generate, near-duplicate descriptors by cropping, pi/2-phasing, rotation, scaling\n\
  -GPU -- if available, prefer GPU over CPU\n\
  -lst[:{in|out}] FN -- optional [input|output] list file name,\n\
     if not specified, input is taken from STDIN, output is written to STDOUT\n\
  -p ImgRepoPath -- optional path to images directory, e.g. /my/images/; default is ./\n\
  -s -- save images generated wiht -g\n\
  -test[:Specific] -- run (specific) unit test(s)\n\
  -u -- output unique, de-duplicated image entries to -lst:out, writing dups to STDOUT\n\
  -v[=N] -- specify verbocity level; 1 is default; 0 when omitted\n\
  ImgageFN -- image file name to match against the list\n\
Output is written in form of an image list file, with each line being\n\
  image.ext[\tneardup1.ext[\tneardup2.ext...]]\n\
with tab separated entries."<<endl
	;
	return ErrorCode;
}

/**
 * Given a list of image files (as an input file name or on STDIN), find all (near-)duplicates.
 * For each image, print its best near-duplicate, if any.
 * @param ac input arg count
 * @param av input arguments
 * @see help()
 */
int main(int ac, char * av[])
{
	string
		RepoPath,
		LstInpFN,
		LstOutFN,
		ImgInpFN,
		SpecificTest;
	bool
		unique = false,
		prefGPU = false;
	unsigned
		flags = 0,
		VerbLevel=0;
	ERunMode RunMode=rmExec;

	try
	{
		if (ac==1) help(*av, 0);
		for (int a=1; a<ac; ++a)
		{
			if (!strcmp("-h", av[a]) || !strcmp("-?", av[a])) return help(*av, 0);
			else if (!strcmp("-a", av[a])) flags |= keepAttributes;
			else if (!strcmp("-d", av[a])) flags |= useDups;
			else if (!strcmp("-GPU", av[a])) prefGPU=true; // TODO: utilize this option in image processing
			else if (!strcmp("-p", av[a])) RepoPath=av[++a];
			else if (!strcmp("-lst", av[a])) LstInpFN=LstOutFN=av[++a];
			else if (!strcmp("-lst:in", av[a])) LstInpFN=av[++a];
			else if (!strcmp("-lst:out", av[a])) LstOutFN=av[++a];
			else if (!strcmp("-test", av[a])) RunMode=rmTest;
			else if (av[a] == strstr(av[a], "-test:"))
			{
				RunMode = rmTest;
				const unsigned len=strlen("-test:");
				SpecificTest=av[a]+len;
			}
			else if (!strcmp("-g", av[a])) flags = genAll;
			else if (av[a] == strstr(av[a], "-g:"))
			{
				if (strchr(av[a]+3, 'c')) flags |= genCrop;
				if (strchr(av[a]+3, 'r')) flags |= genRotate;
				if (strchr(av[a]+3, 's')) flags |= genScale;
				if (strchr(av[a]+3, 'p')) flags |= gen90DegreePhases;
			}
			else if (!strcmp("-s", av[a])) flags=genSave;
			else if (!strcmp("-u", av[a])) unique=true;
			else if (!strcmp("-v", av[a])) VerbLevel=1;
			else if (av[a] == strstr(av[a], "-v="))
			{
				const unsigned len=strlen("-v=");
				VerbLevel=atoi(av[a]+len);
			}
			else if (*av[a]=='-') throw FaceMatch::Exception(format("invalid option %s", av[a]));
			else if (ImgInpFN.empty()) ImgInpFN = av[a];
		}
		setVerbLevel(VerbLevel);
		preferGPU(prefGPU);

		if (RunMode==rmTest)
		{
			TestNearDupImgDetector test
			(
				RepoPath.empty() ? TestRepoPath : RepoPath,
				LstInpFN.empty() ? TestImgLstFN : LstInpFN,
				VerbLevel
			);
			return test.run(SpecificTest);
		}
		if (LstInpFN.empty()) clog<<"warning: no image list file, image file names are taken from STDIN one line at a time."<<endl;

		{ TIMELOG("near-dup")
			InputStream ifs(LstInpFN);
			NearDupImgDetector ndid(ImgInpFN, ifs, RepoPath, flags);
			ifs.close();
			OutputStream ofs(LstOutFN);
			ndid.printNearDups(ofs, unique);
			ofs.close();
			if (ofs.isFile())
				ndid.printNearDups(cout, !unique);
		}
	}
	catch(const exception & e)
	{
		cerr<<"exception: "<<e.what()<<endl;
		return 2;
	}
	catch(...)
	{
		cerr<<"unknown exception"<<endl;
		return 1;
	}
	return 0;
}

string nearImageDuplicates(int ac, char * av[])
{
	return ac+" flowers";
}
