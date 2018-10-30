
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

#pragma once // 2012-2016 (C) 

#include "common.h"

namespace FaceMatch
{

const unsigned
	DefaultFacePatchDim=128, ///< default face patch diameter in pixels
	DefaultWholeImgDim=256; ///< default normalized whole image diameter in pixels

/// a common base for image descriptors
struct ImgDscBase
{
	/// collection of descriptor matches
	typedef vector<DMatch> Matches;

	/// Instantiate.
	ImgDscBase(){}
	/**
	 * Instantiate.
	 * @param fn	input file storage node as defined in OpenCV
	 */
	ImgDscBase(const FileNode & fn){}
	/**
	 * Instantiate.
	 * @param s input binary stream.
	 */
	ImgDscBase(istream & s){}
	/**
	 * Specify static descriptor type as a string.
	 * @return	descriptor type
	 */
	static const string & Type()
	{
		StaticLkdCtor const string type="ImgDscBase";
		return type;
	}
	/**
	 * Specify dynamic descriptor type as a string
	 * @return string of the dynamic descriptor type
	 */
	virtual const string & getType()const{return Type();}
	virtual ~ImgDscBase(){}
	/**
	 * Get descriptor values in an OpenCV matrix object.
	 * @return descriptor values in an OpenCV matrix object
	 */
	virtual const Mat getVectors()const=0;
	/**
	 * Output descriptor to a binary stream.
	 * @param s	output binary stream
	 */
	virtual void write(ostream & s)const{}
	/**
	 * Output descriptor to a text stream.
	 * @param s	output text stream
	 * @param fmt output format
	 */
	virtual void print(ostream & s, const string & fmt="")const{}
	/**
	 * Output descriptor to an OpenCV file storage stream.
	 * @param fs	file storage stream
	 */
	virtual void write(FileStorage& fs)const{}
	/**
	 * Get a distance between this and the given descriptors.
	 * @param a	given descriptor reference
	 * @param matches	optional output colleciton of matches
	 * @return	real-valued distance between this and the given descriptors in [0,1]
	 */
	virtual REALNUM dist(const ImgDscBase & a, Matches * matches=0)const=0;
	/**
	 * Get distance scale value for making this->dist() compatible with the rest of the descriptors
	 * @return distance scale value
	 */
	virtual REALNUM getDistScale(bool OM=false)const=0;
	/**
	 * Set distance scale value for making this->dist() compatible with the rest of the descriptors
	 * @param s distance scale value
	 * @param OM use optimal descriptor matching strategy?
	 */
	virtual void setDistScale(REALNUM s, bool OM=false)const=0;
	/**
	 * Test for emptiness.
	 * @return	is descriptor empty?
	 */
	virtual bool empty()const=0;
	/**
	 * Serialize the descriptor instance to a text stream.
	 * @param s	output text stream
	 * @param d	descriptor instance
	 * @return	output stream
	 */
	friend ostream & operator<<(ostream & s, const ImgDscBase & d);
	/**
	 * Test for equality of two descriptors.
	 * @param lhs	left-hand-side descriptor
	 * @param rhs	right-hand-side descriptor
	 * @return	is the distance between them 0?
	 */
	friend bool operator==(const ImgDscBase & lhs, const ImgDscBase & rhs){return lhs.dist(rhs)==0;}
	/**
	 * Negation of the equality of two descriptors.
	 * @param lhs	left-hand-side descriptor
	 * @param rhs	right-hand-side descriptor
	 * @return	are they not equal?
	 */
	friend bool operator!=(const ImgDscBase & lhs, const ImgDscBase & rhs){return !(lhs==rhs);}
};

/// OpenCV smart pointer to image descriptor base
typedef Ptr<ImgDscBase> PImgDscBase;

}
