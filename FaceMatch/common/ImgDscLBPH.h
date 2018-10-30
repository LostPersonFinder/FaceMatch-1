
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

#pragma once // 2013-2014 (C) 

#include "Image.h"
#include "ImgDscBase.h"

namespace FaceMatch
{
/// Local Binary Pattern Histogram (LBPH) based image descriptor
class LIBDCL ImgDscLBPH: public ImgDscBase
{
	Mat mHistogram;
	static Mat elbp(InputArray src, int radius, int neighbors);
	static Mat SpatialHistogram(InputArray _src, int numPatterns, int grid_x, int grid_y, bool /*normed*/);
	void init(const Image * pSrc=0, unsigned dim=DefaultFacePatchDim, int radius=1, int neighbors=8, int gridx=8, int gridy=8);
public:
	/// Instantiate.
	ImgDscLBPH(istream & s /**< input binary stream to read the instance from */ )
	{
		cv::read(s, mHistogram);
	}
	/// Instantiate.
	ImgDscLBPH(const FileNode & fn /**< input OpenCV file storage node to read the instance from */)
	{
		cv::read(fn["Histogram"], mHistogram);
		if (mHistogram.empty()) throw Exception("no histogram loaded for "+fn.name());
	}
	/// Instantiate.
	ImgDscLBPH(const Image & src, ///< source image
		unsigned dim=DefaultFacePatchDim, ///< normalized image/patch diameter in pixels
		int radius=1, ///< LBPH radius
		int neighbors=8, ///< LBPH neighbors
		int gridx=8, ///< LBPH gridx
		int gridy=8 ///< LBPH gridy
		)
	{
		init(&src, dim, radius, neighbors, gridx, gridy);
	}
	/// Instantiate.
	ImgDscLBPH(Image & img, ///< modifiable source image
		unsigned dim=DefaultFacePatchDim, ///< normalized image/patch diameter in pixels
		int radius=1, ///< LBPH radius
		int neighbors=8, ///< LBPH neighbors
		int gridx=8, ///< LBPH gridx
		int gridy=8 ///< LBPH gridy
		)
	{
		img=Image(img, dim);
		init(&img, dim, radius, neighbors, gridx, gridy);
	}
	/// Instantiate.
	ImgDscLBPH(const Image * pSrc=0, ///< source image pointer
		unsigned dim=DefaultFacePatchDim, ///< normalized image/patch diameter in pixels
		int radius=1, ///< LBPH radius
		int neighbors=8, ///< LBPH neighbors
		int gridx=8, ///< LBPH gridx
		int gridy=8 ///< LBPH gridy
		)
	{
		init(pSrc, dim, radius, neighbors, gridx, gridy);
	}
	virtual const Mat getVectors()const override { return mHistogram; }
	virtual REALNUM dist(const ImgDscBase & a, Matches * matches=0)const override;
	virtual REALNUM getDistScale(bool OM=false)const override;
	virtual void setDistScale(REALNUM s, bool OM=false)const override;
	virtual bool empty()const override
	{
		return mHistogram.empty();
	}
	virtual void write(ostream & s)const override
	{
		cv::write(s, mHistogram);
	}
	virtual void write(FileStorage& fs)const override
	{
		ImgDscBase::write(fs);
		cv::write(fs, "Histogram", mHistogram);
	}
	/// @return descriptor type
	static const string & Type()
	{
		StaticLkdCtor const string type="ImgDscLBPH";
		return type;
	}
	virtual const string & getType()const override {return Type();}
	virtual void print(ostream & s, const string & fmt = "")const override;
};
} // namespace FaceMatch
