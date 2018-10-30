
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
#include "RegionDetector.h"
#include "Image.h"
#include <set> // TODO: hash, unordered_set

namespace FaceMatch
{

PFaceRegionDetector SFaceRegionDetector::mFRD; // static member

void SFaceRegionDetector::init(const string & XMLBasePath, const string & FaceModelFN, const string & ProfileModelFN,
	const string & SkinColorMapperKind, const string & SkinColorParmFN,
	unsigned aFaceDiameterMin, unsigned aFaceDiameterMax,
	REALNUM SkinMassT, REALNUM SkinLikelihoodT,
	REALNUM aFaceAspectLimit, bool prefGPU
	)
{
#pragma omp critical
	mFRD = new FaceRegionDetector(XMLBasePath, FaceModelFN, ProfileModelFN,
		SkinColorMapperKind, SkinColorParmFN,
		aFaceDiameterMin, aFaceDiameterMax,
		SkinMassT, SkinLikelihoodT,
		aFaceAspectLimit, preferGPU(prefGPU));
}
FaceRegionDetector & SFaceRegionDetector::get()
{
#pragma omp critical
	if (!mFRD) init();
	return *mFRD;
}

/// \return GPU ID for a face region kind; -1, if no GPU available.
int getGPUID(const string & kind /**< face region kind, e.g. "f" for face, "p" for profile */)
{
	unsigned cnt = getGPUCount();
	if (!cnt) return -1;
	if (kind == "f") return 0; // ask for face/profile to be scheduled to different devices
	else if (kind == "p") return 1 % cnt;
	static atomic<unsigned> sNextID(2);
	return sNextID++ % cnt;
}

bool RegionDetector::usingGPU()const{ return mCascadeClassifierGPU && !mCascadeClassifierGPU->empty(); }

#define errPfx(PRC) "failed to initialize "#PRC"-based ("+mRegionKind+") face region detector from '"+mModelFN+"'"

RegionDetector::RegionDetector(const string & FN, const string & RegKind,
	unsigned aFaceRgnDimMin, unsigned aFaceRgnDimMax,
	REALNUM AspectRatioLimit, bool preferGPU) :
	mModelFN(FN),
	mFaceRgnDimMin(aFaceRgnDimMin), mFaceRgnDimMax(aFaceRgnDimMax),
	mAspectRatioLimit(AspectRatioLimit),
	mGPUID(getGPUID(RegKind)),
	mRegionKind(RegKind),
	mPreferGPU(preferGPU)
{
	FTIMELOG
	if (FN.empty()) throw Exception("RegionDetector model file name is empty");
	if (mRegionKind.empty()) throw Exception("RegionDetector region kind is empty");
	ParallelErrorStream PES;
#pragma omp parallel sections shared (PES)
	{
#pragma omp section // initGPU
		try
		{
			int GPUCnt = getGPUCount();
			if (mPreferGPU && GPUCnt && !usingGPU())
			{
				GPULocker lkr(mGPUID);
				ParallelErrorStream PES;
				try
				{
					TIMELOG(format("RegionDetector::GPU(%d of %d)::load(%s)", mGPUID, GPUCnt, mModelFN.c_str()));
					mCascadeClassifierGPU = new CascadeClassifier_GPU(mModelFN);
				}
				PCATCH(PES, errPfx(GPU));
				PES.report("new CascadeClassifier_GPU " + mModelFN);
				if (!usingGPU())
				{
					mPreferGPU = false;
					throw Exception(format("quitting to prefer GPU for '%s' reginos due to the initialization failures", mRegionKind.c_str()));
				}
				mCascadeClassifierGPU->findLargestObject = false;
			}
		}
		PCATCH(PES, errPfx(GPU))
#pragma omp section // initCPU
			try
		{
			if (!mCascadeClassifier.load(FN)) throw Exception(errPfx(CPU));
		}
		PCATCH(PES, errPfx(CPU))
	}
	PES.report(__FUNCTION__);
}

bool RegionDetector::detectRgsGPU(RectPack & regions, const Mat & img, unsigned minDim)
{
	if (!mPreferGPU) return false;
	FTIMELOG
	const Size imgDim = img.size();
	const unsigned imgDiam = diameter(img);
	if (imgDiam < min(minDim, cImgDimMin4GPU)) return false;

	if (!usingGPU()) return false;
	
	const Size minDimGPU = mCascadeClassifierGPU->getClassifierSize();
	if (minDim < max(minDimGPU.width, minDimGPU.height)) return false;

	const Size minSize(max<int>(minDim, minDimGPU.width), max<int>(minDim, minDimGPU.height));
	if (minSize.width == 0 || minSize.height == 0) return false;
	if (imgDim.width < 2*minSize.width || imgDim.height < 2*minSize.height) return false;

	bool res = false;
	try
	{
		GPULocker lkr(mGPUID);
		ErrorDumpSuppressor cvErrMute(!getVerbLevel()); // temporarily suppress OpenCV and ncv errors, e.g. from ncvAssertReturn(haar.bNeedsTiltedII == false, NCV_NOIMPL_HAAR_TILTED_FEATURES);
		GpuMat gpuImg(img), gpuFaces;
		int cnt = mCascadeClassifierGPU->detectMultiScale(gpuImg, gpuFaces, 1.1, 3, minSize);
		Mat faces; gpuFaces.colRange(0, cnt).download(faces);
		regions.resize(faces.rows);
		for (int i=0; i<faces.rows; ++i)
			regions[i]=faces.ptr<Rect>()[i];
		res=true; // we ran FD with GPU
	}
	catch (...)
	{
		res = false;
	}
	return res;
}
void RegionDetector::detectRgs(RectPack & regions, const Mat & img, int minDim)
{
	if (!detectRgsGPU(regions, img, 2*minDim))
	{	FTIMELOG
		Size imgDim=img.size(); if (max(imgDim.width, imgDim.height)<minDim) return;
		OMPLocker lkr(mLock);
		mCascadeClassifier.detectMultiScale(img, regions, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING, Size(minDim, minDim), imgDim);
	}
}
/// Filter face/profile sub-regions assuming an up-right, mostly frontal position
void filterSubs(FaceRegion & reg)
{
	set<PFaceRegion> Regs2Remove;
	FaceRegions & subs = reg.mSubregions;
	const unsigned minDim = subs.getMinDim();
	const REALNUM minAspect = subs.getAspectLimit();
	for (auto it=subs.begin(); it!=subs.end(); ++it)
	{
		PFaceRegion psub=*it, pRI, pLI;
		if (!psub || !isRectKindSub(psub->Kind)) continue;
		const FaceRegion & fr = *psub;
		if (fr.diameter()<minDim || fr.minAspect()<minAspect)
			Regs2Remove.insert(*it);
		else if (fr.Kind=="i")
		{
			if (!pRI) pRI=psub;
			if (!pLI) pLI=psub;
			Point c=center(fr), rc=center(*pRI), lc=center(*pLI);
			if (c.x < rc.x)
			{
				if (pLI!=pRI)
					Regs2Remove.insert(pRI);
				pRI=psub;
			}
			if (c.x > lc.x)
			{
				if (pLI!=pRI)
					Regs2Remove.insert(pLI);
				pLI=psub;
			}
		}
		// TODO: other kinds
	}
	if (Regs2Remove.size())
		subs.remove([&](const PFaceRegion pfr)->bool{
			return pfr && Regs2Remove.find(pfr)!=Regs2Remove.end();
		});
	// TODO: optimize via anthropometry and homography
}
void RegionDetector::detect(FaceRegions & regions, const Mat & img, bool cascade, bool intersect, REALNUM MatchT, FaceRegions::ERegRectCombo mode)
{
	if (intersect || mode==FaceRegions::rcMin || !regions.size())
	{
		RectPack objects; detectRgs(objects, img, regions.getMinDim());
		regions.merge(FaceRegions(mRegionKind, objects, mFaceRgnDimMin, mAspectRatioLimit), intersect, MatchT, mode);
	}
	if (!cascade) return;
//--- cascade to sub-regions
	Collection & CD = mChildren;
	const int
		CDCnt=CD.size(),
		RgnCnt=regions.size();
	ParallelErrorStream PES;
#pragma omp parallel for shared(PES, regions, img)
	for (int ir=0; ir<RgnCnt; ++ir)
		try
		{
			auto & ar = regions[ir];
			const string & kind=ar->Kind;
			if (mRegionKind!=kind && !isFRRectKind(kind)) continue;
			FaceRegion & reg=dynamic_cast<FaceRegion&>(*ar);
			Mat subImg = Image::cropad(img, reg);
		#pragma omp parallel for shared(PES, reg)
			for (int i=0; i<CDCnt; ++i) // detect sub-regions
				try
				{
					FaceRegions subregs(mFaceRgnDimMin/2);
					CD[i]->detect(subregs, subImg);
					const string & subKind = CD[i]->mRegionKind;
				#pragma omp critical (LMERGE)
					{
						for (auto sr: subregs)
						{
							auto ic = reg.mSubregions.findBestMatch(sr);
							if (ic!=reg.mSubregions.end() && (*ic)->Kind != subKind)
								reg.mSubregions.erase(ic); // remove conflicts
						}
						reg.mSubregions.merge(subregs);
					}
				}
				PCATCH(PES, format("sub-region %d detection problem ", i))
			PES.report("parallel sub-region detection errors");
			filterSubs(reg);
		}
		PCATCH(PES, format("cascading on region %i problem", ir))
	PES.report("parallel region cascading errors");
}
void FaceFeatureRegionDetector::detect(FaceRegions & regions, const Mat & img, bool cascade, bool intersect, REALNUM MatchT, FaceRegions::ERegRectCombo mode)
{
	const RealRect & R=mRelativeSubRegion;
	const Size d(img.size());
	const Point o(d.width*R.x, d.height*R.y);
	const Size s(d.width*R.width, d.height*R.height);
	const Rect C(o, s);
	const Mat SubImg(img, C); // cropad image to RelativeSubRegion
	RegionDetector::detect(regions, SubImg, cascade, intersect, MatchT, mode);
	if (regions.size()>mDetectedCount)
	{
		regions.sort(d, mRegionKind=="n");
		regions.resize(mDetectedCount); // TODO: better selection of candidates, e.g. in face/profile post-processing
	}
	regions+=o;
}
inline string ModelFileName(string BaseFN, bool preferGPU=false)
{
	BaseFN=getFileName(BaseFN)+".xml";
	return preferGPU ? BaseFN : BaseFN+".gz";
}
FaceRegionDetectorFront::FaceRegionDetectorFront(const string & aFFModelsPath, const string & FaceModelFN,
	unsigned aFaceDiameterMin, unsigned aFaceDiameterMax, REALNUM aFaceAspectLimit, bool preferGPU):
	RegionDetector(aFFModelsPath+FaceModelFN, "f", aFaceDiameterMin, aFaceDiameterMax, aFaceAspectLimit, preferGPU)
{
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_eye", preferGPU), "i", RealRect(RealPoint(0,0), RealPoint(1,0.6)), 2, preferGPU));
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_mcs_nose", preferGPU), "n", RealRect(RealPoint(0.2,0.2), RealPoint(.8,.8)), 1, preferGPU));
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_mcs_mouth", preferGPU), "m", RealRect(RealPoint(0,0.4), RealPoint(1,1)), 1, preferGPU));
}
FaceRegionDetectorProfile::FaceRegionDetectorProfile(const string & aFFModelsPath, const string & ProfileModelFN,
	unsigned aFaceDiameterMin, unsigned aFaceDiameterMax, REALNUM aFaceAspectLimit, bool preferGPU):
	RegionDetector(aFFModelsPath+ProfileModelFN, "p", aFaceDiameterMin, aFaceDiameterMax, aFaceAspectLimit, preferGPU)
{
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_eye", preferGPU), "i", RealRect(RealPoint(0,0), RealPoint(1,0.6)), 2, preferGPU));
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_mcs_leftear", preferGPU), "e", RealRect(RealPoint(0,0), RealPoint(.6,0.8)), 1, preferGPU));
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_mcs_rightear", preferGPU), "e", RealRect(RealPoint(.4,0), RealPoint(1,0.8)), 1, preferGPU));
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_mcs_nose", preferGPU), "n", RealRect(RealPoint(0,0.2), RealPoint(1,0.8)), 1, preferGPU));
	addSubregion(new FaceFeatureRegionDetector(aFFModelsPath+ModelFileName("haarcascade_mcs_mouth", preferGPU), "m", RealRect(RealPoint(0,0.4), RealPoint(1,1)), 1, preferGPU));
}

} // namespace FaceMatch
