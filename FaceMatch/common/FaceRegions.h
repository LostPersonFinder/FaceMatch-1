
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

#pragma once // 2011-2016 (C) 

#include "common.h"
#include "math_supp.h"
#include "Exception.h"

using namespace cv;

namespace FaceMatch
{
/**
 * Output OpenCV size to a text stream.
 * @param s	output text stream
 * @param z	OpenCV size
 * @return	output text stream
 */
template<typename T>
ostream & operator<<(ostream & s, const Size_<T> & z)
{
	s<<z.width<<','<<z.height;
	return s;
}
/**
 * Output OpenCV rectangle to a text stream.
 * @param s	output text stream
 * @param r	OpenCV rectangle
 * @return	output text stream
 */
template<typename T>
ostream & operator<<(ostream & s, const Rect_<T> & r)
{
	s<<'['<<r.x<<','<<r.y<<';'<<r.size()<<']';
	return s;
}
/**
 * Compute the center point of a rectangle.
 * @param r	input rectangle
 * @return center point
 */
inline
Point center(const Rect & r)
{
	return Point(cvRound(r.x + r.width*0.5), cvRound(r.y + r.height*0.5));
}
/**
 * Compute the aspect ratio of a rectangle as w/h.
 * @param r	input rectangle
 * @return	real-valued aspect ratio
 */
inline
REALNUM aspect(const Rect & r)
{
	return r.width/(REALNUM)r.height;
}

/**
 * Stretch/shrink the given rectangle about its center by the scale factor.
 * @param r input rectangle
 * @param sx x scale factor
 * @param sy optional y scale factor
 */
void LIBDCL stretch(Rect & r, REALNUM sx, REALNUM sy=0);
Rect LIBDCL getStretched(const Rect & r, REALNUM sx, REALNUM sy=0);

/**
 * Scale the rectangle by the given factor.
 * @param r input rectangle
 * @param s scale factor
 */
inline
void operator*=(Rect & r, REALNUM s)
{
	r.x=cvRound(r.x*s);	r.y=cvRound(r.y*s);
	r.width=cvRound(r.width*s);	r.height=cvRound(r.height*s);
}

/**
 * Add two rectangles.
 * @param a input rectangle
 * @param b input rectangle
 * @return sum of a and b
 */
inline
Rect operator+(const Rect & a, const Rect & b)
{
	return Rect(a.tl()+b.tl(), a.size()+b.size());
}

/**
 * Compute the arithmetic average of the two given rectangles.
 * @param a input rectangle
 * @param b input rectangle
 * @return average rectangle of a and b
 */
inline
Rect average(const Rect & a, const Rect & b)
{
	Rect c(a+b); c*=0.5;
	return c;
}

/**
 * Read rectangle values from input stream.
 * @param s input stream
 * @param r rectangle to populate
 * @return input stream
 */
istream & operator>>(istream & s, Rect & r);

/**
 * Print rectangle as [x,y;w,h]
 * @param os output stream
 * @param r rectangle to output
 * @return output stream
 */
LIBDCL ostream & operator<<(ostream & os, const Rect & r);

/**
 * Does one rectangle contain the other?
 * @param a	left hand side
 * @param b right hand side
 * @return a in b ?
 */
inline
bool operator<=(const Rect & a, const Rect & b)
{
    return (a & b) == a;
}

/// @return if point is non-zero
template<typename T>
inline bool nz(const Point_<T> & p){return p.x || p.y;}

/// a face attribute base
struct LIBDCL FaceAttributeBase
{
	/// region kind, e.g. u=user/unknown, f=face, p=profile, s=skin, i=eye
	string Kind;
	/// Instantiate.
	FaceAttributeBase(const string & kind="u" /**< region kind */): Kind(kind) {}
	/// Instantiate.
	FaceAttributeBase(istream & s, ///< input stream
		bool bin=false ///< binary?
		);
	virtual ~FaceAttributeBase(){}
	/// Print an instance to a text stream.
	virtual void print(ostream & s /**< output text stream */)const{ s<<Kind; }
	/// Write the instance to a binary stream.
	virtual void write(ostream & s /**< output binary stream */)const{ std::write(s, Kind); }
	/// @return a pointer to a copy of the instance
	virtual FaceAttributeBase * clone()const{ return new FaceAttributeBase(*this); }
};
/// a smart pointer to the face attribute base
typedef Ptr<FaceAttributeBase> PFaceAttributeBase;
/// @return an output text stream with a serialized instance
LIBDCL ostream & operator<<(ostream & s, ///< output text stream
	const FaceAttributeBase & a ///< object to output
	);

/// a (categorical) face attribute class, e.g. gender, age group
struct LIBDCL FaceAttribute: public FaceAttributeBase
{
	string Value; ///< category, e.g. male|female or youth|adult
	/// Instantiate.
	FaceAttribute(const string & kind, ///< region kind, e.g. g=gender, or a=age
		const string & value ///< category value
		): FaceAttributeBase(kind), Value(value) {}
	/// Instantiate.
	FaceAttribute(istream & s, ///< input stream
		bool bin=false /// binary?
		);
	/// @see the base
	virtual void print(ostream & s)const override { FaceAttributeBase::print(s); s<<'['<<Value<<']'; }
	/// @see the base
	virtual void write(ostream & s)const override { FaceAttributeBase::write(s); std::write(s, Value); }
	/// @return a pointer to a copy of the instance
	virtual FaceAttribute * clone()const{ return new FaceAttribute(*this); }
};
/// a smart pointer to the face attribute class
typedef Ptr<FaceAttribute> PFaceAttribute;

/// face region minimum dimension
const unsigned cFaceRegMinDim=8;
/// face feature minimal aspect ratio
const REALNUM cFaceFeatureMinAspect=0.2;

struct FaceRegion;
/// OpenCV smart pointer to a face region
typedef Ptr<FaceRegion> PFaceRegion;

/// a collection of detected regions (as rectangles)
typedef vector<PFaceAttributeBase> FaceRegionsBase;

/// default face region overlap slack
const REALNUM cOverlapSlackTDefault=0.5; // TODO remove: .4

/// a collection of face regions, given/found in an image by a FaceFinder @see FaceRegion
class LIBDCL FaceRegions: public FaceRegionsBase
{
	typedef FaceRegionsBase TBase;
	unsigned mMinDim;
	REALNUM mAspectLimit;
	OMPNestedLock mLock;
public:
	/// region rectangle combination modes
	enum ERegRectCombo
	{
		rcMin, ///< intersection
		rcAvg, ///< average
		rcMax, ///< union
		rcDefault = rcAvg ///< default
	};
	/// Instantiate.
	FaceRegions(unsigned MinDim=cFaceRegMinDim, ///< face region minimum diameter
		REALNUM AspectLimit=0.5 ///< face region minimum aspect
		): mMinDim(MinDim), mAspectLimit(AspectLimit){}
	/// move-construction
	FaceRegions(FaceRegions && src){*this=src;}
	/// move-assignment
	FaceRegions & operator=(FaceRegions && src)
	{
		TBase::operator=(src);
		mMinDim=src.mMinDim;
		mAspectLimit=src.mAspectLimit;
		return *this;
	}
	/// Instantiate by copying the source.
	FaceRegions(const FaceRegions & src /**< source to copy from */){*this=src;}
	/// @return a reference to the deep-copied face region
	const FaceRegions & operator=(const FaceRegions & src /**< source to copy from */);
	/**
	 * Instantiate a region collection.
	 * @param kind	prefix to be used when outputting the regions, e.g. "f" for face or "p" for profile
	 * @param regs	candidate regions to populate
	 * @param MinDim	region minimum dimension in pixels
	 * @param AspectLimit	minimum accepted aspect ratio of a region in a collection
	 */
	FaceRegions(const string & kind, const vector<Rect> & regs, unsigned MinDim, REALNUM AspectLimit);
	/// @return the minimal face region diameter in pixels
	unsigned getMinDim()const{return mMinDim;}
	/// @return region's aspect ratio limit
	REALNUM getAspectLimit()const{ return mAspectLimit; }
	/**
	 * Output a region collection.
	 * @param s output stream
	 * @param rc region collection
	 * @return output stream
	 */
	LIBDCL friend ostream & operator<<(ostream & s, const FaceRegions & rc);
	/**
	 * Does one rectangle approximately match the other by area overlap?
	 * @param a source rectangle
	 * @param b source rectangle
	 * @param tol area overlap tolerance in [0,1] with 0 being a perfect match
	 * @return intersection a&b, if there is a match
	 */
	static Rect MatchOverlap(const Rect & a, const Rect & b, REALNUM tol=cOverlapSlackTDefault);
	/**
	 * Find the best matching region to the given using significant overlap
	 * @param rgn given region to search
	 * @param RegMatchTol	region overlap tolerance in [0,1]
	 * @return	const iterator of the best matching region or end() if none
	 */
	iterator findBestMatch(const PFaceRegion rgn, REALNUM RegMatchTol=cOverlapSlackTDefault);
	/**
	 * Add regions from a string
	 * @param s input string with rectangular regions specified, e.g. f[x,y;w,h]
	 */
	void add(const string & s);
	/**
	 * Add region, merge it with existing ones, if there's a significant overlap
	 * @param rgn new region to add
	 * @param intersect intersect new region with the ones in the collection?
	 * @param MatchT region match/overlap tolerance
	 * @param mode region combination mode
	 * @return smart pointer to the inserted region
	 */
	PFaceRegion add(const PFaceRegion rgn, bool intersect=false, REALNUM MatchT=cOverlapSlackTDefault, ERegRectCombo mode=rcDefault);
	/// Add a face attribute region.
	void add(PFaceAttribute pAttr);
	/**
	 * Add rectangular regions to this collection.
	 * Merge with existing regions, if there is a significant overlap.
	 * @param rgs additional regions
	 * @param intersect	perform set intersection? default is false, i.e. set union
	 * @param MatchT region match/overlap tolerance
	 * @param mode region combination mode
	 */
	void merge(const FaceRegions & rgs, bool intersect=false, REALNUM MatchT=cOverlapSlackTDefault, ERegRectCombo mode=rcDefault);
	/**
	 * Remove the lowest leaf region containing the specified point.
	 * @param p a query point
	 * @return removed?
	 */
	bool removeFirstContainig(const Point & p);
	/**
	 * Move regions containing the specified point to the top.
	 * @param p a query point
	 */
	void prime(const Point & p);
	/**
	 * Intersect/crop the regions with the given rectangle.
	 * @param r rectangular boundary
	 */
	void operator&=(Rect r);
	/**
	 * Scale the regions by the given factor.
	 * @param s scale factor
	 */
	void operator*=(REALNUM s);
	/**
	 * Transform the regions by the given matrix.
	 * @param M transformation matrix
	 */
	void operator*=(const Matx23f & M);
	/**
	 * Rotate the regions.
	 * @param R rotation matrix
	 */
	void rotate(const Matx23f & R);
	/**
	 * Shift the collection by the given point/vector.
	 * @param p shift fector
	 */
	void operator+=(const Point & p);
	/// Output the instance to a binary stream.
	void write(ostream & s /**< output binary stream */)const;
	/// Intput an instance from a binary stream.
	void read(istream & s, ///< input binary stream
		bool bClear=true ///< clear this instance before reading?
	);
	/// Scale the collection in x and y dimensions.
	void scale(REALNUM sx, ///< x scale factor
		REALNUM sy ///< y scale factor
	);
	/// Stretch rectangular regions by specified factors in each dimension.
	void stretch(REALNUM sx, ///< x scale factor
		REALNUM sy=0, ///< y scale factor
		bool recurse=true ///< recurse on sub-regions?
	);
	/// Sort the collection with respect to the central point.
	void sort(const Size & dim, ///< enclosing region dimensions used for determining the central point
		bool AscendDist2Center=true, ///< ascending distance to the center?
		REALNUM DimRatio=0.75 ///< for size dominance
	);
	/// @return iterator to the found region kind
	const_iterator findKind(const string & k)const;
	/// @return does the collection contain a region of the specified kind?
	bool hasKind(const string & k)const{return findKind(k)!=end();}
	/// @return # of regions of the specified kinds
	unsigned count(const string & kinds)const;
	/// @return result of erase(elements for which condition holds)
	template<typename PRED>
	iterator remove(PRED cnd)
	{
		OMPLocker lkr(mLock);
		return erase(remove_if(begin(), end(), cnd), end());
	}
	/// Declare interface for a face region predicate.
	class Predicate
	{
	protected:
		/// @return predicate value for a face region reference
		virtual bool pred(FaceRegion & fr)=0;
	public:
		virtual ~Predicate(){}
		/// @return value of pred
		bool operator()(PFaceRegion pfr);
	};
};

/// smart pointer to a face regions collection
typedef Ptr<FaceRegions> PFaceRegions;

/// image/face region describing an image area corresponding to a face/profile or a facial landmark, e.g. eye, nose, mouth or ear
struct LIBDCL FaceRegion: public FaceAttributeBase, public Rect
{
	FaceRegions mSubregions; ///< sub-regions collection
	/// Instantiate. 
	FaceRegion(const string & kind /**< region kind, e.g. f=face, p=profile */): FaceAttributeBase(kind), mSubregions(cFaceRegMinDim/2) {}
	/// Instantiate by reading the region from a text stream.
	FaceRegion(istream & s, ///< input stream
		bool bin=false ///< binary?
		);
	/**
	 * Instantiate.
	 * @param kind region kind
	 * @param r	region rectangle
	 * @param children children regions
	 */
	FaceRegion(const string & kind, const Rect & r, const FaceRegions & children): FaceAttributeBase(kind), Rect(r), mSubregions(children) {}
	/**
	 * Instantiate.
	 * @param kind region kind
	 * @param r	region rectangle
	 */
	FaceRegion(const string & kind, const Rect & r): FaceAttributeBase(kind), Rect(r) {}
	/**
	 * Instantiate.
	 * @param r region to copy from
	 */
	FaceRegion(const FaceRegion & r): mSubregions(cFaceRegMinDim/2) {*this=r;}
	/// Instantiate by estimating face and landmarks using anthropometry.
	FaceRegion(const string & kind, ///< region kind
		Point LI, ///< left (from the observer) eye center image coordinates
		Point RI, ///< right (from the observer) eye center image coordinates
		Point nose=Point(), ///< nose coordinates
		Point mouth=Point() ///< mouth coordinates
	);
	virtual ~FaceRegion(){}
	/**
	 * Copy the input region.
	 * @param r input tregion
	 * @return this reference
	 */
	FaceRegion & operator=(const FaceRegion & r)
	{
		Rect::operator=(r);
		Kind=r.Kind;
		mSubregions=r.mSubregions;
		return *this;
	}
	virtual void write(ostream & s)const override;
	/**
	 * Intersect/crop the region with the given rectangle.
	 * @param r rectangular boundary
	 */
	void operator&=(Rect r);
	/**
	 * Scale the region and its children by the given factor.
	 * @param s scale factor
	 */
	void operator*=(REALNUM s)
	{
		((Rect&)(*this))*=s;
		mSubregions*=s;
	}
	/**
	 * Transform the region and its children by the given matrix.
	 * @param M transformation matrix
	 */
	void operator*=(const Matx23f & M);
	/**
	 * Rotate the region.
	 * @param R rotation matrix
	 */
	void rotate(const Matx23f & R);
	/// Scale the face region about a point.
	void scale(REALNUM sx, ///< horizontal scale factor
		REALNUM sy=0, ///< vertical scale factor
		Point o = Point(0,0) ///< scale center
	);
	/// Scale the face region about its center.
	void stretch(REALNUM sx, ///< horizontal scale factor
		REALNUM sy=0, ///< vertical scale factor
		bool recurse=true ///< recurse on sub-regions?
	);
	/// @return region's aspect
	REALNUM aspect()const{ return FaceMatch::aspect(*this); }
	/// @return region's min(aspect, 1/aspect)
	REALNUM minAspect()const{ REALNUM a = aspect();  return a ? min(a, 1 / a) : 0; }
	/// @return region's diameter
	unsigned diameter()const{return max(width, height);}
	/**
	 * Print a face region as k[x,y;w,h] or k{[x,y;w,h] k[x,y;w,h]...}
	 * @param s output stream
	 * @param fr face region to output
	 * @return output stream
	 */
	LIBDCL friend ostream & operator<<(ostream & s, const FaceRegion & fr);
	/// expand the region to enclose sub-regions
	void encloseChildren();
	/// crop sub-regions to the region
	void cropChildren(){ mSubregions &= (*this-tl()); }
	/// @return true if the face region has an ear sub-region
	bool hasEar()const;
	/// @return region ID, if any
	string getID()const;
	/// @return face region pointer
	static const FaceRegion * cast(const FaceAttributeBase * p /**< base pointer */){return dynamic_cast<const FaceRegion *>(p);}
	/// @return face region pointer
	static FaceRegion * cast(FaceAttributeBase * p /**< base pointer */){return dynamic_cast<FaceRegion *>(p);}
	virtual void print(ostream & s)const override;
	virtual FaceAttributeBase * clone()const override { return new FaceRegion(*this); }
};

} // namespace FaceMatch
