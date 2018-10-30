
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
#include "ImgDscKeyPt.h"
#include <functional>
#include <unordered_map>

#include <opencv2/gpu/gpu.hpp>
using namespace cv::gpu;

using namespace std;

namespace FaceMatch
{

struct NonFreeInit
{
	NonFreeInit()
	{
		cv::initModule_nonfree();
	}
}nfi;

/**
 * Get median point of a sorted array.
 * @param v input sorted array
 * @return median
 */
template<typename T> inline
const T median(const vector<T> & v)
{
	unsigned s=v.size();
	if (!s) throw Exception("median is undefined for an empty vector");
	unsigned s2=s/2;
	if (s&1) return v[s2];
	return 0.5*(v[s2-1]+v[s2]);
}

REALNUM cReprojT=0; // >0, e.g. 4 if RANSAC is needed

template<class DscMatcher>
class SimpleMatcher: DscMatcher
{
	int mKNN;
	REALNUM mReprojT, mMinDistR;
	unsigned mMinMatchCnt;
	typedef vector<REALNUM> TMatchMask;

	enum EDistKind
	{
		dkMedian,
		dkMean,
		dkMax
	};

	void matchXCheck(const Mat& descriptors1, const Mat& descriptors2,
		TMatch & filteredMatches12,
		int knn=1, const Mat& mask=Mat())const
	{
		FTIMELOG
		filteredMatches12.clear();
		TMatches matches12, matches21;
	// NOTE: for win7+VS2012, sequential kNN matches work faster than as OMP-parallel sections
		this->knnMatch(descriptors1, descriptors2, matches12, knn, mask);
		this->knnMatch(descriptors2, descriptors1, matches21, knn, mask.empty() ? mask : mask.t());
		for (size_t m=0, mSize=matches12.size(); m<mSize; ++m)
		{
			bool foundXCheck = false;
			for (size_t fk=0, fkSize=matches12[m].size(); fk<fkSize; ++fk)
			{
				const DMatch & forward = matches12[m][fk];
				const auto & m21=matches21[forward.trainIdx];
				for (size_t bk=0, bkSize=m21.size(); bk<bkSize; ++bk)
				{
					const DMatch & backward = m21[bk];
					if (backward.trainIdx == forward.queryIdx)
					{
						filteredMatches12.push_back(forward);
						foundXCheck = true;
						break;
					}
				}
				if (foundXCheck) break;
			}
		}
	}
	/// Match descriptor in the landmark pre-set order
	void matchLM(const Mat& desc1, const Mat& desc2, TMatch & matches, const MatUC& mask=MatUC())const
	{
		FTIMELOG
		matches.clear();
		if (desc1.rows!=desc2.rows) throw Exception(format("matchLM error: desc1.rows=%d differ from desc2.rows=%d", desc1.rows, desc2.rows));
		bool all=mask.empty();
		for (unsigned r=0, R=desc1.rows; r<R; ++r)
		{
			TMatch match;
			if (all || mask(r, 1))
				this->match(desc1.row(r), desc2.row(r), match);
			matches.push_back(DMatch(r, r, match.size() ? match[0].distance : std::numeric_limits<float>::max()));
		}
	}

