
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
#include "ImageCollage.h"
#include "TestImageMatch.h"

using namespace FaceMatch;

enum Error
{
	errNone,
	errUnknownOption,
	errUnknownArgument,
	errException,
	errUnknown
};
enum EInputMode
{
	modeQuery,
	modeIngest,
	modeRemove,
	modeList
};

template<class T>
static T & singletonIM(const string & NdxFN, unsigned ImgNormDim, unsigned flags)
{
	StaticLkdCtor T matcher(NdxFN, ImgNormDim, flags);
	return matcher;
}
static ImageMatcher & getImageMatcher(const string & NdxFN, unsigned ImgNormDim, unsigned flags, const string & imMode)
{
	if (imMode=="HAAR") return singletonIM< ImageMatcherWhole<SigIndexHaarWhole> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="SIFT") return singletonIM< ImageMatcherWhole<SigIndexSIFT> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="SURF") return singletonIM< ImageMatcherWhole<SigIndexSURF> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="ORB") return singletonIM< ImageMatcherWhole<SigIndexORB> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="LBPH") return singletonIM< ImageMatcherWhole<SigIndexLBPH> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="RSILC") return singletonIM< ImageMatcherWhole<SigIndexRSILC> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="RSILCS") return singletonIM< ImageMatcherWhole<SigIndexRSILCS> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="MANY") return singletonIM< ImageMatcherWhole<SigIndexMany> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="DIST") return singletonIM< ImageMatcherWhole<SigIndexManyDist> >(NdxFN, ImgNormDim, flags);
	else if (imMode=="RANK") return singletonIM< ImageMatcherWhole<SigIndexManyRank> >(NdxFN, ImgNormDim, flags);
	else throw FaceMatch::Exception("unknown match mode "+imMode);
}
template<class T>
static T & singletonFM(const string & NdxFN, FaceRegionDetector & FRD, unsigned FaceFinderFlags, unsigned ImgNormDim, unsigned flags)
{
	StaticLkdCtor T matcher(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	return matcher;
}
static FaceRegionDetector & getFRD(const string & XMLBasePath = "FFModels/",
	const string & FaceModelFN = "haarcascade_frontalface_alt2.xml",
	const string & ProfileModelFN = "haarcascade_profileface.xml",
	const string & SkinColorMapperKind = "",
	const string & SkinColorParmFN = "",
	unsigned aFaceDiameterMin=cRegDimMin,
	unsigned aFaceDiameterMax=cRegDimMax,
	REALNUM SkinMassT=0.25,
	REALNUM SkinLikelihoodT=0.5,
	REALNUM aFaceAspectLimit=0.5,
	bool preferGPU=false)
{
	StaticLkdCtor FaceRegionDetector FRD(XMLBasePath, FaceModelFN, ProfileModelFN,
		SkinColorMapperKind, SkinColorParmFN,
		aFaceDiameterMin, aFaceDiameterMax,
		SkinMassT, SkinLikelihoodT, aFaceAspectLimit, preferGPU);
	return FRD;
}
static ImageMatcher & getFaceMatcher(const string & NdxFN, const string & fmMode, unsigned ImgNormDim,
	unsigned FaceFinderFlags,
	const string & XMLBasePath = "FFModels/",
	const string & FaceModelFN = "haarcascade_frontalface_alt2.xml",
	const string & ProfileModelFN = "haarcascade_profileface.xml",
	const string & SkinColorMapperKind = "",
	const string & SkinColorParmFN = "",
	unsigned aFaceDiameterMin=cRegDimMin,
	unsigned aFaceDiameterMax=cRegDimMax,
	REALNUM SkinMassT=0.25,
	REALNUM SkinLikelihoodT=0.5,
	REALNUM aFaceAspectLimit=0.5,
	unsigned flags=0,
	bool preferGPU=false)
{
	static FaceRegionDetector & FRD = getFRD(XMLBasePath, FaceModelFN, ProfileModelFN,
		SkinColorMapperKind, SkinColorParmFN,
		aFaceDiameterMin, aFaceDiameterMax,
		SkinMassT, SkinLikelihoodT, aFaceAspectLimit, preferGPU);
	if (fmMode=="HAAR") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexHaarFace> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="SIFT") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexSIFT> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="SIFT.FL") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexSIFTFL> >(NdxFN, FRD, FaceFinderFlags|FaceFinder::cascade, ImgNormDim, flags);
	else if (fmMode=="SURF") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexSURF> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="SURF.FL") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexSURFFL> >(NdxFN, FRD, FaceFinderFlags|FaceFinder::cascade, ImgNormDim, flags);
	else if (fmMode=="ORB") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexORB> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="ORB.FL") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexORBFL> >(NdxFN, FRD, FaceFinderFlags|FaceFinder::cascade, ImgNormDim, flags);
	else if (fmMode=="LBPH") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexLBPH> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="RSILC") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexRSILC> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="RSILCS") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexRSILCS> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="MANY") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexMany> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="DIST") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexManyDist> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else if (fmMode=="RANK") return singletonFM< ImageMatcherFaceRegionsBase<SigIndexManyRank> >(NdxFN, FRD, FaceFinderFlags, ImgNormDim, flags);
	else throw FaceMatch::Exception("unknown match mode "+fmMode);
}

