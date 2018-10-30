
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
#include "ImageMetaData.h"
#include "HistogramU2U.h"
#include "MapImgID2Regions.h"
#include <list>

using namespace std;
using namespace FaceMatch;

/// CLI error codes
enum Errors
{
	errNone,      ///< errNone
	errParameter, ///< errParameter
	errException, ///< errException
	errUnknown    ///< errUnknown
};

/// \return image ID from file name
inline string getImgIDFromFN(const string & ImgFNLine /**< list file line as ImgID[\tReg...] */)
{
	stringstream strmImgFNLine(ImgFNLine);
	string ImgID; getline(strmImgFNLine, ImgID, '\t');
	return ImgID;
}

/// evaluation kinds
enum EEvalKind
{
	evalNone,
	evalGood,
	evalBad,
	evalFN,
	evalFP,
	evalAll
};

/// \return error code after printing help to STDOUT when error code is 0; print to STDERR otherwise
int help(const string & ProgName, ///< program executable name
	int ErrCode=0 ///< error code to return
)
{
	ostream & os = ErrCode ? cerr : cout;
	os<<"Human face/profile/landmarks detection/annotation utility. 2011-2017 (C) FaceMatch@NIH.gov\n"
"Synopsis:\n"
"  "<<getFileName(ProgName, false)<<" [options]\n"
"where options are\n"
"  -d:{min|max} diameter -- {min|max} face diameter (in pixels) to detect; default min=16 pixels\n"
"  -eq:{on|off} -- turn histogram equalization on/off before face detection; default is on\n"
"  -eval[:{good|bad|FN|FP}] -- evaluate face detection using ground truth face regions from the input list\n"
"    if output list is given, use it as pre-computed output; else detect face regions\n"
"    log {all|good|bad|FN|FP} (mis)matches; default is all\n"
"  -fc:{min|max} cnt -- {min|max} face count in the output: filter entries with insufficient face #s\n"
"  -fd:{off|on|sel|new|int|sub[:{[LM[C]]|SC}]|skip:{f|p}|skinClrFP} -- detection options, default: detect\n"
"    off|on|sel - disable|enable|selective detection; new - clear given regions; int - intersect with given\n"
"    sub - enable sub-regions (landmarks) detection; sub: - discard large faces with no landmarks\n"
"    sub:LM[C] - assume sub: and look for landmarks in skin blobs [using color]\n"
"    sub:SC - correct legacy landmark scaling issues\n"
"    skip:{f|p} - skip {face|profile} detection\n"
"    skinClrFP - clear false positives using skin\n"
"  -GPU -- prefer GPU over CPU for face detection\n"
"  -lst:{in|out} FN -- optional [input|output] text/list file name\n"
"    if omitted, input is taken from STDIN, output is written to STDOUT\n"
"    FN.fmt parse input in a specific format, e.g. PittPatt, ColorFERET\n"
"  -m MetaDataFN -- name of the meta-data file, containing tab-delimited image-to-text mapping\n"
"  -ma MinAspect -- face/profile minimum aspect ratio in [0,1]; default is 0.5\n"
"  -p imgRepoPath -- base path to the folder with images, e.g. /my/images/,\n"
"    which will be prepended to each file in the list; default is ./\n"
"  -r -- enable 90-degree rotation variations of the source images;\n"
"  -R -- enable 30-degree rotation variations of the source images\n"
"  -s:{in|out} SkinColorParmFN -- skin tone parameters in/out file name typically in YML/XML format\n"
"  -skin[:{Stat|Hist|ANN|Pack|other}] -- {statistical|histogram|ArtificialNeuralNet|PackWeighted|other} skin tone mapper;\n"
"    default is Lab histogram\n"
"  -size D -- resize input image to diameter D; default is 512; 0 means no scaling\n"
"  -save:{scaled|skinmap} -- save scaled/skinmap images (and lists) in RepoPath/{scaled|skinmap}/\n"
"  -t:{sm|sl|os|sw} val -- tolerance for {skin mass|skin likelihood|overlap slack|substitution weight} match in [0,1]\n"
"    defaults are {0.3|0.5|0.5|0.0}\n"
"  -test[:Specific] -- run unit test(s)\n"
"  -top N - process first N input images\n"
"  -u[:{last|union|int}] -- unify output by taking {last|union|intersection} of the regions for an image ID\n"
"    when omitted, no unification takes place; -u defaults to -u:last\n"
"  -v -- enable visual interface showing detected faces (circles) and profiles (rectangles)\n"
"  -v[=L] -- set verbosity level L for logging and output; causes accuracy headers to appear\n"
"  -vs -- show visual interface only for images with no regions given/found\n"
"  -V -- enable verbose visual interface showing additional windows\n"
"  -x XMLModelPath -- path to face locator models directory; default is ./FFModels/\n"
"  -x:{f|p} ModelFN -- face|profile detection model file names;\n"
"    defaults are haarcascade_{frontalface_alt2|profileface}.xml[.gz]\n"
"    mouse actions:\n"
"      left-click within region makes it prime, right-click within (sub-)region removes it\n"
"      left-drag adds a face (ellipse), right-drag adds a profile region [rectangle]\n"
"    keyboard actions:\n"
"      space - next image; outputting annotations for the current one\n"
"      BS, [p]revious image; one will have to re-do subsequent images\n"
"      DEL, [d]elete, skip the image, do not include it in the output\n"
"      [o]utput image with regions to 'FF.SrcFileName' in the current directory\n"
"      [c]lear, [f]ind face/profile regions and sub-regions\n"
"      [r]egion of interest, [i] eye, [n]ose, [m]outh, [e]ar sub-region input modes\n"
"      ESC|[q]uit program\n"
"  imgFN -- optional input image file name for a single photo face detection\n"
"Input: image file names with optional face/profile regions and other attributes.\n"
"When face regions are provided, detection is off; to force it, use -fd:new.\n"
"Output: image file names with face/profile regions (and their attributes) listed in a priority order as\n"
"  f[x,y;w,h] for a face, or p[x,y;w,h] for a profile.\n"
"Face regions may optionally contain facial features sub-regions, e.g. for eyes, nose, mouth:\n"
"  f{[x,y;w,h]\ti[x,y;w,h]\ti[x,y;w,h]\tn[x,y;w,h]\tm[x,y;w,h]}"
	<<endl;
	return ErrCode;
}

