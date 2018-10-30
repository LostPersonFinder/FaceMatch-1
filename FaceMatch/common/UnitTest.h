
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

#pragma once // 2012-2017 (C) 

#include "Exception.h"
#include "Diagnostics.h"
#include <string>
#include <unordered_map>
#include <sstream>
#include <ostream>

using namespace std;

namespace FaceMatch
{
const static string
	TestRepoPath = "../Data/CEB/HEPL/Images/Original/",
	TestImgLstFN = "../Lists/CEB/HEPL/test.small.lst";
/// Read the whole file as text.
inline void readAll(istream & s, ///< input text stream
	string & text ///< output text with the contents of the file
) { getline(s, text, char(-1)); }

/// common base for all unit tests
template<class DERIVED /**< derived unit test class name needed for delegate method pointers to custom tests */>
class UnitTest
{
protected:
	string
		/// image repository path
		mRepoPath,
		/// image list file name
		mImgLstFN;
	/// verbosity level
	unsigned mVerbose;
	/// test member function pointer type
	typedef void(DERIVED::*PTestCall)();
	/// test record definition
	struct Test
	{
		/// pointer to a test function
		PTestCall TestFunction;
		/// is the test mandatory?
		bool Mandatory;
		/// test outcome status
		string Status;
		/// Instantiate.
		Test(PTestCall pTestFunction=0, ///< pointer to a test function
			bool mandatory=true	///< is the test mandatory?
		): TestFunction(pTestFunction), Status("?"), Mandatory(mandatory) {}
	};
	/// map of test IDs to the test records
	typedef unordered_map<string, Test> TestMap;
	/// the test map
	TestMap mTestMap;
	/// Call the test function by its pointer.
	void call(PTestCall aTest /**< test function pointer to call */)
	{
		if (!aTest) throw FaceMatch::Exception("no test function provided");
		DERIVED * p = static_cast<DERIVED*>(this);
		CHECK(p, "unable to obtain a valid test case pointer");
		(p->*(aTest))();
	}
	/// \return	count of errors after running test
	int run(typename TestMap::iterator it /**< test map iterator to run */)
	{
		try
		{
			cout<<it->first<<": "; cout.flush();
			call(it->second.TestFunction);
			cout<<string(it->second.Status="OK")<<endl;
		}
		catch (const exception & e)
		{
			cout<<string(it->second.Status=string("FAILED: ")+e.what())<<endl;
			return 1;
		}
		catch (...)
		{
			cout<<(it->second.Status="FAILED: unknown exception")<<endl;
			return 1;
		}
		return 0;
	}
	/// \return	number of errors after running all tests
	int runAll(bool force=false /**< force the run ?*/)
	{
		int iErrCnt=0;
		for (typename TestMap::iterator it=mTestMap.begin(); it!=mTestMap.end(); ++it)
			iErrCnt += (force || it->second.Mandatory) ? run(it) : 0;
		return iErrCnt;
	}
	/// \return	output stream after test text serialization
	friend ostream & operator<<(ostream & s, ///< output text stream
		const Test & t ///< a test reference
	)
	{
		s<<t.Status;
		return s;
	}
	/// \return	output text stream after test map serialization
	friend ostream & operator<<(ostream & s, ///< output text stream
		const TestMap & tm ///< test map
	)
	{
		for (typename TestMap::const_iterator it=tm.begin(); it!=tm.end(); ++it)
			s<<it->first<<'\t'<<it->second<<endl;
		return s;
	}
	/// \return	number of errors after running a specific test
	int runSpecific(const string & SpecificTest /**< specific test ID*/)
	{
		typename TestMap::iterator it=mTestMap.find(SpecificTest);
		if (it==mTestMap.end())
		{
			stringstream strmErr;
			strmErr<<"unable to locate test:"<<SpecificTest<<" in the test map "<<endl<<mTestMap;
			throw FaceMatch::Exception(strmErr.str());
		}
		return run(it);
	}
	/// Compare contents of two text streams.
	void compare(istream & strmExpectedOutput, ///< expected output stream
		istream & strmComputedOutput ///< computed output stream
	)
	{
		if (mVerbose>2) // line-by-line comparison
		{
			while (!strmExpectedOutput.eof())
			{
				string ExpectedOutputLn; getline(strmExpectedOutput, ExpectedOutputLn);
			clog<<"ExpectedOutputLn='"<<ExpectedOutputLn<<"'"<<endl;
				string ComputedOutputLn; getline(strmComputedOutput, ComputedOutputLn);
			clog<<"ComputedOutputLn='"<<ComputedOutputLn<<"'"<<endl;
				CHECK(ComputedOutputLn==ExpectedOutputLn,
					"computed output line '"+ComputedOutputLn+"' differs from expected output line '"+ExpectedOutputLn+"'");
			}
		}
		else
		{
			string ExpectedOutput; readAll(strmExpectedOutput, ExpectedOutput);
		if (mVerbose>1) clog<<"ExpectedOutput="<<endl<<ExpectedOutput<<endl;
			string ComputedOutput; readAll(strmComputedOutput, ComputedOutput);
		if (mVerbose>1) clog<<"ComputedOutput="<<endl<<ComputedOutput<<endl;
			CHECK(ComputedOutput==ExpectedOutput, "computed output differs from the expected output");
		}
	}
public:
	/// Instantiate. \note Descendants should populate mTestMap with particular tests
	UnitTest(const string & RepoPath=TestRepoPath, ///< image repository path
		const string & ImgLstFN=TestImgLstFN, ///< image list file name
		unsigned verbose=0 ///< diagnostics verbosity
	): mRepoPath(RepoPath), mImgLstFN(ImgLstFN), mVerbose(verbose) {}
	/// \return	the number of errors incurred during the test run
	int run(const string & SpecificTest="" /**< specify an individual test to run; if empty (default), run all the tests */)
	{
		if (mVerbose)
		{
			int CUDACnt = getGPUCount();
			clog<<"FaceMatch-"<<getVersion()<<", CUDA devices: "<<CUDACnt<<endl;
			if (mVerbose>1) printGPUInfo();
		}
		bool force=SpecificTest=="all";
		return (force || SpecificTest.empty()) ? runAll(force) : runSpecific(SpecificTest);
	}
};

} // namespace FaceMatch