/// \return error code after printing help; when 0, print CLI help to STDOUT, otherwise to STDERR
int help(const string & ProgName, ///< program executable name
	int ErrCode=0, ///< error code to return
	const string & ErrMsg="" ///< error message to print
)
{
	ostream & os = ErrCode ? cerr : cout;
	if (!ErrMsg.empty()) os<<"Error: "<<ErrMsg<<endl;
	os<<"CLI to NLM/CEB image/face matching library. 2011-2017 (C) FaceMatch@NIH.gov\n\
Synopsis:\n\
  "<<getFileName(ProgName, false)<<" [options]\n\
where options are\n\
  -? or -h -- display help\n\
  -c -- use camera to capture images for query or ingest\n\
  -dim D -- face/image dimension to normalize to; 0=unnormalized; default is 128 for faces, 256 for images\n\
  -dm:{FG|BF|FLANN|OM} -- {FaceGroup|BruteForce|FLANN|OptimalMatch} descriptor matching method;\n\
    default is optimal for each descriptor\n\
  -eq:{on|off} -- turn histogram equalization on/off before face detection/matching; default is on\n\
  -fd:{off|on|sel|new|int|sub[:{[LM[C]]|SC}]|skip:{f|p}} -- detection options (see FaceFinder -h) for details\n\
    default is selective detection with 90-degree rotation phases\n\
  -{f|i}m[:{HAAR|SIFT|SURF|ORB|LBPH|MANY|DIST|RANK}[.FL]] -- face/image matching mode [Face Landmarks driven]\n\
    default is DIST\n\
  -g -- order query results by group/label/ID\n\
  -GPU -- if available, prefer GPU over CPU\n\
  -ld dlm -- label/id delimiter; default is '/'\n\
  -lst ListFN -- an image list file name to work with (instead of STDIN)\n\
  -m:{q|i|r|l} -- query|ingest|remove|list image descriptors to/from/in the index; default is query\n\
  -ndx[:{in|out}] FN -- index {input|output} file name with possible extensions\n\
    .xml for XML index storage, can be .xml.gz for file compactness\n\
    .yml or .yaml for YAML index storage, can be .yml.gz for file compactness\n\
    .txt for plain text index dump\n\
    .mdx per-face/image descriptor multi-file index\n\
    .ndx (or any other) for binary\n\
  -o[:{p|s|t}] -- optimize {parameters|scale|threshold} using the current index\n\
  -p ImgRepoPath -- path to images directory, e.g. /my/images/; default is ./\n\
  -qrf {XML|YAML|YML} -- query results format\n\
  -r -- use image pi/2 phase rotations for query\n\
  -s -- exclude self match in queries\n\
  -sal:{LM|ML} -- generate saliency maps for input images using LandMarks and/or Machine Learing\n\
  -skin[:{ANN|Stat|Hist} SkinFN] -- use a skin mapper initialized by optional config file; default is ANN\n\
  -t T -- distance threshold in [0,1] or top-T rank: T=1,5,10...; T<0 gets everything;\n\
    mixed T=n.dddd gets top-n records within 0.dddd distance; default is t=0.5\n\
  -t:{sm|sl} val -- tolerance for {skin mass|skin likelihood} match in [0,1]; defaults are {0.25|0.5}\n\
  -v[=level] -- use verbose mode and visual feedback for query with an optional level\n\
  -var[:{c|r|s}] -- ingest with variations, e.g. crop, rotation, scale; -var causes all variations\n\
  -{x|e}val[:{AntiDist|AntiRank|InvRank}] -- evaluate retrieval accuracy using optional score weighting\n\
    AntiDist=1-dist, AntiRank=1-rank/N, InvRank=1/rank; default is non-weighted; xval excludes self-match\n\
  -test[:SpecificTest] -- run the unit test(s)\n\
  -x ModelPath -- face/profile detection models path\n\
  -x:{f|p} ModelPathFN -- [face|profile] detection model file names\n\
Most command-line options can be set in the interactive mode. In addition, use\n\
  ID <label> -- set ID/label prefix for images/faces captured from a webcam.\n\
  [-]c -- to turn on a webcam; [enter] to ingest new images/faces; [esc] to stop webcam.\n\
Similar images IDs/names are output along with the distance to the input image.\n\
If no list file specified, input is taken from STDIN (enter {x|q} to exit/quit). Output is written to STDOUT.\n\
Error, warning and log messages are written to STDERR."
	<<endl;
	return ErrCode;
}

