
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
#include "ImgDscMany.h"
#include "ImgDscMatcher.h"
#include <numeric>

namespace FaceMatch
{

/**
 * Compute the distance between two descriptors.
 * @param a	input descriptor
 * @param b	input descriptor
 * @return real-valued distance in [0,1]
 */
template<class T> inline
REALNUM DIST(const T & a, const T & b) { return a.dist(b); }

/**
 * Compute the distance between two base descriptors.
 * @param a	input descriptor
 * @param b	input descriptor
 * @return real-valued distance in [0,1]
 */
REALNUM distBase(const PImgDscBase a, const PImgDscBase b)
{
	return a->dist(*b);
}
/// \return combination of input operands, e.g. geometric mean
static REALNUM comb(REALNUM a, REALNUM b)
{
	return sqrt(a*b);
}

const list<string> ImgDscMany::sSubImgDscSeq = {"HAAR", "LBPH", "ORB", "SURF", "SIFT"};

void ImgDscMany::init(Image & ImgSrc, unsigned ImgNormDim)
{
	mImgDscPack.clear();
	for (const auto & cn : sSubImgDscSeq)
	{
		Image img = ImgSrc; // copy since creator may modify img
		add(newDescriptor(cn, img, &clog));
	}
}
ImgDscMany::ImgDscMany(const Image * pImgSrc, ///< input image pointer; if NULL, produce an empty descriptor
	unsigned ImgNormDim, ///< image region diameter in pixels
	unsigned size ///< number of sub-descriptors
) : mImgDscPack(size)
{
	if (!pImgSrc) return;
	Image img(*pImgSrc);
	init(img, ImgNormDim);
}
ImgDscMany::ImgDscMany(const FileNode & fn): ImgDscBase(fn)
{
	for (const auto & cn : fn)
		add(newDescriptor(cn.name(), cn, &clog));
}
ImgDscMany::ImgDscMany(istream & s): ImgDscBase(s)
{
	unsigned len=0; readSimple(s, len);
	for (unsigned i=0; i<len; ++i)
	{
		string type; std::read(s, type);
		add(newDescriptor(type, s, &clog));
	}
}

static atomic<REALNUM> mDistScaleMany(0.9565); // optimized with CalTech
REALNUM ImgDscMany::getDistScale(bool OM)const
{
	return mDistScaleMany;
}
void ImgDscMany::setDistScale(REALNUM s, bool OM)const
{
	mDistScaleMany=s;
}

REALNUM ImgDscMany::dist(const ImgDscBase & a, Matches*)const
{
	const ImgDscMany & scene = dynamic_cast<const ImgDscMany&>(a);
	REALNUM res = std::inner_product(mImgDscPack.begin(), mImgDscPack.end(),
		scene.mImgDscPack.begin(), (REALNUM)1,
		comb, distBase);
	return res*mDistScaleMany;
}

void ImgDscMany::print(ostream & s, const string &)const
{
	s<<endl; // newline before enumerating
	for (const auto & pDsc : mImgDscPack)
		s<<'\t'<<*pDsc<<endl;
}
void ImgDscMany::write(FileStorage& fs)const
{
	for (const auto & pDsc : mImgDscPack)
	{
		fs<<pDsc->getType()<<"{";
		pDsc->write(fs);
		fs<<"}";
	}
}
void ImgDscMany::write(ostream & s)const
{
	unsigned len=mImgDscPack.size();
	writeSimple(s, len);
	for (unsigned i=0; i<len; ++i)
	{
		std::write(s, mImgDscPack[i]->getType());
		mImgDscPack[i]->write(s);
	}
}
bool ImgDscMany::empty()const
{
	if (size()==0)
		return true;
	for (const auto & pDsc : mImgDscPack)
		if (!pDsc->empty()) return false;
	return true;
}

} // namespace FaceMatch
