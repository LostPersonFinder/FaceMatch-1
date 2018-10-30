
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

#pragma once // 2011-2016 (C) : char/string supplemental routines

#include <algorithm>
#include <string>
#include <sstream>

namespace std
{

/// Define symbols for special characters.
#if defined(WIN32) || defined(WIN64)
const char cPathSeparator =	'\\';
typedef unsigned SpecialChar;
#define StrCmpNoCase stricmp
const SpecialChar
	SPC = ' ',
	TAB = '\t',
	RET = '\r',
	BkSp = 0x8,
	ESC = 0x1B,
	PgUp = 0x210000,
	PgDn = 0x220000,
	END = 0x230000,
	HOME = 0x240000,
	Left = 0x250000,
	Up = 0x260000,
	Right = 0x270000,
	Down = 0x280000,
	DEL = 0x2e0000;
#else // assume Linux
const char cPathSeparator =	'/';
typedef unsigned short SpecialChar;
#define StrCmpNoCase strcasecmp
const SpecialChar
	ESC = 27,
	SPC = ' ',
	RET = '\n',
	TAB = '\t',
	BkSp = 0xff08,
	HOME = 0xff50,
	Left = 0xff51,
	Up = 0xff52,
	Right = 0xff53,
	Down = 0xff54,
	PgUp = 0xff55,
	PgDn = 0xff56,
	END = 0xff57,
	DEL = 0xffff;
#endif

/// default set of whitespace delimiters
static const string
	sDefaultDelims=" \t\r\n";

/// @return string stripped of the leading and trailing whitespace
inline string & trim(string & str, ///< [in/out] string
	const string & delim = sDefaultDelims ///< white space delimiters, e.g. " \t\r\n"
)
{
	string::size_type pos = str.find_last_not_of(delim);
	if(pos == string::npos) str.clear();
	else
	{
		str.erase(pos+1);
		pos = str.find_first_not_of(delim);
		if(pos != string::npos) str.erase(0, pos);
	}
	return str;
}
/// @return trimmed copy of the input
inline string trimc(const string & str, ///< input street
	const string & delim = sDefaultDelims ///< white space delimiters
)
{
	string res=str; trim(res); return res;
}
/// @return	file name with optional path or extension
inline string getFileName(const string & FileName, ///< input string containing the target path
	bool withPath=true, ///< include path in the output?
	bool withExt=false ///< include extension in the output?
)
{
	string FN; stringstream strm(FileName); getline(strm, FN, '\t'); trim(FN);
	int
		start = withPath ? 0 : FN.find_last_of("/\\")+1,
		len = (withExt ? FN.length() : FN.rfind("."))-start;
	return FN.substr(start, len);
}
/// @return	file extension including the period: .ext
inline string getFileExt(const string & FN /**< input path string */)
{
	auto pos = FN.rfind(".");
	return pos == string::npos ? "" : FN.substr(pos);
}
/// @return file name path
inline string getFilePath(const string & FN /**< input file name */)
{
	int end = FN.find_last_of("/\\");
	return end==string::npos ? "" : FN.substr(0, end+1);
}
/// @return transformed to the lower case string
inline string & toLower(string & s /**< input/output string */)
{
	transform(s.begin(), s.end(), s.begin(), ::tolower); return s;
}
/// @return transformed to lower case string
inline string toLC(string s /**< input string */)
{
	return toLower(s);
}
/// @return transformed to the UPPER case string
inline string & toUpper(string & s /**< [in/out] string to be modified */)
{
	transform(s.begin(), s.end(), s.begin(), ::toupper); return s;
}
/// @return transformed to UPPER case string
inline string toUC(string s /**< input string */)
{
	return toUpper(s);
}
/// @return	truncated string reference
inline string & truncate(string & s, ///< input/output string
	unsigned n=1 ///< number of characters to remove from the back
)
{
	return s=s.substr(0, s.length()-n);
}
/// @return number of context string replacements
inline unsigned replace(string & str, ///< [in/out] string to be modified
	const string & from, ///< string to find
	const string & to, ///< what replace it with
	bool all=true ///< replace all occurrences?
)
{
	unsigned cnt=0;
	for (size_t start_pos=0; start_pos != string::npos; ++cnt)
	{
		start_pos=str.find(from, start_pos);
		if (start_pos == string::npos) break;
		str.replace(start_pos, from.length(), to);
		if (all) continue;
	}
	return cnt;
}
/// @return input string transformed to a file name
inline string toFN(const string & s /**< [in] string */)
{
	string FN=s;
	replace(FN, "/", "$"); replace(FN, "\\", "$");
	replace(FN, " ", "-"); replace(FN, "\t", "_");
	replace(FN, ".", "!");
	return FN;
}
/// @return number of the new-line characters
inline unsigned countLines(const string & src /**< input string */)
{
	return count(src.begin(), src.end(), '\n');
}
/// @return does input contain only digits?
inline bool isNumeric(const string & src /**< input string */)
{
	for (auto c : src) if (!isdigit(c)) return false;
	return true;
}
/// @return string representation of a streamed value
template<typename T>
string toString(const T & v /**< value */)
{
	stringstream strm; strm<<v; return strm.str();
}
/// @return does input string have the specified prefix?
inline bool checkPrefix(const string & src, ///< input string
	const string & pfx ///< prefix
)
{
	return src.find(pfx)==0;
}
/// @return does input string start with the specified prefix?
inline bool startsWith(const string & src, ///< input string
	const string & pfx ///< prefix
)
{
	return checkPrefix(src, pfx);
}
/// @return does input string have the specified suffix?
inline bool checkSuffix(const string & src, ///< input string
	const string & sfx ///< suffix
)
{
	const int L = src.length(), l = sfx.length(), pos = L - l;
	return pos>=0 && src.rfind(sfx) == pos;
}
/// @return does input string end with the specified suffix?
inline bool endsWith(const string & src, ///< input string
	const string & sfx ///< suffix
)
{
	return checkSuffix(src, sfx);
}
/// @return does src contain trg?
template<typename T>
inline bool contains(const string & src, ///< source string to be searched
	const T & trg ///< search item compatible with string::find
)
{
	return src.find(trg)!=string::npos;
}

} // namespace std
