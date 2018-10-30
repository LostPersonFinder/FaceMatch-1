
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
#include "FaceMatcher.h"

namespace FaceMatch
{

void saveImage(const Mat & img, bool shift=false)
{
	static char base='a';
	static unsigned cnt=0;
	if (shift) { ++base; cnt=0; }
	stringstream strmFN; strmFN<<base<<setw(3)<<setfill('0')<<cnt<<".jpg";
	imwrite(strmFN.str(), img);
	++cnt;
}

FaceMatcher::FaceMatcher(const string & ImgDscType, const string & inFN, const string & RepoPath, REALNUM MatchT, REALNUM cropFaces, unsigned FaceFinderFlags, unsigned flags):
	mFlags(flags), mImgDscType(ImgDscType), mRepoPath(RepoPath), mMatchT(MatchT), mCropFaceRgns(cropFaces),
	mIndex(ImgDscType, mFlags)
{
	FTIMELOG
	const string & ext=getFileExt(inFN);
	if (ext==".yml" || ext==".ndx")
		mIndex.load(inFN);
	else if (ext==".lst")
		for (ListReader lr(inFN.c_str()); !lr.end(); )
		{
			const string & FN = lr.fetch(); if (FN.empty() || FN[0]=='#') continue;
			string FNLine = mRepoPath+FN;
			mIndex.insert(FN, computeImageDescriptor(FNLine, FaceFinderFlags));
		}
	else throw Exception("unknown input file type "+ext);
	mIndex.absorb();
}
REALNUM FaceMatcher::distance(const Image & a, const ImgDscBase & A, const Image & b, const ImgDscBase & B, unsigned flags)const
{
	TMatch matches;
	REALNUM d=A.dist(B, &matches);

	if (flags)
	{
		Mat outM;
		static const auto KeyPtDscTypes = {"SIFT", "SURF", "ORB", "RSILC"};
		if (find(KeyPtDscTypes.begin(), KeyPtDscTypes.end(), mImgDscType)==KeyPtDscTypes.end())
			drawMatches(a.mx(), KeyPoints(), b.mx(), KeyPoints(), TMatch(), outM);
		else
		{
			Mat am=a.drawFaceRegions(), bm=b.drawFaceRegions();
			if (mImgDscType=="RSILC") // key-lines
				drawMatches(am, ((const ImgDscRSILC&)A).getKeyLines(), bm, ((const ImgDscRSILC&)B).getKeyLines(), matches, outM);
			else // assume key-point based
				drawMatches(am, dynamic_cast<const ImgDscKeyPt&>(A).getKeypoints(), bm, dynamic_cast<const ImgDscKeyPt&>(B).getKeypoints(), matches, outM);
		}
		if (flags&fmShow)
		{
			stringstream strmTitle; strmTitle<<A.getType()<<": "<<d;
			imshow(strmTitle.str(), outM);
			SpecialChar k=waitKey(0);
			destroyAllWindows();
			if (k==ESC || strchr("qQ",k)) throw k;
			if (strchr("sS",k)) saveImage(outM, k=='S');
		}
		if (flags&fmSave) saveImage(outM);
	}
	return d;
}
static FaceRegionDetector & getFRD()
{
	StaticLkdCtor FaceRegionDetector sFRD("./FFModels/");
	return sFRD;
}
PImgDscBase FaceMatcher::computeImageDescriptor(string & FR, unsigned FaceFinderFlags)const
{
	FaceFinder FF(getFRD(), "", FR, FaceFinderFlags);
	PImage pImg;
	if (FF.gotFaces())
	{
		const FaceRegion & PFR = *FF.getPrimaryFaceRegion();
		pImg = new Image(FF.getImageFN(), PFR);
		stringstream strm; strm<<FF;
		FR = strm.str();
	}
	else pImg = new Image(FR);
	return pImg ? newImgDsc(*pImg) : 0;
}
REALNUM FaceMatcher::distance(const string & aFN, const string & bFN, REALNUM param, unsigned FaceFinderFlags, unsigned flags)const
{
	REALNUM d=0;
	if (flags)
	{
		FaceFinder aFF(getFRD(), "", aFN, FaceFinderFlags), bFF(getFRD(), "", bFN, FaceFinderFlags);
		if (!aFF.gotFaces(true) || !bFF.gotFaces(true)) return 1; // max-out, no face to compare
		const FaceRegion
			*aFR = aFF.getPrimaryFaceRegion(),
			*bFR = bFF.getPrimaryFaceRegion();
		if (!aFR || !bFR) return 1; // max-out, no face to compare
		FaceRegion
			aR=*aFR,
			bR=*bFR;
		if (flags&fmScale)
		{
			stretch(aR, param);
			stretch(bR, param);
		}
		Image // crop to the face regions
			aF(aFF.getOriginalImage(), aR),
			bF(bFF.getOriginalImage(), bR);
		if (flags&fmMirror) bF.mirror();
		if (flags&fmRotate) bF.rotateD(param);
		d=distance(aF, bF, flags);
	}
	else // no transform
	{
		Image a(aFN), b(bFN);
		d=distance(a, b, flags);
	}
	if (flags) clog<<d<<"\t"<<aFN<<"\t"<<bFN<<endl;
	return d;
}

void FaceMatcher::save(const string & FN)const
{
	if (FN.empty()) return;
	const string & ext = getFileExt(FN);
	if (ext==".yml")
	{
		FileStorage fs(FN, FileStorage::WRITE);
		mIndex.write(fs);
	}
	else // assume binary
	{
		ofstream os(FN, ios::binary);
		mIndex.write(os);
	}
}
PScoredLines FaceMatcher::matchOne(string & FN, unsigned FaceFinderFlags, unsigned flags)const
{
	PImgDscBase pDsc = computeImageDescriptor(FN, FaceFinderFlags);
	return mIndex.query(*pDsc, mMatchT);
}
const QueryResults & FaceMatcher::match(const string & inFN, unsigned FaceFinderFlags, unsigned flags)
{
	if (isListFile(inFN)) // list match
		for (ListReader lr(inFN); !lr.end(); )
		{
			string FN = lr.fetch(); if (FN.empty() || FN[0]=='#') continue;
			FN=mRepoPath+FN;
			mResults[FN]=matchOne(FN, FaceFinderFlags, flags);
		}
	else // single image file
	{
		string FR=inFN;
		PScoredLines pSL=matchOne(FR, FaceFinderFlags, flags); // also getting FR, if it was not specified
		mResults[FR]=pSL;
	}
	return mResults;
}
void FaceMatcher::eval(ostream & s)const
{
	mIndex.eval(s);
}

} // FaceMatch
