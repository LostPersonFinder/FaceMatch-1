
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

#pragma once // 2012-2016 (C) 

#include "SkinMapDetectorANN.h"
#include "ColorHistogram.h"
#include "cv_supp.h"
#include <string>

using namespace std;

namespace FaceMatch
{

/**
 * Compute the Mahalanobis norm for a color point.
 * @param p	input color point, e.g. RGB
 * @param mean	mean color value
 * @param iCov	inverse of the color covariance matrix
 * @return Mahalanobis norm for the given pixel
 */
inline REALNUM MahalanobisNorm(const Vec3d & p, const Vec3d & mean, const Mat & iCov)
{
	const Vec3d v = p-mean;
	const Mat y = iCov*Mat(v);
	return v.dot(y);
}
/**
 * Compute color likelihood for a given color point.
 * @param p	input color point/pixel
 * @param mean	mean color value
 * @param iCov	inverse of the color covariance matrix
 * @param div divisor for the Mahalanobis norm
 * @return color likelihood for the given pixel
 */
inline REALNUM ColorLikelihood(const Vec3d & p, const Vec3d & mean, const Mat & iCov, REALNUM div=4)
{
	REALNUM z=MahalanobisNorm(p, mean, iCov);
	return exp(-z/div);
}

/// smart pointer to ColorMapper
typedef Ptr<class ColorMapper> PColorMapper;

/// color map abstraction
class LIBDCL ColorMapper
{
	OMPSimpleLock mLock;
	friend class ColorMapperEnsemble;
	string mImgOutDir;
protected:
	/// a source image
	Image3b mSrcImg;
	/// a color likelihood map representation
	typedef MatReal ColorLikelihoodMap;
	/// a color likelihood map instance
	ColorLikelihoodMap mColorLikelihoodMap;
	/**
	 * Set the source image.
	 * @param img	an input image to reference and build a color map for
	 */
	virtual void setImage(const Image3b & img){mSrcImg=img;}
	/**
	 * Compute a color likelihood value.
	 * @param row	row index in the source image
	 * @param col	column index in the source image
	 * @return color likelihood value in [0,1]
	 */
	virtual REALNUM computeColorLikelihood(int row, int col)const{return 0;}
	/// Populate @see mColorLikelihoodMap
	virtual void computeColorLikelihoodMap();
	/**
	 * Clone the object.
	 * @param deep	perform a deep copy?
	 * @return a smart pointer to a copy of this object
	 */
	PColorMapper clone(bool deep=false)const; // TODO: re-engineer to be more thread-safe
	/// Initialize.
	void init(const Image3b & img, ///< input image
		const string & OutImgDir="" ///< diagnostics output directory
	);
public:
	ColorMapper(){}
	/**
	 * Apply morphological filters to the color-thresholded image.
	 * @param img	input image
	 * @return morphologically enhanced image
	 */
	static Mat morph(const Mat & img);
	/// Get connected components of the bitonal image.
	static void getCCRegions(FaceRegions & outLikelyRegions, ///< [in/out] likely connected regions as a collection of rectangles
		const Mat & bitonal, ///< [in] bitonal (black-and-white) image
		const string & ImgOutDir="" ///< optional image output directory for diagnostics
	);
	/**
	 * Get color likelihood map ranging in [0,1] at each point of the image.
	 * @return  real-valued color likelihood map
	 */
	const ColorLikelihoodMap & getColorLikelihoodMap()const{return mColorLikelihoodMap;}
	/**
	 * Compute likely color blobs.
	 * @param outLikelyRegions output likely skin regions
	 * @param skinThreshold	skin threshold value
	 * @param detectLandmarks detect face landmarks in each likely region based on color?
	 */
	void getLikelyColorBlobs(FaceRegions & outLikelyRegions, REALNUM skinThreshold=0.5, bool detectLandmarks=false);
	virtual ~ColorMapper(){}
	/// @return new image-specific instance
	PColorMapper create(const Image3b & img, ///< source color image
		const string & OutImgDir="" ///< diagnostics output directory
	);
	/**
	 * Integrate/sum the color likelihood map over the supplied region.
	 * @param fr	input region to integrate the color map over
	 * @return value of the integral/sum
	 */
	REALNUM integrate(const FaceRegion & fr)const;
	/**
	 * Locally color-enhance the input image using the color likelihood map.
	 * @param fr	given region to enhance image in
	 * @return a color-enhanced patch
	 */
	Mat enhance(const FaceRegion & fr)const;
	/**
	 * Threshold the color likelihood map.
	 * @param tol	tolerance in [0,1]; default is 0.5
	 * @return a thresholded/binarized map
	 */
	Mat threshold(REALNUM tol=0.5)const;
	/**
	 * Store the color map model in a file.
	 * @param FN output file name
	 */
	virtual void save(const string & FN)const{}
};

/// matrix of color samples
typedef Mat_<Vec3b> MatColorSample;

/// color mapper for a color space converted image
class LIBDCL ColorMapperCvtImg: public ColorMapper
{
protected:
	/// converted image
	Image3b mCnvImg;
	/// OpenCV color space conversion code used in cv::cvtColor
	int mColorConversionCode;
	/**
	 * Set source image.
	 * @param img	source image
	 */
	virtual void setImage(const Image3b & img)
	{
		ColorMapper::setImage(img);
		mCnvImg=convert(img, mColorConversionCode);
	}
	/**
	 * Set color conversion code.
	 * @param ColorSpace target color space
	 */
	void setColorConversionCode(const string & ColorSpace);
public:
	/**
	 * Instantiate.
	 * @param ColorCnvCode OpenCV color conversion code used in cv::cvtColor
	 */
	ColorMapperCvtImg(int ColorCnvCode=0): mColorConversionCode(ColorCnvCode) {}
	/**
	 * Get color conversion code as used in cv::cvtColor.
	 * @return color conversion code used in cv::cvtColor
	 */
	int getColorConversionCode()const{ return mColorConversionCode; }
};

/// statistical parametric color map based on multivariate normal distribution
class LIBDCL ColorMapperStat: public ColorMapperCvtImg
{
	Vec3b mMean; // TODO: use real-valued
	Mat mCov, mCovInv; // TODO: keep only the inverse
protected:
	virtual REALNUM computeColorLikelihood(int row, int col)const override
	{
		return ColorLikelihood(mCnvImg(row, col), mMean, mCovInv);
	}
public:
	/**
	 * Instantiate.
	 * @param ColorSamples	a collection of color samples
	 * @param ColorConversionCode	color space conversion code as defined by OpenCV
	 */
	ColorMapperStat(const MatColorSample & ColorSamples, int ColorConversionCode=0);
	/**
	 * Instantiate.
	 * @param FN	input file name to instantiate from
	 */
	ColorMapperStat(const string & FN);
	virtual void save(const string & FN)const override;
};

/// histogram based (parameterless) color map
class LIBDCL ColorMapperHist: public ColorMapperCvtImg
{
	ColorHistogram mColorHist;
	virtual REALNUM computeColorLikelihood(int row, int col)const override
	{
		return mColorHist.getSkinLikelihood(mCnvImg(row,col));
	}
public:
	/**
	 * Instantiate.
	 * @param FN	input file name to load an instance from
	 */
	ColorMapperHist(const string & FN);
	virtual void save(const string & FN)const override;
};

/// artificial neural network (ANN) based color map
class LIBDCL ColorMapperANN: public ColorMapper
{
	SkinMapDetectorANN mSMDANN;
protected:
	virtual REALNUM computeColorLikelihood(int row, int col)const override
	{
		return mSMDANN.getSkinLikelihood(mSrcImg(row,col));
	}
	virtual void computeColorLikelihoodMap() override;
public:
	/// Instantiate from a binary file.
	ColorMapperANN(const string & annFN, ///< input file name (typically with ext=.yml) to load an instance from
		bool preferGPU = false ///< prefer GPU over CPU, when possible?
	): mSMDANN(annFN, preferGPU) {}
};

/// skin mapper ensemble
class LIBDCL ColorMapperEnsemble: public ColorMapper
{
	typedef ColorMapper TBase;
	struct WeightedColorMapper
	{
		string kind;
		PColorMapper pColorMapper;
		REALNUM weight;
		WeightedColorMapper(const string & k, PColorMapper p, REALNUM w) : kind(k), pColorMapper(p), weight(w) {}
	};
	typedef vector<WeightedColorMapper> WeightedColorMappers;
	WeightedColorMappers mWeightedColorMappers;
protected:
	virtual void setImage(const Image3b & img)override;
	virtual void computeColorLikelihoodMap()override;
public:
	/// Instantiate.
	ColorMapperEnsemble(const string & FN, bool preferGPU = false);
};

} // namespace FaceMatch
