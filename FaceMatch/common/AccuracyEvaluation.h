
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

#pragma once // 2011-2013 (C) 

#include "math_supp.h"
#include <string>
#include <ostream>

using namespace std;

static const char * aeDefaultTitles="Given\tFound\tMatch\tFNCnt\tFPCnt\tRecall\tPrecision\tFScore";

/**
 * a matching accuracy evaluation utility template, typically using int or float for its typename NUMBER
 */
template<typename NUMBER=int> // typically int or float
struct AccuracyEvaluation
{
	/** given count */
	NUMBER mGivenTotal,
	/** found count */
		mFoundTotal,
	/** match count */
		mMatchCnt;
	/** titles for the counts and measures for tabular output */
	string Titles;
	AccuracyEvaluation():
		mGivenTotal(0), mFoundTotal(0), mMatchCnt(0),
		Titles(aeDefaultTitles)
		{}
	/**
	 * Increment accuracy evaluation counts.
	 * @param s	accuracy evaluation counts to be added
	 * @return	incremented accuracy evaluation counts
	 */
	const AccuracyEvaluation & operator+=(const AccuracyEvaluation & s)
	{
		mGivenTotal+=s.mGivenTotal;
		mFoundTotal+=s.mFoundTotal;
		mMatchCnt+=s.mMatchCnt;
		return *this;
	}
	/**
	 * Compute false negative count
	 * @return	false negative count
	 */
	NUMBER FalseNegativeCnt()const{ return mGivenTotal-mMatchCnt; }
	/**
	 * Compute false positive count
	 * @return	false positive count
	 */
	NUMBER FalsePositiveCnt()const{ return mFoundTotal-mMatchCnt; }
	/**
	 * Compute matching recall.
	 * @return	matching recall
	 */
	REALNUM Recall()const
	{
		return mGivenTotal ? mMatchCnt/(REALNUM)mGivenTotal : 1;
	}
	/**
	 * Compute matching precision.
	 * @return	matching precision
	 */
	REALNUM Precision()const
	{
		return mFoundTotal ? mMatchCnt/(REALNUM)mFoundTotal : 1;
	}
	/**
	 * Compute and return the weighted F-score
	 * @param b	precision/recall balancing parameter in [0,1]
	 * @return weighted F-score
	 */
	REALNUM FScore(REALNUM b=1)const
	{
		const REALNUM R = Recall(), P = Precision(),
			b2=b*b, d=(R+b2*P), e=(1+b2)*R*P;
		if (e==0) return 0;
		return d ? e/d : 1;
	}
};

/**
 * Output the evaluation counts and the accuracy measures as text.
 * @param s	output text stream
 * @param ae		accuracy evaluation instance
 * @return		ouptut text stream
 */
template<typename NUM>
inline ostream & operator<<(ostream & s, const AccuracyEvaluation<NUM> & ae)
{
	s<<ae.mGivenTotal<<'\t'<<ae.mFoundTotal<<'\t'<<ae.mMatchCnt<<'\t'
	<<ae.FalseNegativeCnt()<<'\t'<<ae.FalsePositiveCnt()<<'\t'
	<<ae.Recall()<<'\t'<<ae.Precision()<<'\t'<<ae.FScore();
	return s;
}
