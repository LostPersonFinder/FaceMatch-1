
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

#include "common.h"
#include "ImageCollage.h"
#include "Image.h"
#include "ImgDscKeyPt.h"
#include "ImgDscLBPH.h"
#include "ImgDscHaar.h"
#include "string_supp.h"

namespace FaceMatch
{
ImageCollage::ImageCollage(const string & WindowTitle, const string & ImagesPath):
	mImagesPath(ImagesPath), mWindowTitle(WindowTitle)
{
	namedWindow(mWindowTitle);
	setMouseCallback(mWindowTitle, onMouse, this);
}
ImageCollage::Row * ImageCollage::add(const string & ImgLn)
{
	if (ImgLn.empty()) mRows.push_back(new Row());
	else if (startsWith(ImgLn, mImagesPath)) mRows.push_back(new Row(mImagesPath, ImgLn.substr(mImagesPath.length())));
	else mRows.push_back(new Row("", ImgLn));
	return mRows.back();
}
void ImageCollage::Row::onShow(Mat & RowImg)const
{
	if (mImageList.empty()) return;
	unsigned CellCnt=min<unsigned>(DefaultCellCnt, mImageList.size());
	unsigned c=0, x=0;
	vector<Point> pts;
	for (ImageCollage::ImageList::const_iterator it=mImageList.begin(); c<CellCnt && it!=mImageList.end(); ++c, x+=CellSize, ++it)
	{
		const ImageEntry & ie=**it;
		Image img(Image(ie.getImage()), CellSize);
		Rect r(x, 0, img.width(), img.height());
		Mat sub(RowImg, r); // RoI
		img.mx().copyTo(sub);
		stringstream strm;
		REALNUM score=ie.getScore();
		strm<<score<<" "<<ie.getTag();
		putText(sub, strm.str(), Point(1,10), FONT_HERSHEY_SIMPLEX, 0.35, Scalar(0,0xff,0xff));
		pts.push_back(Point(x+CellSize/2, (1-score)*CellSize));
	}
	polylines(RowImg, pts, false, Scalar(0,0xff,0xff), 1, CV_AA);
}
/// @return score value; -1 if none found
float parseScore(const string & s)
{
	static const string numeric="+-0123456789.e";
	auto pos = s.find_first_not_of(numeric); // we can have a stricter check
	if (pos==string::npos) return atof(s.c_str());
	return -1;
}
void ImageCollage::ImageEntry::parseFR(const string & RepoPath, istream & StrmImgRgn)
{
	if (!StrmImgRgn.eof()) mFR=FaceRegion(StrmImgRgn);
	int pos = mImgFN.rfind(':'); // cut optional region index
	mImgFN = mImgFN.substr(0, pos);
	mImage = imread(RepoPath+mImgFN, CV_LOAD_IMAGE_COLOR);
	if (mImage.empty())
	{
		mImage = imread(mImgFN, CV_LOAD_IMAGE_COLOR);
		if (mImage.empty())
			clog<<"warning: in ImageCollage::ImageEntry::parseFR empty image from RepoPath='"+RepoPath+"' mImgFN='"+mImgFN+"'"<<endl;
	}
	if (mFR.height<mImage.rows && mFR.width<mImage.cols)
		mImage = mImage(mFR);
}
/**
 * Instantiate an image entry from an image region.
 * Expect image region in the format: [nnnn.dddd\t]ImgFN[:n][\tRoI].
 */
ImageCollage::ImageEntry::ImageEntry(const string & RepoPath, const string & ScoredImgRgn): mScore(0), mFR("f")
{
	stringstream StrmImgRgn(ScoredImgRgn);
	string token; getline(StrmImgRgn, token, '\t');
	mScore = parseScore(token);
	if (mScore<0) mImgFN=token; // no score
	else getline(StrmImgRgn, mImgFN, '\t');
	parseFR(RepoPath, StrmImgRgn);
}
ImageCollage::ImageEntry::ImageEntry(const string & RepoPath, const string & ImgRgn, REALNUM score): mScore(score), mFR("f")
{
	stringstream StrmImgRgn(ImgRgn);
	getline(StrmImgRgn, mImgFN, '\t');
	parseFR(RepoPath, StrmImgRgn);
}
ImageCollage::ImageEntry::ImageEntry(const string & RepoPath, const string & QryRgn, const string & ResRgn, const string & tag, REALNUM score):
	mScore(score), mTag(tag), mFR("f")
{
	ImageEntry
		QryRgnImgE("", QryRgn),
		ResRgnImgE(RepoPath, ResRgn);
	Image
		QryRgnImg(QryRgnImgE.getImage()),
		ResRgnImg(ResRgnImgE.getImage());
	if (getVerbLevel()>3)
	{
		imshow("QryRgnImg", QryRgnImg.mx());
		imshow("ResRgnImg", ResRgnImg.mx());
		waitKey();
	}
	if (getHistEQ())
	{
		QryRgnImg.eqHist();
		ResRgnImg.eqHist();
	}
	bool KeyPt=false, GivenDist=false;
	PImgDscBase pQryRgnDsc, pResRgnDsc;
	REALNUM Dist=1; // supposed distance
	vector<DMatch> matches;
	try
	{
        if (!QryRgnImg.dim())
			throw FaceMatch::Exception("QryRgnImg is empty from "+QryRgn);
        if (!ResRgnImg.dim())
			throw FaceMatch::Exception("ResRgnImg is empty from "+ResRgn);
		if (tag=="SIFT")
		{
			pQryRgnDsc=new ImgDscSIFT(QryRgnImg);
			pResRgnDsc=new ImgDscSIFT(ResRgnImg);
			KeyPt=true;
		}
		else if (tag=="SURF")
		{
			pQryRgnDsc=new ImgDscSURF(QryRgnImg);
			pResRgnDsc=new ImgDscSURF(ResRgnImg);
			KeyPt=true;
		}
		else if (tag=="ORB")
		{
			pQryRgnDsc=new ImgDscORB(QryRgnImg);
			pResRgnDsc=new ImgDscORB(ResRgnImg);
			KeyPt=true;
		}
		else if (tag=="LBPH")
		{
			pQryRgnDsc=new ImgDscLBPH(QryRgnImg);
			pResRgnDsc=new ImgDscLBPH(ResRgnImg);
		}
		else if (tag=="HAAR")
		{
			pQryRgnDsc=new ImgDscHaarFace(QryRgnImg);
			pResRgnDsc=new ImgDscHaarFace(ResRgnImg);
		}
		else GivenDist=true;
		if (GivenDist) Dist=mScore;
		else
		{
			Dist=pQryRgnDsc->dist(*pResRgnDsc, &matches);
			if (QryRgnImgE.mFR.Kind!=ResRgnImgE.mFR.Kind) Dist=sqrt(Dist);
		}
	}
	catch (const FaceMatch::Exception & e)
	{
		clog<<"ImageEntry::ImageEntry.FaceMatch::Exception: "<<e.what()<<endl;
	}
	if (KeyPt)
	{
		if (pQryRgnDsc && pResRgnDsc)
		{
			const ImgDscKeyPt
				&QryRgnDsc=(const ImgDscKeyPt&)*pQryRgnDsc,
				&ResRgnDsc=(const ImgDscKeyPt&)*pResRgnDsc;
			drawMatches(QryRgnImg.mx(), QryRgnDsc.getKeypoints(), ResRgnImg.mx(), ResRgnDsc.getKeypoints(), matches, mImage);
		}
		else
		{
			mScore=1;
			mImage=ResRgnImg.mx();
		}
	}
	else
	{
		mImage=ResRgnImg.mx();
	}
	if (mScore==1) mScore=Dist;
	else if (mScore!=Dist && getVerbLevel()>1)
	{
		clog<<"warning: "<<tag<<" score="<<mScore<<", Dist="<<Dist<<endl;
		if (pQryRgnDsc && pResRgnDsc && getVerbLevel()>2)
		{
			clog<<"Q: "<<QryRgn<<"="<<*pQryRgnDsc<<endl;
			clog<<"R: "<<ResRgn<<"="<<*pResRgnDsc<<endl;
		}
	}
	if (mImage.empty() && getVerbLevel()>2)
		clog<<"warning: unable to draw match image for "+QryRgn+" vs "+ResRgn<<endl;
}
void ImageCollage::Row::addMatch(const string & RepoPath, const string & DscID, const string & QryRgnID, const string & ResRgnID, REALNUM score)
{
	if (mImageList.size()>=DefaultCellCnt) return;
	mImageList.push_back(new ImageEntry(RepoPath, QryRgnID, ResRgnID, DscID, score));
}
void ImageCollage::addMatch(string QryRgnID, string ResRgnID, string ComboKind)
{
	static const int sDscDnt=5;
	static const char * sDescriptors[sDscDnt] = {"HAAR", "LBPH", "ORB", "SURF", "SIFT"};
	if (mRows.size()==0)
		for (int i=0; i<=sDscDnt; ++i) add(); // extra row for the combo
	if (ResRgnID.find(mImagesPath)==0)
		ResRgnID=ResRgnID.substr(mImagesPath.length());
	ParallelErrorStream PES;
#pragma omp parallel for shared(PES)
	for (int i=0; i<sDscDnt; ++i)
		try
		{
			mRows[i]->addMatch(mImagesPath, sDescriptors[i], QryRgnID, ResRgnID);
		}
		PCATCH(PES, format("ImageCollage::addMatch dsc %d", i))
	PES.report("parallel ImageCollage::addMatch errors");
	REALNUM ComboScore=1; stringstream strm(ResRgnID); strm>>ComboScore;
	mRows[sDscDnt]->addMatch(mImagesPath, ComboKind, QryRgnID, ResRgnID, ComboScore);
}
void ImageCollage::show()
{
	unsigned RowCnt=min<unsigned>(DefaultRowCount, mRows.size());
	if (!RowCnt) return;
	unsigned CellCnt=0;
	for (unsigned r=0; r<RowCnt; ++r)
		CellCnt=max<unsigned>(CellCnt, mRows[r]->getSize());
	mCollage = Mat::zeros(CellSize*RowCnt, CellSize*CellCnt, CV_8UC3);
	for (unsigned r=0; r<RowCnt; ++r)
	{
		Rect R(0, r*CellSize, CellSize*CellCnt, CellSize);
		Mat sub(mCollage, R);
		mRows[r]->onShow(sub);
	}
 	imshow(mWindowTitle, mCollage);
}
void ImageCollage::out(string ImgFN)
{
	if (ImgFN.empty())
	{
		stringstream strmTitle(mWindowTitle);
		getline(strmTitle, ImgFN, '\t');
		ImgFN=getFileName(ImgFN, false)+".IC.jpg";
	}
	if (getVerbLevel()) clog<<"ImageCollage::out "<<ImgFN<<endl;
	imwrite(ImgFN, mCollage);
}
}
