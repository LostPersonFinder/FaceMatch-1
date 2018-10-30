
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
#include "ImgDscLBPH.h"

namespace FaceMatch
{
static Mat histc_(const Mat& src, int minVal=0, int maxVal=0xFF, bool normed=false)
{
	Mat result;
	// Establish the number of bins.
	int histSize = maxVal-minVal+1;
	// Set the ranges.
	float range[] = { static_cast<float>(minVal), static_cast<float>(maxVal+1) };
	const float* histRange = { range };
	// calc histogram
	calcHist(&src, 1, 0, Mat(), result, 1, &histSize, &histRange, true, false);
	// normalize
	if(normed) result /= (int)src.total();
	return result.reshape(1,1);
}
static Mat histc(InputArray _src, int minVal, int maxVal, bool normed)
{
	Mat src = _src.getMat();
	switch (src.type()) {
	case CV_8SC1:
		return histc_(Mat_<float>(src), minVal, maxVal, normed);
		break;
	case CV_8UC1:
		return histc_(src, minVal, maxVal, normed);
		break;
	case CV_16SC1:
		return histc_(Mat_<float>(src), minVal, maxVal, normed);
		break;
	case CV_16UC1:
		return histc_(src, minVal, maxVal, normed);
		break;
	case CV_32SC1:
		return histc_(Mat_<float>(src), minVal, maxVal, normed);
		break;
	case CV_32FC1:
		return histc_(src, minVal, maxVal, normed);
		break;
	default:
		CV_Error(CV_StsUnmatchedFormats, "This type is not implemented yet."); break;
	}
	return Mat();
}

template <typename _Tp>
static void elbp_(InputArray _src, OutputArray _dst, int radius, int neighbors)
{
	//get matrices
	Mat src = _src.getMat();
	// allocate memory for result
	_dst.create(src.rows-2*radius, src.cols-2*radius, CV_32SC1);
	Mat dst = _dst.getMat();
	// zero
	dst.setTo(0);
	for(int n=0; n<neighbors; n++)
	{
		// sample points
		float x = static_cast<float>(-radius * sin(2.0*CV_PI*n/static_cast<float>(neighbors)));
		float y = static_cast<float>(radius * cos(2.0*CV_PI*n/static_cast<float>(neighbors)));
		// relative indices
		int fx = static_cast<int>(floor(x));
		int fy = static_cast<int>(floor(y));
		int cx = static_cast<int>(ceil(x));
		int cy = static_cast<int>(ceil(y));
		// fractional part
		float ty = y - fy;
		float tx = x - fx;
		// set interpolation weights
		float w1 = (1 - tx) * (1 - ty);
		float w2 =      tx  * (1 - ty);
		float w3 = (1 - tx) *      ty;
		float w4 =      tx  *      ty;
		// iterate through your data
		for(int i=radius; i < src.rows-radius;i++)
		{
			for(int j=radius;j < src.cols-radius;j++)
			{
				// calculate interpolated value
				float t = static_cast<float>(w1*src.at<_Tp>(i+fy,j+fx) + w2*src.at<_Tp>(i+fy,j+cx) + w3*src.at<_Tp>(i+cy,j+fx) + w4*src.at<_Tp>(i+cy,j+cx));
				// floating point precision, so check some machine-dependent epsilon
				dst.at<int>(i-radius,j-radius) += ((t > src.at<_Tp>(i,j)) || (std::abs(t-src.at<_Tp>(i,j)) < std::numeric_limits<float>::epsilon())) << n;
			}
		}
	}
}
static void elbp(InputArray src, OutputArray dst, int radius, int neighbors)
{
	int type = src.type();
	switch (type)
	{
	case CV_8SC1:   elbp_<char>(src,dst, radius, neighbors); break;
	case CV_8UC1:   elbp_<unsigned char>(src, dst, radius, neighbors); break;
	case CV_16SC1:  elbp_<short>(src,dst, radius, neighbors); break;
	case CV_16UC1:  elbp_<unsigned short>(src,dst, radius, neighbors); break;
	case CV_32SC1:  elbp_<int>(src,dst, radius, neighbors); break;
	case CV_32FC1:  elbp_<float>(src,dst, radius, neighbors); break;
	case CV_64FC1:  elbp_<double>(src,dst, radius, neighbors); break;
	default:
		string error_msg = format("Using Original Local Binary Patterns for feature extraction only works on single-channel images (given %d). Please pass the image data as a grayscale image!", type);
		CV_Error(CV_StsNotImplemented, error_msg);
		break;
	}
}
void ImgDscLBPH::init(const Image * pSrc, unsigned dim, int radius, int neighbors, int gridx, int gridy)
{
	if (!pSrc) return;
	const Image && img = dim ? Image(*pSrc, dim) : *pSrc;
	Mat ImgGS = img.GrayScale();
	Mat ImgLBP = elbp(ImgGS, radius, neighbors);
	mHistogram = SpatialHistogram(ImgLBP, 1<<neighbors, gridx, gridy, true);
}
Mat ImgDscLBPH::elbp(InputArray src, int radius, int neighbors)
{
	Mat dst;
	FaceMatch::elbp(src, dst, radius, neighbors);
	return dst;
}
Mat ImgDscLBPH::SpatialHistogram(InputArray _src, int numPatterns, int grid_x, int grid_y, bool /*normed*/)
{
	Mat src = _src.getMat();
	// calculate LBP patch size
	int width = src.cols/grid_x;
	int height = src.rows/grid_y;
	// allocate memory for the spatial histogram
	Mat result = Mat::zeros(grid_x * grid_y, numPatterns, CV_32FC1);
	// return matrix with zeros if no data was given
	if(src.empty())
		return result.reshape(1,1);
	// initial result_row
	int resultRowIdx = 0;
	// iterate through grid
	for(int i = 0; i < grid_y; i++)
	{
		for(int j = 0; j < grid_x; j++)
		{
			Mat src_cell = Mat(src, Range(i*height,(i+1)*height), Range(j*width,(j+1)*width));
			Mat cell_hist = histc(src_cell, 0, (numPatterns-1), true);
			// copy to the result matrix
			Mat result_row = result.row(resultRowIdx);
			cell_hist.reshape(1,1).convertTo(result_row, CV_32FC1);
			// increase row count in result matrix
			resultRowIdx++;
		}
	}
	// return result as reshaped feature vector
	return result.reshape(1,1);
}

static atomic<REALNUM> // optimized with CalTech
	sDistScale(0.01446), sDistScaleOM(0.6425);
REALNUM ImgDscLBPH::dist(const ImgDscBase & a, Matches*)const
{
	REALNUM hd=compareHist(mHistogram, dynamic_cast<const ImgDscLBPH &>(a).mHistogram, CV_COMP_CHISQR);
	return R2Unit(sDistScale*hd);
}
REALNUM ImgDscLBPH::getDistScale(bool OM)const{ return OM ? sDistScaleOM : sDistScale; }
void ImgDscLBPH::setDistScale(REALNUM s, bool OM)const{ if (OM) sDistScaleOM=s; else sDistScale=s; }
REALNUM dist(const ImgDscLBPH & a, const ImgDscLBPH & b){return a.dist(b);}
void ImgDscLBPH::print(ostream & s, const string & fmt)const
{
	ImgDscBase::print(s, fmt);
	unsigned VL = getVerbLevel();
	if (VL>1)
		s << "Histogram(" << mHistogram.rows << ',' << mHistogram.cols << ")";
	if (VL > 2)
		s << "=" << mHistogram;
}
} // namespace FaceMatch
