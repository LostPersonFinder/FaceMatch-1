
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

#pragma once // 2012-2017 (C) 

#include "ColorMapManager.h"
#include <opencv2/gpu/gpu.hpp>
using namespace cv::gpu;

namespace FaceMatch
{
const unsigned // defaults
	/// region minimal dimension in pixels
	cRegDimMin = 16,
	///	region maximal dimension in pixels
	cRegDimMax = cMaxScrImgDim,
	/// region minimal area in pixels
	cRegFaceAreaMin = cRegDimMin*cRegDimMin,
	/// minimal region diameter for GPU processing
	cRegDimMin4GPU = 4*cRegDimMin,
	/// minimal image dimension for GPU processing
	cImgDimMin4GPU = 8*cRegDimMin;

/**
 * \brief generic face region detector
 *
 * Define a face region detector based on OpenCV's implementation of Viola-Jones
 * <a href="http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html">object detector</a>.
 */
class LIBDCL RegionDetector
{
	OMPSimpleLock mLock;
	CascadeClassifier mCascadeClassifier;
	Ptr<CascadeClassifier_GPU> mCascadeClassifierGPU;
	typedef vector<Rect> RectPack;
	typedef Ptr<RegionDetector> PRegionDetector;
	typedef vector<PRegionDetector> Collection;
	Collection mChildren;
	string mModelFN;
	unsigned mFaceRgnDimMin, mFaceRgnDimMax;
	REALNUM mAspectRatioLimit;
	int mGPUID=0;
	bool mPreferGPU=false;
protected:
	/** region kind; typically single letter, e.g. "f" for face or "p" for profile */
	string mRegionKind;
	/**
	 * Add a sub-region detector to the collection.
	 * \param SR	sub-region non-null detector pointer; it will be freed by the collection.
	 */
	void addSubregion(RegionDetector * SR)
	{
		mChildren.push_back(SR);
	}
	/// Detect face regions.
	void detectRgs(RectPack & regions, ///<[in|out] detected regions
		const Mat & img, ///< input image/patch
		int minDim ///< minimal region diameter in pixels
	);
	/// \return has face region detection been called using GPU successfully?
	bool detectRgsGPU(RectPack & regions, ///<[in|out] detected regions
		const Mat & img, ///< input image/patch
		unsigned minDim ///< minimal region diameter in pixels
	);
public:
	/// Construct an instance.
	RegionDetector(const string & FN, ///< image file name
		const string & RegKind, ///< region kind; usually a single letter (e.g. "f" for face or "p" for profile)
		unsigned aFaceRgnDimMin=cRegDimMin, ///< minimal face region diameter=max(width,height) in pixels
		unsigned aFaceRgnDimMax=cRegDimMax, ///< maximal face region diameter=max(width,height) in pixels
		REALNUM AspectRatioLimit=0.5, ///< minimal face region aspect ratio for filtering unlikely (too narrow) face candidates, computed as min(width/height, height/width), hence ranging in [0,1]
		bool preferGPU=false ///< preferGPU	prefer GPU (if available) over CPU?
		);
	virtual ~RegionDetector(){}
	/// \return using GPU?
	bool usingGPU()const;
	/// Detect a face (sub)region using OpenCV <a href="http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html">face detector</a>.
	virtual void detect(FaceRegions & regions, ///< [in,out] regions collection that was detected
		const Mat & img, ///< [in] image matrix
		bool cascade=false, ///< cascade to sub-region detection?
		bool intersect=false, ///< intersect with the existing regions?
		REALNUM MatchT=cOverlapSlackTDefault, ///< region overlap slack match threshold
		FaceRegions::ERegRectCombo mode=FaceRegions::rcDefault ///< rectangular region combination mode
	);
};

/** a real-valued 2D point */
typedef Point_<REALNUM> RealPoint;
/** a real-valued 2D rectangle */
typedef Rect_<REALNUM> RealRect;

/**
 * \brief face feature (landmark) region detector
 *
 * Define an OpenCV-based color-blind face region detector, typically used for face landmarks, e.g. eyes, nose, mouth and ears.
 */
class LIBDCL FaceFeatureRegionDetector: public RegionDetector
{
	const RealRect mRelativeSubRegion;
	unsigned mDetectedCount;
public:
	/// Instantiate.
	FaceFeatureRegionDetector(const string & ModelFN, ///< face detector model (XML or xml.gz) file name
		const string & label, ///< region label (its kind), e.g. "i" for eye, "n" for nose, "m" for mouth, "e" for ear
		const RealRect & aRelativeSubRegion=RealRect(0,0,1,1), ///< a normalized (with dimensions in [0,1]) rectangular region of interest, relative to the parent region; for example, eyes are expected in the upper portion of the parent region, so aRelativeSubRegion=RealRect(0,0,1,0.5)
		unsigned count=1, ///< expected region count, e.g. 2 for eyes or 1 for nose
		bool preferGPU=false ///< prefer GPU, when available?
		):
			RegionDetector(ModelFN, label, cRegDimMin/4, cRegDimMax/4, cFaceFeatureMinAspect, preferGPU),
			mRelativeSubRegion(aRelativeSubRegion), mDetectedCount(count)
	{ }
	virtual ~FaceFeatureRegionDetector(){}
	/// Run detector on face relative sub-region, corresponding to a landmark.
	virtual void detect(FaceRegions & regions, const Mat & img, bool cascade=false,
			bool intersect=false,
			REALNUM MatchT=cOverlapSlackTDefault,
			FaceRegions::ERegRectCombo mode=FaceRegions::rcDefault) override;
};

/**
 * \brief frontal face region localizer based on OpenCV gray-scale cascaded face <a href="http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html">detector</a>
 *
 * Define an OpenCV-based color-blind face detector cascading to important face sub-regions (landmarks), e.g. eyes, nose and mouth,
 * restricted to their expected areas within the parent face region.
 *
 * \note The same eye model is used for both eyes.
 *
 * \see FaceFeatureRegionDetector
 */
struct LIBDCL FaceRegionDetectorFront: public RegionDetector
{
	/// Instantiate a face detector along with sub detectors for landmarks, restricted to their expected areas within the parent region.
	FaceRegionDetectorFront(const string & aFFModelsPath="FFModels/", ///< path to the face detection models
		const string & FaceModelFN="haarcascade_frontalface_alt2.xml", ///< file name of the frontal face OpenCV or custom-trained detector model
		unsigned aFaceDiameterMin=cRegDimMin, ///< minimal face region diameter=max(width,height) in pixels
		unsigned aFaceDiameterMax=cRegDimMax, ///< maximal face region diameter=max(width,height) in pixels
		REALNUM aFaceAspectLimit=0.5, ///< minimal face region aspect ratio for filtering unlikely (too narrow) face candidates, computed as min(width/height, height/width), hence ranging in [0,1]
		bool preferGPU=false ///< prefer GPU, if available?
		);
	virtual ~FaceRegionDetectorFront(){}
};

/**
 * \brief profile region localizer based on OpenCV gray-scale cascaded face <a href="http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html">detector</a>
 *
 * Define an OpenCV-based color-blind face detector cascading to important face sub-regions (landmarks), e.g. eyes, nose, mouth, and ear,
 * restricted to their expected areas within the parent profile region.
 *
 * \note The same detector models are used for both eyes and for both ears, which are run in parallel, when possible.
 *
 * \see FaceFeatureRegionDetector
 */
struct LIBDCL FaceRegionDetectorProfile: public RegionDetector
{
	/// Instantiate profile detector along with sub detectors for landmarks, restricted to their expected areas within the parent region.
	FaceRegionDetectorProfile(const string & aFFModelsPath="FFModels/", ///< path to the face detection models
		const string & ProfileModelFN="haarcascade_profileface.xml", ///< file name of the profile OpenCV or custom-trained detector model
		unsigned aFaceDiameterMin=cRegDimMin, ///< minimal face region diameter=max(width,height) in pixels
		unsigned aFaceDiameterMax=cRegDimMax, ///< maximal face region diameter=max(width,height) in pixels
		REALNUM aFaceAspectLimit=0.5, ///< minimal face region aspect ratio for filtering unlikely (too narrow) face candidates, computed as min(width/height, height/width), hence ranging in [0,1]
		bool preferGPU=false ///< prefer GPU, if available?
	);
	virtual ~FaceRegionDetectorProfile(){}
};

/**
 * \brief Define a frontal and profile view face detector, optionally cascading to facial landmarks (eyes, nose, mouth, ears).
 *
 * \image HTML FaceRegionDetectorDiagram.png "FaceRegionDetector structure and dependencies. Detectors: FrontalDetector = frontal face, ProfileDetector = profile. Skin models: Stat = Gaussian parametric, Hist = color histogram non-parametric, ANN = artificial neural network"
 *
 * Optionally employ a color-aware skin mapper for more accurate face detection, correcting the base-line
 * OpenCV <a href="http://docs.opencv.org/doc/tutorials/objdetect/cascade_classifier/cascade_classifier.html">color-blind detector</a>,
 * eliminating (false positive) hallucinations and recovering missed (false negative) candidates,
 * due to the unconstrained nature of images with faces in arbitrary lighting, poses, wearing glasses, makeup, facial hair, etc.
 * Custom face region models (e.g. for eye-glasses) can be supplied as alternative to the standard (eye) detectors.
 *
 * \note The object is typically instantiated as a \a singleton for the face/skin detection models to be shared across multiple instantiations of FaceMatch::FaceFinder.
 * As with any singleton, one needs to instantiate it statically in a function to catch possible exceptions, and use proper locking in multi-threaded environments,
 * as OpenCV face detectors are not guaranteed to be thread-safe.
 *
 * \see FaceMatcher CLI unit tests for typical use cases
 */
struct FaceRegionDetector
{
	/// frontal face detector for localizing mostly up-right, front-looking faces; relies on OpenCV <a href="http://docs.opencv.org/modules/objdetect/doc/cascade_classification.html?highlight=cascadeclassifier">cascaded classifier</a>
	FaceRegionDetectorFront FrontalDetector;
	/// profile face detector for localizing mostly up-right, but somewhat turned away faces typically with one ear clearly visible; relies on OpenCV <a href="http://docs.opencv.org/modules/objdetect/doc/cascade_classification.html?highlight=cascadeclassifier">cascaded classifier</a>
	FaceRegionDetectorProfile ProfileDetector;
	/// skin color detector/mapper for producing real-valued maps of skin color likelihood over the input color image; color likelihood values range in [0,1]
	ColorMapManager SkinToneMapper;
	unsigned
		/// minimal face region diameter=max(width, height) in pixels, default is FaceMatch::cRegDimMin
		FaceDiameterMin,
		/// maximal face region diameter=max(width, height) in pixels, default is FaceMatch::cRegDimMax
		FaceDiameterMax;
	/// face region aspect ratio limit (with both width/height and height/width being compared) to filter out too narrow regions
	REALNUM FaceAspectLimit;
	/**
	 * Instantiate, typically as a singleton and share it across all FaceFinder constructions for consistent face detection.
	 * Provide base face detection using the specified models for the Viola-Jones detector and improve upon it using optional color-based skin mapping.
	 * Face candidates are filtered based on their diameter/aspect and optional skin mass/likelihood.
	 * Prefer GPU, when available and specified, to speed-up computation (2-4 times).
	 */
	FaceRegionDetector(const string & XMLBasePath = "FFModels/", ///< face detector models directory with XML (optionally gzipped) files, e.g. from <a href="http://opencv.org">OpneCV</a> or custom-trained
		const string & FaceModelFN = "haarcascade_frontalface_alt2.xml", ///< frontal face detector model file name: for detecting fairly up-right forward-looking faces, similar to passport shots
		const string & ProfileModelFN = "haarcascade_profileface.xml", ///< profile detector model file name: for detecting fairly up-right slightly turned away faces, typically with one ear visible
		const string & SkinColorMapperKind = "", /**< skin color mapper kind, e.g.
			ANN (artificial neural net based: most accurate, but slowest in evaluation or training),
			Hist (histogram based: fast, but requires millions of training samples to approach ANN-based accuracy),
			Stat (parametric statistical model: fast, reasonably accurate, does not require many training samples);
			if empty, no skin model is used. */
		const string & SkinColorParmFN = "", /**< corresponding skin color mapper model/configuration file name, e.g.
			NET_PL_9_15_50_90.18.ann (for ANN), Lab.all.xyz (for histogram using Lab color space), SkinStat.HSV.yml (for parametric model using HSV color space) */
		unsigned aFaceDiameterMin=cRegDimMin, ///< minimal face diameter=max(width, height) in pixels to be considered for detection
		unsigned aFaceDiameterMax=cRegDimMax, ///< maximal face diameter=max(width, height) in pixels to be considered for detection
		REALNUM SkinMassT=0.25, ///< minimal acceptable skin mass in a face region for filtering possible false positives, ranging in [0,1]
		REALNUM SkinLikelihoodT=0.5, ///< skin likelihood map threshold in [0,1] used for detecting skin blobs to recover possible false negatives
		REALNUM aFaceAspectLimit=0.5, ///< minimal face region aspect ratio for filtering unlikely (too narrow) face candidates, computed as min(width/height, height/width), hence ranging in [0,1]
		bool preferGPU=false ///< prefer GPU, when available, to speed-up face detection (about 2-4 times, depending on the GPU and image complexity)?
		):
		FrontalDetector(XMLBasePath, FaceModelFN, aFaceDiameterMin, aFaceDiameterMax, aFaceAspectLimit, preferGPU),
		ProfileDetector(XMLBasePath, ProfileModelFN, aFaceDiameterMin, aFaceDiameterMax, aFaceAspectLimit, preferGPU),
		SkinToneMapper(SkinColorMapperKind, SkinColorParmFN, SkinMassT, SkinLikelihoodT, preferGPU),
		FaceDiameterMin(aFaceDiameterMin), FaceDiameterMax(aFaceDiameterMax),
		FaceAspectLimit(aFaceAspectLimit)
	{}
	/// \return using GPU?
	bool usingGPU()const{ return FrontalDetector.usingGPU() && ProfileDetector.usingGPU(); }
};

typedef Ptr<FaceRegionDetector> PFaceRegionDetector;

/// FaceRegionDetector thread-safe singleton assuring a proper initialization within a function body, so that any exceptions can be safely caught.
class LIBDCL SFaceRegionDetector
{
	static PFaceRegionDetector mFRD;
	SFaceRegionDetector(){}
public:
	/// Initialize a single static instance of FaceRegionDetector, replacing the old one, if any. \see FaceRegionDetector::FaceRegionDetector()
	static void init(const string & XMLBasePath = "FFModels/", ///< face detector models directory with XML (optionally gzipped) files, e.g. from <a href="http://opencv.org">OpneCV</a> or custom-trained
		const string & FaceModelFN = "haarcascade_frontalface_alt2.xml", ///< frontal face detector model file name: for detecting fairly up-right forward-looking faces, similar to passport shots
		const string & ProfileModelFN = "haarcascade_profileface.xml", ///< profile detector model file name: for detecting fairly up-right slightly turned away faces, typically with one ear visible
		const string & SkinColorMapperKind = "", /**< skin color mapper kind, e.g.
			ANN (artificial neural net based: most accurate, but slowest in evaluation or training),
			Hist (histogram based: fast, but requires millions of training samples to approach ANN-based accuracy),
			Stat (parametric statistical model: fast, reasonably accurate, does not require many training samples);
			if empty, no skin model is used. */
		const string & SkinColorParmFN = "", ///< corresponding skin color mapper model/configuration file name, e.g. NET_PL_9_15_50_90.18.ann (for ANN) or Lab.all.xyz (for Lab histogram)
		unsigned aFaceDiameterMin=cRegDimMin, ///< minimal face diameter to be considered for detection
		unsigned aFaceDiameterMax=cRegDimMax, ///< maximal face diameter to be considered for detection
		REALNUM SkinMassT=0.25, ///< minimal acceptable skin mass in a face region for filtering possible false positives, ranging in [0,1]
		REALNUM SkinLikelihoodT=0.5, ///< skin likelihood map threshold in [0,1] used for detecting skin blobs to recover possible false negatives
		REALNUM aFaceAspectLimit=0.5, ///< minimal face region aspect ratio for filtering unlikely faces, e.g. too narrow regions, ranging in [0,1]
		bool preferGPU=false ///< prefer GPU when available?
	);
	/// \return single static instance of FaceRegionDetector, initializing a new one, if necessary
	static FaceRegionDetector & get();
};

} // namespace FaceMatch
