
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
#include "Exception.h"
#include <iostream>
#include <fstream>

using namespace std;

namespace FaceMatch
{

/// a robust input stream
class InputStream: public istream
{
	filebuf mFB;
public:
	/**
	 * Instantiate an input stream.
	 * @param FN	input file name, or "CAM" for webcam input
	 */
	InputStream(const string & FN): istream(cin.rdbuf())
	{
		if (FN.empty()) return;
		else
		{
			mFB.open(FN.c_str(), ios::in);
			if (!mFB.is_open()) throw Exception(string("file not found: ")+FN);
			rdbuf(&mFB);
		}
	}
	/**
	 * Close the input stream.
	 */
	void close() { mFB.close(); }
};

/// a string list reader
class ListReader: public InputStream
{
protected:
	string mLine; ///< next line holder
public:
	/**
	 * Instantiate a list reader.
	 * @param FN input file name; if empty, use STDIN
	 */
	ListReader(const string & FN = ""): InputStream(FN) {}
	/**
	 * Check end of the list.
	 * @return end of the list?
	 */
	bool end() const { return eof(); }
	/// @return next line in the list
	string & fetch(string & line, ///< [out] container for the input line
		char FetchDelim = '\n', ///< line fetch delimiter, @see std::getline
		const string & TrimDelim = "" ///< characters to trim from the input line
	)
	{
		std::getline(*this, line, FetchDelim);
		if (!TrimDelim.empty()) trim(line, TrimDelim);
		return line;
	}
	/// @return next line in the list
	const string & fetch(char FetchDelim = '\n', ///< line fetch delimiter, @see std::getline
		const string & TrimDelim = sDefaultDelims ///< characters to trim from the input line
	)
	{
		return fetch(mLine, FetchDelim, TrimDelim);
	}
};

/// a robust output stream
class OutputStream: public ostream
{
	filebuf mFB;
public:
	/**
	 * Instantiate a list writer.
	 * @param FN input file name; if empty, use STDOUT
	 */
	OutputStream(const string & FN): ostream(cout.rdbuf())
	{
		if (FN.empty()) return;
		mFB.open(FN.c_str(), ios::out);
		if (!mFB.is_open()) throw Exception(string("file not found: ")+FN);
		rdbuf(&mFB);
	}
	/// Close the stream.
	void close() { mFB.close(); }
	/// @return using a file for output?
	bool isFile()const{return mFB.is_open();}
};

/**
 * Check if the file exists.
 * @param FN	given file name
 * @return	true, if the file exists; false otherwise
 */
inline
bool FileExists(const string & FN)
{
	ifstream s(FN.c_str());
	return s.good();
}

}
