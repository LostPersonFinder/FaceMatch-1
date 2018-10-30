
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
#include "ColorFaceLandmarks.h"
#include "ObjectDetectorANN.h"
#include "Image.h"

using namespace std;
using namespace cv;

namespace FaceMatch
{

typedef vector<Point> PointPack;
typedef vector<Rect> RectPack;

void split2YCbCr(const Mat & BGRImg, MatReal& Y_band, MatReal& Cr_band, MatReal& Cb_band)
{
	Mat ImgYCrCb;
	cvtColor(BGRImg, ImgYCrCb, CV_BGR2YCrCb);

	vector<Mat> ycbcr_channels;
	split(ImgYCrCb, ycbcr_channels);

	Y_band = ycbcr_channels[0];
	Cr_band = ycbcr_channels[1];
	Cb_band = ycbcr_channels[2];
}
void getEyeMapC(Mat& EyeMapC, const Mat& Cb_band, const Mat& Cr_band)
{
	MatReal Cb2 = Cb_band.mul(Cb_band);
	normalize(Cb2, Cb2, 0, 255, NORM_MINMAX);

	double maxval_Cr;
	MatReal Neg_Cr;
	minMaxIdx(Cr_band, 0, &maxval_Cr, 0, 0);
	Neg_Cr = maxval_Cr-Cr_band;  // Negative of Cr (i.e. 255-Cr);
	Neg_Cr = Neg_Cr.mul(Neg_Cr);  // (Neg_Cr).^2
	normalize(Neg_Cr, Neg_Cr, 0, 255, NORM_MINMAX); // Normalize((Neg_Cr).^2)

	MatReal Cb_Cr = Cb_band/Cr_band;
	normalize(Cb_Cr, Cb_Cr, 0, 255, NORM_MINMAX);

	EyeMapC = (Cb2 + Neg_Cr + Cb_Cr)/3;
	normalize(EyeMapC, EyeMapC, 0, 255, NORM_MINMAX);
	MatUC EyeMapUC=EyeMapC;
	equalizeHist(EyeMapUC, EyeMapUC);
	EyeMapC=MatReal(EyeMapUC);
}
void getEyeMapL(Mat& EyeMapL, const Mat& Y_band)
{
	static Mat ker = getStructuringElement(MORPH_ELLIPSE, Size(5,5));
	MatReal dilatedImg, erodedImg;
	dilate(Y_band, dilatedImg, ker, Point(-1,-1), 2);
	erode(Y_band, erodedImg, ker, Point(-1,-1), 2);
	erodedImg = erodedImg+1; // avoiding 0's in denomenator
	EyeMapL =  dilatedImg/erodedImg;
	normalize(EyeMapL, EyeMapL, 0, 255, NORM_MINMAX);
}
void getMouthMap(Mat& MouthMap, const Mat& Cb_band, const Mat& Cr_band, const REALNUM eta=0.95)
{
	Mat Cr2 = Cr_band.mul(Cr_band);
	normalize(Cr2, Cr2, 0, 255, NORM_MINMAX, -1, Mat() );
	Mat Cr_Cb = Cr_band/Cb_band;
	normalize(Cr_Cb, Cr_Cb, 0, 255, NORM_MINMAX, -1, Mat() );
	Mat temp = Cr2-(eta*Cr_Cb);
	temp = temp.mul(temp); //(Cr2-(eta*Cr_Cb)).^2;
	MouthMap = Cr2.mul(temp);
	normalize(MouthMap, MouthMap, 0, 255, NORM_MINMAX, -1, Mat() );
}
void getEyeMap(Mat & EyeMap, const MatReal & Y_band, const MatReal & Cr_band, const MatReal & Cb_band)
{
	MatReal EyeMapC, EyeMapL;
	getEyeMapC(EyeMapC, Cb_band, Cr_band);
	getEyeMapL(EyeMapL, Y_band);
	EyeMap=EyeMapL.mul(EyeMapC); // combine
	static const Mat ker = getStructuringElement(MORPH_ELLIPSE, Size(5,5));
	dilate(EyeMap, EyeMap, ker);
	normalize(EyeMap, EyeMap, 0, 255, NORM_MINMAX);
}
Rect getPeakRegion(const Mat & binary, const Point & peak)
{
    Rect rect;
    MatReal Labels;
	binary.convertTo(Labels, Labels.type());
    int label=2; // starts at 2 because 0,1 are used already

	int x=peak.x, y=peak.y;
    if((int)Labels(y,x) != 1) return rect;

    floodFill(Labels, peak, label, &rect, 0, 0, 4);
	return rect;
}
Mat morphCleanup(const Mat & img)
{
	Mat res=img.clone();
	erode(res, res, Mat()); // clean up noise
	static const Mat ker=getStructuringElement(MORPH_ELLIPSE, Size(5,5)); // TODO: param/config
	dilate(res, res, ker, Point(-1,-1), 2); // close holes: two iterations with 5x5 ker are faster than one iteration with 8x8 ker
	return res;
}

/*! @brief suppress non-maximal values
 *
 * nonMaximaSuppression produces a mask (dst) such that every non-zero
 * value of the mask corresponds to a local maxima of src. The criteria
 * for local maxima is as follows:
 *
 * 	For every possible (sz x sz) region within src, an element is a
 * 	local maxima of src iff it is strictly greater than all other elements
 * 	of windows which intersect the given element
 *
 * Intuitively, this means that all maxima must be at least sz+1 pixels
 * apart, though the spacing may be greater
 *
 * A gradient image or a constant image has no local maxima by the definition
 * given above
 *
 * The method is derived from the following paper:
 * A. Neubeck and L. Van Gool. "Efficient Non-Maximum Suppression," ICPR 2006
 *
 * Example:
 * \code
 * 	// create a random test image
 * 	Mat random(Size(2000,2000), DataType<float>::type);
 * 	randn(random, 1, 1);
 *
 * 	// only look for local maxima above the value of 1
 * 	Mat mask = (random > 1);
 *
 * 	// find the local maxima with a window of 50
 * 	Mat maxima;
 * 	nonMaximaSuppression(random, 50, maxima, mask);
 *
 * 	// optionally set all non-maxima to zero
 * 	random.setTo(0, maxima == 0);
 * \endcode
 *
 * @param src the input image/matrix, of any valid cv type
 * @param sz the size of the window
 * @param mask an input mask to skip particular elements
 * @return map of local peaks
 */
SparseMatReal nonMaximaSuppression(const Mat& src, const int sz, const Mat &mask=Mat())
{
	// initialize the block mask and destination
	const int M = src.rows;
	const int N = src.cols;
	const bool masked = !mask.empty();
	MatUC block = 0xFF*MatUC::ones(Size(2*sz+1,2*sz+1));
	SparseMatReal sdst(2, src.size);

	// iterate over image blocks
	for (int m = 0; m < M; m+=sz+1)
	{
		for (int n = 0; n < N; n+=sz+1)
		{
			Point  ijmax;
			double vcmax=0;

			// get the maximal candidate within the block
			Range ic(m, min(m+sz+1,M));
			Range jc(n, min(n+sz+1,N));
			minMaxLoc(src(ic,jc), NULL, &vcmax, NULL, &ijmax, masked ? mask(ic,jc) : noArray());
			
			if (vcmax==0) continue; // skip 0 level

			Point cc = ijmax + Point(jc.start,ic.start);

			// search the neighbors centered around the candidate for the true maxima
			Range in(max(cc.y-sz,0), min(cc.y+sz+1,M));
			Range jn(max(cc.x-sz,0), min(cc.x+sz+1,N));

			// mask out the block whose maxima we already know
			MatUC blockmask;
			block(Range(0,in.size()), Range(0,jn.size())).copyTo(blockmask);
			Range iis(ic.start-in.start, min(ic.start-in.start+sz+1, in.size()));
			Range jis(jc.start-jn.start, min(jc.start-jn.start+sz+1, jn.size()));
			blockmask(iis, jis) = MatUC::zeros(Size(jis.size(),iis.size()));

			double vnmax=0;
			minMaxLoc(src(in,jn), NULL, &vnmax, NULL, &ijmax, masked ? mask(in,jn).mul(blockmask) : blockmask);

			// if the block center is also the neighbor center, then it's a local maximum
			if (vcmax >= vnmax) sdst.ref(cc.y, cc.x) = vcmax;
		}
	}
	return sdst;
}

MatReal getHighestPeaks(const SparseMatReal & SrcPeaksMap, int HighestPeaksCnt=32)
{
	SparseMatReal PeaksMap=SrcPeaksMap;
	HighestPeaksCnt=min<int>(HighestPeaksCnt, PeaksMap.nzcount());
	MatReal HighestPeaks=MatReal::zeros(HighestPeaksCnt, 3);
	for(int k=0; k<HighestPeaksCnt; ++k) // in the order of priority
	{
		double maxval;
		int maxIdx[2];
		minMaxLoc(PeaksMap, 0, &maxval, 0, maxIdx);
		if (maxval<=0) break; // no more peaks to be found
		const int
			r=maxIdx[0],
			c=maxIdx[1];
		HighestPeaks(k,0)=r;
		HighestPeaks(k,1)=c;
		HighestPeaks(k,2)=maxval;
		PeaksMap.ref(r,c)=0; // suppress the peak
	}
	return HighestPeaks;
}

/// Plot elements of a sparse matrix
void imshow(const string & title, const SparseMat & simg)
{
	MatUC img; simg.convertTo(img, img.type());
	normalize(img, img, 0, 255, NORM_MINMAX);
	imshow(title, img);
}

/// Print elements of a sparse floating-point matrix and the sum of elements.
ostream & operator<<(ostream & s, const SparseMat & sm)
{
	auto
		it = sm.begin(),
		it_end = sm.end();
	int dims = sm.dims();
	for(; it != it_end; ++it)
	{
		// print element indices and the element value
		const SparseMat::Node* n = it.node();
		s<<"(";
		for(int i=0; i<dims; ++i)
			s << n->idx[i] << (i < dims-1 ? ", " : ")");
		s<<"="<<it.value<float>()<<endl;
	}
	return s;
}

/**
 * Compute a set of highest peaks on the map.
 * @param map	2D landmark likelihood map
 * @param dist	allowable distance between adjasent peaks
 * @param HighestPeaksCnt	highest peaks count
 * @param floor	floor of the likelihood map
 */
MatReal getPeaks(const MatReal & map, int dist=20, const int HighestPeaksCnt=64, REALNUM floor=64)
{
	Mat mask = (map>floor);
	SparseMatReal PeaksMap=nonMaximaSuppression(map, dist, mask);
	return getHighestPeaks(PeaksMap, HighestPeaksCnt);
}

Mat drawRegions(const Mat & ImgBGR, const RectPack & Regs, const Scalar & color)
{
	Mat DstImg=ImgBGR.clone();
	for (const auto & rgn: Regs)
		rectangle(DstImg, rgn, color);
	return DstImg;
}
void AdaptiveBlobDetection(const Mat& MapLM, RectPack & dst_rect, REALNUM threshold_value, int r, int c)
{
	int max_BINARY_value = 1;
	int threshold_type = 0;
	Mat binarized_img;
	threshold(MapLM, binarized_img, threshold_value, max_BINARY_value, threshold_type);
	Rect rgn = getPeakRegion(binarized_img, Point(c,r));
	if (rgn.area()==0) return;
	Rect R(0, 0, MapLM.cols, MapLM.rows);
	FaceMatch::stretch(rgn, 2); // TODO: param/config
	if (rgn.width>=R.width || rgn.height>=R.height) return;
	dst_rect.push_back(rgn&R);
}
void getBlobs(RectPack & regs, const MatReal & map, const MatReal & peaks, const REALNUM PeakRelativeT=0.25)
{
	for(auto i=0; i<peaks.rows; ++i)
		AdaptiveBlobDetection(map, regs, peaks(i,2)*PeakRelativeT, peaks(i,0), peaks(i,1));
}
static const REALNUM cValidationToleranceDefault=0.75;
void filterBlobs(RectPack & regs, const Mat & img, const ObjectDetectorANN & od, const REALNUM tol=cValidationToleranceDefault)
{
	for (auto it=regs.begin(); it!=regs.end(); )
	{
		Mat sub = GrayScale(img(*it));
		REALNUM s=od.detect(sub);
		if (s<tol)
			it=regs.erase(it);
		else ++it;
	}
}
ObjectDetectorANN & getObjectDetectorANNEyes()
{
	static const char
		*EncFN="eyes2_32x32_128_46106.ann.yml", // TODO: config/param
		*DecFN="eyes2_128_64_2_AFLW_128_64_199_95.53.ann.yml"; // TODO: config/param
	StaticLkdCtor ObjectDetectorANN od(EncFN, DecFN);
	return od;
}
void filterEyes(RectPack & regs, const Mat & img)
{
	filterBlobs(regs, img, getObjectDetectorANNEyes());
}
ObjectDetectorANN & getObjectDetectorANNMouth()
{
	static const char
		*EncFN="mouth_32x32_128_37502.ann.yml", // TODO: config/param
		*DecFN="mouth_128_64_2_AFLW_128_64_152_94.51.ann.yml"; // TODO: config/param
	StaticLkdCtor ObjectDetectorANN od(EncFN, DecFN);
	return od;
}
static void filterMouth(RectPack & regs, const Mat & img)
{
	filterBlobs(regs, img, getObjectDetectorANNMouth());
}
void scatterPeaks(const string & title, const Mat & src, const MatReal & peaks)
{
	SparseMatReal scatterPlot(2, src.size);
	for (auto r=0; r<peaks.rows; ++r)
		scatterPlot.ref(peaks(r,0), peaks(r,1))=peaks(r,2);
	imshow(title, scatterPlot);
}
bool isValidLandmark(const FaceRegion & fr, const Mat & img)
{
	Mat sub = img(fr);
	REALNUM val=1; // assume valid
	if (fr.Kind=="i")
		val=getObjectDetectorANNEyes().detect(sub);
	else if (fr.Kind=="m")
		val=getObjectDetectorANNMouth().detect(sub);
	return val>cValidationToleranceDefault;
}
void getFaceColorLandmarks(const Mat & ImgBGR, RectPack & EyeRegs, RectPack & MouthRegs)
{
	MatReal Y_band, Cr_band, Cb_band;
	split2YCbCr(ImgBGR, Y_band, Cr_band, Cb_band);

	MatReal EyeMap, MouthMap;
	getEyeMap(EyeMap, Y_band, Cr_band, Cb_band);
	auto VerbLevel = getVerbLevel();
	if (VerbLevel>1) imshow("EyeMap", MatUC(EyeMap));
	getMouthMap(MouthMap, Cb_band, Cr_band);
	if (VerbLevel>1) imshow("MouthMap", MatUC(MouthMap));

	MatReal
		EyePeaks = getPeaks(EyeMap),
		MouthPeaks = getPeaks(MouthMap);
	if (VerbLevel>1)
	{
		if (VerbLevel>2) clog<<"EyePeaks="<<EyePeaks<<endl;
		scatterPeaks("EyePeaks", EyeMap, EyePeaks);
		if (VerbLevel>2) clog<<"MouthPeaks="<<MouthPeaks<<endl;
		scatterPeaks("MouthPeaks", MouthMap, MouthPeaks);
	}

	getBlobs(EyeRegs, EyeMap, EyePeaks);
	getBlobs(MouthRegs, MouthMap, MouthPeaks);
	if (VerbLevel>1)
	{
		Mat ImgEyes = drawRegions(ImgBGR, EyeRegs, Scalar(0xFF,0,0)); imshow("ImgEyes", ImgEyes); 
		Mat	ImgMouth = drawRegions(ImgBGR, MouthRegs, Scalar(0xFF,0,0)); imshow("ImgMouth", ImgMouth); 
	}

	filterEyes(EyeRegs, ImgBGR);
	filterMouth(MouthRegs, ImgBGR);
	if (VerbLevel>1) waitKey();
}
void getFaceLandmarkRegions(FaceRegions & outLikelyRegions, const Mat & img)
{
	RectPack EyeRegs, MouthRegs;
	getFaceColorLandmarks(img, EyeRegs, MouthRegs);
	static const REALNUM minAspect=0.2; // TODO: param/config
	const unsigned minDim=outLikelyRegions.getMinDim();
	outLikelyRegions=FaceRegions("i", EyeRegs, minDim, minAspect);
	outLikelyRegions.merge(FaceRegions("m", MouthRegs, minDim, minAspect));
}
Mat computeShowFaceLandmarks(const Mat & ImgBGR)
{
	RectPack EyeMapRegs, MouhtMapRegs;
	getFaceColorLandmarks(ImgBGR, EyeMapRegs, MouhtMapRegs);
	Mat DstImg=drawRegions(ImgBGR, EyeMapRegs, Scalar(0xFF,0,0));
	return drawRegions(DstImg, MouhtMapRegs, Scalar(0,0,0xFF));
}

static void meshgrid(const Mat &xgv, const Mat &ygv, Mat &X, Mat &Y)
{
	repeat(xgv.reshape(1,1), ygv.total(), 1, X);
	repeat(ygv.reshape(1,1).t(), 1, xgv.total(), Y);
}
MatReal getGaussianBell(const Rect & landmark, REALNUM sigma)
{
	MatReal f = MatReal::zeros(landmark.width, landmark.height);

	Mat X, Y;
	vector<float> t_x, t_y;
	float  interval; 

	interval = (float)1/(landmark.width-1);
	for (float i = -0.5; i <= 0.51; i = i+interval) t_x.push_back(i);

	interval = (float)1/(landmark.height-1);
	for (float i = -0.5; i <= 0.51; i = i+interval) t_y.push_back(i); 

	meshgrid(Mat(t_x), Mat(t_y), X, Y);

	Mat_<float> XX = X.mul(X); 
	Mat_<float> YY = Y.mul(Y); 
	f = -(XX + YY)/(2*(sigma*sigma));
	exp(f,f); 
	
	float TotalSum = 0;
	for (int i = 0; i <= f.rows-1; i++)
		for (int j = 0; j <= f.cols-1; j++)
			TotalSum = TotalSum + f(i,j);
	
	f = f/TotalSum;
	normalize(f, f, 0, 1, NORM_MINMAX, -1, Mat());
	return f;
}
void placeBell2Map(MatReal & SaliencyMap,MatReal & bell,Rect landmark)
{
	for (int i = 0; i <= bell.rows-1; i++)
		for (int j = 0; j <= bell.cols-1; j++)
		{
			int
				salmaprow = landmark.y + i-1,
				salmapcol = landmark.x + j-1;
			if(bell(i,j)>SaliencyMap(salmaprow,salmapcol)) 
				SaliencyMap(salmaprow,salmapcol) = bell(i,j);	
		}
}
void GaussianBell(MatReal & dst, const Rect & r)
{
	MatReal GB = getGaussianBell(r, 0.5);
	placeBell2Map(dst, GB, r);
}

} // namespace FaceMatch
