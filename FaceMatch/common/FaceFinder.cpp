
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
#include "FaceFinder.h"
#include "ResultsDisplay.h"
#include "ListStreamer.h"
#include "ColorFaceLandmarks.h"

using namespace std;

namespace FaceMatch
{
const unsigned
	cMinSubDim=48, cMinSubDim2=64,
	cMinSubDimGPU = cMinSubDim2, cMinSubDim2GPU = cRegDimMin4GPU;

const REALNUM cScale4RotMrg=0.21;

class Trivial : public FaceRegions::Predicate
{
	const unsigned mMinDim;
	const REALNUM mMinAspect;
protected:
	virtual bool pred(FaceRegion & fr)override
	{
		return fr.diameter() < mMinDim || fr.minAspect() < mMinAspect;
	}
public:
	Trivial(const FaceRegions & regs) : mMinDim(regs.getMinDim()), mMinAspect(regs.getAspectLimit()) {}
};
class BasicFaces: public FaceRegions::Predicate
{
	const bool mGPU=false;
	const unsigned minSubDim=0, minSubDim2=0;
	/// @return basic face/profile, e.g. to be removed?
	virtual bool pred(FaceRegion & rgn) override
	{
		FaceRegions & subs = rgn.mSubregions;
		subs.remove(Trivial(subs));
		return OK2remove(rgn);
	}
public:
	BasicFaces(bool bGPU): mGPU(bGPU),
		minSubDim(bGPU ? cMinSubDimGPU : cMinSubDim),
		minSubDim2(bGPU ? cMinSubDim2GPU : cMinSubDim2)
	{}
	bool OK2remove(const FaceRegion & rgn)const
	{
		if (rgn.Kind=="u") return false;
		unsigned dim = rgn.diameter(), cnt = rgn.mSubregions.count(cRectKindSub);
		return (dim > minSubDim && cnt<1) || (!mGPU && dim > minSubDim2 && cnt<2);
	}
};

/// Rotate image regions by a given pi/4 phase.
void turn(FaceRegions & regions, ///< image regions collection
	int phase, ///< one of the three pi/4 rotation phases: rpCCW, rpUD, rpCW
	const Size & TargetParentSize ///< target parent (of regions) size
)
{
	phase = phase % 4;	if (!phase) return;
	for (auto & er: regions)
	{
		if (!isFRRectKind(er->Kind)) continue;
		FaceRegion & fr = dynamic_cast<FaceRegion &>(*er);
		int x=0, y=0, w=0, h=0;
		switch (phase)
		{
		case Image::tphCCW:
			x = fr.y;
			y = TargetParentSize.height - (fr.x + fr.width);
			w = fr.height;
			h = fr.width;
			break;
		case Image::tphUD:
			x = TargetParentSize.width - (fr.x + fr.width);
			y = TargetParentSize.height - (fr.y + fr.height);
			w = fr.width;
			h = fr.height;
			break;
		case Image::tphCW:
			x = TargetParentSize.width - (fr.y + fr.height);
			y = fr.x;
			w = fr.height;
			h = fr.width;
			break;
		}
		fr.x = x;
		fr.y = y;
		fr.width = w;
		fr.height = h;
		turn(fr.mSubregions, phase, fr.size());
	}
}
void unturn(FaceRegions & regions, int phase, const Size & SourceParentSize)
{
	Size TargetParentSize(SourceParentSize.height, SourceParentSize.width);
	if (phase==Image::tphCCW) phase=Image::tphCW;
	else if (phase==Image::tphCW) phase=Image::tphCCW;
	else TargetParentSize=SourceParentSize;
	turn(regions, phase, TargetParentSize);
}
bool FaceFinder::gotFaces(bool relaxed)const
{
	if (relaxed) return mFaces.count(cFaceKind);
	if (mFaces.hasKind("u")) return true;
	bool bCascade=mFlags&cascade;
	for (const auto & p: mFaces)
	{
		if (!isFaceOrProfileKind(p->Kind)) continue;
		if (!bCascade) return true;
		const FaceRegion & fr=dynamic_cast<const FaceRegion &>(*p);
		if (fr.diameter()<cFaceRegMinDim) continue;
		BasicFaces bf(mFRD.usingGPU());
		if (!bf.OK2remove(fr)) return true;
		for (const auto & c: fr.mSubregions) // look for sub-regions
			if (isRectKindSub(c->Kind)) return true;
	}
	return false;
}
inline FaceRegions::ERegRectCombo getRegRectCombo(unsigned flags)
{
	return (flags&FaceFinder::tight) ? FaceRegions::rcMin : FaceRegions::rcDefault;
}
void FaceFinder::merge(FaceRegions & regions, const FaceRegions & found)
{
	if (regions.empty()) regions = found;
	else regions.merge(found, mFlags&intersect, cOverlapSlackTDefault, getRegRectCombo(mFlags));
}
void FaceFinder::removeBasicRegions(FaceRegions & regions)
{
	regions.remove(BasicFaces(mFRD.usingGPU()));
}
void FaceFinder::clearLandmarks(FaceRegions & regions)
{
	for (auto & fr: regions)
	{
		if (!isFRRectKind(fr->Kind)) continue;
		FaceRegion & rgn = dynamic_cast<FaceRegion &>(*fr);
		for (FaceRegions::iterator it=rgn.mSubregions.begin(); it!=rgn.mSubregions.end(); )
		{
			if (isFRRectKind((*it)->Kind))
				it = rgn.mSubregions.erase(it);
			else ++it;
		}
	}
}
void FaceFinder::detectRotatedRegions(FaceRegions & regions, RegionDetector & RD, const Mat & img)
{	FTIMELOG
	const bool bCascade=mFlags&cascade, bIntersect=mFlags&intersect;
	const FaceRegions::ERegRectCombo rcMethod = getRegRectCombo(mFlags);
	auto detect = [&](FaceRegions & regions, const Mat & img)
	{
		RD.detect(regions, img, bCascade, bIntersect, cOverlapSlackTDefault, rcMethod);
	};
	if (mFlags&rotation)
	{
		ParallelErrorStream PES;
		auto detectRotationPhases = [&](FaceRegions & regions, const Image & src)
		{
			FaceRegions res;
		#pragma omp parallel for shared(res)
			for (int tph=Image::tphZero; tph<Image::tphCount; ++tph)
				try
				{
					Image tmp(src, (Image::ETurnPhase)tph);
					const Size sz = tmp.size();
					FaceRegions rgs=regions;
					turn(rgs, tph, sz);
					detect(rgs, tmp.mx());
					unturn(rgs, tph, sz);
				#pragma omp critical(RGNS_MERGE)
					{
						if (getVerbLevel()>2)
							clog<<"phase "<<tph<<": rgs="<<rgs<<endl;
						merge(res, rgs);
					}
				}
				PCATCH(PES, format("phase %d region detection", tph))
			PES.report("four-way rotated region detection errors");
			regions = res;
		};
		detectRotationPhases(regions, img);
	//--- multi-way image rotations
		if (mFlags&rotationMultiway)
		{
			TIMELOG("detectRotatedRegions::multiway");
			static const int PISlices=6, // TODO: config/param
				steps=PISlices/2-1;
			static const REALNUM delta=180/PISlices;
			const int // pad the region for more robust face region detection
				wmargin = cScale4RotMrg*img.cols,
				hmargin = cScale4RotMrg*img.rows;
			Mat imgPad; copyMakeBorder(img, imgPad, hmargin, hmargin, wmargin, wmargin, BORDER_CONSTANT);
		#pragma omp parallel for shared(regions, imgPad, PES)
			for (int k=1; k<=steps; ++k)
				try
				{
					const REALNUM a=k*delta;
				#pragma omp critical (ROT_SUBANGLE)
					if (getVerbLevel()>2)
						clog<<"k="<<k<<", a="<<a<<endl; // TODO: comment
					Image rimg(rotate(imgPad, a)); // TODO: use GPU streams?
					FaceRegions rgs(mFRD.FaceDiameterMin);
					detectRotationPhases(rgs, rimg);
					rgs.rotate(getRotMx(imgPad.size(), -a)); // unrotate
					rgs+=Point(-wmargin, -hmargin); // unpad
					merge(regions, rgs);
				}
				PCATCH(PES, format("region rotation phase=%d angle=%d degrees", k, int(k*delta)))
			PES.report("multi-way rotated region detection errors");
		}
	}
	else detect(regions, img);
	if (mFlags&(keepCascaded)) removeBasicRegions(regions);
}
void FaceFinder::showFaces()
{
	if (mScaledImage.empty()) return;
	ResultsDisplay rd(mImgFN, mScaledImage, this);
	bool live=mFlags&LiveFeed, block=!live;
	mLastKey=rd.run(block); // show found regions, capture user key
	if (VisualVerbose()) rd.save(mImgOutDir+"FF.Results.jpg");
	if (block) destroyAllWindows();
}

/// @return	average of the color channels
Mat avgChannels(const Mat_<Vec3b> & src /**< source image */)
{
	Size dim = src.size();
	Mat_<uchar> dst(dim);
	for (int i = 0; i < src.rows; ++i)
		for (int j = 0; j < src.cols; ++j)
		{
			const Vec3b & p = src(i,j);
			int v = 0;
			for (int c=0; c<3; ++c) v+=p[c];
			dst(i,j)=v/3;
		}
	return dst;
}

/**
 * Compute a color-balanced version of an image.
 * @param src	input image
 * @param mean	mean color vector
 * @param lvl	gray-point level
 * @return	color-balanced version of the input image
 */
Mat colorBalance(const Mat_<Vec3b> & src, const Vec3b & mean, uchar lvl=0xFF)
{
	Mat_<Vec3b> dst(src.size());
	for (int i = 0; i < src.rows; ++i)
		for (int j = 0; j < src.cols; ++j)
		{
			const Vec3b & s = src(i,j);
			Vec3b & d = dst(i,j);
			for (int c=0; c<3; ++c)
			{
				if (mean[c])
					d[c] = saturate_cast<uchar>(lvl*s[c]/mean[c]);
				else d[c] = s[c];
			}
		}
	return dst;
}

/**
 * Compute a skin-tone enhanced version of an image.
 * @param src	input image
 * @param mean	mean color vector
 * @param var	color variance vector
 * @param level	skin-point level
 * @return skin-tone enhanced version of the input image
 */
Mat skinEnhance(const Mat_<Vec3b> & src, const Vec3b & mean, const Vec3d & var, uchar level=0xFF)
{ // TODO: use GPU, if available
	Mat_<Vec3b> dst = src.zeros(src.size());
	for (int i = 0; i < src.rows; ++i)
		for (int j = 0; j < src.cols; ++j)
		{
			const Vec3b & s = src(i,j);
			REALNUM e = norm(s-mean);
			Vec3b & d = dst(i,j);
			for (int c=0; c<3; ++c)
			{
				if (mean[c])
				{
					REALNUM k = exp(-e*e/2/var[c]);
					d[c] = min<REALNUM>(255, k*level*s[c]/mean[c]);
				}
				else d[c] = s[c];
			}
		}
	return dst;
}

/**
 * Compute a mean of a vector, typically representing a color pixel.
 * @param p	input vector/pixel
 * @return	mean value of input vector components
 */
inline
REALNUM mean(const Vec3d & p)
{
	REALNUM s=0;
	for (int c=0; c<3; ++c) s+=p[c];
	return s/3;
}

/**
 * Compute a skin-enhanced gray-scale version of the input image.
 * @param srcImg	input image
 * @param mean	mean color vector
 * @param Cov	color covariance matrix
 * @return skin-enhanced gray-scale version of the input image
 */
Mat skinEnhanceCovGS(const Mat_<Vec3b> & srcImg, const Vec3d mean, const Mat & Cov)
{
	Mat iCov; invert(Cov, iCov, DECOMP_SVD);
	Mat_<uchar> dstImg(srcImg.size());
	for (int r=0; r<srcImg.rows; ++r)
		for (int c=0; c<srcImg.cols; ++c)
		{
			const Vec3d & p = srcImg(r,c);
			dstImg(r,c) = FaceMatch::mean(p)*ColorLikelihood(p, mean, iCov);
		}
	return dstImg;
}

/**
 * Compute maximal absolute difference of the two input matrices.
 * @param a	input matrix
 * @param b	input matrix
 * @return	real-valued absolute difference of the values
 */
double maxAbsDiff(const Mat & a, const Mat & b)
{
	Mat d; absdiff(a, b, d);
	double minv, maxv;
	minMaxIdx(d, &minv, &maxv);
	return maxv;
}
const REALNUM cColorCorrT = 1e-4; ///< default correlation tolerance
/// \return	are image channels correlated?
bool areImgBandsCorrelated(const Mat & img, ///< input image
	REALNUM tol = cColorCorrT ///< correlation tolerance
	)
{	FTIMELOG
	vector<Mat> ch; split(img, ch);
	for (byte i=1, len=ch.size(); i<len; ++i)
	{
		MatReal d = match(ch[i], ch[0]);
		if (d(0,0) < 1-tol) return false;
	}
	return true;
}
/// \return	is input image gray-scale?
inline bool isGrayScale(const Mat & img, ///< input image
	REALNUM tol = cColorCorrT ///< correlation tolerance
	)
{
	return img.channels()==1 || areImgBandsCorrelated(img, tol);
}
void FaceFinder::detectFacesGS(FaceRegions & FaceRgns, const Mat & img, bool eqHist)
{	FTIMELOG
	Mat ImgGS = GrayScale(img);
	if (eqHist && VisualVerbose()) imOut(mImgOutDir+"preEQ", ImgGS);
	if (eqHist) equalizeHist(ImgGS, ImgGS);
	if (VisualVerbose()) imOut(mImgOutDir+"preVJ", ImgGS);
	if (getVerbLevel() > 1)
		clog << __FUNCTION__ << endl
		<< " FaceRgns=" << FaceRgns << endl;
	FaceRegions Faces = FaceRgns, Profiles = FaceRgns; // without this initialization multi-way rotated detection is slow
	ParallelErrorStream PES;
#pragma omp parallel sections shared(PES, ImgGS)
	{
	#pragma omp section
		try
		{
			if (!(mFlags&ignoreFrontal)) detectRotatedRegions(Faces, mFRD.FrontalDetector, ImgGS);
		}
		PCATCH(PES, "detect rotated faces")
	#pragma omp section
		try
		{
			if (!(mFlags&ignoreProfile)) detectRotatedRegions(Profiles, mFRD.ProfileDetector, ImgGS);
		}
		PCATCH(PES, "detect rotated profiles")
	}
	PES.report("detect rotated regions in parallel errors");
	if (getVerbLevel() > 1)
		clog << " Profiles=" << Profiles << endl
		<< " Faces=" << Faces << endl;
	merge(FaceRgns, Profiles);
	merge(FaceRgns, Faces);
	if (getVerbLevel()>1)
		clog<<" FaceRgns="<<FaceRgns<<endl;
}
void FaceFinder::detectSubregionsGS(FaceRegions & SR, const Mat & EnhFRImg, const Mat & FRImg)
{	FTIMELOG
	detectFacesGS(SR, EnhFRImg, false); // skin-enhanced region
	detectFacesGS(SR, FRImg, true); // histogram EQ works differently on a patch than on the whole image
}
void FaceFinder::initSkinMap(const Mat & img)
{// TODO: options for using global, local and augmented (global+local) skin mapper
	FTIMELOG
	mSkinToneMapperImage = mSkinColorSamples.rows ?
		mFRD.SkinToneMapper.create(img, mSkinColorSamples, mImgOutDir) :
		mFRD.SkinToneMapper.create(img, mImgOutDir);
	// TODO: option to global=augmented
	if ((mFlags&saveSkinMap) && mSkinToneMapperImage)
	{
		const string & SkinMapPath=mBasePath+"SkinMaps/";
		makeDirectory(SkinMapPath);
		const string & SkinMapFN=SkinMapPath+mImgFN; // TODO: param/config
		MatUC SkinMap(mSkinToneMapperImage->getColorLikelihoodMap()*0xFF);
		if (!imwrite(SkinMapFN, SkinMap)) clog<<"ERROR ";
		clog<<"writing skin map image "+SkinMapFN<<endl;
	}
	// TODO: option for loading pre-computed maps
}
/// \return face roll angle using the eye line and possibly other landmarks
REALNUM computeFaceRollAngle(const FaceRegion & reg /**< face/profile region */)
{
	const FaceRegions & subs = reg.mSubregions;
	REALNUM RollAngle=0;
	if (subs.count(cRectKindSub)>1) // align eyes horizontally
	{
		Point LI(maxInteger, maxInteger), RI(0, 0);
		for (auto psub: subs)
		{
			if (!isRectKindSub(psub->Kind)) continue;
			const FaceRegion & c = dynamic_cast<const FaceRegion &>(*psub);
			if (c.Kind=="i")
			{
				Point p=center(c);
				if (p.x>RI.x) RI=p;
				if (p.x<LI.x) LI=p;
			}
		}
		if (LI.x<RI.x) // compute the angle using the eyes
		{
			Point a=(RI-LI);
			RollAngle=asin(a.y/norm(a));
		}
		// TODO: use other landmarks, if available
	}
	return RollAngle;
}
const REALNUM
	PIo12=PI/12,
	PIo3=PI/3,
	cSkinBlobStretchScale=1.2;

void FaceFinder::recoverFalseNegatives(const Mat & img)
{	FTIMELOG
//--- get likely face regions from the skin blobs
	FaceRegions LikelyFaceRegions(mFRD.FaceDiameterMin);
	const REALNUM SkinT = mFRD.SkinToneMapper.getColorLikelihoodT();
	mSkinToneMapperImage->getLikelyColorBlobs(LikelyFaceRegions, SkinT, mFlags&seekLandmarksColor);
	const Rect ImgR(Point(0, 0), img.size());
//--- enhance the relevant regions and attempt to recover faces
	{ TIMELOG("enhanceSkinRegionsDetectFaces");
		unsigned VisVerb = VisualVerbose();
		Mat Img2Show; if (VisVerb) Img2Show=Mat::zeros(img.size(), img.type());
		for (const auto & LFR: LikelyFaceRegions)
		{
			if (!LFR || LFR->Kind != "s") continue;

			FaceRegion fr=(const FaceRegion&)(*LFR); // copy and stretch the region
			fr.Kind = mFlags&ignoreFrontal ? "p" : "f";
			fr.stretch(cSkinBlobStretchScale, cSkinBlobStretchScale, false);
			fr &= ImgR;

			unsigned dim=fr.diameter(); // check dimensions
			if (dim<mFaces.getMinDim()) continue; // skip tiny blobs

		//--- form enhanced face region
			Mat EnhFRImg = mSkinToneMapperImage->enhance(fr);
			FaceRegions outFaceRegions(mFRD.FaceDiameterMin);
			PFaceRegion EnhRgn = new FaceRegion(fr);
			*EnhRgn-=fr.tl();

			if ((mFlags&seekLandmarks) && dim>cMinSubDim2)
			{ // get face candidates from skin blobs and landmarks in them
				FaceRegions SR(mFRD.FaceDiameterMin);
				detectSubregionsGS(SR, EnhFRImg, img(fr)); // merge with gray-scale landmarks
				SR.add(EnhRgn, false, cOverlapSlackTDefault, FaceRegions::rcMin);
				merge(outFaceRegions, SR);
				if (VisVerb>2) ResultsDisplay::show("outFaceRegions+=SR", EnhFRImg, outFaceRegions, false);
			}
			if (VisVerb)
			{ // show skin-enhanced patch within the boundaries of the source image
				Mat reg(Img2Show(fr)); EnhFRImg.copyTo(reg);
				rectangle(Img2Show, fr, CV_RGB(0,0xFF,0));
			}
			{ TIMELOG("skinBlobVerifyFaceGS");
				REALNUM
					RollAngle=computeFaceRollAngle(*EnhRgn),
					absRollAngle=fabs(RollAngle);
				bool bRotPatch = absRollAngle>=PIo12 && absRollAngle<=PIo3; // TODO: param/config
				if (bRotPatch) // rotate the patch to make eye-line horizontal
				{
					if (VisVerb) clog<<"RollAngle="<<RollAngle<<endl;
					Image src(EnhFRImg);
					src.rotateR(RollAngle);
					EnhFRImg=src.mx();
				}
				const int // pad the region for more robust face region detection
					wmargin=cScale4RotMrg*img.cols,
					hmargin=cScale4RotMrg*img.rows;
				Mat imgPad; copyMakeBorder(EnhFRImg, imgPad, hmargin, hmargin, wmargin, wmargin, BORDER_CONSTANT);
				FaceRegions rgs; detectFacesGS(rgs, imgPad, mFlags&HistEQ);
				rgs+=Point(-wmargin, -hmargin); // unpad
				if (bRotPatch) // un-rotate newly detected regions
				{
					Point2f c(EnhFRImg.size()); c*=.5;
					const Mat & R = getRotationMatrix2D(c, -RollAngle*R2D, 1.0);
					rgs.rotate(R);
				}
				merge(outFaceRegions, rgs);
				// TODO: additional/alternative verifier, e.g. based on anthropometry
			}
			outFaceRegions+=fr.tl();
			merge(mFaces, outFaceRegions);
			if (VisVerb>2) ResultsDisplay::show("mFaces+=outFaceRegions", EnhFRImg, mFaces);
		}
		cropFaceRegions(mScaledImage);
		if (VisVerb) imOut(mImgOutDir+"SkinEnhanced", Img2Show);
	}
}
void FaceFinder::removeFalsePositives(const Mat & img)
{	FTIMELOG
	const REALNUM SkinMassT = mFRD.SkinToneMapper.getColorMassT();
	for (FaceRegions::iterator it=mFaces.begin(); it!=mFaces.end(); )
	{
		if (!isFRRectKind((*it)->Kind)) {++it; continue;}
		const FaceRegion & fr = dynamic_cast<const FaceRegion&>(**it);
		if (isFaceOrProfileKind(fr.Kind))
		{
			unsigned dim=fr.diameter(), ccnt=fr.mSubregions.count(cRectKindSub);
			if ((dim>=cMinSubDim && ccnt) || (dim>=cMinSubDim2 && ccnt>1)) {++it; continue;} // TODO: verify landmarks
			REALNUM SkinMass = mSkinToneMapperImage->integrate(fr);
			if (mFlags&verbose) clog<<fr<<" SkinMass="<<SkinMass<<endl;
			if (SkinMass<SkinMassT) // false positive
			{
				it=mFaces.erase(it);
				continue;
			}
		}
		++it;
	}
}
void FaceFinder::useSkinTone(const Mat & img)
{	FTIMELOG
	initSkinMap(img);
	recoverFalseNegatives(img);
	removeFalsePositives(img);
}
/// Order face candidates in ascending distance to center order, filter extra landmarks.
void order(FaceRegions & rgs, ///< [in,out] regions to be ordered
	const Size & sz ///< (sub)image extent, whose center point is used as the ordering origin
)
{
	rgs.sort(sz);
	for (PFaceRegion pfr: rgs)
	{
		if (!pfr || !isFaceKind(pfr->Kind)) continue;
		FaceRegions & subs = pfr->mSubregions;
		subs.remove(Trivial(subs));
		subs.sort(pfr->size());
		if (getVerbLevel()>1) clog<<"order fr: "<<*pfr<<endl;
		unsigned NoseCnt=0, // prefer first (central)
			EyeCnt=subs.count("i"), // prefer last (peripheral)
			MouthCnt=subs.count("m"),
			EarCnt=subs.count("e");
		for (auto sit=subs.begin(); sit!=subs.end(); ) // clean subs
		{
			PFaceRegion psub = *sit; if (!psub) { ++sit; continue; }
			const string & kind = psub->Kind;
			if (!isRectKindSub(kind)) { ++sit; continue; }
			if (kind=="n" && NoseCnt++) { sit=subs.erase(sit); continue; }
			else if (kind=="m" && MouthCnt-->1) { sit=subs.erase(sit); continue; }
			else if (kind=="i" && EyeCnt-->2) { sit=subs.erase(sit); continue; }
			else if (kind=="e" && EarCnt-->2) { sit = subs.erase(sit); continue; }
			++sit;
		}
	}
	// TODO: use anthropometry and homography to validate landmarks
}
void FaceFinder::detectFaces()
{	FTIMELOG
	bool bRoI = mRoI.area();
	Mat img = bRoI ? mScaledImage(mRoI) : mScaledImage,
		imgGS = bRoI ? mScaledImageGS(mRoI) : mScaledImageGS;
	detectFacesGS(mFaces, imgGS, mFlags&HistEQ);
	cropFaceRegions(mScaledImage);
	if (!mFRD.SkinToneMapper.empty() && !isGrayScale(img))
	{
		if (VisualVerbose())
		{
			ResultsDisplay rd("NoSkinHelper", img, this);
			rd.run();
		}
		useSkinTone(img);
	}
	order(mFaces, img.size());
	if (bRoI) mFaces+=mRoI.tl(); // shift face regions
	if (VisualVerbose()) ResultsDisplay::show("FaceFinder", mScaledImage, mFaces, true);
}
bool FaceFinder::UserAccepted()const
{
	SpecialChar c=mLastKey;
	return c==SPC || c==RET;
}
unsigned FaceFinder::ImgMaxDim=512; // TODO: MaxScrImgDim for better landmark localization
void FaceFinder::setImgMaxDim(unsigned m)
{
	ImgMaxDim=m;
}
unsigned FaceFinder::updateFlags(unsigned & flags, const string & CLIOpt)
{
	if ("-R"==CLIOpt) flags |= rotation | rotationMultiway;
	else if ("-r"==CLIOpt) flags |= rotation;
	else if ("-eq:on"==CLIOpt) flags |= HistEQ;
	else if ("-eq:off"==CLIOpt) flags &= ~unsigned(HistEQ);
	else if ("-fd:skip:f"==CLIOpt) flags |= ignoreFrontal;
	else if ("-fd:skip:p"==CLIOpt) flags |= ignoreProfile;
	else if ("-fd:sub"==CLIOpt) flags |= cascade;
	else if ("-fd:sub:"==CLIOpt) flags |= cascade | keepCascaded;
	else if ("-fd:sub:SC"==CLIOpt) flags |= keepCascaded | subScaleCorrection;
	else if ("-fd:sub:LM"==CLIOpt) flags |= cascade | keepCascaded | seekLandmarks;
	else if ("-fd:sub:LMC"==CLIOpt) flags |= cascade | keepCascaded | seekLandmarks | seekLandmarksColor;
	else if ("-fd:off"==CLIOpt) flags &= detectOff;
	else if ("-fd:on"==CLIOpt) flags |= detection;
	else if ("-fd:sel"==CLIOpt) flags |= selective;
	else if ("-fd:new"==CLIOpt) flags |= discard | detection;
	else if ("-fd:int"==CLIOpt) flags |= intersect | detection;
	else if ("-fd:skinClrFP"==CLIOpt) flags |= skinClrFP;
	else if ("-save:scaled"==CLIOpt) flags|=saveScaled;
	else if ("-save:skinmap"==CLIOpt) flags|=saveSkinMap;
	else if ("-save:faces"==CLIOpt) flags|=saveFaces;
	return flags;
}
REALNUM FaceFinder::init(const Mat & img)
{
	mLastKey=0;
	if (mImgFN.empty()) mImgFN="CAM";
	mOriginalImage = img;
	const static unsigned need2mod = visual|sampling|detectOn;
	bool pass = (mFlags&LiveFeed) || !(mFlags&need2mod);
	REALNUM scale = 1;
	if (pass) mScaledImage = mOriginalImage;
	else
	{
		REALNUM diam = max(mOriginalImage.rows, mOriginalImage.cols);
		scale = ImgMaxDim ? (diam>ImgMaxDim ? ImgMaxDim/diam : 1) : 1;
		mScaledImage = cv::scale(mOriginalImage, scale);
	}
	if (mScaledImage.channels()>3) // convert to 3-channel
		mScaledImage=convert(mScaledImage, CV_BGRA2BGR);
	if (mScaledImage.depth() != CV_8U) // enforce byte channel depth
		mScaledImage = normalize(mScaledImage);
	mScaledImageGS = GrayScale(mScaledImage);
	if (mScaledImage.channels()>1 && areImgBandsCorrelated(mScaledImage))
		mScaledImage = mScaledImageGS;
	return scale;
}
template<typename T>
inline istream & operator>>(istream & s, Point_<T> & pt)
{
	s>>pt.x>>pt.y;
	return s;
}
REALNUM FaceFinder::init(const string & BasePath, const string & ImgFNLine, const string & sFaceRegions, unsigned flags)
{
	FTIMELOG
	mBasePath = BasePath;
	mFlags = flags;
//--- split and parse image line
	stringstream strm(ImgFNLine);
	string fmt;
	if (ImgFNLine[0]=='.' && ImgFNLine.find(':')!=string::npos)
	{
		getline(strm, fmt, ':');
		getline(strm, mImgFN, ',');
	}
	else getline(strm, mImgFN, '\t');
	string ImgPathFN = BasePath+mImgFN, // full image file name
		ImgDN=getFileName(mImgFN, false); // directory for diagnostics images output
	if (VisualVerbose())
		makeDirectory(mImgOutDir="Images/"+ImgDN+'/');

	REALNUM imgScale = mFlags&(detectOn|visual) ? init(Image::load(ImgPathFN)) : 1;

	if (VisualVerbose())
	{
		if(!mOriginalImage.empty())
			imwrite(mImgOutDir+ImgDN+".jpg", mOriginalImage);
		if(!mScaledImage.empty())
			imwrite(mImgOutDir+ImgDN+".scaled.jpg", mScaledImage);
	}

	if ((mFlags&discard)==0) getline(strm, mImgAttr); // image attributes
	if (fmt==".ColorFERET") // parse attributes accordingly
	{
		string FN=getFileName(mImgFN, false);
		stringstream strmFN(FN);
		string ID; getline(strmFN, ID, '_');
		string path=BasePath.substr(0, BasePath.find("images"));
		Point LI, RI, nose, mouth;
		REALNUM roll=0, pitch=0, yaw=0;
		int yoc=1993;
		string other;
		for (ListReader lr(path+"GT/"+ID+"/"+FN+".txt"); !lr.end(); )
		{
			string ln; trim(toLower(lr.fetch(ln))); if (lr.end()) break;
			stringstream strm(ln);
			if (getVerbLevel()>2) clog<<ln<<endl;
			string key; getline(strm, key, '=');
			string val;
			if (key=="capture_date")
			{
				string val; getline(strm, val, '/'); getline(strm, val, '/');
				strm>>yoc;
			}
			else if (key=="yaw") strm>>yaw;
			else if (key=="pitch") strm>>pitch;
			else if (key=="roll") strm>>roll;
			else if (key=="glasses") { strm>>val; if (val=="yes") other+="glasses,"; }
			else if (key=="beard") { strm>>val; if (val=="yes") other+="beard,"; }
			else if (key=="mustache") { strm>>val; if (val=="yes") other+="mustache,"; }
			else if (key=="left_eye_coordinates") strm>>LI;
			else if (key=="right_eye_coordinates") strm>>RI;
			else if (key=="nose_coordinates") strm>>nose;
			else if (key=="mouth_coordinates") strm>>mouth;
		}
		string kind = fabs(yaw)<45 ? (mFlags|=ignoreProfile, "f") : (mFlags|=ignoreFrontal, "p");
		bool hasEyes = nz(LI) && nz(RI);
		PFaceRegion pfr;
		if (hasEyes)
		{
			pfr = new FaceRegion(kind, LI, RI, nose, mouth);
			mFlags &= detectOff;
		}
		else
		{
			pfr = new FaceRegion(kind, getStretched(getImageRect(), .75)); // TODO: config/param
			mFlags |= tight;
			other += "guess,";
		}
		FaceRegions & children = pfr->mSubregions;
		children.add(new FaceAttribute("d", ID));
		if (yaw ||pitch || roll) children.add(new FaceAttribute("r", format("%g,%g,%g",roll, pitch, yaw)));
		for (ListReader lr(path+"GT/"+ID+"/"+ID+".txt"); !lr.end(); )
		{
			string ln; trim(lr.fetch(ln)); if (lr.end()) break;
			stringstream strm(ln);
			if (getVerbLevel()>2) clog<<ln<<endl;
			string key; getline(strm, key, '=');
			if (key=="gender")
			{
				string val; strm>>val; toLower(val);
				children.add(new FaceAttribute("g", val));
			}
			else if (key=="yob")
			{
				int val; strm>>val;
				children.add(new FaceAttribute("a", yoc-val<20 ? "youth" : "adult"));
			}
			else if (key=="race")
			{
				string val; strm>>val; toLower(val);
				if (startsWith(val,"black")) children.add(new FaceAttribute("t", "dark"));
				else if (startsWith(val,"white")) children.add(new FaceAttribute("t", "light"));
			}
		}
		if (!other.empty())
			children.add(new FaceAttribute("o", other.substr(0, other.length()-1)));
		CHECK(pfr, "pointer to ColorFERET face region should not be NULL");
		stringstream strmFR; strmFR<<*pfr;
		if (!hasEyes) // ensure the final face candidate gets all the attributes
		{
			pfr->Kind = fmt + ":" + kind;
			strmFR <<'\t'<< *pfr;
		}
		mImgAttr = strmFR.str();
	}
	else if (fmt==".PittPatt") // parse attributes accordingly
	{
		replace(mImgAttr.begin(), mImgAttr.end(), ',', '\t');
		stringstream strmAttr(mImgAttr);
		Point2f LI, RI;
		strmAttr>>LI>>RI;
		FaceRegion fr("f", LI, RI); // TODO: profiles
		stringstream strmFR; strmFR<<fr;
		mImgAttr = strmFR.str();
	}
///--- parse attributes
	stringstream StrmNonRegAttr;
	for (stringstream StrmRegs(mImgAttr); !StrmRegs.eof(); )
	{
		char c = StrmRegs.peek();
		if (isWhiteSpace(c)) StrmRegs.get(); // skip whitespace
		else if (isRectKindTop(c))
			addRegion(StrmRegs);
		else
		{
			string attr; getline(StrmRegs, attr, '\t');
			if (!attr.empty())
			{
				if (gotFaces(true))
				{
					FaceRegion & pfr = *findPrimaryFaceRegion();
					const char * pAttr = attr.c_str();
					if (isNumeric(attr)) pfr.mSubregions.add(new FaceAttribute("a", attr));
					else if (StrCmpNoCase(pAttr, "youth")==0) pfr.mSubregions.add(new FaceAttribute("g", "youth"));
					else if (StrCmpNoCase(pAttr, "adult")==0) pfr.mSubregions.add(new FaceAttribute("g", "adult"));
					else if (StrCmpNoCase(pAttr, "female")==0) pfr.mSubregions.add(new FaceAttribute("g", "female"));
					else if (StrCmpNoCase(pAttr, "male")==0) pfr.mSubregions.add(new FaceAttribute("g", "male"));
					else if (StrCmpNoCase(pAttr, "male")==0) pfr.mSubregions.add(new FaceAttribute("g", "male"));
					else if (StrCmpNoCase(pAttr, "AgeUnknown") && StrCmpNoCase(pAttr, "GenderUnknown"))
						StrmNonRegAttr<<'\t'<<attr;
				}
				else StrmNonRegAttr<<'\t'<<attr;
			}
		}
	}
	mImgAttr = StrmNonRegAttr.str();
	if (getVerbLevel()>1) clog<<"mImgAttr="<<mImgAttr<<endl;
///--- parse IDs
	mSingleID = sFaceRegions.find('\t') == string::npos ? sFaceRegions : "";
	if (mSingleID.empty())
		for (stringstream StrmRegs(sFaceRegions); !StrmRegs.eof();)
		{
			string ln; getline(StrmRegs, ln); if (ln.empty()) continue;
			stringstream StrmToks(ln);
			string ID; getline(StrmToks, ID, '\t'); // TODO: change syntax parsing in favor of d[] attributes
			PFaceRegion pFR = addRegion(StrmToks);
			if (!pFR || pFR->mSubregions.hasKind("d")) continue;
			pFR->mSubregions.add(new FaceAttribute("d", ID));
		}
//--- if image is gray-scale, keep just one channel
	if (mFlags&saveScaled)
	{
		string ScaledPath=BasePath+"Scaled/"; // TODO: param/config
		makeDirectory(ScaledPath);
		if (!imwrite(ScaledPath+mImgFN, mScaledImage)) clog<<"ERROR ";
		clog<<"writing scaled image "+ScaledPath+mImgFN<<endl;
	}
	return imgScale;
}
void FaceFinder::cropFaceRegions(const Mat & img)
{
	if (img.empty()) return;
	Rect R(Point(), img.size());
	bool bSubRegions=mFlags&(cascade|keepCascaded);
	for (auto ri=mFaces.begin(); ri!=mFaces.end(); )
	{
		CHECK(*ri, "cropFaceRegions got a NULL face region");
		if (!isFRRectKind((*ri)->Kind)) continue;
		FaceRegion & fr = dynamic_cast<FaceRegion &>(**ri);
		if (bSubRegions) fr.encloseChildren();
		Rect cr(fr); cr &= R;
		const REALNUM A=cr.area();
		if (!A)
		{
			if (getVerbLevel()) clog<<"removing "<<fr<<" outside the image boundary "<<R<<endl;
			ri=mFaces.erase(ri); continue;
		}
		else if (A<fr.area())
		{
			if (getVerbLevel()) clog<<"cropping "<<fr<<" to "<<cr<<" for the image boundary "<<R<<endl;
			fr &= R;
		}
		if (bSubRegions) fr.cropChildren();
		++ri;
	}
}
void FaceFinder::skinRemoveFP()
{
	if (mFRD.SkinToneMapper.empty()) return;
	Mat img = mRoI.area() ? mScaledImage(mRoI) : mScaledImage;
	if (img.empty() || isGrayScale(img)) return;
	initSkinMap(img);
	removeFalsePositives(img);
}
void FaceFinder::process(const string & ImgFNLine, REALNUM scale)
{	FTIMELOG
	bool bScaling = scale && scale!=1;
	REALNUM unscale = bScaling ? 1/scale : 1;
	if (bScaling)
	{
		mFaces*=scale;
		if (mFlags&subScaleCorrection) // correct legacy erroneous scaling of landmarks
			for (auto fr: mFaces)
			{
				if (!isFaceKind(fr->Kind)) continue;
				dynamic_cast<FaceRegion&>(*fr).mSubregions *= unscale;
			}
	}
	if (mFlags&discard) clearRegions();

	bool haveFaces = gotFaces(), noFaces = !haveFaces,
		selectiveDetection = (mFlags&selective) && noFaces;
	if ((mFlags&detection) || selectiveDetection)
		detectFaces();
	else if (mFlags&skinClrFP) skinRemoveFP();

	cropFaceRegions(mScaledImage);

	if (mFlags&keepCascaded) removeBasicRegions(mFaces);
	if (!(mFlags&cascade)) clearLandmarks(mFaces); // else pass regions as they are

	PFaceRegion pGTFR;
	auto mergeGTFR = [&]()
	{
		if (!pGTFR) return;
		if (mFaces.size())
		{
			mFaces.resize(1);
			findPrimaryFaceRegion()->mSubregions.merge(pGTFR->mSubregions);
		}
		else mFaces.add(pGTFR);
	};
	if (!trim(mImgAttr).empty())
	{
		if (startsWith(mImgAttr, ".ColorFERET:"))
		{
			string sFR=mImgAttr.substr(strlen(".ColorFERET:"));
			stringstream strm(sFR);
			pGTFR = new FaceRegion(strm);
			*pGTFR*=scale;
			mergeGTFR();
			mImgAttr="";
		}
	}
	if (mFlags&visual) showFaces();
	mergeGTFR();
	if (bScaling) mFaces*=unscale;
	if (haveFaces) cropFaceRegions(mOriginalImage);
	if ((mFlags&generateID) && !mSingleID.empty())
	{
		unsigned ID = 0; // assign it to the primary region
		for (auto pfr : mFaces)
		{
			if (!isFaceOrProfileKind(pfr->Kind)) continue;
			FaceRegion & fr = (FaceRegion&)*pfr;
			if (fr.mSubregions.hasKind("d")) continue;
			fr.mSubregions.add(new FaceAttribute("d", mSingleID + (ID ? format(".%d", ID) : "")));
			++ID;
		}
	}
	if (mFlags&saveFaces) // crop, scale and save
	{
		PFaceRegion pFR=getPrimaryFaceRegion();
		if (pFR)
		{
			const int dim=160; // TODO: param/config
			Image ImgFR(mOriginalImage(*pFR), dim, dim); // crop, scale
			Mat img=ImgFR.GrayScale(); // TODO: param/config
			string ImgFN=toFN(ImgFNLine.substr(0, ImgFNLine.find('\t')));
			replace(ImgFN, getFileExt(ImgFN), ".bmp"); // TODO: param/config
			clog<<"saving '"<<ImgFN<<"'"<<endl;
			imwrite(ImgFN, img);
		}
	}
}
PFaceRegion FaceFinder::addRegion(FaceRegions & rgns, Rect & r, const string & kind, const Rect & ParentRect)
{
	r&=ParentRect; // crop
	r-=ParentRect.tl(); // shift
	return r.area() ? rgns.add(new FaceRegion(kind, r, FaceRegions(mFRD.FaceDiameterMin/4))) : PFaceRegion();
}
void FaceFinder::addFace(Rect & r)
{
	addRegion(mFaces, r, "f", getScaledImageRect());
}
void FaceFinder::updateSkinColor(const PFaceRegion pfr)
{
	if (isGrayScale(mScaledImage)) return;
	Mat_<Vec3b> src = mScaledImage(*pfr);
	for (int i = 0; i < src.rows; ++i)
		for (int j = 0; j < src.cols; ++j)
		{
			const Vec3b & p = src(i,j);
			MatColorSample s(p.t());
			mSkinColorSamples.push_back(s);
		}
}
void FaceFinder::addSkin(Rect & r)
{
	PFaceRegion p = addRegion(mFaces, r, "s", Rect(Point(), mScaledImage.size()));
	if (p) updateSkinColor(p);
}
void FaceFinder::setRoI(const Rect & r)
{
	mRoI = r & Rect(Point(), mScaledImage.size());
}
void FaceFinder::addFaceFeature(const string & kind, Rect & r, FaceRegions & collection)
// find the container face, add the region to it
{
	for (FaceRegions::iterator it=collection.begin(); it!=collection.end(); ++it)
	{
		if (!isFRRectKind((*it)->Kind)) continue;
		FaceRegion & fr = dynamic_cast<FaceRegion &>(**it);
		if (r<=fr)
		{
			addRegion(fr.mSubregions, r, kind, fr);
			break;
		}
	}
}
void FaceFinder::addFaceFeature(const string & kind, Rect & r)
{
	addFaceFeature(kind, r, mFaces);
}
void FaceFinder::addProfile(Rect & r)
{
	addRegion(mFaces, r, "p", Rect(Point(), mScaledImage.size()));
}
PFaceRegion FaceFinder::addRegion(istream & strm, const Size & ImgSize)
{
	PFaceRegion pFR = new FaceRegion(strm);
	if (!pFR->area()) return 0;
	if (ImgSize.area()) (*pFR)&=(Rect(Point(), ImgSize));
	if (pFR->Kind=="s") updateSkinColor(pFR);
	return mFaces.add(pFR);
}
void FaceFinder::addRegion(const string & rgn, const Size & ImgSize)
{
	stringstream strm(rgn);
	addRegion(strm, ImgSize);
}
void FaceFinder::clearRegions()
{
	mFaces.clear();
	mRoI=Rect();
	mSkinColorSamples = MatColorSample();
}
PFaceRegion FaceFinder::findPrimaryFaceRegion()const
{
	for (const auto & e: mFaces)
		if (isFaceKind(e->Kind))
			return e;
	return PFaceRegion();
}
void FaceFinder::rotateScaledImage(REALNUM a)
{
	clearRegions();
	Image img(mScaledImage);
	img.rotateR(a);
	mScaledImage=img.mx();
}
unsigned FaceFinder::setMaxFaceCount(unsigned cnt)
{
	mFaces.resize(min<unsigned>(cnt, mFaces.size()));
	return mFaces.size();
}

FaceFinder::FaceFinder(FaceRegionDetector & FRD, const Mat & img, unsigned flags):
	mFRD(FRD), mFaces(FRD.FaceDiameterMin, FRD.FaceAspectLimit), mFlags(flags)
{
	process("CAM", init(img));
}
FaceFinder::FaceFinder(FaceRegionDetector & FRD, const string & BasePath, const string & ImgFNLine, unsigned flags):
	mFRD(FRD), mFaces(FRD.FaceDiameterMin, FRD.FaceAspectLimit)
{
	process(ImgFNLine, init(BasePath, ImgFNLine, getFileName(ImgFNLine, false, true), flags));
}
FaceFinder::FaceFinder(FaceRegionDetector & FRD, const string & BasePath, const string & ImgFNLine, const string & FaceRegions, unsigned flags):
	mFRD(FRD), mFaces(FRD.FaceDiameterMin)
{
	process(ImgFNLine, init(BasePath, ImgFNLine, FaceRegions, flags));
}

/// Filter the face regions that should not count for evaluation
void filter(FaceRegions & rgs)
{
	for (auto it=rgs.begin(); it!=rgs.end(); )
	{
		if (isEvalKind((*it)->Kind))
		{
			FaceRegion & fr=dynamic_cast<FaceRegion &>(**it);
			filter(fr.mSubregions);
			++it;
		}
		else it=rgs.erase(it);
	}
}
/**
 * Count image/face regions.
 * @param rgns	input regions collection
 * @param RegMinDim	region minimum dimension
 * @return number of image regions larger than the min dimension
 */
unsigned countRegions(const FaceRegions & rgns, unsigned RegMinDim)
{
	unsigned cnt=0;
	for (FaceRegions::const_iterator gi=rgns.begin(); gi!=rgns.end(); ++gi)
		if (isFRRectKind((*gi)->Kind))
			if (dynamic_cast<const FaceRegion&>(**gi).diameter()>=RegMinDim) ++cnt;
	return cnt;
}
FaceFinder::EvalStats::EvalStats(const FaceRegions & given, const FaceRegions & detected,
	REALNUM OverlapSlackT, REALNUM SubWeight, bool cascade)
{
	FaceRegions GT(given), FF(detected);
	filter(GT); filter(FF);
	REALNUM
		GivenCnt = GT.size(),
		FoundCnt = FF.size();
	if (getVerbLevel()>2)
		clog<<"GT="<<GT<<endl<<"FF="<<FF<<endl;
	REALNUM MatchCnt=0;
	for (FaceRegions::iterator gi=GT.begin(); gi!=GT.end(); )
	{
		if (!isFRRectKind((*gi)->Kind)) continue;
		FaceRegion & gfr=dynamic_cast<FaceRegion &>(**gi);
		FaceRegions::iterator bi = FF.findBestMatch(*gi, OverlapSlackT);
		if (bi==FF.end())
		{
			if (cascade)
			{
				gfr.mSubregions+=gfr.tl();
				(*this)+=FaceFinder::EvalStats(gfr.mSubregions, FaceRegions(), gfr.mSubregions.getMinDim(), OverlapSlackT, SubWeight);
			}
			++gi;
		}
		else // found an overlap
		{
			if (isFRRectKind((*bi)->Kind))
			{
				FaceRegion & bfr=dynamic_cast<FaceRegion &>(**bi);
				if (cascade)
				{
					gfr.mSubregions+=gfr.tl();
					bfr.mSubregions+=bfr.tl();
					(*this)+=FaceFinder::EvalStats(gfr.mSubregions, bfr.mSubregions, sqrt(OverlapSlackT), SubWeight);
				}
				MatchCnt += (gfr.Kind==bfr.Kind ? 1 : SubWeight);
				FF.erase(bi);
				gi = GT.erase(gi);
			}
		}
	}
	if (cascade) // count the remaining children
		for (auto it=FF.begin(); it!=FF.end(); ++it)
			if (isFRRectKind((*it)->Kind))
				FoundCnt += dynamic_cast<const FaceRegion&>(**it).mSubregions.size();

	mMatchCnt += MatchCnt;
	mFoundTotal += FoundCnt;
	mGivenTotal += GivenCnt;
}

}