string
	XMLModelPath = "FFModels/",
	FaceModelFN = "haarcascade_frontalface_alt2.xml",
	ProfileModelFN = "haarcascade_profileface.xml",
	ImageBasePath,
	SkinColorMapperKind, SkinColorParmFN, SkinColorParmOutFN,
	MetaDataFN,
	inLstFN, outLstFN,
	imgFN,
	SpecificTest;
unsigned
	flags = FaceFinder::ProcFlagsDefault,
	FaceDiameterMin=cRegDimMin, FaceDiameterMax=cRegDimMax,
	FaceCountMin=0, FaceCountMax=unsigned(-1),
	TopNLines=0,
	VerbLevel=0;
REALNUM
	SkinMassT=0.25, SkinLikelihoodT=0.5,
	OverlapSlackT=cOverlapSlackTDefault, RectSubWeight=0.5,
	MinAspect=0.5;
ERunMode RunMode=rmExec;
EDedupMode DedupMode=ddmNone;
EEvalKind eval=evalNone;
bool prefGPU=false;

/// Evaluate face finder accuracy.
void evaluate(EEvalKind eval, FaceFinder::EvalStats & AccEvalStats, FaceRegionDetector & FRD, const string & FNLine, const MapImgID2Attributes & FFOut)
{
	FaceFinder inFF(FRD, ImageBasePath, FNLine, flags&FaceFinder::detectOff); // treating as GT
	const string & ImgID=getImgIDFromFN(FNLine);
	FaceRegions outFaceRegs(FaceDiameterMin);
	MapImgID2Attributes::const_iterator it=FFOut.find(ImgID);
	if (it==FFOut.end()) // no such ImgID in pre-computed output
	{
		FaceFinder outFF(FRD, ImageBasePath, ImgID, flags);
		outFaceRegs=outFF.getFaces();
	}
	else // ImgID is in pre-computed output
	{
		const string & Regs=it->second;
		if (!Regs.empty())
		{
			FaceFinder outFF(FRD, ImageBasePath, ImgID + "\t" + Regs, flags&FaceFinder::detectOff); // just load the regions
			outFaceRegs=outFF.getFaces();
		}
	}
	FaceFinder::EvalStats es(inFF.getFaces(), outFaceRegs, OverlapSlackT, RectSubWeight, flags&FaceFinder::cascade);
	switch (eval)
	{
	case evalBad: // list bad matches to clog, and their GT to cout
		if (es.FScore()<1)
		{
			cout<<FNLine<<endl; // GT
			clog<<ImgID<<outFaceRegs<<endl; // FF
		}
		break;
	case evalFN: // list false negatives to clog, and their GT to cout
		if (es.Recall()<1)
		{
			cout<<FNLine<<endl; // GT
			clog<<ImgID<<outFaceRegs<<endl; // FF
		}
		break;
	case evalFP: // list false positives to clog, and their GT to cout
		if (es.Precision()<1)
		{
			cout<<FNLine<<endl; // GT
			clog<<ImgID<<outFaceRegs<<endl; // FF
		}
		break;
	case evalGood:
		if (es.FScore()==1) clog<<inFF<<endl;
		break;
	case evalAll:
		if (VerbLevel)
			clog<<FNLine<<endl
			<<"FOUND:"<<outFaceRegs<<endl
			<<"EVAL:"<<'\t'<<es<<endl;
		else
			clog<<ImgID<<outFaceRegs<<endl; // FF
		break;
	}
	AccEvalStats += es;
}

