
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

#include "common.h"
#include "FaceRegions.h"

namespace FaceMatch
{
ostream & operator<<(ostream & os, const Rect & r)
// [x,y;w,h]
{
	os<<'['<<r.x<<','<<r.y<<';'<<r.width<<','<<r.height<<']';
	return os;
}
/// print face region
ostream & operator<<(ostream & s, const FaceRegion & fr)
{
	fr.print(s);
	return s;
}
void FaceRegion::print(ostream & s)const
{
	FaceAttributeBase::print(s);
	bool hasChildren = mSubregions.size();
	if (hasChildren) s<<'{';
	s<<(const Rect&)(*this);
	if (hasChildren) s<<mSubregions<<'}';
}
ostream & operator<<(ostream & s, const FaceAttributeBase & a)
{
	a.print(s);
	return s;
}
/// print face regions
ostream & operator<<(ostream & s, const FaceRegions & rc)
{
	for (FaceRegions::const_iterator ir = rc.begin(); ir!=rc.end(); ++ir)
		s<<'\t'<<**ir;
	return s;
}
/**
 * Input a region collection from a text stream.
 * @param s	input text stream
 * @param rgs	region collection to be populated
 * @return	input stream
 */
istream & operator>>(istream & s, FaceRegions & rgs)
{
	for (;;)
	{
		char c = s.peek();
		if (c=='}') { s.get(); break; }
		if (isWhiteSpace(c)) s.get(); // skip whitespace
		else if (isRectKind(c))
			rgs.add(new FaceRegion(s));
		else rgs.add(new FaceAttribute(s));
	}
	return s;
}
istream & operator>>(istream & s, Rect & r)
{
	char c=0;
	if (s.peek()=='[') s.get(); // skip the bracket
	s>>r.x>>c>>r.y>>c>>r.width>>c>>r.height;
	if (s.peek()==']') s.get();
	return s;
}

FaceAttributeBase::FaceAttributeBase(istream & s, bool bin)
{
	if (s.eof())
		throw Exception("emtpy stream to construct FaceAttributeBase");
	if (bin) read(s, Kind);
	else Kind=s.get();
}
FaceAttribute::FaceAttribute(istream & s, bool bin): FaceAttributeBase(s, bin)
{
	if (s.eof())
		throw Exception("emtpy stream to construct FaceAttribute");
	if (bin) read(s, Value);
	else
	{
		s.get(); // skip the bracket
		getline(s, Value, ']');
	}
}

FaceRegion::FaceRegion(istream & s, bool bin): FaceAttributeBase(s, bin),
	mSubregions(cFaceRegMinDim/2, cFaceFeatureMinAspect)
{
	if (s.eof())
		throw Exception("emtpy stream to construct FaceRegion");
	if (bin)
	{
		readSimple(s,x);
		readSimple(s,y);
		readSimple(s,width);
		readSimple(s,height);
		mSubregions.read(s);
	}
	else
	{
		if (isFRRectKind(Kind)) // assume it's a rectangular face region
		{
			char c=s.peek();
			if (c=='[')
				s>>(Rect&)*this;
			else if (c=='{') // complex region
			{
				s.get();
				s>>(Rect&)*this;
				s>>mSubregions;
			}
		}
		else
		{
			s.seekg(-Kind.size(), ios_base::cur);
			throw Exception("invalid face region kind: "+Kind);
		}
	}
}
FaceRegion::FaceRegion(const string & kind,	Point LeftICtr, Point RightICtr, Point nose, Point mouth):
	FaceAttributeBase(kind), mSubregions(cFaceRegMinDim/2)
{
	const REALNUM // take inter-eye distance as basis for everything
		DistI2I = norm(LeftICtr-RightICtr),
	// face region and landmar diameters in proportion to inter-eye distance
		DiamFR = 2.5*DistI2I,
		DiamI = 1.2*DistI2I/2,
		DiamN = 1.4*DistI2I/2,
		DiamM = 1.1*DistI2I/2;
	const Point // auxilliary points
		EyeLineCtr = (LeftICtr+RightICtr)*0.5,
		ShiftTopLeft(DiamFR/2, DiamFR/3),
		Origin = EyeLineCtr - ShiftTopLeft,
		ShiftI = Point(DiamI/2, DiamI/2) + Origin,
		LeftEyeTL = LeftICtr - ShiftI,
		RightEyeTL = RightICtr - ShiftI,
		ShiftN = Point(DiamN/2, DiamN/2) + Origin,
		ShiftM = Point(DiamM, DiamM/2) + Origin;
	// face region dimensions
	x = Origin.x; y = Origin.y;
	width = height = DiamFR;
	// landmark dimensions
	const Size
		DimI(DiamI, DiamI),
		DimN(DiamN, DiamN),
		DimM(2 * DiamM, 1.1 * DiamM);
	mSubregions.add(new FaceRegion("i", Rect(LeftEyeTL, DimI)));
	mSubregions.add(new FaceRegion("i", Rect(RightEyeTL, DimI)));
	if (nz(nose)) mSubregions.add(new FaceRegion("n", Rect(nose-ShiftN, DimN)));
	if (nz(mouth)) mSubregions.add(new FaceRegion("m", Rect(mouth-ShiftM, DimM)));
}
void FaceRegion::write(ostream & s)const
{
	FaceAttributeBase::write(s);
	writeSimple(s,x);
	writeSimple(s,y);
	writeSimple(s,width);
	writeSimple(s,height);
	mSubregions.write(s);
}
void FaceRegion::encloseChildren()
{
	Point TL=tl();
	for (const auto & c: mSubregions)
	{
		if (!c) continue;
		if (!isRectKindSub(c->Kind[0])) continue;
		const FaceRegion * sr = FaceRegion::cast(c);
		if (!sr) continue;
		(*this)|=(*sr+TL);
	}
	if (TL!=tl()) mSubregions+=(TL-tl());
}
string FaceRegion::getID()const
{
	const auto it = mSubregions.findKind("d");
	return it == mSubregions.end() ? nemo : dynamic_cast<const FaceAttribute&>(**it).Value;
}
bool FaceRegion::hasEar()const
{
	for (const auto & c: mSubregions)
	{
		if (!c) continue;
		if (c->Kind=="e") return true;
	}
	return false;
}
FaceRegions::FaceRegions(const string & kind, const vector<Rect> & regs, unsigned MinDim, REALNUM AspectLimit):
	mMinDim(MinDim), mAspectLimit(AspectLimit)
{
	for (vector<Rect>::const_iterator ir=regs.begin(); ir!=regs.end(); ++ir)
		add(new FaceRegion(kind, *ir, FaceRegions(MinDim/regs.size())));
}
const FaceRegions & FaceRegions::operator=(const FaceRegions & src)
{
	clear();
	for (const auto & e:src) push_back(e->clone());
	mMinDim=src.mMinDim;
	mAspectLimit=src.mAspectLimit;
	return *this;
}
Rect FaceRegions::MatchOverlap(const Rect & a, const Rect & b, REALNUM tol)
{
	const static Rect o;
	Rect c(a&b);
	REALNUM A=a.area(), B=b.area(), C=c.area();
	if (!A || !B || !C) return o;
	REALNUM D = C/(A+B-C);
	return (1-D < tol) ? c : o;
}
const FaceRegion & getDominant(const FaceRegion & a, const FaceRegion & b)
{
	if (a.Kind==b.Kind) return a;
	if (!isFaceOrProfileKind(a.Kind)) return b;
	if (!isFaceOrProfileKind(b.Kind)) return a;
	if (a.Kind=="p" && a.hasEar()) return a;
	if (b.Kind=="p" && b.hasEar()) return b;
	if (a.Kind=="f") return a;
	if (b.Kind=="f") return b;
	return a;
}
/// @return face region as a combination of two given regions
PFaceRegion combine(const FaceRegion & given, ///< given face region
	const FaceRegion & found, ///< found face region
	bool intersect, ///< should sub-regions be set-intersected?
	REALNUM MatchT, ///< region overlap slack threshold
	FaceRegions::ERegRectCombo mode ///< region combination mode
	)
{
	PFaceRegion p = new FaceRegion(given); // at given
	FaceRegion & m = *p;
	m.Kind = getDominant(given, found).Kind;
	m.mSubregions+=(m.tl()-found.tl()); // shift children to found for merging
	m.mSubregions.merge(found.mSubregions, intersect, MatchT, mode);
	((Rect&)m) = ((m.Kind=="s" || mode==FaceRegions::rcMax) ? (given|found) :
			(mode == FaceRegions::rcMin) ? (given&found) : average(given, found));
	m.mSubregions+=(found.tl()-m.tl()); // shift children to m
	m.cropChildren();
	return p;
}
void FaceRegions::add(PFaceAttribute pAttr)
{
	if (!pAttr) return;
	for (PFaceAttribute e: *this)
		if (e && pAttr->Kind==e->Kind)
		{
			e->Value = pAttr->Value;
			return;
		}
	push_back(pAttr);
}
PFaceRegion FaceRegions::add(PFaceRegion pRgn, bool intersect, REALNUM MatchT, ERegRectCombo mode)
{
	if (!pRgn) return 0;
	const FaceRegion & rgn = *pRgn;
	if (rgn.diameter() < mMinDim) return 0;
	if (rgn.Kind!="s")
	{
		const REALNUM asp = aspect(rgn);
		if (asp < mAspectLimit || 1/asp < mAspectLimit) return 0;
	}
	static const REALNUM StretchFactor=1.2; // TODO: param/config
	const int minArea = mMinDim*mMinDim;
	const auto OrgSize = size();
//--- insert a rectangular region
	OMPLocker lkr(mLock);
	for (iterator rit=begin(); rit!=end(); ) // merging with possible iterator removal/replacement
	{
		FaceRegion
			&xfr = (FaceRegion&)**rit, // given
			&rgn = *pRgn; // found
		if (isFRRectKind(xfr.Kind))
		{
			auto combineErase=[&](FaceRegion & a, FaceRegion & b)
			{
				pRgn = combine(a, b, intersect, MatchT, mode);
				rit = erase(rit);
			};
			auto swallowContained=[&](FaceRegion & a, FaceRegion & b) -> bool
			{
				Rect A=a; FaceMatch::stretch(A, StretchFactor);
				bool cnt = b<=A;
				if (cnt)
					combineErase(a,b);
				return cnt;
			};
			if (xfr.Kind==rgn.Kind)
			{
			//--- test overlap
				Rect C;
				if (xfr.Kind!="s") // non-skin?
				{
					C = FaceRegions::MatchOverlap(xfr, rgn, MatchT);
					if (C.area()>minArea) // overlap enough
					{
						combineErase(xfr, rgn);
						continue;
					}
				}
			//--- test for containment
				if (swallowContained(xfr, rgn)) continue;
				if (swallowContained(rgn, xfr)) continue;
				if (xfr.Kind!="s")
				{
				//--- test for penetration overlap
					C=xfr&rgn;
					if (C.area()>MatchT*rgn.area())
						return 0; // ignore small found region as penetrating a large given
					if (C.area()>MatchT*xfr.area())
					{
						rit = erase(rit);
						continue; // remove small given region as penetrating the large found
					}
				}
			}
			else // different kinds: prefer faces over ear-less profiles
			{
				if (xfr.Kind=="s" || rgn.Kind=="s") { ++rit; continue; } // no merging of skin with another kind
				Rect C(FaceRegions::MatchOverlap(xfr, rgn, MatchT));
				auto resolveDifferences = [&]()-> PFaceRegion
				// if a profile contains an ear, prefer it over the face
				{
					if (xfr.Kind=="p") // rgn.Kind=="f"
					{
						if (xfr.hasEar()) return 0;
						else rit=erase(rit);
					}
					else if (rgn.Kind=="p") // fr.Kind=="f"
					{
						if (rgn.hasEar()) rit=erase(rit);
						else return 0;
					}
					else rit=erase(rit);
					return pRgn;
				};
				if (C.area()>minArea)
				{
					if (!resolveDifferences()) return 0;
					continue;
				}
			//--- test for containment
				if (swallowContained(xfr, rgn)) continue;
				if (swallowContained(rgn, xfr)) continue;
			//--- test for penetration overlap
				C=xfr&rgn;
				if (C.area()>MatchT*xfr.area() || C.area()>MatchT*rgn.area())
				{
					if (!resolveDifferences()) return 0;
					continue;
				}
			}
		}
		++rit;
	}
	if (size()==OrgSize && intersect) return 0; // reject, if it did not merge with given set
	push_back(pRgn); // union
	return pRgn;
}
FaceRegions::iterator FaceRegions::findBestMatch(const PFaceRegion pRgn, REALNUM RegMatchTol)
{
	const FaceRegion & rgn = *pRgn;
	iterator itSameKind=end(), itDiffKind=end();
	const unsigned minArea = mMinDim*mMinDim;
	unsigned
		maxOverlapSameKind=0,
		maxOverlapDiffKind=0;
	for (auto rit=begin(); rit!=end(); ++rit)
	{
		if (!isFRRectKind((*rit)->Kind)) continue;
		const FaceRegion & fr = (const FaceRegion &)**rit;
		Rect C = FaceRegions::MatchOverlap(fr, rgn, RegMatchTol);
		unsigned a = C.area();
		if (a<minArea) continue;
		if (fr.Kind==rgn.Kind)
		{
			if (a>maxOverlapSameKind) { itSameKind=rit; maxOverlapSameKind=a; }
		}
		else
		{
			if (a>maxOverlapDiffKind) { itDiffKind=rit; maxOverlapDiffKind=a; }
		}
	}
	return itSameKind==end() ? itDiffKind : itSameKind; // prefer same kind
}
void FaceRegions::add(const string & s)
{
	stringstream strm(s);
	add(new FaceRegion(strm));
}
bool FaceRegions::removeFirstContainig(const Point & p)
{
	for (iterator it=begin(); it!=end(); ++it)
	{
		if (!isFRRectKind((*it)->Kind)) continue;
		FaceRegion & fr = dynamic_cast<FaceRegion&>(**it);
		if (fr.contains(p))
		{
			if (fr.mSubregions.removeFirstContainig(p-fr.tl())) return true;
			erase(it);
			return true;
		}
	}
	return false;
}
void FaceRegions::prime(const Point & p)
{
	for (iterator it=begin(); it!=end(); ++it)
	{
		if (!isFaceKind((*it)->Kind)) continue;
		FaceRegion & fr = dynamic_cast<FaceRegion&>(**it);
		if (fr.contains(p)) std::swap(*it, *begin());
	}
}
void FaceRegions::merge(const FaceRegions & rgs, bool intersect, REALNUM MatchT, ERegRectCombo mode)
{
	for (const PFaceAttributeBase & p: rgs)
	{
		const PFaceRegion & rgn=p;
		if (rgn) add(rgn, intersect, MatchT, mode);
		else add((PFaceAttribute&)p);
	}
}
void FaceRegions::operator&=(Rect r)
{
	for (iterator it=begin(); it!=end(); ++it)
		if (isFRRectKind((*it)->Kind))
			dynamic_cast<FaceRegion&>(**it) &= r;
}
void FaceRegions::operator*=(REALNUM s)
{
	for (auto pfr: *this)
		if (isFRRectKind(pfr->Kind))
			dynamic_cast<FaceRegion&>(*pfr) *= s;
}
void FaceRegions::scale(REALNUM sx, REALNUM sy)
{
	for (iterator it=begin(); it!=end(); ++it)
		if (isFRRectKind((*it)->Kind))
			dynamic_cast<FaceRegion&>(**it).scale(sx,sy);
}
void FaceRegions::stretch(REALNUM sx, REALNUM sy, bool recurse)
{
	if (!sy) sy = sx;
	for (iterator it = begin(); it != end(); ++it)
		if (isFRRectKind((*it)->Kind))
			dynamic_cast<FaceRegion&>(**it).stretch(sx, sy, recurse);
}
void FaceRegions::rotate(const Matx23f & R)
{
	for (iterator it=begin(); it!=end(); ++it)
		if (isFRRectKind((*it)->Kind))
			dynamic_cast<FaceRegion&>(**it).rotate(R);
}
void FaceRegions::operator*=(const Matx23f & M)
{
	for (auto pfr: *this)
		if (isFRRectKind(pfr->Kind))
			dynamic_cast<FaceRegion&>(*pfr) *= M;
}
void FaceRegions::operator+=(const Point & p)
{
	for (iterator it=begin(); it!=end(); ++it)
		if (isFRRectKind((*it)->Kind))
			dynamic_cast<FaceRegion&>(**it) += p;
}
void FaceRegions::write(ostream & s)const
{
	writeSimple(s, size());
	for (auto it=begin(); it!=end(); ++it) (*it)->write(s);
}
void FaceRegions::read(istream & s, bool bClear)
{
	if (bClear) clear();
	size_type len; readSimple(s, len);
	resize(len);
	for (size_type i=0; i<len; ++i) at(i) = new FaceRegion(s, true);
}
/// face region orderer w.r.t. the image/container center and region dominance: larger, more central regions go first
class Orderer
{
	const Point C;
	const bool mAscDst2Ctr;
	const REALNUM mDimRatio;
	/// @return	does a precede b?
	bool LT(const FaceRegion * a, ///< face region pointer; NULL in case of an attribute
		const FaceRegion * b ///< face region pointer; NULL in case of an attribute
	)
	{
		if (!a) return false; // b precede a
		if (!b) return true; // a precede b
		const Point aC(center(*a)-C), bC(center(*b)-C);
		const REALNUM
			aD=norm(aC), bD=norm(bC), diff=fabs(aD-bD),
			aDim=a->diameter(), bDim=b->diameter(),
			minDim=min(aDim, bDim), maxDim=max(aDim, bDim);
		if (diff<minDim && minDim < mDimRatio*maxDim)
		{
			if (a->Kind==b->Kind)
				return aDim>bDim; // let size play a role
			return a->Kind < b->Kind; // prefer faces over profiles
		}
		bool asc = aD<bD;
		return mAscDst2Ctr ? asc : !asc;
	}
public:
	/// Instantiate.
	Orderer(const Size & dim, ///< container/image dimensions used for central point calculation
		bool AscDst2Ctr=true, ///< ascending distance to center?
		const REALNUM DimRatio=0.75 ///< dimensions ratio for region dominance calculation
		): C(dim.width/2, dim.height/2), mAscDst2Ctr(AscDst2Ctr), mDimRatio(DimRatio) {}
	/// @return the imposed ordering *a < *b ?
	bool operator()(const FaceAttributeBase * a, ///< left hand side
		const FaceAttributeBase * b ///< right hand side
		)
	{
		return LT(dynamic_cast<const FaceRegion*>(a), dynamic_cast<const FaceRegion*>(b));
	}
};
void FaceRegions::sort(const Size & dim, bool AscDst2Ctr, REALNUM DimRatio)
{
	std::stable_sort(begin(), end(), Orderer(dim, AscDst2Ctr, DimRatio)); // NOTE: stable_sort tends to produce spurious null ptrs to face regions
}
FaceRegions::const_iterator FaceRegions::findKind(const string & k)const
{
	for (auto it=begin(); it!=end(); ++it)
		if ((*it)->Kind==k) return it;
	return end();
}
unsigned FaceRegions::count(const string & kinds)const
{
	unsigned cnt=0;
	for (const auto & e: *this)
		if (contains(kinds, e->Kind)) ++cnt;
	return cnt;
}
void stretch(Rect & r, REALNUM sx, REALNUM sy)
{
	Size sz = r.size();
	sz.width*=sx;
	sz.height*=(sy ? sy : sx);
	r.x+=(r.width-sz.width)/2.0;
	r.y+=(r.height-sz.height)/2.0;
	r.width = sz.width;
	r.height = sz.height;
}
Rect getStretched(const Rect & r, REALNUM sx, REALNUM sy)
{
	Rect o=r; stretch(o, sx, sy); return o;
}
/// @return the result of matrix-vector M*c multiplication
const Point2f & operator*=(Point2f & c, ///< vector/point to be multiplied by the matrix
	const Matx23f & M ///< transformation matrix
	)
{
	Matx31f p(c.x,c.y,1);
	Matx21f o=M*p;
	c.x=o(0,0); c.y=o(1,0);
	return c;
}
void FaceRegion::operator*=(const Matx23f & M)
{
	mSubregions+=tl(); // pre-offset sub-regions
	Point c=center(*this);
	RotatedRect rr(c, size(), 0);
	const unsigned vrtxCnt=4; Point2f vrtx[vrtxCnt]; rr.points(vrtx);
	Point TL=tl(), BR=br();
	for (unsigned i=0; i<vrtxCnt; ++i)
	{
		const Point2f & v = vrtx[i]*=M; // transform
		if (v.x<TL.x) TL.x=v.x; if (v.y<TL.y) TL.y=v.y;
		if (v.x>BR.x) BR.x=v.x; if (v.y>BR.y) BR.y=v.y;
	}
	((Rect&)*this)=Rect(TL, BR); // new bounding box
	mSubregions*=M;
	mSubregions+=-tl(); // un-offset sub-regions
}
void FaceRegion::rotate(const Matx23f & R)
{
	mSubregions+=tl(); // pre-offset subregions
	Point2f v=center(*this); v*=R; // rotate
	const unsigned W2=width/2, H2=height/2;
	((Rect&)*this)=Rect(v.x-W2, v.y-H2, width, height); // new bounding box
	mSubregions.rotate(R);
	mSubregions+=-tl(); // un-offset subregions
}
void FaceRegion::scale(REALNUM sx, REALNUM sy, Point o)
{
	x=sx*(x-o.x)+o.x; width*=sx;
	if (!sy) sy = sx;
	y=sy*(y-o.y)+o.y; height*=sy;
	mSubregions.scale(sx,sy);
}
void FaceRegion::stretch(REALNUM sx, REALNUM sy, bool recurse)
{
	Point TL = tl();
	FaceMatch::stretch(*this, sx, sy);
	mSubregions += TL - tl();
	if (recurse) mSubregions.stretch(sx, sy, recurse);
}
void FaceRegion::operator&=(Rect r)
{
	Point TL = tl();
	((Rect&)(*this)) &= r;
	mSubregions += TL - tl();
	cropChildren();
}
bool FaceRegions::Predicate::operator()(PFaceRegion pfr)
{
	bool res = pfr && pred(*pfr);
	if (res && getVerbLevel()>1)
		clog<<"pred "<<*pfr<<endl;
	return res;
}
} // namespace FaceMatch
