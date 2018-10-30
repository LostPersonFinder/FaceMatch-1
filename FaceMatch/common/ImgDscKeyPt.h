
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

#include "Image.h"
#include "ImgDscBase.h"

namespace cv
{
inline ostream & operator<<(ostream & s, const KeyPoint & p)
{
	s<<"{"<<p.pt<<", "<<p.size<<", "<<p.angle<<", "<<p.response<<", "<<p.octave<<", "<<p.class_id<<"}";
	return s;
}
}
namespace FaceMatch
{

/// minimal key point matching count default
const unsigned cMinKeyPtMatchCntDefault=4;

/// base for key-point descriptors
class LIBDCL ImgDscKeyPt: public ImgDscBase
{
	string mType;
public:
	/** collection of key-point weights */
	typedef vector<REALNUM> Weights;
protected:
	/** key points */
	KeyPoints mKeyPoints;
	/** key point descriptors */
	Mat mDescriptors;
	/** key point weights */
	Weights mWeights;
	/** face sub-regions */
	FaceRegions mSubRegions;
	/**
	 * Initialize.
	 * @param cvDscType OpenCV descriptor type: SIFT, SURF, ORB
	 * @param img input image reference
	 * @param dim image region dimension in pixels
	 */
	void init(const string & cvDscType, Image & img, unsigned dim);
	/// Compute landmarks/sub-regions weights using the saliency map, if supplied in the alpha (4th in color, 2nd in gray-scale) channel.
	void computeWeights(const Mat & img /**< input image with optional alpha channel */);
	/// Scale landmarks/sub-regions according to the diameter.
	void scaleSubRegions(const FaceRegions & SubRegs, ///< face sub-regions/landmarks
		Size OrgImgSize, ///< original image size in pixels
		unsigned dim ///< normalized image diameter in pixels
		);
public:
	ImgDscKeyPt(): mType("ImgDscKeyPt") {}
	/**
	 * Instantiate.
	 * @param cvDscType OpenCV descriptor type: SIFT, SURF, ORB
	 * @param pImgSrc input image pointer; if NULL, produce an empty descriptor
	 * @param dim image region dimension in pixels
	 */
	ImgDscKeyPt(const string & cvDscType, const Image * pImgSrc=0, unsigned dim=DefaultFacePatchDim);
	/**
	 * Instantiate.
	 * @param cvDscType OpenCV descriptor type: SIFT, SURF, ORB
	 * @param rImgSrc input image reference
	 * @param dim image region dimension in pixels
	 */
	ImgDscKeyPt(const string & cvDscType, Image & rImgSrc, unsigned dim=DefaultFacePatchDim);
	/**
	 * Instantiate.
	 * @param fn input OpenCV file storage node
	 */
	ImgDscKeyPt(const FileNode & fn);
	/**
	 * Instantiate.
	 * @param type OpenCV CV_type of the descriptor
	 * @param s input binary stream
	 */
	ImgDscKeyPt(const string & type, istream & s);
	virtual ~ImgDscKeyPt() {}
	/**
	 * Specify static descriptor type as a string.
	 * @return	descriptor type
	 */
	static const string & Type()
	{
		StaticLkdCtor const string type="ImgDscKeyPt";
		return type;
	}
	virtual const string & getType()const override {return mType;}
	virtual bool empty()const{return mKeyPoints.size()==0;}
	virtual void write(ostream & s)const;
	virtual void write(FileStorage& fs)const;
	virtual void print(ostream & s, const string & fmt="")const override;

	/**
	 * Read a descriptor instance from a binary stream
	 * @param s input binary stream
	 */
	void read(istream & s);
	/**
	 * Get a reference to key-points.
	 * @return a const reference to descriptor's key-points
	 */
	const KeyPoints& getKeypoints()const{ return mKeyPoints; }
	/**
	 * Get a reference to point descriptors.
	 * @return reference to point descriptors
	 */
	virtual const Mat getVectors()const override { return mDescriptors; }
	/// @return weights that are used in matching descriptors
	const Weights & getWeights()const{ return mWeights; }
	/**
	 * Get face regions.
	 * @return face regions reference
	 */
	const FaceRegions & getSubregions()const{ return mSubRegions; }
	/**
	 * Get face region kind.
	 * @return face region kind reference
	 */
	const string & getSubregionKind(unsigned ndx)const;
};
/// smart pointer to ImgDscKeyPt
typedef Ptr<ImgDscKeyPt> PImgDscKeyPt;

#define classImgDscKeyPt(TYPE) struct LIBDCL ImgDsc##TYPE: public ImgDscKeyPt {\
	ImgDsc##TYPE(Image & rImgSrc, unsigned dim=DefaultFacePatchDim): ImgDscKeyPt(#TYPE, rImgSrc, dim){}\
	ImgDsc##TYPE(const Image * pImgSrc=0, unsigned dim=DefaultFacePatchDim): ImgDscKeyPt(#TYPE, pImgSrc, dim){}\
	ImgDsc##TYPE(istream& s): ImgDscKeyPt(#TYPE, s) {}\
	ImgDsc##TYPE(const FileNode & fn): ImgDscKeyPt(fn) {}\
	static const string & Type(){ StaticLkdCtor const string type="ImgDsc"#TYPE; return type; }\
	virtual const string & getType()const override{return ImgDsc##TYPE::Type();}\
	virtual REALNUM dist(const ImgDscBase & a, Matches * matches=0)const override;\
	virtual REALNUM getDistScale(bool OM=false)const override;\
	virtual void setDistScale(REALNUM s, bool OM=false)const override;\
}

/// macro definition of ImgDscSURF
classImgDscKeyPt(SURF);
/// macro definition of ImgDscSURF with fixed landmark points
classImgDscKeyPt(SURFFL);
/// macro definition of ImgDscSIFT
classImgDscKeyPt(SIFT);
/// macro definition of ImgDscSIFT with fixed landmark points
classImgDscKeyPt(SIFTFL);
/// macro definition of ImgDscORB
classImgDscKeyPt(ORB);
/// macro definition of ImgDscORBFL with fixed landmark points
classImgDscKeyPt(ORBFL);

}
