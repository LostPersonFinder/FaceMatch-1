
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

#pragma once // 2012-2015 (C) FaceMatch@NIH.gov

#include "SkinHIST.h"
#include <string>

using namespace std;

namespace FaceMatch
{

/// color histogram based classifier
class LIBDCL ColorHistogram: protected SkinHIST
{
	string mColorSpace;
	unsigned loadDataBuildHist(const string & FN, unsigned nMaxItemsToBeRead=0);
public:
	/**
	 * Instantiate.
	 * @param DataDim	data point dimensions; default is 3
	 * @param BinCount	number of bins in each dimension; default is 16
	 */
	ColorHistogram(unsigned DataDim=3, unsigned BinCount=16):
		SkinHIST(DataDim, BinCount)
	{
		/* TODO: create Bayes-rule based histogram for skin mapping by estimating
		 * 		p(s|c) = p(c|s)p(s)/p(c), where
		 * p(s|c) - probability of skin, given color
		 * p(c|s) - probability of color, given skin as a histogram from skin-labeled pixels
		 * p(c) - probability of color as a color distribution histogram from all data-set images
		 * p(s) - probability of skin can be estimated as the ratio skin/all pixels
		 * Alternatively, we can estimate p(s|c) as
		 * 		p(s|c) = p(c|s)p(s)/[p(c|s)p(s)+p(c|~s)p(~s)], where
		 * p(~s) - probability of non-skin can be estimated as the ratio non-skin/all pixels OR as 1-p(s)
		 * p(c|~s) - probability of color, given non-skin as a histogram from non-skin patches/images
		 */
	}
	/**
	 * Get color space used for the histogram.
	 * @return color space
	 */
	const string & getColorSpace()const{return mColorSpace;}
	/**
	 * Load color points data from a text file, each line of which specifies a tab separated list of color coordinates, e.g. R G B.
	 * @param FN	input file name
	 * @return number of imported color points
	 */
	unsigned loadData(const string & FN)
	{
		return loadDataBuildHist(FN);
	}
	/**
	 * Output the instance to an OpenCV file storage.
	 * @param fs	output file storage
	 */
	void write(FileStorage & fs)const;
	/**
	 * Input the instance from an OpenCV file storage.
	 * @param fs	input file storage
	 */
	void read(FileStorage & fs);
	/**
	 * Store the instance in a binary file.
	 * @param FN	output file name
	 */
	void save(const string & FN)const
	{
		SaveHistogramToFile(FN.c_str(), mColorSpace.c_str());
	}
	/**
	 * Load the instance from a binary file.
	 * @param FN	input file name
	 */
	void load(const string & FN)
	{
		mColorSpace=LoadHistogramFromFile(FN);
	}
	/**
	 * Load histogram from a tab-delimited text file.
	 * Each line in the input describes a tab-delimited list of bin indexes and the value.
	 * @param FN	input file name
	 */
	void inXYZV(const string & FN);
	/**
	 * Output the histogram values in [X Y Z V] tab separated tuples consumable by visualization tools such as Matlab.
	 * @param FN	output file name
	 */
	void outXYZV(const string & FN)const;
	/**
	 * Output a histogram instance to a text stream.
	 * @param s	output text stream
	 * @param hst	color histogram instance
	 * @return	output stream
	 */
	friend LIBDCL ostream & operator<<(ostream & s, const ColorHistogram & hst);
	/**
	 * Compute a skin likelihood value.
	 * @param p	input color point
	 * @return real valued skin likelihood in [0,1]
	 */
	REALNUM getSkinLikelihood(const Vec3b & p)const;
};

} // namespace FaceMatch
