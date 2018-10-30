
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
#include "dcl.h"
#include "ColorMapper.h"

namespace FaceMatch
{

/// color map encapsulation and management
class LIBDCL ColorMapManager
{
	PColorMapper mClrMpr;
	REALNUM mColorMassT, mColorLikelihoodT;
	PColorMapper newMapper(const MatColorSample & ColorSamples);
public:
	/// Instantiate. @see load
	ColorMapManager(const string & ColorMapperKind, ///< color map kinds: Stat, Hist, ANN
		const string & ParamFN, ///< model/parameters file name
		REALNUM ColorMassT=0.25, ///< minimum color mass threshold
		REALNUM ColorLikelihoodT=0.5, ///< color likelihood threshold
		bool bPreferGPU=false ///< prefer GPU, when available?
		):
		mColorMassT(ColorMassT), mColorLikelihoodT(ColorLikelihoodT)
	{
		load(ColorMapperKind, ParamFN, ColorMassT, ColorLikelihoodT, bPreferGPU);
	}
	/**
	 * Load an instance from a parameter/configuration file.
	 * @param ColorMapperKind	color map kinds: Stat, Hist, ANN
	 * @param ParamFN	parameters file name: XML/YML OpenCV storage or a binary file with any extension
	 * @param ColorMassT	color mass threshold
	 * @param ColorLikelihoodT	color likelihood threshold
	 * @param bPreferGPU	prefer GPU, when available?
	 */
	void load(const string & ColorMapperKind, const string & ParamFN, REALNUM ColorMassT = 0.25, REALNUM ColorLikelihoodT = 0.5, bool bPreferGPU = false);
	/**
	 * Get color mass.
	 * @return	color mass
	 */
	REALNUM getColorMassT()const{return mColorMassT;}
	/**
	 * Get color likelihood.
	 * @return color likelihood in [0,1]
	 */
	REALNUM getColorLikelihoodT()const{return mColorLikelihoodT;}
	/**
	 * Initialize.
	 * @param ColorSamples a matrix of color samples: each row is a sample
	 */
	void init(const MatColorSample & ColorSamples)
	{
		mClrMpr=newMapper(ColorSamples);
	}
	/// @return initialized color map pointer
	PColorMapper create(const Mat & img, ///< input color image
		const MatColorSample & ColorSamples, ///< Nx3 array of color samples
		const string & OutImgDir="" ///< diagnostics output directory
	);
	/// @return initialized color map pointer
	PColorMapper create(const Mat & img, ///< source image
		const string & OutImgDir="" ///< diagnostics output directory
		)
	{
		return mClrMpr ? mClrMpr->create(img, OutImgDir) : 0;
	}
	/// Clear the color map.
	void clear(){ mClrMpr=0; }
	/**
	 * Is the color map empty?
	 * @return true if the color map is empty, false otherwise.
	 */
	bool empty()const{return !mClrMpr;}
	/**
	 * Store the instance in a file.
	 * @param FN	output file name
	 */
	void save(const string & FN)const
	{
		if (mClrMpr) mClrMpr->save(FN);
	}
};

} // namespace FaceMatch
