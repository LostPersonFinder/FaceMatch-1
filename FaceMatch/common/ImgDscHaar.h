
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

#include "ImgFtrHaarWaveletWhole.h"
#include "ImgFtrHaarWaveletFace.h"

namespace FaceMatch
{

/// Haar wavelet based image descriptor
template<class ImgFtrHaar>
class ImgDscHaar: public ImgDscBase
{
	ImgFtrHaar mImgFtrHaar;
public:
	/**
	 * Instantiate.
	 * @param img input image pointer; when NULL, produce an empty descriptor
	 * @param dim	normalized image/face diameter in pixels
	 */
	ImgDscHaar(const Image * img=0, unsigned dim=DefaultWholeImgDim)
	{
		if (!img) return;
		mImgFtrHaar.compute(*img, dim);
	}
	/**
	 * Instantiate.
	 * @param img input image reference
	 * @param dim	normalized image/face diameter in pixels
	 */
	ImgDscHaar(const Image & img, unsigned dim=DefaultWholeImgDim)
	{
		mImgFtrHaar.compute(img, dim);
	}
	/**
	 * Instantiate.
	 * @param fn OpenCV input file storage node
	 */
	ImgDscHaar(const FileNode & fn): ImgDscBase(fn)
	{
		mImgFtrHaar.read(fn);
	}
	/**
	 * Instantiate.
	 * @param s	input binary stream
	 */
	ImgDscHaar(istream & s): ImgDscBase(s)
	{
		mImgFtrHaar.read(s);
	}
	/**
	 * Instantiate.
	 * @param imgFN	input image file name for the future reference; the file is not loaded or parsed
	 * @param img	input image reference
	 */
	ImgDscHaar(const string & imgFN, const Image & img)
	{
		mImgFtrHaar.compute(img);
	}
	/**
	 * Instantiate.
	 * @param imgFN	input image file name; the file is read and image is loaded
	 */
	ImgDscHaar(const string & imgFN)
	{
		mImgFtrHaar.compute(Image(imgFN));
	}
	virtual ~ImgDscHaar(){}
	virtual const Mat getVectors()const override
	{
		return mImgFtrHaar.getVectors();
	}
	virtual bool empty()const override {return !mImgFtrHaar.size();}
	virtual const string & getType()const override {return ImgDscHaar::Type();}
	virtual void write(ostream & s) const override
	{
		ImgDscBase::write(s);
		mImgFtrHaar.write(s);
	}
	virtual void write(FileStorage & s)const override
	{
		ImgDscBase::write(s);
		mImgFtrHaar.write(s);
	}
	virtual void print(ostream & s, const string & fmt="")const override
	{
		ImgDscBase::print(s, fmt);
		unsigned VL = getVerbLevel();
		if (VL > 1)
			s << "Signature(" << mImgFtrHaar.size() << ',' << mImgFtrHaar.getSignatureLength() << ")";
		if (VL > 2)
			s << mImgFtrHaar;
	}
	virtual REALNUM dist(const ImgDscBase & a, Matches * matches=0)const override
	{
		const ImgDscHaar & rhs=dynamic_cast<const ImgDscHaar &>(a);
		return this->mImgFtrHaar.dist(rhs.mImgFtrHaar);
	}
	virtual REALNUM getDistScale(bool OM=false)const override {return this->mImgFtrHaar.getDistScale();}
	virtual void setDistScale(REALNUM s, bool OM=false)const override {this->mImgFtrHaar.setDistScale(s);}
	/// @return	descriptor type
	static const string & Type() { return ImgFtrHaar::Type(); }
	/**
	 * Output descriptor to a text stream.
	 * @param s	output text stream
	 * @param d	descriptor to output
	 * @return	output text stream
	 */
	friend ostream & operator<<(ostream & s, const ImgDscHaar & d)
	{
		d.print(s);
		return s;
	}
	/**
	 * Test two descriptors for similarity via distance threshold.
	 * @param d1	input descriptor
	 * @param d2	input descriptor
	 * @return	are they similar?
	 */
	friend bool similar(const ImgDscHaar & d1, const ImgDscHaar & d2)
	{
		return similar(d1.mImgFtrHaar, d2.mImgFtrHaar);
	}
	/**
	 * Compute non-normalized real-valued difference between two descriptors.
	 * @param d1	input descriptor
	 * @param d2	input descriptor
	 * @return real-valued difference between two descriptors
	 */
	friend REALNUM diff(const ImgDscHaar & d1, const ImgDscHaar & d2)
	{
		return diff(d1.mImgFtrHaar, d2.mImgFtrHaar);
	}
};

/// Haar wavelet based image descriptor for whole image
typedef ImgDscHaar<ImgFtrHaarWaveletWhole> ImgDscHaarWhole;
/// Haar wavelet based image descriptor for image/face regions
typedef ImgDscHaar<ImgFtrHaarWaveletFace> ImgDscHaarFace;

} // FaceMatch
