
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

#include "string_supp.h"
#include "Diagnostics.h"

#include <ctime>
#include <iostream>
#include <unordered_map>
#include <list>

using namespace std;
using namespace cv;

LIBDCL int getYear();

/// measure time difference in seconds
class Timing
{
	time_t start;
public:
	Timing(): start(time(0)) { }
	/// \return time difference between instantiation and now using standard time routines
	double diff() { return difftime(time(0), start); }
};

/// measure time difference in CPU clocks
class Clocking
{
	clock_t start;
public:
	Clocking(): start(clock()) {}
	/// \return time difference between instantiation and now using standard wall clock
	double diff() { return clock()-start; }
};

/// Define a labeled bin histogram class that can be used for code profiling.
class LIBDCL TimeHist
{
	OMPSimpleLock mLock;
	struct TStat
	{
		unsigned cnt;
		double acc;
	};
	typedef list<string> TLabels;
	TLabels mLabels; // keeping the insertion order
	typedef	unordered_map<string, TStat> THist;
	THist mHist;
public:
	/// Output the instance to a text stream.
	void print(ostream & s /**< output text stream */);
	/// Increment the labeled bin by the given value.
	void inc(const string & label, ///< bin label
		double val=1 ///< increment
	);
	virtual ~TimeHist();
	/// Serialize the instance to a text stream
	friend ostream & operator<<(ostream & s, ///< output text stream
		const TimeHist & h ///< a histogram instance
	);
};

LIBDCL void incOffset();
LIBDCL void decOffset();
LIBDCL string getOffset();
LIBDCL void incTimeHist(const string & label, double val=1);

/// time difference logging for profiling purposes
template<class T>
class TProfiling: public T
{
	const string mLabel;
public:
	/// Instantiate.
	TProfiling(const string & label /**< text label for profiling logging */): mLabel(label)
	{
		incOffset();
	}
	virtual ~TProfiling()
	{
		double d = T::diff();
		incTimeHist(getOffset()+mLabel, d);
		if (getVerbLevel()) clog<<getOffset()<<mLabel<<"="<<d<<endl;
		decOffset();
	}
};

/// measure time difference in fractions of a second, OpneCV style
class CVTiming
{
	int64 start;
	double freq;
public:
	/// Instantiate.
	CVTiming(): freq(getTickFrequency()) { init(); }
	/// Initialize.
	void init() { start=getTickCount(); }
	/// \return time difference between instantiation and now using OpenCV time routines
	double diff() { return (getTickCount()-start)/freq; }
};

typedef TProfiling<CVTiming> CVTimeLog;

#ifdef _TIMING
#define TIMELOG(LBL) CVTimeLog tmg##_COUNTER__(string("TIMELOG: ")+LBL);
#define FTIMELOG TIMELOG(__FUNCTION__)
#define TimeLog(ID) CVTimeLog tmg##ID("TIMELOG: "#ID);
#else // _TIMING
#define TIMELOG(LBL)
#define FTIMELOG
#define TimeLog(LBL)
#endif
