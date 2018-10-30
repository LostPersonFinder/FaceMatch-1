
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

#include "common.h" // 2012-2017 (C) 
#include "ResultsDisplay.h"
#include "FaceFinder.h"

namespace FaceMatch
{
ResultsDisplay::~ResultsDisplay()
{
	if (!(mFF->mFlags&FaceFinder::LiveFeed)) destroyWindow(mWinTitle);
}
void ResultsDisplay::editRegionAttr(int x, int y)
{
	Point p(x,y);
	// TODO: mFF->editRegionAttr(p);
	cout<<"f[x,y,w,h]: ";
	string in; getline(cin, in);
	show();
}
void ResultsDisplay::removeRegion(int x, int y)
{
	Point p(x,y);
	mFF->mFaces.removeFirstContainig(p);
	show();
}
void ResultsDisplay::makePrimeRgn(int x, int y)
{
	Point p(x,y);
	mFF->mFaces.prime(p);
	show();
}
void ResultsDisplay::addProfile(Rect & r)
{
	if (!mInputMode.empty())
	{
		mFF->addFaceFeature(mInputMode, r);
		mInputMode.erase();
	}
	else mFF->addProfile(r);
	show();
}
void ResultsDisplay::addFace(Rect & r)
{
	if (!mInputMode.empty())
	{
		mFF->addFaceFeature(mInputMode, r);
		mInputMode.erase();
	}
	else mFF->addFace(r);
	show();
}
void ResultsDisplay::addSkin(Rect & r)
{
	mFF->addSkin(r);
	show();
}
void ResultsDisplay::setRoI(Rect & r)
{
	mFF->setRoI(r);
	show();
}
void ResultsDisplay::clearRegions(bool bShow)
{
	mFF->clearRegions();
	if (bShow) show();
}
void ResultsDisplay::findRegions(bool bShow)
{
	mFF->detectFaces();
	if (bShow) show();
}
void ResultsDisplay::rotateImage(bool ClockWise)
{
	static const REALNUM a = PI/12;
	mFF->rotateScaledImage(ClockWise ? -a : a);
	mSourceImage = mFF->mScaledImage;
	show();
}
void ResultsDisplay::show(bool imWriteOut)
{
	Image src(mSourceImage, mFF->mFaces);
	unsigned srcDim=src.dim();
	Image img(src, min(cMaxScrImgDim, srcDim));
	mScale = REALNUM(srcDim)/img.dim();
	mImage = img.mx();
	if (mFF->mRoI.area()) rectangle(mImage, mFF->mRoI, CV_RGB(0xFF,0,0xFF));
//--- show results(img.getFaceRegions());
	drawRegions(mImage, img.getFaceRegions(), mFF->mRoI.tl());
	if (imWriteOut || mFF->VisualVerbose())
	{
		string FN=mFF->mImgOutDir+"FF."+getFileName(mWinTitle, false)+".jpg";
		imwrite(FN, mImage);
	}
	putText(mImage, "Quit [ ]next Prev Del Clear Find Out",
		Point(2, mImage.rows-8), CV_FONT_HERSHEY_SIMPLEX,
		0.33, cvScalar(0,0xff,0xff));
	imshow(mWinTitle, mImage);
}
/// \return key code pressed, optionally waiting for a key
int kbd(bool wait4Key /**< wait till next key is pressed? */)
{
	int k = wait4Key ? waitKey(0) : waitKey(2); // config/param
	if (k==-1) return k;
	SpecialChar c = (SpecialChar) k;
	if (getVerbLevel()>1) clog<<hex<<"k=0x"<<k<<" c=0x"<<c<<dec<<endl;
	switch (c)
	{
	case ESC: // quit
	case HOME: // top
	case END: // bottom
	case PgUp: // page up
	case PgDn: // page down
		throw c;
	case BkSp: case Up: throw BkSp; // prev image
	case DEL: case Down: throw DEL; // skip image
	}
	return k;
}
int ResultsDisplay::run(bool wait4Key)
{
	do
	{
		show();
		int k=kbd(wait4Key);
		SpecialChar c = k;
		if (c==RET || c==SPC) return k; // accept image
		else if (c==Left) rotateImage(false); // ccw
		else if (c==Right) rotateImage(true); // cw
	// process non-special characters
		else if (strchr("pP", c)) throw BkSp; // prev image
		else if (strchr("dD", c)) throw DEL; // skip image
		else if (strchr("qQ", c)) throw ESC; // quit program
		else if (strchr("iI", c)) setInputMode('i'); // eye
		else if (strchr("nN", c)) setInputMode('n'); // nose
		else if (strchr("mM", c)) setInputMode('m'); // mouth
		else if (strchr("eE", c)) setInputMode('e'); // ear
		else if (strchr("sS", c)) setInputMode('s'); // skin
		else if (strchr("rR", c)) setInputMode('r'); // RoI
		else if (strchr("cC", c)) clearRegions(false);
		else if (strchr("fF", c)) findRegions(false);
		else if (strchr("oO", c)) show(true);
	}while(wait4Key);
	return 0;
}
void ResultsDisplay::onMouse(int event, int x, int y, int flags, void * param)
{
	static int minDrag = 4; // pixels
	static bool dragSelect = false;
	static Point selAnchor;
	ResultsDisplay *pRD = static_cast<ResultsDisplay*>(param);
	if (!pRD) return;
	switch(event)
	{
	case CV_EVENT_LBUTTONDOWN:
	case CV_EVENT_RBUTTONDOWN:
		selAnchor.x = x;
		selAnchor.y = y;
		break;
	case CV_EVENT_LBUTTONUP:
	case CV_EVENT_RBUTTONUP:
		if (dragSelect)
		{
			const Rect ImgRect(Point(0,0), pRD->mImage.size());
			Rect r(selAnchor, Point((short)x,(short)y)); // using short to prevent clipping
			r &= ImgRect;
			r*=pRD->mScale;
			if (pRD->mInputMode=="s") pRD->addSkin(r);
			else if (pRD->mInputMode=="r") pRD->setRoI(r);
			else if (event==CV_EVENT_RBUTTONUP) pRD->addProfile(r);
			else pRD->addFace(r); // LBUp
			dragSelect = false;
		}
		else
		{
			x*=pRD->mScale;
			y*=pRD->mScale;
			if (event==CV_EVENT_LBUTTONUP) pRD->makePrimeRgn(x,y);
			else pRD->removeRegion(x,y); // RBUp
		}
		break;
	case CV_EVENT_MOUSEMOVE:
		Point p(x,y);
		if (flags & (CV_EVENT_FLAG_LBUTTON | CV_EVENT_FLAG_RBUTTON)) // drag selection
		{
			if (abs(p.x-selAnchor.x)<minDrag || abs(p.y-selAnchor.y)<minDrag) break;
			dragSelect = true;
			Mat im = pRD->mImage.clone(); // TODO: a nicer way of overlaying a selection rectangle
			Rect box(selAnchor, p);
			if ((flags&CV_EVENT_FLAG_LBUTTON) && (pRD->mInputMode.empty() || isRegionRound(pRD->mInputMode)))
				ellipse(im, RotatedRect(center(box), box.size(), 0), CV_RGB(0,0,0xFF)); // face
			else rectangle(im, box, CV_RGB(0,0,0xFF));
			imshow(pRD->mWinTitle, im);
		}
		break;
	}
}
void ResultsDisplay::drawRegion(Mat & img, const FaceRegion & fr, bool prime, const Point & offset)
{
	const int LineThinkness = getRegionLineThickness(fr.Kind);
	const Rect & r = fr+offset;
	if (isRegionRound(fr.Kind))
	{
		Scalar color = getRegionColor(fr.Kind, prime); // cColors[i%4];
		RotatedRect box(center(r), r.size(), 0);
		ellipse(img, box, color, LineThinkness);
	}
	else // rect
	{
		Scalar color = getRegionColor(fr.Kind, prime); // cColors[4+i%4];
		rectangle(img, r, color, LineThinkness);
	}
	drawRegions(img, fr.mSubregions, r.tl());
}
void ResultsDisplay::drawRegions(Mat & img, const FaceRegions & res, const Point & offset)
{
	int i=0;
	for(const auto & r : res)
		if (isFRRectKind(r->Kind))
			drawRegion(img, dynamic_cast<const FaceRegion&>(*r), i==0, offset);
}
void ResultsDisplay::show(const string & caption, const Mat & src, const FaceRegions & rgs, bool wait4Key)
{
	Mat img = src.clone();
	drawRegions(img, rgs);
	imshow(caption, img);
	kbd(wait4Key);
}
Scalar ResultsDisplay::getRegionColor(const string & kind, bool prime)
{
	static
	const Scalar Colors[] =
	{
		CV_RGB(0,0,0xFF),
		CV_RGB(0,0x80,0xFF),
		CV_RGB(0,0xFF,0xFF),
		CV_RGB(0,0xFF,0),
		CV_RGB(0xFF,0x80,0),
		CV_RGB(0xFF,0xFF,0),
		CV_RGB(0xFF,0,0),
		CV_RGB(0xFF,0,0xFF)
	};
	enum
	{
		clrFace,
		clrEye,
		clrNose,
		clrMouth,
		clrEar
	};
	Scalar d = prime ? 1 : 2;
	Scalar c = 0;
	if (kind=="f" || kind=="p") c = Colors[clrFace]/d;
	else if (kind=="i") c = Colors[clrEye];
	else if (kind=="n") c = Colors[clrNose];
	else if (kind=="m") c = Colors[clrMouth];
	else if (kind=="e") c = Colors[clrEar];
	return c;
}
} // namespace FaceMatch



