
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

#pragma once // 2011-2014 (C) 

#include "IndexFLANN.h"
#include <FaceFinder.h>
#include <math_supp.h>
#include <ListStreamer.h>

namespace FaceMatch
{

const REALNUM cMatchT = 0.5;
const unsigned FaceFinderDefaultFlags = FaceFinder::selective|FaceFinder::HistEQ;

enum EFaceMatchFlags
{
	fmNone,
	fmShow=1,
	fmSave=1<<1,
	fmFLANN=1<<2,
	fmMirror=1<<3,
	fmScale=1<<4,
	fmRotate=1<<5
};

class FaceMatcher
{
	unsigned mFlags;
	typedef list<string> ListLines;
	mutable IndexFLANN mIndex;
	string mImgDscType;
	const string mRepoPath;
	const REALNUM mMatchT;
	REALNUM mCropFaceRgns;
	QueryResults mResults;
//=== helpers
	REALNUM distance(const Image & a, const ImgDscBase & A, const Image & b, const ImgDscBase & B, unsigned flags=0)const;
	PImgDscBase computeImageDescriptor(string & FR, unsigned FaceFinderFlags)const;
	Mat computeDescriptors(const string & inFN, unsigned FaceFinderFlags)const;
	PImgDscBase newImgDsc(Image & ImgSrc)const
	{
		return newDescriptor(mImgDscType, ImgSrc);
	}
public:
	/**
	 * Instantiate a face matcher.
	 * @param ImgDscType image descriptor type: HAAR|SURF|SIFT|ORB
	 */
	FaceMatcher(const string & ImgDscType="SURF", unsigned flags=fmNone): mFlags(flags), mIndex(ImgDscType, mFlags),
		mImgDscType(ImgDscType), mMatchT(cMatchT), mCropFaceRgns(0) {}
	/**
	 * Instantiate a face matcher.
	 * @param ImgDscType image descriptor type: HAAR|SURF|SIFT|ORB
	 * @param inFN input image list or index file name
	 * @param repoPath optional image repository path
	 * @param MatchT optional matching threshold; default is 0.5
	 * @param cropFaces shrink/grow face region multiplier; default is 1 = no face detection or region shrinking; optimum at about 0.6
	 */
	FaceMatcher(const string & ImgDscType, const string & inFN, const string & repoPath="", REALNUM MatchT=cMatchT, REALNUM cropFaces=1, unsigned FaceFinderFlags=0, unsigned flags=0);
	/**
	 * Destroy the instance.
	 */
	virtual ~FaceMatcher(){}
	/**
	 * Compute distance between (faces in) given images.
	 * Given images may be scaled to optimize matching.
	 * @param a	input image
	 * @param b	input image
	 * @return real distance value in [0,1]
	 */
	REALNUM distance(Image & a, Image & b, unsigned visual=0)const
	{
		const PImgDscBase A = newImgDsc(a), B = newImgDsc(b);
		return distance(a, *A, b, *B, visual);
	}
	/**
	 * Compute distance between faces in given images.
	 * @param aFN	input face file name
	 * @param bFN	input face file name
	 * @param fFaceRectShrink - optional face area shrink multiplier; default is 0 = no face detection or shrinking; optimal value 0.6
	 * @return real distance value in [0,1]
	 */
	// REALNUM distance(const string & aFN, const string & bFN, REALNUM fFaceRectShrink=0, unsigned FaceFinderFlags=FaceFinderDefaultFlags, unsigned visual=0)const;
	REALNUM distance(const string & aFN, const string & bFN, REALNUM param=0, unsigned FaceFinderFlags=FaceFinderDefaultFlags, unsigned flags=0)const;
	// 
	PScoredLines matchOne(string & FR, unsigned FaceFinderFlags, unsigned visual)const;
	const QueryResults & match(const string & inFN, unsigned FaceFinderFlags=FaceFinderDefaultFlags, unsigned visual=0);
	void save(const string & FN)const;
	void eval(ostream & s)const;
};

}
