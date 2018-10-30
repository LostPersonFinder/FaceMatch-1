
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
#include "Image.h"
#include "ResultsDisplay.h"

namespace FaceMatch
{

Mat Image::load(const string & FN, int CVLoadFlags)
{	FTIMELOG
	Mat img;
	try	{ img=imread(FN, CVLoadFlags); }
	catch(const exception & e){	throw Exception(FN+": "+e.what()); }
	CHECK(!img.empty(), FN+": empty image");
	return img;
}

void Image::rotPh(Image & img, ETurnPhase rp)
{
	Mat & M = img.mImage;
	switch (rp)
	{
	case tphZero: break;
	case tphCCW: rotCCW(M); break;
	case tphUD: rotUD(M); break;
	case tphCW: rotCW(M); break;
	default: rotPh(img, (ETurnPhase)(rp%tphCount)); break;
	}
	// TODO: rotate regions
}

Mat & Image::eqHist(Mat & img)
{
	if (img.empty()) return img;
	byte CC=img.channels();
	if (CC>=3) cvtColor(img, img, CV_BGR2YCrCb, CC);
	vector<Mat> channels;
	split(img, channels);
	channels[0] = cv::eqHist(channels[0]);
	merge(channels, img);
	if (CC>=3) cvtColor(img, img, CV_YCrCb2BGR, CC);
	return img;
}

Image::Image(const Image & src, unsigned dim): mFaceRegions(src.mFaceRegions)
{
	if (dim==0) { mImage=src.mImage.clone(); return; }
	const unsigned
		W=src.width(), H=src.height(), D=max(W,H);
	if (!D) return;
	const REALNUM s=dim/(REALNUM)D;
	resize(src.mImage, mImage, Size(), s, s);
	mFaceRegions.scale(s, s);
}

Mat Image::cropad(const Mat & src, Rect RoI)
{
	Mat dst = Mat::zeros(RoI.size(), src.type());
	Rect
		R(Point(0,0), src.size()),
		r = R & RoI;
	if (r.area()) // dst(r-RoI.tl()) = src(r);
		src(r).copyTo(dst(r-RoI.tl()));
	if (getVerbLevel()>2 && r!=RoI)
		imshow(__FUNCTION__, dst);
	return dst;
}

Mat Image::drawFaceRegions()const
{
	Mat dst=mImage.clone();
	for (auto it=mFaceRegions.begin(); it!=mFaceRegions.end(); ++it)
	{
		if (!isFRRectKind((*it)->Kind)) continue;
		const FaceRegion & fr=dynamic_cast<const FaceRegion&>(**it);
		const int LineThinkness = ResultsDisplay::getRegionLineThickness(fr.Kind);
		Scalar color = ResultsDisplay::getRegionColor(fr.Kind, false);
		rectangle(dst, fr, color, LineThinkness);
	}
	return dst;
}

/// matrix for RGB->YIQ conversion
const REALNUM mxRGB2YIQ[3][3]=
{
	{0.299f, 0.587f, 0.114f},
	{0.596f, -0.274f, -0.322f},
	{0.212f, -0.523f,  0.311f}
};
/// RGB->YIQ transformation
void xfmRGB2YIQ
(
	const REALNUM * R, const REALNUM * G, const REALNUM * B,
	REALNUM * Y, REALNUM * I, REALNUM * Q,
	const unsigned cImageArea
)
{
	for (unsigned i=0; i<cImageArea; ++i)
	{
		Y[i] = mxRGB2YIQ[0][0]*R[i] + mxRGB2YIQ[0][1]*G[i] + mxRGB2YIQ[0][2]*B[i];
		I[i] = mxRGB2YIQ[1][0]*R[i] + mxRGB2YIQ[1][1]*G[i] + mxRGB2YIQ[1][2]*B[i];
		Q[i] = mxRGB2YIQ[2][0]*R[i] + mxRGB2YIQ[2][1]*G[i] + mxRGB2YIQ[2][2]*B[i];
	}
}

} // FaceMatch
