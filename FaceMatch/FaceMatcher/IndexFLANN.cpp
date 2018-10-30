
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

#include "stdafx.h"
#include "IndexFLANN.h"

using namespace std;

namespace FaceMatch
{
void IndexFLANN::load(const string & inFN)
{
	FTIMELOG
	const string & ext=getFileExt(inFN);
	if (ext==".yml")
	{
		FileStorage fs(inFN, FileStorage::READ);
		fs["DscType"]>>mDscType;
		fs["NdxRadius"]>>mNdxRadius;
		fs["NdxCenter"]>>mNdxCenter;
		mImgDscNdx.read(mDscType, fs["ImgDscNdx"]);
		mPNdx->read(fs["Ndx"]);
	}
	else if (ext==".ndx")
	{
		ifstream s(inFN, ios::binary);
		std::read(s, mDscType);
		readSimple(s, mNdxRadius);
		cv::read(s, mNdxCenter);
		mImgDscNdx.read(mDscType, s);
		// TODO: read clusters, if any
	}
	else throw Exception("unknown index file type "+ext);
}
void IndexFLANN::absorb()
{
	ImgDscMatcher::absorb();
	Mat avg;
	const unsigned icnt=mImgDscNdx.size();
	for (unsigned i=0; i<icnt; ++i)
	{
		Mat d = mImgDscNdx[i].Dsc->getVectors();
		Mat a; reduce(d, a, 0, CV_REDUCE_AVG);
		avg.push_back(a);
	}
	reduce(avg, mNdxCenter, 0, CV_REDUCE_AVG); // centroid
	TMatches matches;
	mPNdx->radiusMatch(mNdxCenter, matches, numeric_limits<REALNUM>::max());
	mNdxRadius=matches[0].back().distance; // worst match = max distance
}
void IndexFLANN::write(FileStorage & fs)const
{
	fs<<"DscType"<<mDscType;
	fs<<"NdxRadius"<<mNdxRadius;
	fs<<"NdxCenter"<<mNdxCenter;
	fs<<"ImgDscNdx"<<mImgDscNdx;
	fs<<"Ndx"<<"{";
		mPNdx->write(fs); // TODO: output clusters, if any
	fs<<"}";
}
void IndexFLANN::write(ostream & s)const
{
	std::write(s, mDscType);
	writeSimple(s, mNdxRadius);
	cv::write(s, mNdxCenter);
	mImgDscNdx.write(s);
	// TODO: output clusters, if any
}
void IndexFLANN::eval(ostream & s)const
{
	s<<"to be implemented"<<endl;
}
} // namespace FaceMatch