/// run image matcher CLI: given a list of image files, find all similar ones. For each image, print its best matches with distance measures in [0,1], if any. For more details \see help()
int main(int ac, ///< input argument count
	char * av[] ///< vector of input arguments
)
{
	enum EAppFlags
	{
		oNone,
		oPreferGPU=1,
		oFaceMatch=1<<1
	};
	unsigned AppFlags=oNone;
	string
		MatchMode = "HAAR",
		XMLModelPath = "./FFModels/",
		FaceModelFN = "haarcascade_frontalface_alt2.xml",
		ProfileModelFN = "haarcascade_profileface.xml",
		LabelDelimiter,
		QueryOutputFormat,
		SkinColorMapperKind, SkinColorParmFN,
		NdxInFN, NdxOutFN,
		RepoPath, ImgLstFN,
		SpecificTest,
		LstOutFN;
	unsigned
		RunMode = rmExec,
		InputMode = modeQuery,
		ImgVar = ivNone,
		FaceFinderFlags=FaceFinder::selective|FaceFinder::rotation|FaceFinder::HistEQ,
		FaceDiameterMin=cRegDimMin, FaceDiameterMax=cRegDimMax,
		ImgNormDim=DefaultWholeImgDim, // DefaultFacePatchDim
		VerbLevel=0,
		WeighedEval=weNonWeighed,
		ImageMatcherFlags=moHistEQ,
		QueryOptions=0,
		OptimizationFlags=0;
	REALNUM
		MatchT=0.5,
		SkinMassT=0.25,
		SkinLikelihoodT=0.5,
		MinAspect=0.5;

	try
	{
		TimeLog(Total);
		if (ac==1) help(*av, 0);
		for (int a=1; a<ac; ++a)
		{
			if (!strcmp("-h", av[a]) || !strcmp("-?", av[a])) return help(*av, 0);
			else if (!strcmp("-c", av[a])) { ImgLstFN="CAM"; }
			else if (!strcmp("-dim", av[a])) ImgNormDim=atoi(av[++a]);
			else if (!strcmp("-dm:FG", av[a])) ImageMatcherFlags&=~(dm|dmOM);
			else if (!strcmp("-dm:BF", av[a])) ImageMatcherFlags|=dmBF;
			else if (!strcmp("-dm:FLANN", av[a])) ImageMatcherFlags|=dmFLANN;
			else if (!strcmp("-dm:OM", av[a])) ImageMatcherFlags|=dmOM;
			else if (!strcmp("-eq:on", av[a])) ImageMatcherFlags|=moHistEQ;
			else if (!strcmp("-eq:off", av[a])) ImageMatcherFlags &= ~unsigned(moHistEQ);
			else if (checkPrefix(av[a], "-fd")) FaceFinder::updateFlags(FaceFinderFlags, av[a]);
			else if (strstr(av[a], "-fm")==av[a])
			{
				AppFlags|=oFaceMatch;
				if (ImgNormDim==DefaultWholeImgDim) ImgNormDim=DefaultFacePatchDim;
				MatchMode = (strstr(av[a], "-fm:")==av[a]) ? (av[a]+strlen("-fm:")) : "DIST";
			}
			else if (strstr(av[a], "-im")==av[a])
			{
				AppFlags&=~unsigned(oFaceMatch);
				if (ImgNormDim==DefaultFacePatchDim) ImgNormDim=DefaultWholeImgDim;
				MatchMode = (strstr(av[a], "-im:")==av[a]) ? (av[a]+strlen("-im:")) : "HAAR";
			}
			else if (!strcmp("-g", av[a])) ImageMatcherFlags|=moGroupLabels;
			else if (!strcmp("-GPU", av[a])) AppFlags|=oPreferGPU;
			else if (!strcmp("-ld", av[a])) LabelDelimiter = av[++a];
			else if (!strcmp("-lst", av[a])) ImgLstFN = av[++a];
			else if (!strcmp("-lst:out", av[a])) LstOutFN = av[++a];
			else if (!strcmp("-m:q", av[a])) InputMode=modeQuery;
			else if (!strcmp("-m:i", av[a])) InputMode=modeIngest;
			else if (!strcmp("-m:r", av[a])) InputMode=modeRemove;
			else if (!strcmp("-m:l", av[a])) InputMode=modeList;
			else if (!strcmp("-ndx", av[a])) NdxInFN=NdxOutFN=av[++a];
			else if (!strcmp("-ndx:in", av[a])) NdxInFN=av[++a];
			else if (!strcmp("-ndx:out", av[a])) NdxOutFN=av[++a];
			else if (!strcmp("-o", av[a]))
			{
				RunMode = rmOptm;
				OptimizationFlags = optParams|optScale|optThreshold;
			}
			else if (!strcmp("-o:p", av[a]))
			{
				RunMode = rmOptm;
				OptimizationFlags = optParams;
			}
			else if (!strcmp("-o:s", av[a]))
			{
				RunMode = rmOptm;
				OptimizationFlags = optScale;
			}
			else if (!strcmp("-o:t", av[a]))
			{
				RunMode = rmOptm;
				OptimizationFlags = optThreshold;
			}
			else if (!strcmp("-p", av[a])) RepoPath=av[++a];
			else if (!strcmp("-r", av[a])) QueryOptions|=qoRotationPhases;
			else if (!strcmp("-qrf", av[a])) QueryOutputFormat = av[++a];
			else if (!strcmp("-s", av[a])) QueryOptions|=qoSkipSelf;
			else if (!strcmp("-t", av[a])) MatchT=atof(av[++a]);
			else if (!strcmp("-t:sm", av[a])) SkinMassT=atof(av[++a]);
			else if (!strcmp("-t:sl", av[a])) SkinLikelihoodT=atof(av[++a]);
			else if (!strcmp("-v", av[a]))
			{
				FaceFinderFlags |= FaceFinder::visual;
				VerbLevel=1;
			}
			else if (av[a] == strstr(av[a], "-v="))
			{
				const unsigned len=strlen("-v=");
				VerbLevel=atoi(av[a]+len);
			}
			else if (!strcmp("-x", av[a])) XMLModelPath=av[++a];
			else if (!strcmp("-x:f", av[a])) FaceModelFN=av[++a];
			else if (!strcmp("-x:p", av[a])) ProfileModelFN=av[++a];
			else if (!strcmp("-sal:LM", av[a])) ImageMatcherFlags|=smmLandMarks;
			else if (!strcmp("-sal:ML", av[a])) ImageMatcherFlags|=smmMachineLearning;
			else if (!strcmp("-skin", av[a]))
			{
				SkinColorMapperKind="Hist";
				SkinColorParmFN="Lab.all.xyz";
			}
			else if (strstr(av[a], "-skin:")==av[a])
			{
				SkinColorMapperKind=av[a]+strlen("-skin:");
				SkinColorParmFN=av[++a];
			}
			else if (!strcmp("-var", av[a])) ImgVar=ivAll;
			else if (av[a] == strstr(av[a], "-var:"))
			{
				const unsigned len=strlen("-var:");
				if (strchr(av[a]+len, 'c')) ImgVar |= ivCrop;
				if (strchr(av[a]+len, 'r')) ImgVar |= ivRotate;
				if (strchr(av[a]+len, 's')) ImgVar |= ivScale;
			}
			else if (!strcmp("-test", av[a])) RunMode = rmTest;
			else if (av[a] == strstr(av[a], "-test:"))
			{
				RunMode = rmTest;
				const unsigned len=strlen("-test:");
				SpecificTest=av[a]+len;
			}
			else if (av[a] == strstr(av[a], "-eval") || av[a] == strstr(av[a], "-xval"))
			{
				const unsigned len=strlen("-eval");
				if (!strcmp(av[a]+len, ":AntiDist")) WeighedEval=weAntiDist;
				else if (!strcmp(av[a]+len, ":AntiRank")) WeighedEval=weAntiRank;
				else if (!strcmp(av[a]+len, ":InvRank")) WeighedEval=weInvRank;
				RunMode = rmEval;
				if (av[a] == strstr(av[a], "-xval")) WeighedEval|=weSelfExclude;
			}
			else if (av[a][0] == '-') throw FaceMatch::Exception("unknown command line option "+string(av[a]));
			else throw FaceMatch::Exception("unknown command line argument "+string(av[a]));
		} // arg processing

		if (ImageMatcherFlags&smmAll) FaceFinderFlags |= (FaceFinder::keepCascaded|FaceFinder::seekLandmarks); // TODO: |FaceFinder::seekLandmarksColor);

		if (ImageMatcherFlags&moHistEQ)
		{
			FaceFinderFlags|=FaceFinder::HistEQ;
			setHistEQ(true); // diagnostics
		}
		else
		{
			FaceFinderFlags &= ~unsigned(FaceFinder::HistEQ);
			setHistEQ(false); // diagnostics
		}

		//--- diagnostics
		if (FaceFinderFlags&FaceFinder::visual) setVisVerbLevel(VerbLevel);
		else setVerbLevel(VerbLevel);
		setRepoPath(RepoPath);
		preferGPU(AppFlags&oPreferGPU);

		ImageMatcher & IM = (AppFlags&oFaceMatch) ?
			(ImageMatcher&)getFaceMatcher(NdxInFN, MatchMode, ImgNormDim,
					FaceFinderFlags, XMLModelPath, FaceModelFN, ProfileModelFN,
					SkinColorMapperKind, SkinColorParmFN,
					FaceDiameterMin, FaceDiameterMax,
					SkinMassT, SkinLikelihoodT, MinAspect,
					ImageMatcherFlags, AppFlags&oPreferGPU):
			(ImageMatcher&)getImageMatcher(NdxInFN, ImgNormDim, ImageMatcherFlags, MatchMode);

		if (!LabelDelimiter.empty())
			IM.LabelDelimiter(LabelDelimiter);

		switch (RunMode)
		{
			case rmTest:
			{
				TestImageMatch test
				(
					RepoPath.empty() ? TestRepoPath : RepoPath,
					ImgLstFN.empty() ? TestImgLstFN : ImgLstFN,
					VerbLevel, AppFlags&oPreferGPU
				);
				return test.run(SpecificTest);
			}
			case rmEval:
			{
				REALNUM A = IM.eval(RepoPath, ImgLstFN, MatchT, ImgVar, WeighedEval);
				cout<<A<<endl;
				if (!NdxOutFN.empty()) IM.save(NdxOutFN);
				return 0;
			}
			case rmOptm:
			{
				WeighedEval|=weSelfExclude;
				REALNUM F=0;
				if (VerbLevel)
				{
					IM.eval(RepoPath, ImgLstFN, -1, ImgVar, WeighedEval);
					clog<<"optimization at MatchT="<<MatchT<<endl;
				}
				F = IM.optimize(MatchT, OptimizationFlags, RepoPath, ImgLstFN, ImgVar);
				cout<<"F("<<MatchT<<")="<<F<<endl;
				if (VerbLevel)
				{
					clog<<"after optimization F("<<MatchT<<")="<<F<<endl;
					F = IM.eval(RepoPath, "", -1, ImgVar, WeighedEval); // no need to re-ingest
				}
				if (!NdxOutFN.empty()) IM.save(NdxOutFN);
				return 0;
			}
		}
		
		class InputReader: public ListReader
		{
			VideoCapture mVC;
		public:
			InputReader(const string & IN): ListReader(IN=="CAM" ? "" : IN)
			{
				if (IN=="CAM") visual(true);
			}
			bool visual()const{ return mVC.isOpened(); }
			void visual(bool v)
			{
				if (v)
				{
					if (visual()) return;
					mVC.open(0);
				}
				else
				{
					if (!visual()) return;
					mVC.release();
					destroyAllWindows();
				}
				mLine.clear();
			}
			const string & fetch(unsigned flags, const string & FN)
			{
				if (mVC.isOpened())
				{
					Mat img; 
					PFaceRegion FR;
					while (!FR)
					{
						mVC>>img;
						try
						{
							FaceFinder FF(getFRD(), img, FaceFinder::detection|FaceFinder::visual|FaceFinder::LiveFeed); // TODO: flags
							if (FF.UserAccepted())
								FR=FF.getPrimaryFaceRegion();
						}
						catch(SpecialChar c)
						{
							if (getVerbLevel()) clog<<"SpecialChar="<<c<<endl;
							if (c==ESC)
							{
								visual(false);
								return mLine;
							}
						}
					}
					imwrite(FN, img);
					stringstream strm; strm<<FN<<'\t'<<*FR;
					mLine=strm.str();
				}
				else return ListReader::fetch();
				return mLine;
			}
		};

		TimeLog(ListProcessing);
		string request, ID, IDdelim="_", pfx;
		unsigned stamp=0;
		ofstream OL(LstOutFN);
		for (InputReader rdr(ImgLstFN); !rdr.end(); ) //=== main loop
		{
			switch (InputMode)
			{
				case modeQuery: cout<<"-m:q>"; break;
				case modeIngest: cout<<"-m:i>"; break;
				case modeRemove: cout<<"-m:r>"; break;
				case modeList: cout<<"-m:l>"; break;
			}
			try
			{
				if (InputMode==modeIngest)
				{
					stamp=time(0);
					pfx=ID+IDdelim;
				}
				else
				{
					stamp=0;
					pfx="img";
				}
				string FN=format("%s%04d.jpg", pfx.c_str(), stamp);
				request=rdr.fetch(FaceFinderFlags, FN);
				trim(request, "# \t\r\n");

				if (request.empty() || request.at(0)=='#') continue;
				else if (request=="x" || request=="-x") break;
				else if (request=="c" || request=="-c") { rdr.visual(true); continue; }
				else if (request=="h" || request=="?" || request=="-h" || request=="-?") { help(*av, 0); continue; }
				else if (request == "-m:q") { InputMode = modeQuery; continue; }
				else if (request == "-m:i") { InputMode = modeIngest; continue; }
				else if (request == "-v") { VerbLevel=1; FaceFinderFlags |= FaceFinder::visual; continue; }
				else if (request=="-ld")
				{
					cout<<"label delimiter='"<<IM.LabelDelimiter()<<"'"<<endl;
					continue;
				}
				else if (checkPrefix(request, "-ld"))
				{
					IM.LabelDelimiter(request.substr(4));
					continue;
				}
				else if (request=="-m:r") { InputMode=modeRemove; continue; }
				else if (checkPrefix(request, "-m:r"))
				{
					InputMode = modeRemove;
					request = request.substr(5);
				}
				else if (request=="-m:l") { InputMode=modeList; continue; }
				else if (checkPrefix(request, "-m:l"))
				{
					InputMode = modeList;
					request = request.substr(5);
				}
				else if (request.find("-t")==0)
				{
					stringstream StrmReq(request.c_str()+2);
					StrmReq>>MatchT;
					cout<<"FaceMatchT="<<MatchT<<endl;
					continue;
				}
				else if (checkPrefix(request, "-p"))
				{
					RepoPath=request.substr(3);
					cout<<"RepoPath="<<RepoPath<<endl;
					continue;
				}
				else if (checkPrefix(request, "ID"))
				{
					ID=request.substr(3);
					cout<<"ID="<<ID<<endl;
					continue;
				}
				else if (checkPrefix(request, "-qrf")) QueryOutputFormat = trimc(request.substr(5));


				// TODO: handle other options

				if (request.empty()) continue;

				string QueryResult, ImgPath = rdr.visual() ? request : RepoPath+request;
				unsigned cnt = 0;
				switch (InputMode)
				{
				case modeQuery:
					{
						TIMELOG("modeQuery");
						bool more=false;
						do // chain queries
						{
							CVTiming qtime;
							cnt = IM.query(QueryResult=QueryOutputFormat, ImgPath, MatchT, QueryOptions);
							if (VerbLevel) clog<<"q-time="<<qtime.diff()<<" sec"<<endl;
							cout<<request<<" results cnt="<<cnt<<endl<<QueryResult;
							if (FaceFinderFlags&FaceFinder::visual)
							try
							{
								if (VerbLevel>1)
									displayQueryMatches(RepoPath, QueryResult, MatchMode);
								else displayQueryResults(QueryResult);
								more=false;
							}
							catch(const string & s)
							{
								ImgPath=RepoPath+s;
								more=true;
							}
						}while(more);
					}
					break;
				case modeIngest:
					{
						TIMELOG("modeIngest");
						stringstream strmReq(request);
						string ImgFN; getline(strmReq, ImgFN, '\t');
						string ImgID=ImageRegionIDSplit(ImgFN, IM.LabelDelimiter()).Label;
						stringstream strmRgns;
						for (unsigned ID=0; !strmReq.eof(); ++ID)
						{
							try
							{
								FaceRegion fr(strmReq);
								if (fr.area()) strmRgns<<ImgID<<":"<<ID<<"\t"<<fr<<"\n";
							}
							catch (const FaceMatch::Exception & e)
							{
								string attr; strmReq>>attr;
								clog<<"skipping "<<attr<<endl;
							}
							strmReq>>ws; // eat the white space
						}
						string Rgns = strmRgns.str();
						if (Rgns.empty()) Rgns = ImgID;
						cnt = IM.ingest(RepoPath+ImgFN, Rgns, ImgVar);
						if (cnt && OL.is_open()) OL<<request<<endl;
						cout<<request<<" ingested "<<cnt<<" descriptors"<<endl;
					}
					break;
				case modeRemove:
					{
						TIMELOG("modeRemove");
						cnt = IM.remove(request);
						cout<<request<<" removed "<<cnt<<" descriptors"<<endl;
					}
					break;
				case modeList:
					{
						TIMELOG("modeList");
						cnt = IM.list(QueryResult, request);
						cout<<request<<" found "<<cnt<<" descriptors"<<endl<<QueryResult;
					}
					break;
				}
			}
			catch(const exception & e)
			{
				cerr<<"exception: "<<e.what()<<endl
				<<"during request: "<<request<<endl;
				if (ImgLstFN.empty()) cerr<<"enter '?' or 'h' to see help"<<endl;
			}
			catch(const SpecialChar & c)
			{
				clog<<"SpecialChar="<<c<<endl;
				if (c==ESC) break;
			}
		}
		cout<<endl; // concluding newline
		if (InputMode && !NdxOutFN.empty())
			IM.save(NdxOutFN);
	}
	catch(const exception & e)
	{
		cerr<<"exception: "<<e.what()<<endl;
		return errException;
	}
	catch(...)
	{
		cerr<<"unknown exception"<<endl;
		return errUnknown;
	}
	return 0;
}
