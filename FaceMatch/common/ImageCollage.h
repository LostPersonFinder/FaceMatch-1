
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

#pragma once // 2011-2016 (C) 

namespace FaceMatch
{
const unsigned
	DefaultCellCnt=12, ///< default cell count in a collage
	DefaultRowCount=8; ///< default row count in a collage
/**
 * Define a facility for displaying one or many images or some regions of interest in a dynamically composed collage.
 */
class LIBDCL ImageCollage
{
	class LIBDCL ImageEntry
	{
		REALNUM mScore;
		string mImgFN, mTag;
		FaceRegion mFR;
		Mat mImage;
		void parseFR(const string & RepoPath, istream & StrmImgRgn);
	public:
		ImageEntry(const string & RepoPath, const string & ScoredImgRgn);
		ImageEntry(const string & RepoPath, const string & ImgRgn, REALNUM score);
		ImageEntry(const string & RepoPath, const string & QryRgn, const string & ResRgn, const string & tag, REALNUM score=1);
		string getImgFileLine()const
		{
			stringstream strmFL; strmFL<<mImgFN<<"\t"<<mFR;
			return strmFL.str();
		}
		REALNUM getScore()const{return mScore;}
		const string & getTag()const{return mTag;}
		const Mat & getImage()const{return mImage;}
	};
	typedef Ptr<ImageEntry> PImageEntry;
	typedef vector<PImageEntry> ImageList;
	
	string mWindowTitle, mImagesPath;
	Mat mCollage;
	static const unsigned
		CellSize = 96;

	void throwNext(int x, int y)
	{
		unsigned
			row=y/CellSize,
			col=x/CellSize;
		string ImgFN =  mRows.getImgFN(row, col);
		if (!ImgFN.empty()) throw ImgFN;
	}
	static void onMouse(int event, int x, int y, int flags, void * param)
	{
		ImageCollage *pRD = (ImageCollage*)param;
		switch(event)
		{
		case CV_EVENT_LBUTTONDBLCLK:
			pRD->throwNext(x,y);
			break;
		}
	}
public:
	/// image row in a collage
	class Row
	{
		ImageList mImageList;
	public:
		Row(){}
		/// Instantiate.
		Row(const string & RepoPath, ///< image repository path
			const string & ImgRgn ///< image (face/profile) region
			){ add(RepoPath, ImgRgn); }
		/// add a scored image/profile region to the instance
		void add(const string & RepoPath, ///< image repository path
			const string & ScoredImgRgn ///< scored image (face/profile) region
			)
		{
			if (mImageList.size()<DefaultCellCnt) mImageList.push_back(new ImageEntry(RepoPath, ScoredImgRgn));
		}
		/// add a scored image/profile region to the instance
		void add(const string & RepoPath, ///< image repository path
			const string & ImgRgn, ///< image (face/profile) region
			REALNUM score ///< real-valued score, e.g. distance in [0,1]
			)
		{
			if (mImageList.size()<DefaultCellCnt) mImageList.push_back(new ImageEntry(RepoPath, ImgRgn, score));
		}
		/// add a match to the instance
		void addMatch(const string & RepoPath, ///< image repository path
			const string & DscID, ///< desriptor ID
			const string & QryRgnID, ///< query region ID
			const string & ResRegID, ///< result region ID
			REALNUM score=1 ///< real-valued score, e.g. distance in [0,1]
			);
		/// define on-show behavior
		void onShow(Mat & RowImg /**<[in|out] row image */)const;
		/// @return image file line (with optional regions and attributes)
		string getImgFileLine(unsigned pos /**< image position in the row */)const
		{
			return pos<mImageList.size() ? mImageList.at(pos)->getImgFileLine() : string();
		}
		/// @return length of the row in cells
		unsigned getSize()const{return mImageList.size();}
	};
	/// a smart pointer to an image row
	typedef Ptr<Row> PRow;
	/// Instantiate.
	ImageCollage(const string & WindowTitle="ImageCollage", ///< image collage window title
			const string & ImagesPath="" ///< images path
		);
	virtual ~ImageCollage()
	{
		destroyWindow(mWindowTitle);
	}
	/// @return a pointer to the added image row
	Row * add(const string & ImgLn="" /**< image/regions/attribute line */);
	/// add a match for a query image descriptor
	void addMatch(string QryRgnID, ///< query region ID
		string ResRgnID, ///< query result region ID
		string ComboKind ///< combination kind, e.g. MANY, DIST, RANK
		);
	/// show the instance
	void show();
	/// write the image to a file
	void out(string ImgFN="" /**< output image file name */);
protected:
	/// result image rows in the collage
	struct Rows: public vector<PRow>
	{
		/// @return image file name at a given position in the collage
		string getImgFN(unsigned row /**< 0-based row index*/, unsigned col /**< 0-based column index*/)const
		{
			return row<size() ? at(row)->getImgFileLine(col) : string();
		}
	}mRows; ///< an instance of collage rows
};
} // namespace FaceMatch