/**
* For each input image, locate likely face/profile regions and output them in a priority order, e.g. most central first.
* Support visual verification/editing of face/profile regions, allowing some attribute editing, e.g. gender, age, etc.
* Provide a face/profile detection accuracy evaluation mode.
* \param ac number of input arguments including program name
* \param av input arguments
* \see help()
*/
int main(int ac, char * av[])
{
	if (ac==1)
	{
		help(*av, 0);
		clog<<"Warning: no arguments given, input file list is taken from STDIN one line at a time."<<endl;
	}

	try
	{
		for (int a=1; a<ac; ++a) // parse the input arguments
		{
			if (!strcmp("-h", av[a]) || !strcmp("-?", av[a])) return help(*av, 0);
			else if (!strcmp("-p", av[a])) ImageBasePath=av[++a];
			else if (!strcmp("-m", av[a])) MetaDataFN=av[++a];
			else if (!strcmp("-v", av[a])) { flags |= FaceFinder::visual; VerbLevel=1; }
			else if (!strcmp("-vs", av[a])) { flags |= FaceFinder::visual|FaceFinder::selective; VerbLevel=1; }
			else if (av[a] == strstr(av[a], "-v="))
			{
				flags |= FaceFinder::verbose;
				const unsigned len=strlen("-v=");
				VerbLevel=atoi(av[a]+len);
			}
			else if (!strcmp("-V", av[a])) { flags |= FaceFinder::visual|FaceFinder::verbose; VerbLevel=2; }
			else if (!strcmp("-fc:min", av[a])) FaceCountMin=atoi(av[++a]);
			else if (!strcmp("-fc:max", av[a])) FaceCountMax=atoi(av[++a]);
			else if (checkPrefix(av[a], "-eq") || checkPrefix(av[a], "-fd") || checkPrefix(av[a], "-save") || checkPrefix(toLC(av[a]), "-r"))
				FaceFinder::updateFlags(flags, av[a]);
			else if (!strcmp("-GPU", av[a])) prefGPU=true;
			else if (!strcmp("-d:min", av[a])) FaceDiameterMin=atoi(av[++a]);
			else if (!strcmp("-d:max", av[a])) FaceDiameterMax=atoi(av[++a]);
			else if (!strcmp("-lst:in", av[a])) inLstFN=av[++a];
			else if (!strcmp("-lst:out", av[a])) outLstFN=av[++a];
			else if (!strcmp("-ma", av[a])) MinAspect=atof(av[++a]);
			else if (!strcmp("-s:in", av[a])) SkinColorParmFN=av[++a];
			else if (!strcmp("-s:out", av[a])) { SkinColorParmOutFN=av[++a]; flags |= FaceFinder::sampling; }
			else if (!strcmp("-skin", av[a]))
			{
				SkinColorMapperKind="Hist";
				SkinColorParmFN="Lab.all.xyz";
			}
			else if (!strcmp("-size", av[a])) FaceFinder::setImgMaxDim(atoi(av[++a]));
			else if (strstr(av[a], "-skin:")==av[a]) SkinColorMapperKind=av[a]+strlen("-skin:");
			else if (!strcmp("-t:sm", av[a])) SkinMassT=atof(av[++a]);
			else if (!strcmp("-t:sl", av[a])) SkinLikelihoodT=atof(av[++a]);
			else if (!strcmp("-t:os", av[a])) OverlapSlackT=atof(av[++a]);
			else if (!strcmp("-t:sw", av[a])) RectSubWeight=atof(av[++a]);
			else if (!strcmp("-top", av[a])) TopNLines=atoi(av[++a]);
			else if (!strcmp("-x", av[a])) XMLModelPath=av[++a];
			else if (!strcmp("-x:f", av[a])) FaceModelFN=av[++a];
			else if (!strcmp("-x:p", av[a])) ProfileModelFN=av[++a];
			else if (!strcmp("-u", av[a])) DedupMode=ddmLast;
			else if (!strcmp("-u:last", av[a])) DedupMode=ddmLast;
			else if (!strcmp("-u:union", av[a])) DedupMode=ddmUnion;
			else if (!strcmp("-u:int", av[a])) DedupMode=ddmIntersect;
			else if (!strcmp("-u:intersect", av[a])) DedupMode=ddmIntersect;
			else if (!strcmp("-test", av[a])) RunMode=rmTest;
			else if (av[a] == strstr(av[a], "-test:"))
			{
				RunMode = rmTest;
				const unsigned len=strlen("-test:");
				SpecificTest=av[a]+len;
			}
			else if (!strcmp("-eval", av[a])) eval=evalAll;
			else if (!strcmp("-eval:good", av[a])) eval=evalGood;
			else if (!strcmp("-eval:bad", av[a])) eval=evalBad;
			else if (!strcmp("-eval:FN", av[a])) eval=evalFN;
			else if (!strcmp("-eval:FP", av[a])) eval=evalFP;
			else if ('-'==av[a][0]) throw FaceMatch::Exception(format("invalid option %s", av[a]));
			else imgFN = av[a];
		}

		//--- diagnostics
		if (flags&FaceFinder::visual) setVisVerbLevel(VerbLevel);
		else setVerbLevel(VerbLevel);
		setRepoPath(ImageBasePath);
		preferGPU(prefGPU);

		FaceRegionDetector FRD(XMLModelPath, FaceModelFN, ProfileModelFN,
			SkinColorMapperKind, SkinColorParmFN,
			FaceDiameterMin, FaceDiameterMax,
			SkinMassT, SkinLikelihoodT,
			MinAspect, prefGPU);

		if (RunMode==rmTest)
		{
			TestFaceFinder test
			(
				FRD, flags,
				ImageBasePath.empty() ? TestRepoPath : ImageBasePath,
				inLstFN.empty() ? TestImgLstFN : inLstFN,
				VerbLevel
			);
			return test.run(SpecificTest);
		}


		//--- process a batch of images
		ImageMetaData md(MetaDataFN);
		FaceFinder::EvalStats AccEvalStats;

		bool bOutExists = FileExists(outLstFN);
		MapImgID2Attributes FFOut(eval ? (bOutExists ? outLstFN : nemo) : nemo);
		if (eval==evalAll && VerbLevel) clog<<"FileName"<<"\t"+AccEvalStats.Titles<<endl;

		OutputStream out(eval&&bOutExists ? nemo : outLstFN);
		if (!imgFN.empty()) // single file case
		{
			FaceFinder FF(FRD, "", imgFN, flags);
			out<<FF<<endl;
			return 0;
		}

		typedef list<string> Lines;
		Lines lines;
		for (ListReader lr(inLstFN); !lr.end(); )
		{
			const string & line = lr.fetch();
			if (lr.end() && line.empty()) break; // ignore the extra line break at the end
			lines.push_back(line);
		}

		HistogramU2U RgnCntHist, RgnDiamHist;
		MatColorSample SkinColorSamples;
		MapImgID2FaceRegions DedupOutputMap(DedupMode);
		TimeLog(ProcessInputList);
		unsigned LineCnt=0;
		string ext = getFileExt(inLstFN),
			fmt = (ext==".lst" ? nemo : ext+":");
		for (Lines::const_iterator it=lines.begin(); it!=lines.end(); )
		{
			const string & FNLine = *it;
			try
			{
				if (FNLine.empty() || FNLine[0]=='#') out<<FNLine<<endl; // preserve the original formatting
				else if (TopNLines && ++LineCnt>TopNLines) break;
				else if (eval) evaluate(eval, AccEvalStats, FRD, FNLine, FFOut);
				else // non-eval case
				{
					md.FindAndLog(FNLine); // clog<<FNLine<<metainfo<<endl;
					FaceFinder inFF(FRD, ImageBasePath, fmt+FNLine, flags);
					const FaceRegions & FRgs = inFF.getFaces();
					const unsigned FRgsCnt=FRgs.size();
					inFF.setMaxFaceCount(FaceCountMax);
					if (FRgsCnt>=FaceCountMin)
					{
						if (DedupMode)
						{
							DedupOutputMap.merge(inFF.getImageFN(), FRgs);
							cout<<inFF<<endl;
						}
						else out<<inFF<<endl;

						RgnCntHist.inc(FRgsCnt); // update regions stats
						for (FaceRegions::const_iterator rit=FRgs.begin(); rit!=FRgs.end(); ++rit)
							if (isFRRectKind((*rit)->Kind))
								RgnDiamHist.inc(dynamic_cast<const FaceRegion&>(**rit).diameter());
					}
					SkinColorSamples.push_back(inFF.getSkinColorSamples());
				}
				++it;
			}
			catch(const exception & e)
			{
				cerr<<FNLine<<"\tERROR: "<<e.what()<<endl; out.flush();
				++it;
			}
			catch(const SpecialChar & c)
			{
				const unsigned PageLen = 8; // TODO: config/param
				if (VerbLevel>1) clog<<"SpecialChar=0x"<<hex<<(unsigned)c<<dec<<endl;
				switch(c)
				{
				case ESC: it=lines.end(); break; // terminate
				case DEL: ++it; break; // ignore the image
				case BkSp: if (it!=lines.begin()) --it; break;
				case PgUp: for (unsigned k=0; k<PageLen && it!=lines.begin(); ++k) --it; break;
				case PgDn: for (unsigned k=0; k<PageLen && it!=lines.end(); ++k) ++it; break;
				case HOME: it=lines.begin(); break;
				case END: it=lines.end(); if (it!=lines.begin()) --it; break;
				}
			}
			catch(...)
			{
				cerr<<"unknown exception"<<endl;
				++it;
			}
		}
		//--- output statistics
		if (DedupMode) out<<DedupOutputMap;

		if (eval)
		{
			if (VerbLevel) cout<<"FileName"<<"\t"+AccEvalStats.Titles<<endl;
			string FN = outLstFN.empty() ? inLstFN : outLstFN;
			cout<<FN<<"\t"<<AccEvalStats<<endl;
		}
		else
		{
			clog<<"=== region count histogram:"<<RgnCntHist<<endl;
			clog<<"=== region diameter histogram:"<<RgnDiamHist<<endl;
		}
		if (!SkinColorParmOutFN.empty())
		{
			if (SkinColorSamples.rows) FRD.SkinToneMapper.init(SkinColorSamples);
			FRD.SkinToneMapper.save(SkinColorParmOutFN);
		}
	}
	catch(const exception & e)
	{
		cerr<<"exception: "<<e.what()<<endl;
		return errException;
	}
	catch(const SpecialChar c)
	{
		if (c==ESC) clog<<"escaped"<<endl;
		else cerr<<"unkonwn SpecialChar: "<<c<<" 0x"<<hex<<unsigned(c)<<endl;
	}
	catch(...)
	{
		cerr<<"unknown exception"<<endl;
		return errParameter;
	}
	return errNone;
}
