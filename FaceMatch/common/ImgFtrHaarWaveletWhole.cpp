
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
#include "ImgFtrHaarWaveletWhole.h"

namespace FaceMatch
{

const char tb='\t';

static const unsigned SIG_LEN = 128;

static REALNUM
	WEIGHTS[cWeightBinCnt][cWeightChannelCnt]=
	{
		{4.04f, 15.14f, 22.62f},
		{0.78f, 0.92f, 0.40f},
		{0.46f, 0.53f, 0.63f},
		{0.42f, 0.26f, 0.25f},
		{0.41f, 0.14f, 0.15f},
		{0.32f, 0.07f, 0.38f}
	}; // TODO: make adjustable via training

REALNUM ImgFtrHaarWaveletWhole::getWeightsAvgL(unsigned c)const{ return WEIGHTS[0][c]; }

REALNUM * ImgFtrHaarWaveletWhole::getWeights()
{
	return (REALNUM*)WEIGHTS;
}

const REALNUM * ImgFtrHaarWaveletWhole::getBinWeight(int val)const
{
	return WEIGHTS[wbin(val)];
}

//===================================================================== helpers

static void printChannel(ostream & s,
	const char * Label, const REALNUM * ch,
	unsigned dim, bool elementWise = false)
{
	const unsigned ScaledImageArea=dim*dim;
	s << Label << endl;
	if (elementWise)
		for (unsigned r=0; r<dim; ++r)
		{
			const unsigned rBase = r * dim;
			for (unsigned c=0; c<dim; ++c)
				s << "[" << rBase + c << ":" << r << "," << c << "]="
						<< ch[rBase + c] << endl;
		}
	else
		for (unsigned i = 0; i < ScaledImageArea; ++i)
			s << ' ' << ch[i];
	s << endl;
}

/// Print image values to a text stream.
void printImage(ostream & s, ///< output text stream
	const char * Label, ///< output text label
	unsigned dim, ///< image diameter
	const REALNUM * c1, ///< color channel
	const REALNUM * c2, ///< color channel
	const REALNUM * c3 ///< color channel
)
{
	if (Label) s << Label << endl;
	printChannel(s, "===c1:", c1, dim);
	printChannel(s, "===c2:", c2, dim);
	printChannel(s, "===c3:", c3, dim);
}

//===================================================================== methods

typedef Mat_<uchar> MatUC;

void ImgFtrHaarWaveletWhole::compute(const Image & img, unsigned dim)
{
//--- resize image for wavelet transform
	TBase::create(Image(img, dim, dim), SIG_LEN);
}

} // FaceMatch
