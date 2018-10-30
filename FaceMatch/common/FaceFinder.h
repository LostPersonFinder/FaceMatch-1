
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

#include "Image.h"
#include "FaceRegions.h"
#include "AccuracyEvaluation.h"
#include "RegionDetector.h"

using namespace std;

namespace FaceMatch
{
/**
@brief Define a face finder utility that can be used as a pre-processing stage for face matching.

@image HTML FaceDetection.png "Face detection process. Boxes: blue = module, gray = data. FP = false positive, FN = false negative, LM = landmark, EQ = intensity equalization, ANN = artificial neural network, Hist = color histogram"

An instance would be constructed per image with some processing options/flags to detect faces and their landmarks.
The detected face and profile regions (with optional landmarks, e.g. eyes, nose, mouth, ears) then can be accessed and/or serialized to a text stream.
A (shared) FaceMatch::FaceRegionDetector object needs to be passed to each FaceFinder constructor.
Tilted (30 degrees or more) faces, typically missed by base OpenCV detector, can be recovered by detecting landmarks
from large enough skin blobs, as specified by respective options.

When serialized to a text stream, the face regions are separated by the tab characters.
In their most basic form, text-serialized face regions have the format f[x,y;w,h] (for the frontal views) and p[x,y;w,h] (for the profiles),
where (x,y) is the top-left offset of the region w.r.t. the image origin, and (w,h) give the region's pixel-wise width and height.
Facial landmarks (eyes, nose, mouth, ears) for each face can optionally be detected,
when the appropriate flags (e.g. cascade) from FaceMatch::FaceFinder::ProcFlags are specified.
When facial landmarks are detected, the text-serialization of the face regions reflect that by a more complex format, e.g.
@code
f{[x,y;w,h]	i[x,y;w,h]	i[x,y;w,h]	n[x,y;w,h]	m[x,y;w,h]	e[x,y;w,h]}
@endcode
where i stands for eye, n for nose, e for ear, and m for mouth. The sub-region coordinates are given relative to their parent face region.

When face and profile candidates considerably overlap (as determined within FaceRegions::add()),
a frontal face region is preferred by FaceFinder over a profile region.
However, when an ear sub-region is found, then a profile candidate is preferred over the frontal face.
A considerable overlap of two candidate regions of the same kind is resolved by computing an average of the two regions, with
the sub-region positions adjusted accordingly.
When one of the overlapping candidates is considerably larger (e.g. x2 or more), then the smaller candidate is neglected.

@see FaceRegions

@note A FaceFinder instance can be optionally used for visual feedback and (correction of) annotation purposes,
in which case an OpenCV window is displayed and the user can interactively add, remove face regions and facial landmarks.
FaceFinder command-line interface (CLI) provides a good example of FaceMatch::FaceFinder class utilization.
 */
class LIBDCL FaceFinder
{
	friend class ResultsDisplay;
	FaceRegionDetector & mFRD;
	string mBasePath, mImgFN, mImgOutDir, mImgAttr, mSingleID;
	Mat mOriginalImage, mScaledImage, mScaledImageGS;
	FaceRegions mFaces;
	unsigned mFlags;
	Rect mRoI;
	MatColorSample mSkinColorSamples;
	PColorMapper mSkinToneMapperImage;
	int mLastKey;

