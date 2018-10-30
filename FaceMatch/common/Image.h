
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

#include "common.h"

namespace FaceMatch
{

/// image pixel structure
struct BGRPixel
{
	/// color enumeration
	enum{blue, green, red, cnt};
	byte
	/// B blue value
	B,
	/// G green value
	G,
	/// R red value
	R;
};

/// image variations/distortions
enum EImgageVariation
{
	ivNone,       //!< ivNone
	ivCrop,       //!< ivCrop
	ivRotate=1<<1,//!< ivRotate
	ivScale=1<<2, //!< ivScale
	ivAll=~0      //!< ivAll
};

/// C++ light wrapper for cv::Mat with face (sub)regions
class LIBDCL Image
{
	Mat mImage;
	FaceRegions mFaceRegions;
public:
	/// image/region pi/4 rotation phases
	enum ETurnPhase
	{
		tphZero, ///< no turning
		tphOne, ///< first phase
		tphCCW=tphOne, ///< counter-clock-wise
		tphUD, ///< up-side-down
		tphCW, ///< clock-wise
		tphCount ///< phase count
	};
	/// Instantiate from a source.
	Image(const Image & src /**< source image to copy from */): mImage(src.mImage), mFaceRegions(src.mFaceRegions){}
	/// \return image copy assigned to the instance
	const Image & operator=(const Image & src /**< source image to copy from */)
	{
		mImage=src.mImage;
		mFaceRegions=src.mFaceRegions;
		return *this;
	}
	/**
	 * Copy source image in the specified phase.
	 * @param src	source image
	 * @param rp	rotation phase, e.g. zero, CW, CCW, UpDn
	 */
	Image(const Image & src, ETurnPhase rp) // TODO: mFaceRegions
	{
		if (rp==tphZero) *this=src;
		else
		{
			src.mImage.copyTo(mImage);
			rotPh(*this, rp);
		}
	}
	/**
	 * Re-sample the source image to the specified dimensions.
	 * @param src source image
	 * @param width (optional) target width, if 0, copy image
	 * @param height (optional) target width, if 0, copy image
	 */
	Image(const Image & src, unsigned width, unsigned height)
		// TODO: mFaceRegions
	{
		if (width && height)
			resize(src.mImage, mImage, Size(width, height));
		else mImage=src.mImage;
	}
	/**
	 * Re-sample/deep-copy source image to the specified max dimension.
	 * @param src	source image
	 * @param dim	max dimension; if 0, deep-copy
	 */
	Image(const Image & src, unsigned dim);
	/**
	 * Load image from file.
	 * @param imgFN input file name
	 */
	Image(const string & imgFN): mImage(load(imgFN)) { }
	/**
	 * Load the region of interest from an image file.
	 * @param imgFN image file name
	 * @param r region of interest
	 */
	Image(const string & imgFN, const Rect & r): mImage(load(imgFN)(r)) { }
	/**
	 * Instantiate image from the given matrix.
	 * @param m input image matrix
	 */
	Image(const Mat & m): mImage(m) { }
	/// Instantiate.
	Image(const Mat & m, ///< source image
		const Rect & r ///< crop rectangle
		): mImage(cropad(m,r)) { }
	/**
	 * Instantiate image as the RoI of the given matrix.
	 * @param m input image matrix
	 * @param r region of interest
	 */
	Image(const Mat & m, const FaceRegion & r): mImage(cropad(m,r)), mFaceRegions(r.mSubregions) {}
	/// Instantiate.
	Image(const Mat & m, ///< source image
		const FaceRegions & r ///< source face regions
		): mImage(m), mFaceRegions(r) {}
	virtual ~Image() {}
	/// @return face regions collection associated with the image
	const FaceRegions & getFaceRegions()const{return mFaceRegions;}
	/// @return an image with face regions drawn
	Mat drawFaceRegions()const;
	/// @return image width
	int width()const{ return mImage.cols; }
	/// @return image height
	int height()const{ return mImage.rows; }
	/// @return image area in pixels
	unsigned area()const{return width()*height();}
	/// @return image diameter in pixels
	unsigned dim()const{return diameter(mImage);}
	 /// @return const Mat reference
	const Mat & mx() const { return mImage; }
	/// \return image as OpenCV matrix
	Mat & mx() { return mImage; }
	/// @return const Mat reference
	const Size size() const { return mImage.size(); }
	/// @return RoI rect of this image within the parent image
	Rect RoI()const
	{
		Size size; Point ofs;
		mImage.locateROI(size, ofs);
		return Rect(ofs, mImage.size());
	}
	/**
	 * Store image in a file.
	 * @param FN output file name
	 */
	void store(const string & FN) const
	{
		imwrite(FN, mImage);
	}
	/**
	 * Rotate the image about its center by the specified angle
	 * @param angle in degrees
	 * @param flags warping interpolation method
	 */
	void rotateD(REALNUM angle, int flags=-1)
	{
		mImage = rotate(mImage, angle, flags);
	}
	/**
	 * Rotate the image about its center by the specified angle
	 * @param angle in radians
	 */
	void rotateR(REALNUM angle) { rotateD(180*angle/CV_PI); }
	/// Equalize image histogram.
	void eqHist()
	{
		eqHist(mImage);
	}
	/// Mirror the image.
	void mirror()
	{
		flip(mImage, mImage, 1);
	}
	/// \return a gray-level version of the image
	Mat GrayScale()const
	{
		return ::GrayScale(mImage);
	}
	/// Show the image.
	void show(const string & caption, ///< window caption
		unsigned MaxDim=cMaxScrImgDim ///< max image diameter for display purposes
		)
	{
		Image img(*this, min(MaxDim, dim()));
		imshow(caption, img.mx());
	}
	/**
	 * Counter-clockwise image rotation (PI/2)
	 * @param img	image to be rotated
	 * @return transformed image ref
	 */
	static Mat & rotCCW(Mat & img)
	{
		Mat tmp(img);
		flip(tmp.t(), img, 0); // CCW
		return img;
	}
	/**
	 * Clockwise image rotation (3*PI/2)
	 * @param img	image to be rotated
	 * @return transformed image ref
	 */
	static Mat & rotCW(Mat & img)
	{
		Mat tmp(img);
		flip(tmp.t(), img, 1); // CW
		return img;
	}
	/**
	 * Upside-down image rotation (PI)
	 * @param img	image to be rotated
	 * @return transformed image ref
	 */
	static Mat & rotUD(Mat & img)
	{
		Mat tmp(img);
		flip(tmp, img, -1); // pi
		return img;
	}
	/**
	 * Output an image pi/2 rotation phase.
	 * @param img	output image
	 * @param rp	rotation phase, e.g. CW, CCW, UpDn
	 */
	static void rotPh(Image & img, ETurnPhase rp);
	/// @return an image with histogram equalized
	static Mat & eqHist(Mat & img /**< image to equalize histogram in */);
	/**
	 * Load an image, producing an excpetion noting the trouble some file name.
	 * @param FN input file name
	 * @param CVLoadFlags cv::imread flags
	 * @return image matrix
	 */
	static Mat load(const string & FN, int CVLoadFlags=CV_LOAD_IMAGE_UNCHANGED);
	/// @return RoI-cropped source, padded with black if necessary
	static Mat cropad(const Mat & src, ///< source image matrix
		Rect RoI ///< region of interest
	);
};
/// smart pointer to Image
typedef Ptr<Image> PImage;

/// Show and output an image (e.g. for diagnostics purposes).
inline void imOut(const string & name, ///< image/window name
	const Mat & img ///< source image
	)
{
	Image(img).show(name);
	imwrite(name+".jpg", img);
}

/**
 RGB -> (REAL) YIQ
	[ Y ]     [ 0.299   0.587   0.114 ] [ R ]
	[ I ]  =  [ 0.596  -0.274  -0.322 ] [ G ]
	[ Q ]     [ 0.212  -0.523   0.311 ] [ B ]
source: http://astronomy.swin.edu.au/~pbourke/colour/convert/
*/
void xfmRGB2YIQ
(
	const REALNUM * R, ///<[in] R band
	const REALNUM * G, ///<[in] G band
	const REALNUM * B, ///<[in] B band
	REALNUM * Y, ///<[out] Y band
	REALNUM * I, ///<[out] I band
	REALNUM * Q, ///<[out] Q band
	const unsigned cImageArea ///< image area in pixels
);

} // FaceMatch
