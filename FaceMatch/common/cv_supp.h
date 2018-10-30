
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

#pragma once // 2017 (C) 

#include <cv.h>
#include <highgui.h>
#include <cvaux.h>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/gpu/gpu.hpp> // GPU-aware routines

namespace cv
{
	/// batch of individual descriptor/signature matching objects
	typedef vector<DMatch> TMatch;
	/// collection of descriptor matching batch objects
	typedef vector<TMatch> TMatches;
	/// collection pack of descriptor matching batch objects
	typedef vector<TMatches> TMatchesPack;

	/// unsigned char (byte) matrix type
	typedef Mat_<uchar> MatUC;
	/// unsigned char (byte) matrix type
	typedef Mat_<int> MatInt;
	/// real-valued (float) matrix type
	typedef Mat_<REALNUM> MatReal;
	/// double-precision real-valued matrix type
	typedef Mat_<double> MatDouble;
	/// sparse real-valued (float) matrix type
	typedef SparseMat_<REALNUM> SparseMatReal;

	/// \return diameter of an image
	inline unsigned diameter(const Mat & src){ return max(src.rows, src.cols); }
	/// \return converted image of the source, using GPU when necessary
	LIBDCL Mat convert(const Mat & src, int code);
	/// \return histogram equalized version of source, using GPU when necessary
	LIBDCL Mat eqHist(const Mat & src);
	/// \return normalized image of the source, using GPU when necessary
	LIBDCL Mat normalize(const Mat & src, double alpha = 0, double beta = 0xFF, int norm_type = NORM_MINMAX, int dtype = CV_8U, const Mat & = Mat());
	/// \return grayscale image of the source, using GPU when necessary
	LIBDCL Mat GrayScale(const Mat & src /**< source image */);
	/// \return 2x3 rotation matrix for an image of the given size and angle
	LIBDCL Mat getRotMx(const Size & dim, ///< image dimensions
		REALNUM angle ///< rotation angle in degrees
	);
	/// \return a rotated by angle (degrees) for the input image
	LIBDCL Mat rotate(const Mat & src, REALNUM angle, int flags=-1);
	/// \return a scaled version of image
	LIBDCL Mat scale(const Mat & src, REALNUM s);
	/// \return Gaussian-blurred image
	LIBDCL Mat gblur(const Mat & src, ///< source image
		Size sz=Size(3,3), ///< kernel size
		REALNUM sgm=0 ///< sigma, \see cv::GaussianBlur, cv::gpu::GaussianBlur
	);
	/// \return template-to-image match map
	MatReal match(const Mat & image, ///< source image
		const Mat & tmpl, ///< template, \see cv::matchTemplate, cv::gpu::matchTemplate
		int method=CV_TM_CCORR_NORMED ///< matching method
	);

	/// \return converted pixel value
	template<class PIXEL>
	inline PIXEL convertPixel(PIXEL q, int code)
	{
		Mat_<PIXEL> A(1, 1, q); // cvtColor appears to work only with matrices, not with pixels
		cvtColor(A, A, code);
		return A(0, 0);
	}

	/// color image matrix
	typedef Mat_<Vec3b> Image3b;

	/// utility for OpenCV error stream suppression
	struct ErrorDumpSuppressor
	{
		ErrorDumpSuppressor(bool supp) { if (supp) on(); }
		// suppress OpenCV dump to cerr
		void on()const{ redirectError([](int, const char*, const char*, const char*, int, void*){return 0; }); }
		// restore OpenCV dump to cerr
		virtual void off(){ redirectError(ErrorCallback()); }
		virtual ~ErrorDumpSuppressor(){ off(); }
	};
	/**
	* Write an OpenCV matrix to a stream.
	* \param s	output binary stream
	* \param m an OpenCV matrix
	*/
	LIBDCL void write(ostream & s, const Mat & m);
	/**
	* Read an OpenCV matrix from a stream.
	* \param s	input binary stream
	* \param m an OpenCV matrix
	*/
	LIBDCL void read(istream & s, Mat & m);
	/// collection of key-points
	typedef vector<KeyPoint> KeyPoints;
	/// Output keypoints to a binary stream.
	LIBDCL void write(ostream & s, const KeyPoints & kpts);
	/// Input keypoints from a binary stream.
	LIBDCL void read(istream & s, KeyPoints & kpts);
	/// \return result of multiplication img*=map
	LIBDCL Mat & operator*=(Mat & img, const MatReal & map);
} // namespace cv