	void showFaces();
	void detectFaces();
	void clearLandmarks(FaceRegions & regions);
	/// Remove fairly large face regions without enough of face landmarks.
	void removeBasicRegions(FaceRegions & regions);
	void detectRotatedRegions(FaceRegions & regions, RegionDetector & FaceClass, const Mat & img);
	void tryDetectRotatedRegionsOMP(FaceRegions & regions, RegionDetector & FaceClass, const Mat & img);
	void detectFacesGS(FaceRegions & FaceRgns, const Mat & img, bool eqHist=true);
	void detectSubregionsGS(FaceRegions & SR, const Mat & EnhFRImg, const Mat & FRImg);
	void detectSubregionsColor(FaceRegions & SR, const Mat & EnhFRImg, const Mat & FRImg);
	void merge(FaceRegions & regions, const FaceRegions & found);
	void cropFaceRegions(const Mat & img);
	REALNUM init(const Mat & img);
	REALNUM init(const string & BasePath, const string & ImgFNLine, const string & FaceRegions, unsigned flags);
	void process(const string & ImgFNLine, REALNUM scale);
	PFaceRegion addRegion(FaceRegions & rgns, Rect & r, const string & kind, const Rect & ParentRect);
	void addRegion(const string & rgn, const Size & ImgSize=Size());
	PFaceRegion addRegion(istream & strm, const Size & ImgSize=Size());
	void addFace(Rect & r);
	void addSkin(Rect & r);
	void addFaceFeature(const string & kind, Rect & r);
	void addFaceFeature(const string & kind, Rect & r, FaceRegions & collection);
	void addProfile(Rect & r);
	void updateSkinColor(const PFaceRegion pfr);
	void clearRegions();
	/**
	 * Clear face regions and rotate scaled image by the specified angle.
	 * @param a angle in radians
	 */
	void rotateScaledImage(REALNUM a);
	void getColorStats(Vec3b & mean, Vec3d & var);
	void setRoI(const Rect & r);
	void initSkinMap(const Mat & img);
	void removeFalsePositives(const Mat & img);
	void recoverFalseNegatives(const Mat & img);
	void getLikelyFaceSkinBlobs(FaceRegions & LikelyFaceRegions);
	void skinRemoveFP();
	void useSkinTone(const Mat & img);
	unsigned VisualVerbose()const{return (mFlags&visual)&&(mFlags&verbose) ? getVerbLevel() : 0;}
	PFaceRegion findPrimaryFaceRegion()const;
	static unsigned ImgMaxDim;
public:
	/// Set image maximal diameter.
	static void setImgMaxDim(unsigned m /**< image max diameter in pixels */);
	/// Specify accuracy evaluation for FaceFinder face regions.
	struct LIBDCL EvalStats: public AccuracyEvaluation<REALNUM>
	{
		EvalStats(): AccuracyEvaluation<REALNUM>() {}
		/**
		 * Instantiate.
		 * @param GT	given regions
		 * @param FF	found regions
		 * @param OverlapSlackT	region overlap slack tolerance: 0=exact overlap, 1=anything overlaps; default @see cOverlapSlackTDefault
		 * @param aSubWeight	overlapping face/profile substitution weight: 0=no credit for overlapping face&profile (default); 1=full credit;
		 * 	a real value between 0 and 1 results in a partial overlap credit
		 * @param cascade	evaluate the overlaps of the sub-regions?
		 */
		EvalStats(const FaceRegions & GT, const FaceRegions & FF, REALNUM OverlapSlackT=cOverlapSlackTDefault, REALNUM aSubWeight=0, bool cascade=false);
	};
	/// face detection flags
	enum ProcFlags : unsigned
	{
		none = 0, ///< no additional processing
		detection = 1, ///< detect faces even if some face regions are given
		selective = detection << 1, ///< detect faces only when no face is given
		rotation = selective << 1, ///< use 90-degree in-plane face rotations for detection
		rotationMultiway = rotation<<1, ///< use multi-way (every 30-degrees) rotation for detection
		cascade = rotationMultiway<<1, ///< detect facial features (landmarks)
		visual = cascade<<1, ///< use OpenCV GUI for visual feedback and/or annotations
		verbose = visual<<1, ///< verbose visuals
		HistEQ = verbose<<1, ///< histogram equalization is to correct low contrast images
		discard = HistEQ<<1, ///< discard any given annotations
		ignoreFrontal = discard<<1, ///< ignore frontal faces during detection
		ignoreProfile = ignoreFrontal<<1, ///< ignore profiles during detection
		intersect = ignoreProfile<<1, ///< set-intersect the found face regions with the given face regions
		tight = intersect<<1, ///< intersect region rectangles instead of averaging or uniting them
		sampling = tight<<1, ///< skin color sampling is on
		skinClrFP = sampling<<1, ///< remove false positives based on skin map
		keepCascaded = skinClrFP<<1, ///< keep only those larger faces, if they have landmarks
		seekLandmarks = keepCascaded<<1, ///< seek face landmarks in large skin blobs
		seekLandmarksColor = seekLandmarks<<1, ///< seek landmarks in skin blobs using color features
		generateID = seekLandmarksColor<<1, ///< generate face region ID(s) automatically, if they are not provided
		saveScaled = generateID <<1, ///< save scaled version of images
		saveSkinMap = saveScaled<<1, ///< save skin maps of images
		saveFaces = saveSkinMap<<1, ///< save normalized face patches
		subScaleCorrection = saveFaces<<1, ///< correct legacy landmarks scaling problems
		LiveFeed = subScaleCorrection<<1, ///< do not block results display with a web-cam feed
	/// useful flag combinations
		detectOn = detection|selective, ///< detection on
		detectOff = ~detectOn ///< detection off: mFlags &= detectOff
	};
	const static unsigned ProcFlagsDefault=selective|HistEQ;
	/// @return updated face finder flags according to the supplied command-line option
	static unsigned updateFlags(unsigned & flags, ///< [in,out] face finder flags/options @see ProcFlags
		const string & CLIOpt ///< command line option/switch, e.g. -fd:sub
		);
	/**
	 * Instantiate. Locate faces (and their landmarks) in a given image according to the flags.
	 * @param FRD	initialized face region detector
	 * @param BasePath		input image base path
	 * @param ImgFNLine		input image file name optionally followed by tab-separated image attributes
	 * @param FaceRegions	new-line separated list of face regions with optional attributes.
	 * 	Each rectangular region is given by {f|p}[x,y;w,h] optionally followed by tab-separated attributes, e.g.<br>
	 * 		ID\\tf[22,36;50,60]\\tJoe Smith\\tM\\t32\\n<br>
	 * 		ID\\tp[29,32;60,62]\\tMary Bell\\tF\\t25\\n<br>
	 * 		...<br>
	 *  Single region ID supplies ID prefix for face regions without explicitly specified IDs.
	 * 	If no region list is given, the faces are detected automatically.
	 * @param flags		optional processing parameters specified by FaceFinder::ProcFlags
	 */
	FaceFinder(FaceRegionDetector & FRD,
		const string & BasePath, const string & ImgFNLine, const string & FaceRegions, unsigned flags=ProcFlagsDefault);
	/**
	 * Instantiate. Locate faces (and their landmarks) in a given image according to the flags.
	 * @param FRD	initialized face region detector
	 * @param BasePath		input image base path
	 * @param ImgFNLine		input image file name optionally followed by tab-separated face regions and image attributes
	 * 	If no region list is given, the faces are detected automatically.
	 * @param flags		optional processing parameters specified by FaceFinder::ProcFlags
	 */
	FaceFinder(FaceRegionDetector & FRD, const string & BasePath, const string & ImgFNLine, unsigned flags=ProcFlagsDefault);
	/// Instantiate using an image reference.
	FaceFinder(FaceRegionDetector & FRD, ///< (shared) face region detector
		const Mat & img, ///< input image reference
		unsigned flags = ProcFlagsDefault ///< detection options as specified by FaceFinder::ProcFlags
		);
	/**
	 * Output face finder structure.
	 * @param os output stream
	 * @param ff face finder structure
	 * @return output stream
	 */
	friend ostream & operator<<(ostream & os, const FaceFinder & ff)
	{
		os<<ff.mImgFN<<ff.mFaces<<ff.mImgAttr;
		return os;
	}
	/// Indicate if faces are present. Check for landmarks, if sub-regions are enabled.
	/// When relaxed=true, just return the top face regions count.
	/// @return true if we have a face region, false otherwise
	bool gotFaces(bool relaxed=false /**< check just the size of face collection without going deeper*/)const;
	/// @return primary face region
	const PFaceRegion getPrimaryFaceRegion()const
	{
		return findPrimaryFaceRegion();
	}
	/// @return original image reference.
	const Mat & getOriginalImage()const{ return mOriginalImage; }
	/// @return image file name
	const string & getImageFN()const{ return mImgFN; }
	/// @return face regions
	const FaceRegions & getFaces()const{return mFaces;}
	/// @return skin color samples collection
	const MatColorSample & getSkinColorSamples()const{return mSkinColorSamples;}
	/// @return source image rectangle.
	Rect getImageRect()const{return Rect(Point(), mOriginalImage.size());}
	/// @return scaled image rectangle.
	Rect getScaledImageRect()const{return Rect(Point(), mScaledImage.size());}
	/**
	 * Set max face count, truncate if necessary.
	 * @param cnt maximal face/profile count
	 * @return actual face count after a possible truncation
	 */
	unsigned setMaxFaceCount(unsigned cnt);
	/// @return user accepted (visual) face detection results?
	bool UserAccepted()const;
};
typedef Ptr<FaceFinder> PFaceFinder;
}
