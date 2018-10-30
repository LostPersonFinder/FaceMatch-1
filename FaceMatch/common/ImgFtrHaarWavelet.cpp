
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
#include "ImgFtrHaarWavelet.h"
#include <vector>
#include <cstring>

using namespace std;

namespace FaceMatch
{
static const REALNUM SQRT2=sqrt(2.);

/// application-dependent weight sets
enum
{
	wtScanned,
	wtPainted,
	wtUniform,
	wtTableCnt
};

const REALNUM
	PRECOMPUTED_WEIGHTS[wtTableCnt][cWeightBinCnt][cWeightChannelCnt]=
	{ // from the paper
		{ // for scanned query images
			{5.00f, 19.21f, 34.37f},
			{0.83f, 1.26f, 0.36f},
			{1.01f, 0.44f, 0.45f},
			{0.52f, 0.53f, 0.14f},
			{0.47f, 0.28f, 0.18f},
			{0.3f , 0.14f, 0.27f}
		},
		{ // for painted query images
			{ 4.04f, 15.14f, 22.62f },
			{ 0.78f, 0.92f, 0.40f },
			{ 0.46f, 0.53f, 0.63f },
			{ 0.42f, 0.26f, 0.25f },
			{ 0.41f, 0.14f, 0.15f },
			{ 0.32f, 0.07f, 0.38f }
		},
		{ // uniform
			{1,1,1},
			{1,1,1},
			{1,1,1},
			{1,1,1},
			{1,1,1},
			{1,1,1}
		}
	};
static
const REALNUM
	WEIGHTS[cWeightBinCnt][cWeightChannelCnt]=
	{ // uniform
			{1,1,1},
			{1,1,1},
			{1,1,1},
			{1,1,1},
			{1,1,1},
			{1,1,1}
	};

ostream & printHex(ostream & s, const ImgFtrHaarWavelet & d)
{
	const ImgFtrHaarWavelet::TImgSig & sig=d.getSig();
	if (sig.empty()) return s;
	const byte CC=d.getChannelCount();
	const unsigned SL=d.getSignatureLength();
	for (byte c=0; c<CC; ++c)
	{
		s<<"c"<<(int)c<<": "<<sig[c].mAvgL<<"|";
		s << hex;
		const byte * f=(const byte*)sig[c].mBandSig.data();
		unsigned i=0, len=SL*sizeof(int);
		for (; i<len-1; ++i) s << (unsigned)f[i] << ":";
		s<<(unsigned)f[i]<<endl;
		s<<dec;
	}
	return s;
}
ostream & print(ostream & s, const ImgFtrHaarWavelet & d)
{
	const byte CC=d.getChannelCount();
	const unsigned SL=d.getSignatureLength();
	for (byte c=0; c<CC; ++c)
	{
		s<<"c"<<(int)c<<": "<<d.getSig()[c].mAvgL<<"|";
		for (unsigned i=0 ;i<SL; ++i) s<< (i==0 ? "" : ",")<<d.getSigVal(c,i);
		if (c<CC-1) s<<endl;
	}
	return s;
}
ostream & operator<<(ostream & s, const ImgFtrHaarWavelet & d)
{
	s<<"ImgFtrHaarWavelet: "<<endl;
	return print(s, d);
}

typedef vector<REALNUM> RealImageBand;
typedef Ptr<RealImageBand> PRealImageBand;
struct RealImageBandPack: public vector<PRealImageBand>
{
	/// Instantiate.
	RealImageBandPack(const byte ChnCnt, ///< color channel count
		const unsigned dim /// color band dimension/length
		)
	{
		resize(ChnCnt);
		for (byte c=0; c<ChnCnt; ++c) at(c) = new RealImageBand(dim);
	}
};
typedef Ptr<RealImageBandPack> PRealImageBandPack;

unsigned toReal(RealImageBand & RealBand, const MatUC & img)
{
	unsigned len=img.rows*img.cols;
	RealBand.resize(len);
	for (int i=0; i<len; ++i) RealBand[i]=img.data[i]/256.;
	return len;
}
PRealImageBandPack cvtRGB2Haar(const PRealImageBandPack & in, unsigned ImgDim)
// handle the color case
{
	const unsigned ImgArea=ImgDim*ImgDim;
	const RealImageBandPack &rIn=*in;
	const byte ChnCnt=in->size();
	CV_Assert(ChnCnt>=3); // color image
//--- compute YIQ
	PRealImageBandPack pYIQ = new RealImageBandPack(ChnCnt, ImgArea);
	RealImageBandPack &rYIQ = *pYIQ;
	xfmRGB2YIQ(rIn[BGRPixel::red]->data(), rIn[BGRPixel::green]->data(), rIn[BGRPixel::blue]->data(),
		rYIQ[0]->data(), rYIQ[1]->data(), rYIQ[2]->data(),
		ImgArea);
//--- compute 2D color Haar xform
	PRealImageBandPack pHaar = new RealImageBandPack(ChnCnt, ImgArea);
	RealImageBandPack &rHaar = *pHaar;
	xfmHaar2D(ImgDim,
		rYIQ[0]->data(), rYIQ[1]->data(), rYIQ[2]->data(),
		rHaar[0]->data(), rHaar[1]->data(), rHaar[2]->data());
	return pHaar;
}
PRealImageBand cvtGS2Haar(const PRealImageBand & in, unsigned ImgDim)
{
	PRealImageBand out=new RealImageBand(in->size());
	xfmHaar2D(ImgDim, in->data(), out->data());
	return out;
}
PRealImageBandPack splitConvert(const Image & img)
{
	if (img.height()!=img.width())
		throw Exception("input image should be square for Haar transform to work");
	Mat src=img.mx();
	const byte CC = src.channels();
	vector<MatUC> channels; cv::split(src, channels);
	const unsigned ImgDim = img.height();
	PRealImageBandPack pRImgBands = new RealImageBandPack(CC, ImgDim);
	RealImageBandPack & rRImgBands = *pRImgBands;
	for (byte c=0; c<CC; ++c) toReal(*rRImgBands[c], channels[c]);
	if (CC==1) rRImgBands[0]=cvtGS2Haar(rRImgBands[0], ImgDim);
	else pRImgBands=cvtRGB2Haar(pRImgBands, ImgDim);
	return pRImgBands;
}
static void truncChannel(const REALNUM * chan, int * sig, REALNUM & avgL, unsigned ImgArea, unsigned SigLen)
{
	avgL=chan[0];
//--- get SIG_LEN first largest by abs val elements of chan as the signature
	typedef REALNUM CoefType;
	typedef const CoefType* CoefTypePtr;
	++chan; // shift to the signature
	unsigned len=ImgArea-1;
	vector<CoefTypePtr> CoefPtrs(len); // prepare to sort pointers to coefs
	for (unsigned i=0; i<len; ++i) CoefPtrs[i]=chan+i;
	stable_sort(CoefPtrs.begin(), CoefPtrs.end(), DescendAbsVals); // descend positions by abs value of chand coefs
	for (unsigned i=0; i<SigLen && i<len; ++i)
	{
		CoefTypePtr p=CoefPtrs[i];
		sig[i]=sign(*p)*(p-chan); // signed position of next significant coefficient
	}
}
void ImgFtrHaarWavelet::create(const Image & image, unsigned SigLen)
{
	mImgDim = image.dim();
	PRealImageBandPack pBands=splitConvert(image);
	RealImageBandPack & rBands = *pBands;
	const byte CC=rBands.size();
	mSig.resize(CC);
//TODO: #pragma omp parallel for shared(rBands)
	for (byte c=0; c<CC; ++c)
	{
		mSig[c].mBandSig.resize(SigLen);
		truncChannel(rBands[c]->data(), mSig[c].mBandSig.data(), mSig[c].mAvgL, image.area(), SigLen);
	}
	/* TODO:
	if (getVerbLevel()>2)
	{
		imOut("ImgFtrHaarWavelet.jpg", image.mx());
		// TODO: show/store haar and signature
	}*/
}

const Mat ImgFtrHaarWavelet::getVectors()const
{
	typedef REALNUM T;
	const static byte MaxCC=3;
	const byte CC=min(getChannelCount(), MaxCC);
	const unsigned
		SL=getSignatureLength(),
		len=MaxCC*(1+SL);
	Mat_<T> M(1, len);
	unsigned k=0;
	T* pData = (T*)M.data;
	for (byte c=0; c<CC && k<len; ++c)
	{
		const TChanSig & rCSig=mSig[c];
		pData[k++]=rCSig.mAvgL*0xFF;
		for (unsigned j=0; j<SL && k<len; ++j)
			pData[k++]=rCSig.mBandSig[j];
	}
	return M;
}

bool sameSig(const byte c, const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2)
{
	const ImgFtrHaarWavelet::TImgSig
		&is1=h1.getSig(),
		&is2=h2.getSig();
	if (is1[c].mAvgL!=is2[c].mAvgL) return false;
	const unsigned
		SL1=is1[c].mBandSig.size(),
		SL2=is2[c].mBandSig.size();
	if (SL1!=SL2) return false;
	const unsigned SigLen = min(SL1, SL2);
	for (unsigned i=0; i<SigLen; ++i)
		if (is1[c].mBandSig[i]!=is2[c].mBandSig[i]) return false;
	return true;
}

bool operator==(const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2)
{
	if (h1.mImgDim!=h2.mImgDim) return false;
	const byte CC1=h1.getChannelCount(), CC2=h2.getChannelCount();
	if (CC1!=CC2) return false;
	for (byte c=0; c<CC1; ++c) if (!sameSig(c, h1, h2)) return false;
	return true;
}

unsigned ImgFtrHaarWavelet::wbin(int val)const
{
	val=abs(val);
	div_t d = div(val, mImgDim);
	return std::min<unsigned>(cWeightBinCnt-1, std::max(d.quot, d.rem));
}

const REALNUM * ImgFtrHaarWavelet::getBinWeight(int val)const
{
	return WEIGHTS[wbin(val)];
}

REALNUM ImgFtrHaarWavelet::getWeightsAvgL(unsigned c)const{ return WEIGHTS[0][c]; }

REALNUM diffSig(const byte c, const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2)
{
	const ImgFtrHaarWavelet::TImgSig
		&is1=h1.getSig(),
		&is2=h2.getSig();
	REALNUM score=0;
	score += fabs(is1[c].mAvgL-is2[c].mAvgL) * h1.getWeightsAvgL(c); // WEIGHTS[0][c]
	const unsigned SigLen = min(is1[c].mBandSig.size(), is2[c].mBandSig.size());
	for (unsigned i=0; i<SigLen; ++i)
	{
		const int
			s1 = is1[c].mBandSig[i];
		if (s1 && s1==is2[c].mBandSig[i])
			score -= h1.getBinWeight(s1)[c]; // WEIGHTS[ImgBinLU[abs(s1)]][c];
	}
	return score;
}

REALNUM diff(const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2)
{
	const byte CC = min(h1.getChannelCount(), h2.getChannelCount());
	REALNUM score=0;
	for (byte c=0; c<CC; ++c) score += diffSig(c, h1, h2);
	return score;
}

/**
 * compute symmetric, normalized to [0,1] distance between two descriptors as
 * |1 - 2*diff(h1,h2)/(diff(h1,h1)+diff(h2,h2))|^2;
 */
inline REALNUM distOld(const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2)
{
	const REALNUM
		d=diff(h1,h2),
		a=diff(h1,h1),
		b=diff(h2,h2),
		s=a+b;
	if (s==0) return 1;
	const REALNUM v = 1-2*d/s;
	return min<REALNUM>(1, v*v);
}

static atomic<REALNUM> // optimized with CalTech.tiny
	sDistScale(.01);
REALNUM ImgFtrHaarWavelet::getDistScale(){return sDistScale;}
void ImgFtrHaarWavelet::setDistScale(REALNUM s){sDistScale=s;}

REALNUM ImgFtrHaarWavelet::dist(const ImgFtrHaarWavelet & rhs)const
{
	const ImgFtrHaarWavelet	&h1=*this, &h2=rhs;
	if (h1==h2) return 0;
	REALNUM d=R2Unit01(sDistScale*diff(h1,h2));
	return d;
}

void ImgFtrHaarWavelet::write(FileStorage & s)const
{
	s<<"ImgDim"<<(int)mImgDim;
	s<<"ImgFtrHaar"; mSig.write(s);
}
void ImgFtrHaarWavelet::read(const FileNode & n)
{
	n["ImgDim"]>>(int&)mImgDim;
	mSig.read(n["ImgFtrHaar"]);
}

/// Read channel signature from a file storage node.
void read(const FileNode & n, ///< file storage node
	ImgFtrHaarWavelet::TChanSig & v, ///< channel signature
	const ImgFtrHaarWavelet::TChanSig & def = ImgFtrHaarWavelet::TChanSig() ///< default channel signature
)
{
	n["AvgL"] >> v.mAvgL;
	n["BandSig"] >> v.mBandSig;
}

//===================================================================== helpers

static void printChannel(ostream & s,
	const char * Label,
	const unsigned dim,
	const REALNUM * ch,
	bool byElement = false)
{
	const unsigned area = dim*dim;
	s << Label << endl;
	if (byElement)
		for (unsigned r = 0; r < dim; ++r)
		{
			const unsigned rBase = r * dim;
			for (unsigned c = 0; c < dim; ++c)
				s << "[" << rBase + c << ":" << r << "," << c << "]="
						<< ch[rBase + c] << endl;
		}
	else
		for (unsigned i = 0; i < area; ++i)
			s << ' ' << ch[i];
	s << endl;
}

/**
 * Print image values
 * @param s	output stream
 * @param dim	image channel dimension in pixels
 * @param Label	text label
 * @param c1	input color channel
 * @param c2	input color channel
 * @param c3	input color channel
 */
void printImage(ostream & s,
	const unsigned dim,
	const char * Label, const REALNUM * c1,
	const REALNUM * c2, const REALNUM * c3)
{
	s << Label << endl;
	printChannel(s, "===c1:", dim, c1);
	printChannel(s, "===c2:", dim, c2);
	printChannel(s, "===c3:", dim, c3);
}

void xfmHaar2D(const unsigned dim,	const REALNUM * c, REALNUM * d)
{
	const unsigned area = dim*dim;
	memcpy(d, c, area*sizeof(REALNUM));
	vector<REALNUM> Ab1(dim);
	for (unsigned r=0; r<dim; ++r) // rows
	{
		unsigned
			h=dim, // must be a power of 2!
			base=r*dim;
		//--- A=A/sqrt(h)
		const REALNUM normFctr=1./sqrt((REALNUM)h);
		for (unsigned j=0; j<dim; ++j) d[base+j] *= normFctr;
		while(h>1)
		{
			h/=2;
			for (unsigned k=0; k<h; ++k)
			{
				const unsigned dk=2*k;
				//--- A'[k]=(A[2k]+A[2k+1])/sqrt(2)
				Ab1[k]=(d[base+dk]+d[base+dk+1])/SQRT2;
				//--- A'[h+k]=(A[2k]-A[2k+1])/sqrt(2)
				Ab1[h+k]=(d[base+dk]-d[base+dk+1])/SQRT2;
			}
			//--- A=A'
			const unsigned size=2*h*sizeof(REALNUM);
			memcpy(d+base, Ab1.data(), size);
		}
	}
	for (unsigned c=0; c<dim; ++c) // cols
	{
		unsigned h=dim; // must be a power of 2!
		//--- A=A/sqrt(h)
		const REALNUM normFctr=1./sqrt((REALNUM)h);
		for (unsigned j=0; j<dim; ++j)
		{
			const unsigned base=j*dim;
			d[base+c] *= normFctr;
		}
		while (h>1)
		{
			h/=2;
			for (unsigned k=0; k<h; ++k)
			{
				const unsigned
					dk=2*k,
					base=dk*dim,
					base1=base+dim; // (2k+1)*SCALED_LENGTH
				//--- A'[k]=(A[2k]+A[2k+1])/sqrt(2)
				Ab1[k]=(d[c+base]+d[c+base1])/SQRT2;
				//--- A'[h+k]=(A[2k]-A[2k+1])/sqrt(2)
				Ab1[h+k]=(d[c+base]-d[c+base1])/SQRT2;
			}
			//--- A=A'
			for (unsigned j=0; j<2*h; ++j)
			{
				const unsigned base=j*dim;
				d[base+c]=Ab1[j];
			}
		}
	}
}

/**
 * Perform 2D Haar transform
 * @param dim image dimension
 * @param c1	input color channel
 * @param c2	input color channel
 * @param c3	input color channel
 * @param d1	output Haar channel
 * @param d2	output Haar channel
 * @param d3	output Haar channel
 */
void xfmHaar2D
(
	const unsigned dim,
	const REALNUM * c1, const REALNUM * c2, const REALNUM * c3,
	REALNUM * d1, REALNUM * d2, REALNUM * d3
)
{
	const unsigned area = dim*dim;
	memcpy(d1, c1, area*sizeof(REALNUM));
	memcpy(d2, c2, area*sizeof(REALNUM));
	memcpy(d3, c3, area*sizeof(REALNUM));

	vector<REALNUM> // auto-disposable buffers
		Ab1v(dim),
		Ab2v(dim),
		Ab3v(dim);
	REALNUM // temp decomposition A'
		*Ab1 = &Ab1v.front(),
		*Ab2 = &Ab2v.front(),
		*Ab3 = &Ab3v.front();

	//--- decompose rows
	for (unsigned r=0; r<dim; ++r)
	{
		unsigned
			h=dim, // must be a power of 2!
			base=r*dim;
		//--- A=A/sqrt(h)
		const REALNUM normFctr=1./sqrt((REALNUM)h);
		for (unsigned j=0; j<dim; ++j)
		{
			d1[base+j] *= normFctr;
			d2[base+j] *= normFctr;
			d3[base+j] *= normFctr;
		}
		while(h>1)
		{
			h/=2;
			for (unsigned k=0; k<h; ++k)
			{
				const unsigned dk=2*k;
				//--- A'[k]=(A[2k]+A[2k+1])/sqrt(2)
				Ab1[k]=(d1[base+dk]+d1[base+dk+1])/SQRT2;
				Ab2[k]=(d2[base+dk]+d2[base+dk+1])/SQRT2;
				Ab3[k]=(d3[base+dk]+d3[base+dk+1])/SQRT2;
				//--- A'[h+k]=(A[2k]-A[2k+1])/sqrt(2)
				Ab1[h+k]=(d1[base+dk]-d1[base+dk+1])/SQRT2;
				Ab2[h+k]=(d2[base+dk]-d2[base+dk+1])/SQRT2;
				Ab3[h+k]=(d3[base+dk]-d3[base+dk+1])/SQRT2;
			}
			//--- A=A'
			const unsigned size=2*h*sizeof(REALNUM);
			memcpy(d1+base, Ab1, size);
			memcpy(d2+base, Ab2, size);
			memcpy(d3+base, Ab3, size);
		}
	}
	//--- decompose cols
	for (unsigned c=0; c<dim; ++c)
	{
		unsigned h=dim; // must be a power of 2!
		//--- A=A/sqrt(h)
		const REALNUM normFctr=1./sqrt((REALNUM)h);
		for (unsigned j=0; j<dim; ++j)
		{
			const unsigned base=j*dim;
			d1[base+c] *= normFctr;
			d2[base+c] *= normFctr;
			d3[base+c] *= normFctr;
		}
		while (h>1)
		{
			h/=2;
			for (unsigned k=0; k<h; ++k)
			{
				const unsigned
					dk=2*k,
					base=dk*dim,
					base1=base+dim; // (2k+1)*SCALED_LENGTH
				//--- A'[k]=(A[2k]+A[2k+1])/sqrt(2)
				Ab1[k]=(d1[c+base]+d1[c+base1])/SQRT2;
				Ab2[k]=(d2[c+base]+d2[c+base1])/SQRT2;
				Ab3[k]=(d3[c+base]+d3[c+base1])/SQRT2;
				//--- A'[h+k]=(A[2k]-A[2k+1])/sqrt(2)
				Ab1[h+k]=(d1[c+base]-d1[c+base1])/SQRT2;
				Ab2[h+k]=(d2[c+base]-d2[c+base1])/SQRT2;
				Ab3[h+k]=(d3[c+base]-d3[c+base1])/SQRT2;
			}
			//--- A=A'
			for (unsigned j=0; j<2*h; ++j)
			{
				const unsigned base=j*dim;
				d1[base+c]=Ab1[j];
				d2[base+c]=Ab2[j];
				d3[base+c]=Ab3[j];
			}
		}
	}
}

} // FaceMatch
