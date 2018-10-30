
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

#pragma once // 2013-2017 (C) 

#include "Image.h"

namespace FaceMatch
{

/** \brief image region descriptor base for feature combination */
class LIBDCL ImgDscMany: public ImgDscBase
{
	typedef vector<PImgDscBase> ImgDscPack;
	ImgDscPack mImgDscPack;
	void init(Image & ImgSrc, ///< input image
		unsigned ImgNormDim ///< image region diameter in pixels
	);
public:
	static const list<string> sSubImgDscSeq;
	/// Instantiate.
	ImgDscMany(Image & ImgSrc, ///< input image
		unsigned ImgNormDim = DefaultFacePatchDim, ///< image region diameter in pixels
		unsigned size = 0 ///< number of sub-descriptors
	) : mImgDscPack(size) { init(ImgSrc, ImgNormDim); }
	/// Instantiate.
	ImgDscMany(const Image * pImgSrc = 0, ///< input image pointer; if NULL, produce an empty descriptor
		unsigned ImgNormDim = DefaultFacePatchDim, ///< image region diameter in pixels
		unsigned size = 0 ///< number of sub-descriptors
	);
	/// Instantiate.
	ImgDscMany(const FileNode & fn /**< input OpenCV file storage node*/);
	/// Instantiate.
	ImgDscMany(istream & s /**< input binary stream */);
	virtual ~ImgDscMany() {}
	/// @return number of descriptor types used in the ensemble
	unsigned size()const{return mImgDscPack.size();}
	virtual bool empty()const override;
	/**
	 * Get descriptor values as an OpenCV matrix
	 * @return a matrix of descriptor values
	 */
	virtual const Mat getVectors()const override
	{
		return Mat(); // TODO: implement
	}
	/**
	 * Get a reference to the n'th image descriptor.
	 * @param n	offset of of the descriptor
	 * @return reference to the n'th image descriptor
	 */
	PImgDscBase & operator[](unsigned n){ return mImgDscPack[n]; }
	/**
	 * Get a reference to the n'th image descriptor.
	 * @param n	offset of of the descriptor
	 * @return reference to the n'th image descriptor
	 */
	const PImgDscBase & operator[](unsigned n)const{ return mImgDscPack[n]; }
	/**
	 * Add an image descriptor to the ensemble.
	 * @param pd smart descriptor pointer to add
	 */
	void add(PImgDscBase pd)
	{
		mImgDscPack.push_back(pd);
	}

	virtual REALNUM dist(const ImgDscBase & a, Matches * matches=0)const override;
	virtual REALNUM getDistScale(bool OM=false)const override;
	virtual void setDistScale(REALNUM s, bool OM=false)const override;

	/**
	 * Specify static descriptor type as a string.
	 * @return	descriptor type
	 */
	static const string & Type()
	{
		StaticLkdCtor const string type="ImgDscMany"; // TODO: consider a simplification
		return type;
	}
	virtual const string & getType()const override{return Type();}
	virtual void print(ostream & s, const string & fmt)const override;
	virtual void write(FileStorage& fs)const override;
	virtual void write(ostream & s)const override;
};

} // namespace FaceMatch
