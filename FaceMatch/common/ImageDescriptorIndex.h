
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

#pragma once // 2011-2017 (C) 

#include "ImageDescriptorIndexBase.h"

using namespace std;
using namespace cv;

namespace FaceMatch
{

/**
 * \brief image descriptor index template
 *
 * All FaceMatch specific image descriptor index types are derived from this template
 * with image descriptor type (typically derived from ImgDscBase) as its type parameter
 */
template<class DSC>
class LIBDCL ImageDescriptorIndex: public ImageDescriptorIndexBase
{
	unsigned mImgNormDim;
protected:
	/// safe pointer to image descriptor
	typedef Ptr<DSC> PDSC;
	/// image descriptor index base
	typedef ImageDescriptorIndexBase TBase;

//=== helpers
	/// \return scored lines for a given query descriptor
	virtual PScoredLines queryDist(const string & RgnID, ///< image/face region ID
		const ImgDscBase & RgnDsc, ///< query image descriptor
		const REALNUM MatchT, ///< query threshold
		bool skipSelfMatch=false ///< skip self-match?
		)const override
	{
		if (RgnDsc.getType()!=DSC::Type())
			throw Exception("got query descriptor "+RgnDsc.getType()+" while expecting "+DSC::Type());
		return TBase::queryDist(RgnID, RgnDsc, MatchT, skipSelfMatch);
	}
public:
	/// Instantiate.
	ImageDescriptorIndex(const string & NdxFN, ///< index file name
		unsigned flags ///< flags/options
		): ImageDescriptorIndexBase(DSC::Type(), flags),
		mImgNormDim(DefaultFacePatchDim)
	{
		if (!NdxFN.empty()) load(NdxFN); // loading in the base does not work
	}
	/// Instantiate.
	ImageDescriptorIndex(unsigned ImgNormDim, ///< normalized image diameter
		unsigned flags ///< flags/options
		): ImageDescriptorIndexBase(DSC::Type(), flags), mImgNormDim(ImgNormDim){}
	/// Instantiate.
	ImageDescriptorIndex(istream & s, ///< input stream
		unsigned flags=0 ///< flags/options
		): ImageDescriptorIndexBase(DSC::Type(), flags), mImgNormDim(DefaultFacePatchDim)
	{
		read(s); // reading in the base may not work
	}
	/// Destroy.
	virtual ~ImageDescriptorIndex(){}
	/// \return smart pointer to a new descriptor
	virtual PImgDscBase newDescriptor(const Image * pImg=0, ///< source image pointer
		unsigned dim=0 ///< image diameter to scale the source to
		)const override
	{	TIMELOG("ImageDescriptorIndex::newDescriptor::img")
		return new DSC(pImg, dim ? dim : mImgNormDim);
	}
	/// \return smart pointer to a new descriptor
	virtual PImgDscBase newDescriptor(istream & s /**< input stream */)const override
	{	TIMELOG("ImageDescriptorIndex::newDescriptor::strm")
		return new DSC(s);
	}
	/// \return smart pointer to a new descriptor
	virtual PImgDscBase newDescriptor(const FileNode & dn /**< input file storage node */)const override
	{
		return new DSC(dn);
	}
};

} // namespace FaceMatch
