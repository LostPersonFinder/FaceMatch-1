
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

#include "Image.h"
#include "math_supp.h"
#include <vector>

using namespace std;

namespace FaceMatch
{

const unsigned
/// weight color channel count
	cWeightChannelCnt = 3,
/// weight bin count
	cWeightBinCnt = 6;

/// wavelet image similarity for near-duplicate detection
const REALNUM cImgFtrHaarWaveletSimilarityT=0.1; // TODO: config/param

/// image descriptor based on color Haar wavelet
class LIBDCL ImgFtrHaarWavelet
{
public:
	/// color band signature type
	typedef vector<int> TBandSig;
	/// color channel signature
	struct TChanSig
	{
		/** color band average value */
		REALNUM mAvgL;
		/** color band signature */
		TBandSig mBandSig;
		/** Instantiate. */
		TChanSig(): mAvgL(0) {}
		/**
		 * Read from a binary stream.
		 * @param s input binary stream
		 */
		void read(istream & s)
		{
			readSimple(s, mAvgL);
			readVector(s, mBandSig);
		}
		/// Read from an OpenCV file storage node.
		LIBDCL friend void read(const FileNode & n, ///< input node
			TChanSig & v, ///< output channel signature (descriptor)
			const TChanSig & def ///< default value for the channel signature
		);
		/**
		 * Write to a binary stream.
		 * @param s output binary stream
		 */
		void write(ostream & s)const
		{
			writeSimple(s, mAvgL);
			writeVector(s, mBandSig);
		}
		/**
		 * Write to an OpenCV file storage.
		 * @param s output file storage
		 * @param cs channel signature (descriptor)
		 */
		friend void write(FileStorage & s, const TChanSig & cs)
		{
			cs.write(s);
		}
		/**
		 * Write to an OpenCV file storage.
		 * @param s output file storage
		 */
		void write(FileStorage & s)const
		{
			s<<"{";
				s<<"AvgL"<<mAvgL;
				s<<"BandSig"<<mBandSig;
			s<<"}";
		}
	};
	/// image signature/descriptor
	struct TImgSig: public vector<TChanSig>
	{
		/**
		 * Read from a binary stream.
		 * @param s input binary stream
		 */
		void read(istream & s)
		{
			byte CC=0; readSimple(s, CC);
			resize(CC);
			for (byte c=0; c<CC; ++c) at(c).read(s);
		}
		/**
		 * Read from an OpenCV file storage node.
		 * @param n input node
		 */
		void read(const FileNode & n)
		{
			byte CC=0; n["ChCnt"]>>CC;
			resize(CC);
			for (byte c=0; c<CC; ++c)
				n[format("ChanSig%d", c)]>>at(c);
		}
		/**
		 * Write to a binary stream.
		 * @param s output binary stream
		 */
		void write(ostream & s)const
		{
			byte CC=size(); writeSimple(s, CC);
			for (byte c=0; c<CC; ++c) at(c).write(s);
		}
		/**
		 * Write to an OpenCV file storage.
		 * @param s output file storage
		 */
		void write(FileStorage & s)const
		{
			s<<"{";
				byte CC=size(); s<<"ChCnt"<<CC;
				for (byte c=0; c<CC; ++c)
				{
					s<<format("ChanSig%d", c);
					at(c).write(s);
				}
			s<<"}";
		}
	};

protected:
	/** normalized image diameter */
	unsigned mImgDim;
	/** image signature/descriptor */
	TImgSig mSig;
	/**
	 * Get weight bin to be used during signature matching.
	 * @param val input value
	 * @return weight bin
	 */
	unsigned wbin(int val)const;

public:
	/** Instantiate. */
	ImgFtrHaarWavelet(): mImgDim(0) {}
	/** Destroy. */
	virtual ~ImgFtrHaarWavelet(){}
	/**
	 * Get image signature/descriptor.
	 * @return a reference to this image signature.
	 */
	const TImgSig & getSig()const{return mSig;}
	/**
	 * Get image signature/descriptor as an OpenCV matrix object.
	 * @return image signature/descriptor as an OpenCV matrix object
	 */
	const Mat getVectors()const;
	/**
	 * Get signature/descriptor size/length.
	 * @return signature/descriptor size/length
	 */
	unsigned size()const{return mSig.size();}
	/**
	 * Get color channel count.
	 * @return color channel count
	 */
	byte getChannelCount()const{return mSig.size();}
	/**
	 * Get wavelet signature buffer length.
	 * @return wavelet signature buffer length
	 */
	unsigned getSignatureLength()const{return getChannelCount() ? mSig[0].mBandSig.size() : 0;}
	/**
	 * Get the image signature value for the specified color channel and offset
	 * @param c color channel
	 * @param i value offset
	 * @return signature value
	 */
	int getSigVal(unsigned c, unsigned i)const{ return mSig[c].mBandSig[i]; }
	/**
	 * Read Haar wavelet feature from a binary stream.
	 * @param s binary input stream
	 */
	virtual void read(istream & s)
	{
		readSimple(s, mImgDim);
		mSig.read(s);
	}
	/**
	 * Write feature to a binary stream.
	 * @param s binary output stream
	 */
	virtual void write(ostream & s)const
	{
		writeSimple(s, mImgDim);
		mSig.write(s);
	}
	/**
	 * Read from an OpenCV XML/YML file storage.
	 * @param s	XML/YML storage stream
	 */
	virtual void read(const FileNode & s);
	/**
	 * Write to an OpenCV XML/YML file storage.
	 * @param s	XML/YML storage stream
	 */
	virtual void write(FileStorage & s)const;
	/**
	 * Compute Haar wavelet feature from an image.
	 * @param image  input image
	 * @param SigLen image signature/descriptor length
	 * @see ImageFeature
	 */
	virtual void create(const Image & image, unsigned SigLen);
	