	/**
	 * Filter the outliers whose distance is greater than 2 min dist.
	 * @param MatchMask	real-valued vector with match weights
	 * @param matches	matches to be filtered
	 */
	unsigned filterMinDist(unsigned & MatchCnt, TMatchMask & MatchMask, const ImgDscKeyPt::Matches & matches)const
	{
		FTIMELOG
		if (!MatchCnt || mMinDistR<=0) return MatchCnt;
		REALNUM minDist=numeric_limits<REALNUM>::max();
		const int icnt=matches.size();
		for (int i=0; i<icnt; ++i)
		{
			if (!MatchMask[i]) continue;
			if (minDist>matches[i].distance) minDist=matches[i].distance;
		}
		const REALNUM radius=max<REALNUM>(2*minDist, mMinDistR); // e.g. 0.02
		for (int i=0; i<icnt; ++i)
		{
			if (!MatchMask[i]) continue;
			if (matches[i].distance>radius)
			{
				MatchMask[i]=0;
				--MatchCnt;
			}
		}
		return MatchCnt;
	}
	/**
	 * Filter key-point outliers by geometric distances consistency occurring during scale, shift or in-plane rotation.
	 * MEdian based Anomalous Distance Outlier Weeding (MEADOW) method that filters distances outside of the median diviation from the median sample.
	 * NOTE: more elaborate affine transformations can be handled by homography based RANSAC.
	 * @param MatchMask	real-valued vector with match weights
	 * @param matches	matches to be filtered
	 * @param object	object key point descriptor
	 * @param scene	scene key point descriptor
	 * @return matches count
	 */
	unsigned filterGeomDist(unsigned & MatchCnt, TMatchMask & MatchMask, const ImgDscKeyPt::Matches & matches, const ImgDscKeyPt & object, const ImgDscKeyPt & scene)const
	{
		if (!MatchCnt || mReprojT>0) return MatchCnt; // homography RANSAC will be used instead
		TIMELOG("SimpleMatcher::filterGeomDist")
		const KeyPoints
			&ObjKpts=object.getKeypoints(),
			&ScnKpts=scene.getKeypoints();
		const size_t MatchSize=matches.size();
	//--- sort/filter by the geometric distance
		struct GeomDist
		{
			unsigned ndx;
			REALNUM dst;
			GeomDist(unsigned andx, REALNUM adst): ndx(andx), dst(adst){}
		};
		vector<GeomDist> GeomDistances;
		for(size_t i=0, icnt=MatchSize; i<icnt; ++i)
		{
			if (!MatchMask[i]) continue;
			const unsigned
				ondx=matches[i].queryIdx,
				sndx=matches[i].trainIdx;
			const REALNUM d=norm(Mat(ObjKpts[ondx].pt), Mat(ScnKpts[sndx].pt));
			GeomDistances.push_back(GeomDist(i, d));
		}
		sort(GeomDistances, [](const GeomDist & a, const GeomDist & b){ return a.dst>b.dst; }); // descending
		const int
			N=GeomDistances.size(),
			mdnNdx=N/2;
		const REALNUM
			mdnDist=GeomDistances[mdnNdx].dst;
//			tol=2*MDist-GeomDistances.back().dst; // medVal+minDrop

	//--- compute median deviation
		for (auto & gd : GeomDistances) gd.dst = abs(gd.dst-mdnDist);
		sort(GeomDistances, [](const GeomDist & a, const GeomDist & b){ return a.dst>b.dst; }); // descending

		const REALNUM
			tol=GeomDistances[mdnNdx].dst;

	//--- mask out the outliers
		for(int i=0; i<mdnNdx; ++i, --MatchCnt)
			MatchMask[GeomDistances[i].ndx]=0;

		return MatchCnt;
	}
	/**
	 * Filter outliers by RANSAC/homography re-projection for in-plane rotation.
	 * NOTE: out-of-plane rotations can't be treated by this block.
	 * @param MatchMask	real-valued vector with match weights
	 * @param matches	matches to be filtered
	 * @param object	object key point descriptor
	 * @param scene	scene key point descriptor
	 * @return matches count
	 */
	unsigned filterHomography(unsigned & MatchCnt, TMatchMask & MatchMask, const ImgDscKeyPt::Matches & matches, const ImgDscKeyPt & object, const ImgDscKeyPt & scene)const
	{
		if (!MatchCnt || mReprojT<=0) return MatchCnt;
		FTIMELOG
		const KeyPoints
			&keypoints1=object.getKeypoints(),
			&keypoints2=scene.getKeypoints();
		const size_t MatchSize=matches.size();
		vector<int> queryIdxs(MatchSize), trainIdxs(MatchSize);
		for(size_t i=0, icnt=MatchSize; i<icnt; ++i)
		{
			if (!MatchMask[i]) continue;
			queryIdxs[i] = matches[i].queryIdx;
			trainIdxs[i] = matches[i].trainIdx;
		}
		vector<Point2f> points1, points2;
		KeyPoint::convert(keypoints1, points1, queryIdxs);
		KeyPoint::convert(keypoints2, points2, trainIdxs);
		Mat mask,
			H12 = findHomography(Mat(points1), Mat(points2), CV_RANSAC, mReprojT, mask);
		if (H12.empty()) return MatchCnt;
		for(unsigned i=0, len=points1.size(); i<len; ++i)
		{
			if (!mask.at<bool>(i))
			{
				MatchMask[i]=0;
				--MatchCnt;
			}
		}
		return MatchCnt;
	}
	/**
	 * Filter outliers by restricting matches to the corresponding face sub-regions.
	 * @param MatchMask	real-valued vector with match weights
	 * @param matches	matches to be filtered
	 * @param object	object key point descriptor
	 * @param scene	scene key point descriptor
	 * @return matches count
	 */
	unsigned filterSubRegions(unsigned & MatchCnt, TMatchMask & MatchMask, ImgDscKeyPt::Matches & matches, const ImgDscKeyPt & object, const ImgDscKeyPt & scene)const
	{
		if (!MatchCnt) return MatchCnt;
		if (object.getSubregions().empty() || scene.getSubregions().empty()) return MatchCnt;
 		for(int i=0, icnt=matches.size(); i<icnt; ++i)
		{
			if (!MatchMask[i]) continue;
			int ondx = matches[i].queryIdx, sndx = matches[i].trainIdx;
			const string
				&obRgnKind=object.getSubregionKind(ondx),
				&scRgnKind=scene.getSubregionKind(sndx);
			if (obRgnKind!=scRgnKind)
				if (isFaceKind(obRgnKind) || isFaceKind(scRgnKind))
					matches[i].distance*=2; // just punish
				else
				{
					MatchMask[i]=0;
					--MatchCnt;
				}
		}
		return MatchCnt;
	}

public:
	/// Instantiate.
	SimpleMatcher(
		unsigned MinMatchCnt=cMinKeyPtMatchCntDefault, ///< minimum number of key-point matches required for a successful match
		int KNN=1, ///< k-th nearest neighbor parameter; when 0, invoke a default match; when negative, invoke landmark-based match
		REALNUM ReprojT=cReprojT, ///< RANSAC Homography re-projection error tolerance
		REALNUM MinDistR=0 ///< minimum distance radius for filtering invalid matches
		): mMinMatchCnt(MinMatchCnt), mKNN(KNN), mReprojT(ReprojT), mMinDistR(MinDistR)
	{}
	/**
	 * Compute a distance between two key-point based descriptors. Thread-aware.
	 * @param object input descriptor
	 * @param scene input descriptor
	 * @param pMatches output match results
	 * @return real-valued distance in [0,1]
	 */
	REALNUM dist(const ImgDscKeyPt & object, const ImgDscKeyPt & scene, ImgDscKeyPt::Matches * pMatches)const
	{
		const string & OT=object.getType(), ST=scene.getType();
		TIMELOG("SimpleMatcher::dist::"+OT);
		CHECK(OT==ST, "object descriptor '"+OT+"' is different from scene type '"+ST+"'");
		if (object.getKeypoints().empty() || scene.getKeypoints().empty()) return 1; // max-out

	//--- populate mask by thresholding saliency weights
		MatUC DscMask; // to be used during descriptor matching
		const ImgDscKeyPt::Weights
			&ObjWts=object.getWeights(),
			&ScnWts=scene.getWeights();
		const unsigned
			ObjWtsSize=ObjWts.size(),
			ScnWtsSize=ScnWts.size();
		const REALNUM WeightT=1e-5; // TODO: param/config
		if (ObjWtsSize && ScnWtsSize)
		{
			DscMask=MatUC::ones(ObjWtsSize, ScnWtsSize);
			for (unsigned i=0; i<ObjWtsSize; ++i)
				for (unsigned j=0; j<ScnWtsSize; ++j)
					DscMask(i,j) = sqrt(ObjWts[i]*ScnWts[j]) > WeightT;
		}

		ImgDscKeyPt::Matches M,  &matches = pMatches ? *pMatches : M; // descriptor matches collection

		if (mKNN>0) matchXCheck(object.getVectors(), scene.getVectors(), matches, mKNN, DscMask);
		else if (mKNN<0) matchLM(object.getVectors(), scene.getVectors(), matches, DscMask);
		else this->match(object.getVectors(), scene.getVectors(), matches, DscMask);

		unsigned N=matches.size();
		if (N==0) return 1; // no matches

	//--- mask spurious matches
		unsigned MatchCnt=N;
		vector<REALNUM> MatchMask(MatchCnt, 1);
		if (!filterSubRegions(MatchCnt, MatchMask, matches, object, scene)) return 1;
		if (!filterMinDist(MatchCnt, MatchMask, matches)) return 1;
		if (!filterGeomDist(MatchCnt, MatchMask, matches, object, scene)) return 1;
		if (!filterHomography(MatchCnt, MatchMask, matches, object, scene)) return 1;

	//--- filter/diminish spurious matches
		ImgDscKeyPt::Matches GoodMatches;
		for (int i=0; i<N; ++i)
		{
			REALNUM w = MatchMask[i];
			if (w<WeightT)
				continue; // insignificant
			GoodMatches.push_back(matches[i]);
		}
		matches=GoodMatches; // TODO: use move semantics, if possible
		MatchCnt=matches.size();

	//--- accumulate the distance
		REALNUM res=1;
		EDistKind DK=dkMedian; // TODO: config/param
		switch(DK)
		{
		case dkMax:
			{
				REALNUM Dmax=0;
				for (int i=0, icnt=matches.size(); i<icnt; ++i)
				{
					REALNUM d = matches[i].distance;
					if (d>Dmax) Dmax=d;
				}
				res = Dmax;
			}
			break;
		case dkMedian:
			{
				int icnt=matches.size();
				vector<REALNUM> distances(icnt);
				for (int i=0; i<icnt; ++i)
					distances[i] = matches[i].distance;
				sort(distances.begin(), distances.end());
				REALNUM Dmid=median(distances);
				res = Dmid;
			}
			break;
		case dkMean:
			{
				REALNUM Dsum=0;
				for (int i=0, icnt=matches.size(); i<icnt; ++i)
				{
					REALNUM d = matches[i].distance;
					Dsum+=d;
				}
				res = Dsum/MatchCnt;
			}
			break;
		}
		if (res && MatchCnt<mMinMatchCnt) return 1; // max-out
		res=R2Unit(object.getDistScale()*res);
		if (res<0 || res>1) cerr<<"invalid dist="<<res<<endl;
		return res;
	}
};

static atomic<REALNUM> // optimized on CalTech
	sDistScaleSURF(0.4648), sDistScaleSURFFL(sDistScaleSURF.load()),
	sDistScaleSURFOM(4.6875), sDistScaleSURFFLOM(sDistScaleSURFOM.load()),
	sDistScaleSIFT(0.000333), sDistScaleSIFTFL(sDistScaleSIFT.load()),
	sDistScaleSIFTOM(0.0041), sDistScaleSIFTFLOM(sDistScaleSIFTOM.load()),
	sDistScaleORB(0.01211), sDistScaleORBFL(sDistScaleORB.load()),
	sDistScaleORBOM(0.02422), sDistScaleORBFLOM(sDistScaleORBOM.load());

#define implementKeyPtDscDefault(TYPE, Matcher, kNN) \
REALNUM ImgDsc##TYPE::dist(const ImgDscBase & a, Matches * matches)const\
{\
	static SimpleMatcher<Matcher> sMatcher(cMinKeyPtMatchCntDefault, kNN);\
	return sMatcher.dist(*this, dynamic_cast<const ImgDsc##TYPE&>(a), matches);\
}\
REALNUM ImgDsc##TYPE::getDistScale(bool OM)const{return OM ? sDistScale##TYPE##OM : sDistScale##TYPE;}\
void ImgDsc##TYPE::setDistScale(REALNUM s, bool OM)const{if (OM) sDistScale##TYPE##OM=s; else sDistScale##TYPE=s;}

const int cKNNDefault=1;
typedef BruteForceMatcher<L1<REALNUM>> BFLMetricMatcher;
typedef BruteForceMatcher<Hamming> BFHammingMatcher;

implementKeyPtDscDefault(SURF, BFLMetricMatcher, cKNNDefault)
implementKeyPtDscDefault(SURFFL, BFLMetricMatcher, -1)
implementKeyPtDscDefault(SIFT, BFLMetricMatcher, cKNNDefault)
implementKeyPtDscDefault(SIFTFL, BFLMetricMatcher, -1)
//implementKeyPtDscDefault(SUFT, BFLMetricMatcher, cKNNDefault)
//implementKeyPtDscDefault(SIRT, BFLMetricMatcher, cKNNDefault)

implementKeyPtDscDefault(ORB, BFHammingMatcher, cKNNDefault)
implementKeyPtDscDefault(ORBFL, BFHammingMatcher, -1)

ImgDscKeyPt::ImgDscKeyPt(const FileNode & fn): ImgDscBase(fn)
{
	cv::read(fn["Keypoints"], mKeyPoints);
	if (mKeyPoints.empty())
		throw Exception("no key-points loaded for "+fn.name());
	cv::read(fn["Descriptors"], mDescriptors);
	if (mDescriptors.empty())
		throw Exception("no descriptors loaded for "+fn.name());
	cv::read(fn["Weights"], mWeights);
}
ImgDscKeyPt::ImgDscKeyPt(const string & type, istream & s): ImgDscBase(s)
{
	read(s);
	if (mKeyPoints.empty())
		throw Exception("no key-points loaded for "+type);
	if (mDescriptors.empty())
		throw Exception("no descriptors loaded for "+type);
}
void ImgDscKeyPt::write(FileStorage& fs)const
{
	ImgDscBase::write(fs);
	cv::write(fs, "Keypoints", mKeyPoints);
	cv::write(fs, "Descriptors", mDescriptors);
	cv::write(fs, "Weights", mWeights);
}
void ImgDscKeyPt::print(ostream & s, const string & fmt)const
{
	ImgDscBase::print(s, fmt);
	unsigned VL = getVerbLevel();
	if (VL>1)
		s<<"Keypoints("<<mKeyPoints.size()<<")";
	if (VL>2)
		s<<"="<<mKeyPoints<<endl
		<<"Descriptors("<<mDescriptors.rows<<","<<mDescriptors.cols<<")="<<mDescriptors;
}
void ImgDscKeyPt::computeWeights(const Mat & img)
{
	const byte ChCnt=img.channels();
	if (ChCnt==2 || ChCnt==4) // populate weights using the saliency map
	{
		vector<Mat> channels; split(img, channels);
		MatUC alpha=channels[ChCnt-1];
		unsigned N=mKeyPoints.size();
		mWeights.assign(N, 1);
		for (unsigned i=0; i<N; ++i)
		{
			Point pt=mKeyPoints[i].pt;
			REALNUM v = alpha(pt);
			mWeights[i]=v/0xFF;
		}
	}
}
void ImgDscKeyPt::scaleSubRegions(const FaceRegions & SubRegs, Size OrgImgSize, unsigned dim)
{
    if (!SubRegs.size()) return;
	mSubRegions=SubRegs;
	const REALNUM sx=dim/(REALNUM)OrgImgSize.width, sy=dim/(REALNUM)OrgImgSize.height;
	mSubRegions.scale(sx,sy);
}

template<typename FTR>
const Feature2D & getFeature2D()
{
	StaticLkdCtor FTR ftr;
	return ftr;
}
const Feature2D & getFeature2D(const string & DscType)
{
	if (checkPrefix(DscType, "SURF")) return getFeature2D<SURF>();
	else if (checkPrefix(DscType, "SIFT")) return getFeature2D<SIFT>();
	else if (checkPrefix(DscType, "ORB")) return getFeature2D<ORB>();
	else throw Exception("unsupported descriptor kind: "+DscType);
}
void ImgDscKeyPt::init(const string & cvDscType, Image & ImgInOut, unsigned dim)
{
	TIMELOG(cvDscType+"::init");
	bool bScale=dim!=ImgInOut.dim();
	Image && img = bScale ? Image(ImgInOut, dim) : ImgInOut;
	Mat srcMx(img.mx());
	const byte ChCnt=srcMx.channels();
	Mat gsImg; if (ChCnt<3) gsImg = srcMx; else cvtColor(srcMx, gsImg, CV_BGR2GRAY);
	const auto & FaceRegions=ImgInOut.getFaceRegions();
	if (bScale) scaleSubRegions(FaceRegions, ImgInOut.size(), dim);
	if (cvDscType=="ORB")
	{
		const Feature2D & ext=getFeature2D<ORB>();
		static const Mat noM;
		ext(img.mx(), noM, mKeyPoints, mDescriptors);
		if (mKeyPoints.empty()) throw Exception("no key-points computed for ORB");
		if (mDescriptors.empty()) throw Exception("no descriptors computed for ORB");
	}
	else if (checkSuffix(cvDscType, "FL")) // populate mKeyPoints from the landmarks
	{
		enum { efrLEye, efrREye, efrNose, efrMouth, efrEar, efrCount };
		const FaceRegion * pFaceRegions[efrCount]; for (int i=0; i<efrCount; ++i) pFaceRegions[i]=0;
		for (const auto & e: FaceRegions)
		{
			if (!e) continue;
			int k=e->Kind[0];
			if (!isRectKind(k)) continue;
			const FaceRegion * pfr = FaceRegion::cast(e);
			if (!pfr) continue;
			switch (k)
			{
			case 'i': // TODO: more elaborate geometric reasoning
				if (!pFaceRegions[efrLEye]) pFaceRegions[efrLEye] = pfr;
				else if (!pFaceRegions[efrREye]) pFaceRegions[efrREye] = pfr;
				break;
			case 'n': pFaceRegions[efrNose]=pfr; break;
			case 'm': pFaceRegions[efrMouth]=pfr; break;
			case 'e': pFaceRegions[efrEar]=pfr; break;
			}
		}
		if (pFaceRegions[efrLEye] && pFaceRegions[efrREye] && pFaceRegions[efrLEye]->x > pFaceRegions[efrREye]->x)
			swap(pFaceRegions[efrLEye], pFaceRegions[efrREye]);
		const Feature2D & ftr = getFeature2D(cvDscType);
		KeyPoints kpts; // non-empty key-points
		const int KptRgnSize=9; // TODO: config/param
		for (int i=0; i<efrCount; ++i)
		{
			const FaceRegion * pfr=pFaceRegions[i];
			if (!pfr) continue;
			const FaceRegion & fr = *pfr;
			const REALNUM d=min(fr.width, fr.height);
			const Point c=center(*pfr);
			kpts.push_back(KeyPoint(c, d, -1, 0, 0, i)); // center
			// center-mid points
			kpts.push_back(KeyPoint(Point(fr.x, fr.y+fr.height/2), d, -1, 0, 0, i));
			kpts.push_back(KeyPoint(Point(fr.x+fr.width, fr.y+fr.height/2), d, -1, 0, 0, i));
			kpts.push_back(KeyPoint(Point(fr.x+fr.width/2, fr.y), d, -1, 0, 0, i));
			kpts.push_back(KeyPoint(Point(fr.x+fr.width/2, fr.y+fr.height), d, -1, 0, 0, i));
			// corner points
			kpts.push_back(KeyPoint(fr.tl(), d, -1, 0, 0, i));
			kpts.push_back(KeyPoint(fr.br(), d, -1, 0, 0, i));
			kpts.push_back(KeyPoint(Point(fr.x, fr.y+fr.height), d, -1, 0, 0, i));
			kpts.push_back(KeyPoint(Point(fr.x+fr.width, fr.y), d, -1, 0, 0, i));
		}
		//clog<<"kpts.size="<<kpts.size()<<endl; // TODO: remove
		Mat dscr; ftr(gsImg, Mat(), kpts, dscr, true); // non-empty descriptors
		//clog<<"kpts.size="<<kpts.size()<<", dscr.rows="<<dscr.rows<<endl; // TODO: remove
		if (dscr.rows!=kpts.size())
			throw Exception(format("computed %d descriptors for %d key-points", dscr.rows, kpts.size()));
		auto addEmptyKeyPtDscr = [&]()
		{
			static KeyPoint nop;
			StaticLkdCtor Mat nod=Mat::zeros(1, ftr.descriptorSize(), ftr.descriptorType());
			mKeyPoints.push_back(nop);
			mDescriptors.push_back(nod);
		};
		for (int i=0, j=0; i<efrCount; ++i)
			if (pFaceRegions[i] && j<kpts.size())
				for (int k=0; k<KptRgnSize; ++k)
				{
					if (kpts[j].class_id==i)
					{
						mKeyPoints.push_back(kpts[j]);
						mDescriptors.push_back(dscr.row(j));
						++j;
					}
					else addEmptyKeyPtDscr();
				}
			else for (int k=0; k<KptRgnSize; ++k) addEmptyKeyPtDscr();
	}
	else // detect key-points and L-metric descriptors
	{
		Ptr<FeatureDetector> fdp=0; // TODO: react to fixed landmarks flag
		if (cvDscType=="SUFT") fdp=FeatureDetector::create("SURF");
		else if (cvDscType=="SIRT") fdp=FeatureDetector::create("SIFT");
		else fdp=FeatureDetector::create(cvDscType);
		if (!fdp) throw Exception("unable to create feature detector for "+cvDscType);
		{ TIMELOG(cvDscType+"::detectKeyPts"+cvDscType);
		fdp->detect(gsImg, mKeyPoints); } // TODO: use GPU, if possible
		if (mKeyPoints.empty())
			throw Exception("no key-points computed for "+cvDscType);
	//--- compute descriptor
		Ptr<DescriptorExtractor> dep=0;
		if (cvDscType=="SUFT") dep=DescriptorExtractor::create("SIFT");
		else if (cvDscType=="SIRT") dep=DescriptorExtractor::create("SURF");
		else dep=DescriptorExtractor::create(cvDscType);
		if (!dep) throw Exception("unable to create descriptor extractor for "+cvDscType);
		{ TIMELOG(cvDscType+"::computeDesc"+cvDscType);
		dep->compute(gsImg, mKeyPoints, mDescriptors); } // TODO: use GPU, if possible
		if (mDescriptors.empty()) throw Exception("no descriptors computed for "+cvDscType);
	}
	computeWeights(srcMx);
	ImgInOut=img;
}
ImgDscKeyPt::ImgDscKeyPt(const string & cvDscType, const Image * pImgSrc, unsigned dim): mType("ImgDsc"+cvDscType)
{
	if (!pImgSrc) return;
	Image ImgSrc(*pImgSrc, dim);
	init(cvDscType, ImgSrc, dim);
}
ImgDscKeyPt::ImgDscKeyPt(const string & cvDscType, Image & ImgSrc, unsigned dim): mType("ImgDsc"+cvDscType)
{
	init(cvDscType, ImgSrc, dim);
}
const string & ImgDscKeyPt::getSubregionKind(unsigned ndx)const
{
	if (mSubRegions.empty()) return nemo;
	const KeyPoint & kpt = mKeyPoints[ndx];
	for (auto it=mSubRegions.begin(); it!=mSubRegions.end(); ++it)
	{
		if (!isFRRectKind((*it)->Kind)) continue;
		const FaceRegion& fr=dynamic_cast<const FaceRegion&>(**it);
		if (fr.contains(kpt.pt)) return fr.Kind;
	}
	return nemo;
}
void ImgDscKeyPt::write(ostream & s)const
{
	ImgDscBase::write(s);
	cv::write(s, mKeyPoints);
	cv::write(s, mDescriptors);
	writeVector(s, mWeights);
	mSubRegions.write(s);
}
void ImgDscKeyPt::read(istream & s)
{
	cv::read(s, mKeyPoints);
	cv::read(s, mDescriptors);
	readVector(s, mWeights);
	mSubRegions.read(s);
}

} // namespace FaceMatch
