
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

#include <iostream>
#include <vector>

namespace std
{
/**
 * Write a simple type variable (e.g. int or double) to a stream.
 * @param s output stream
 * @param v simple variable
 */
template<typename SIMPLE> // e.g. int, double
void writeSimple(ostream & s, const SIMPLE & v)
{
	s.write((const char*)&v, sizeof(v));
}
/**
 * Write a buffer of data to a stream.
 * @param s output stream
 * @param p pointer to data buffer
 * @param byteCnt number of bytes to write
 */
template<typename POINTER>
void writeBulk(ostream & s, POINTER p, unsigned byteCnt)
{
	s.write((const char*)p, byteCnt);
}
/**
 * Read a simple type variable (e.g. int or double) from a stream.
 * @param s input stream
 * @param v simple variable reference
 */
template<typename SIMPLE> // e.g. int, double
void readSimple(istream & s, SIMPLE & v)
{
	s.read((char*)&v, sizeof(v));
}
/**
 * Read a simple type variable (e.g. int or double) from a stream.
 * @param s input stream
 * @return simple value, e.g. int or double
 */
template<typename SIMPLE> // e.g. int, double
SIMPLE readSimple(istream & s)
{
	SIMPLE v;
	s.read((char*)&v, sizeof(v));
	return v;
}
/**
 * Read a buffer of data from a stream.
 * @param s input stream
 * @param p pointer to data buffer
 * @param byteCnt number of bytes to read
 */
template<typename POINTER>
void readBulk(istream & s, POINTER p, unsigned byteCnt)
{
	s.read((char*)p, byteCnt);
}
/**
 * Write string to a stream.
 * @param s output stream
 * @param v string value
 */
inline void write(ostream & s, const string & v)
{
	writeSimple(s, v.length());
	s.write(v.c_str(), v.length());
}
/**
 * Read string from a stream
 * @param s input stream
 * @param v string reference
 */
inline void read(istream & s, string & v)
{
	string::size_type len=readSimple<string::size_type>(s);
	vector<char> buf(len+1); s.read(buf.data(), len); buf[len]=0;
	v=buf.data(); // resizing and writing to output string buffer directly is unsafe
}
/**
 * Read a vector of simple elements, e.g. int or float
 */
template<class T>
inline void readVector(istream & s, vector<T> & v)
{
	unsigned len; readSimple(s, len);
	v.resize(len); if (len==0) return;
	readBulk(s, v.data(), len*sizeof(T));
}
/**
 * Write a vector of simple elements, e.g. int or float
 */
template<class T>
inline void writeVector(ostream & s, const vector<T> & v)
{
	unsigned len=v.size();
	writeSimple(s, len); if (len==0) return;
	writeBulk(s, v.data(), len*sizeof(T));
}

} // namespace std