	/**
	 * Get weights average value for the channel band.
	 * @param c	channel
	 * @return weights average value for the channel
	 */
	virtual REALNUM getWeightsAvgL(unsigned c)const;
	/**
	 * Get bin weight for the value to be used during signature/descriptor matching.
	 * @param val	input value from a signature
	 * @return bin weight pointer
	 */
	virtual const REALNUM * getBinWeight(int val)const;

	/**
	 * Compute the distance to the given descriptor.
	 * @return real-valued distance to the given descriptor in [0,1]
	 */
	REALNUM dist(const ImgFtrHaarWavelet & rhs)const;
	/// @return distance scale
	static REALNUM getDistScale();
	/// set distance scale
	static void setDistScale(REALNUM s /**< scale value */);

	/**
	 * Output Haar wavelet descriptor to a text stream.
	 * @param s	output text stream
	 * @param d Haar wavelet descriptor
	 * @return output text stream
	 */
	friend LIBDCL ostream & operator<<(ostream & s, const ImgFtrHaarWavelet & d);
	/**
	 * Are the two descriptors equal?
	 * @param h1 first descriptor
	 * @param h2 second descriptor
	 * @return true if they are equal, false otherwise
	 */
	friend LIBDCL bool operator==(const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2);
	/**
	 * Compute a difference between two descriptors.
	 * @param h1 first descriptor
	 * @param h2 second descriptor
	 * @return real-valued difference (not in [0,1]), which can be negative.
	 */
	friend LIBDCL REALNUM diff(const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2);
	/**
	 * Compute the distance between the given descriptors.
	 * @param h1 first descriptor
	 * @param h2 second descriptor
	 * @return real-valued distance to the given descriptor in [0,1]
	 */
	friend REALNUM dist(const ImgFtrHaarWavelet & h1, const ImgFtrHaarWavelet & h2){return h1.dist(h2);}
	/**
	 * Are the two Haar wavelet features similar?
	 * @param f1  input Haar wavelet
	 * @param f2  input Haar wavelet
	 * @param tol real-valued tolerance in [0,1]
	 * @return  true, when similar; false, otherwise
	 */
	friend bool similar(const ImgFtrHaarWavelet & f1, const ImgFtrHaarWavelet & f2, const REALNUM tol=cImgFtrHaarWaveletSimilarityT)
	{
		return f1.dist(f2)<tol;
	}
};

/**
 * Perform Haar 2D transform.
 * @param dim length of input channel band
 * @param c input channel band pointer
 * @param d output channel band pointer
 */
void xfmHaar2D(const unsigned dim,	const REALNUM * c, REALNUM * d);
/**
 * Perform Haar 2D transform.
 * @param dim length of the input channel bands
 * @param c1 input channel band pointer
 * @param c2 input channel band pointer
 * @param c3 input channel band pointer
 * @param d1 output channel band pointer
 * @param d2 output channel band pointer
 * @param d3 output channel band pointer
 */
void xfmHaar2D
(
	const unsigned dim,
	const REALNUM * c1, const REALNUM * c2, const REALNUM * c3,
	REALNUM * d1, REALNUM * d2, REALNUM * d3
);
/**
 * Set signature/descriptor matching weights.
 * @param w weight array
 */
template<class T> inline
void setWeights(const REALNUM * w)
{
	unsigned dim=cWeightBinCnt*cWeightChannelCnt;
	copy(w, w+dim, T::getWeights());
}
/**
 * Write signature/descriptor matching weights to a stream.
 * @param s output binary stream
 */
template<class T> inline
void writeWeights(ostream & s)
{
	unsigned dim=cWeightBinCnt*cWeightChannelCnt;
	writeBulk(s, T::getWeights(), dim*sizeof(REALNUM));
}
/**
 * Read signature/descriptor matching weights from a stream.
 * @param s input binary stream
 */
template<class T> inline
void readWeights(istream & s)
{
	unsigned dim=cWeightBinCnt*cWeightChannelCnt;
	readBulk(s, T::getWeights(), dim*sizeof(REALNUM));
}
/**
 * Print signature/descriptor matching weights to a stream.
 * @param s output text stream
 */
template<class T> inline
void printWeights(ostream & s)
{
	REALNUM * W= T::getWeights();
	for (unsigned b=0; b<cWeightBinCnt; ++b)
	{
		for (unsigned c=0; c<cWeightChannelCnt; ++c)
			s<<'\t'<<W[b*cWeightBinCnt+c];
		s<<endl;
	}
}
/**
 * Scan signature/descriptor matching weights from a stream.
 * @param s input text stream
 */
template<class T> inline
void scanWeights(istream & s)
{
	REALNUM * W= T::getWeights();
	for (unsigned b=0; b<cWeightBinCnt; ++b)
		for (unsigned c=0; c<cWeightChannelCnt; ++c)
			s>>W[b*cWeightBinCnt+c];
}

/**
 * Compare two real numbers by absolute value.
 * The function can be used for sorting an array of real values.
 * @param a pointer to a real number
 * @param b pointer to a real number
 * @return true, if |*a|>|*b|; false, otherwise
 */
inline bool DescendAbsVals(const REALNUM * a, const REALNUM * b)
{
	return fabs(*a)>fabs(*b);
}

} // FaceMatch
