
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

#pragma once // 2012-2015 (C) 

using namespace std;

namespace FaceMatch
{

class FaceFinder;

/// visual feedback implementation
class LIBDCL ResultsDisplay
{
	string mWinTitle;
	Mat mSourceImage, mImage;
	REALNUM mScale;
	FaceFinder * mFF;
	string mInputMode;

	static void onMouse(int event, int x, int y, int flags, void * param);
	void setInputMode(char mode)
	{
		mInputMode=mode;
		clog<<"InputMode="<<mInputMode<<endl;
	}
	void editRegionAttr(int x, int y);
	void removeRegion(int x, int y);
	void makePrimeRgn(int x, int y);
	void addProfile(Rect & r);
	void addFace(Rect & r);
	void addSkin(Rect & r);
	void setRoI(Rect & r);
	void clearRegions(bool bShow=true);
	void findRegions(bool bShow=true);
	void rotateImage(bool ClockWise=false);
	static bool isRegionRound(const string & kind)
	{
		return (kind=="f" || kind=="i" || kind=="e");
	}
	static void drawRegion(Mat & img, const FaceRegion & fr, bool prime, const Point & offset);
	static void drawRegions(Mat & img, const FaceRegions & res, const Point & offset=Point(0,0));
	void show(bool imWriteOut=false);
public:
	/// @return region line thickness
	static int getRegionLineThickness(const string & kind /**< region kind */)
	{
		return (kind=="f" || kind=="p") ? 2 : 1;
	}
	/// @return region color
	static Scalar getRegionColor(const string & kind /**< region kind */, bool prime /**< is it prime? */);

	/**
	 * Instantiate visual feedback display.
	 * @param WindowTitle title of the display window
	 * @param img base image to be displayed
	 * @param pFF	a pointer to the face finder instance
	 */
	ResultsDisplay(const string & WindowTitle, const Mat & img, FaceFinder * pFF = 0):
		mWinTitle(WindowTitle),
		mSourceImage(img),
		mScale(1),
		mFF(pFF)
	{
		namedWindow(mWinTitle);
		setMouseCallback(mWinTitle, onMouse, this);
	}
	virtual ~ResultsDisplay();
	/**
	 * Display visual feedback in a window, which can accept user's feedback, i.e.<br>
	 * [BackSpace], [P] for previous frame<br>
	 * [C] for clearing/deleting all regions<br>
	 * [F] for finding face/profile regions<br>
	 * [Space] for next frame<br>
	 * [ESC], [Q] to quit<br>
	 * input is not case sensitive.<br>
	 * Mouse events are also handled:<br>
	 * Left-drag: new face region<br>
	 * Right-drag: new profile<br>
	 * Left-click: prime regions at mouse location<br>
	 * Right-click: delete regions at mouse location.
	 * @return special character (e.g. SPC, RET) on a normal termination
	 */
	int run(bool wait4Key=true);
	/// Save the displayed image.
	void save(const string & FN="ResultsDisplay.jpg" /**< file name */)const
	{
		imwrite(FN, mImage);
	}
	static void show(const string & caption, const Mat & src, const FaceRegions & rgs, bool wait4Key=true);
};

} // namespace FaceMatch
