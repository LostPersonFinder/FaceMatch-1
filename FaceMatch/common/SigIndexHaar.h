
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

#include "ImageDescriptorIndex.h"
#include "ImgDscHaar.h"
#include <list>
#include <iostream>

using namespace std;

namespace FaceMatch
{

/**
 * Haar signature index that tracks image repository image signatures allowing efficient image match
 * @param ImgDscHaar	Haar wavelet based image descriptor
 */
template<class ImgDscHaar>
class LIBDCL SigIndexHaar: public ImageDescriptorIndex<ImgDscHaar>
{
	typedef ImageDescriptorIndex<ImgDscHaar> TBase;
	using typename TBase::PDSC;

	class Closer2
	{
		typedef pair<string, const ImgDscBase*> ELEM;
		const ImgDscHaar & mQDesc;
		const REALNUM mSelfScore;
	public:
		Closer2(const ImgDscHaar & d): mQDesc(d), mSelfScore(diff(mQDesc, mQDesc)) {}
		bool operator()(const ELEM & lhs, const ELEM & rhs)
		{
			const ImgDscHaar
				&a=(const ImgDscHaar &)*lhs.second,
				&b=(const ImgDscHaar &)*rhs.second;
			return fabs(mSelfScore-diff(mQDesc, a)) < fabs(mSelfScore-diff(mQDesc, b));
		}
	};
public:
	using typename TBase::const_iterator;
	/// Instantiate.
	SigIndexHaar(unsigned dim, ///< normalized image/face diameter in pixels
		unsigned flags ///< flags/options
		): TBase(dim, flags&~dm){}
	/// Instantiate.
	SigIndexHaar(const string & NdxFN="", ///< index file name
		unsigned flags=0 ///< options
		): TBase("", flags&~dm)
	{
		if (!NdxFN.empty()) this->load(NdxFN); // calling TBase(NdxFN) does not work
	}
	/// Instantiate.
	SigIndexHaar(istream & s, ///< input stream
		unsigned flags=0 ///< options
		): TBase(s, flags&~dm) {}
	virtual ~SigIndexHaar() {}
	/**
	 * Find the Haar descriptor in the index closest to the given
	 * @param d input Haar descriptor
	 * @return closest image file name
	 */
	string findClosest(const ImgDscHaar & d)const
	{
		FTIMELOG
			Closer2 closer2(d);
		const_iterator hgi = min_element(this->begin(), this->end(), closer2);
		if (hgi == this->end()) return nemo;
		const ImgDscHaar& r = dynamic_cast<const ImgDscHaar&>(*hgi->second);
		return similar(d, r) ? hgi->first : nemo;
	}
	/// @return corresponding Haar descriptor; empty if none found
	const PDSC find(const string & imgFN /**< input file name*/ )const
	{
		const_iterator it = TBase::find(imgFN);
		if (it == this->end())
			return 0;
		else return (PDSC&)it->second;
	}
	/**
	 * Verify that the given image (specified by file name) is similar to the given Haar descriptor.
	 * @param imgFN input image file name
	 * @param d Haar descriptor to verify the similarity to
	 * @return are they similar?
	 */
	bool verifySimilar(const string & imgFN, const ImgDscHaar & d)const
	{
		const PDSC imgHD = find(imgFN);
		return imgHD && similar(d, *imgHD);
	}
	/**
	 * Output signature index to a text stream.
	 * @param os	output text stream
	 * @param ndx	index to output
	 * @return output stream
	 */
	LIBDCL friend ostream & operator<<(ostream & os, const SigIndexHaar & ndx)
	{
		for (SigIndexHaar::const_iterator it = ndx.begin(); it!=ndx.end(); ++it)
			os<<it->first<<'\t'<<*it->second<<endl;
		return os;
	}
};

/// HAAR signature index for the whole images
typedef SigIndexHaar<ImgDscHaarWhole> SigIndexHaarWhole;
/// HAAR signature index for the face regions
typedef SigIndexHaar<ImgDscHaarFace> SigIndexHaarFace;

} // FaceMatch
