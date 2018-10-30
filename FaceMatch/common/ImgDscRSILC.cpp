
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
#include "ImgDscRSILC.h"

namespace FaceMatch
{

const int
	cLineFilterCount = 6,
	cSectorAngle=180/cLineFilterCount;

/// image filter for the key-lines
class Filter
{
	Mat mLineImage[cLineFilterCount];
public:
	/// Define line filters: degree[i] = 30*i, for i=0:5.
	Filter()
	{
		mLineImage[0] = (MatReal(9,9) // 0
			<< 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0.1080, 0.2590, 0.4839, 0.7041, 0.7980, 0.7041, 0.4839, 0.2590, 0.1080,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0);
		mLineImage[1] = (MatReal(9,9) // 30
			<< 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0.2590, 0.1080,
			0, 0, 0, 0, 0, 0.7041, 0.4839, 0.2590, 0,
			0, 0, 0, 0.7041, 0.7980, 0.7041, 0, 0, 0,
			0, 0.2590, 0.4839, 0.7041, 0, 0, 0, 0, 0,
			0.1080,0.2590, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0);
		mLineImage[2] = mLineImage[1].t(); // 60
		mLineImage[3] = mLineImage[0].t(); // 90
		cv::flip(mLineImage[2], mLineImage[4], 0); // 120
		cv::flip(mLineImage[1], mLineImage[5], 0); // 150
	}
	const Mat & getLineImage(byte i)const{ return mLineImage[i]; }
} sFilter;

template <typename T>
Mat_<T> getRow(const vector<T> & v)
{
	Mat_<T> r(v), t=r.t();
	return t; // strange conversion, but that's how it works
}

template <typename T>
Mat_<T> getMatrix(const vector<vector<T>> & m)
{
	Mat_<T> M;
	for (const auto & v: m)
		M.push_back(getRow(v));
	return M;
}

void FindBlobs(const Mat &binary, vector < vector<Point2i> > &blobs)
{
	FTIMELOG

	blobs.clear();

	// Fill the label_image with the blobs
	// 0  - background
	// 1  - unlabeled foreground
	// 2+ - labeled foreground

	Mat label_image;
	binary.convertTo(label_image, CV_32FC1); // it doesn't support CV_32S!

	int label_count = 2; // starts at 2 because 0,1 are used already

	for(int y=0; y < binary.rows; y++)
	{
		for(int x=0; x < binary.cols; x++)
		{
			if((int)label_image.at<float>(y,x) != 1) continue;

			Rect rect;
			floodFill(label_image, Point(x,y), Scalar(label_count), &rect, Scalar(0), Scalar(0), 8);
			vector <Point2i> blob;

			//TODO: get rid of these for loops by working only with rectangles.
			for(int i=rect.y; i < (rect.y+rect.height); i++){
				for(int j=rect.x; j < (rect.x+rect.width); j++){
					if((int)label_image.at<float>(i,j) != label_count) continue;
					blob.push_back(cv::Point2i(j,i));
				}
			}

			blobs.push_back(blob);
			label_count++;
		}
	}
}

int medFunction(vector<int> &ArrayA)
{
	int medValue = 0;
	int temp = 0;
	int count = 0;

	for(count = 0; count < ArrayA.size(); count++)
		for(int j = (count + 1); j < ArrayA.size(); j++)
		{

			if(ArrayA[count] > ArrayA[j])
			{
				temp = ArrayA[j];
				ArrayA[j] = ArrayA[count];
				ArrayA[count] = temp;
			}
		}

	if((count % 2) == 0)
		medValue = (ArrayA[count/2] + ArrayA[count/2 - 1])/2;
	else
		medValue = ArrayA[count/2];

	return medValue;
}

void mean(int &meanVal, int &minVal, int &maxVal, vector<int> &ArrayA){

	minVal = ArrayA[0];
	maxVal = ArrayA[0];
	int sum = 0;

	for(size_t i = 0; i<ArrayA.size(); i++)
	{
		sum = sum + ArrayA[i];
		if(ArrayA[i]<minVal)
			minVal = ArrayA[i];
		if(ArrayA[i]>maxVal)
			maxVal = ArrayA[i];
	}
	meanVal = (int)sum/ArrayA.size();
}

void ImgDscRSILC::ComputeLineCoords(const MatUC & SrcImg, const Mat & LineImg, int LineIndex)
{
	TIMELOG(getType()+"::ComputeLineCoords");

	Mat RefinedLineImg(LineImg.rows, LineImg.cols, CV_32F, Scalar(0));

	int lineLengthThr = *LineImg.size/40;
	int x=0,y=0;
	vector<int> medArr_c, medArr_r;
	int maxVal_r, maxVal_c, mean_c, mean_r, minVal_r, minVal_c;
	float line_length=0;

	// separate the lines using connected component relations.
	vector < vector<Point2i> > blobs; // blobs hold the coordinates of lines.

	//TODO: instead of saving the coordinates of the regions, compute only rectangles, and return the center of the rectangle (which will be the center of the line).
	FindBlobs(LineImg, blobs);

	for(int cnt = 0; cnt<blobs.size(); cnt++)
	{
		if(blobs[cnt].size() > lineLengthThr)  // do not consider shorter lines
		{
			mean_r=0, mean_c=0, maxVal_r=0, maxVal_c=0;
			minVal_r = std::numeric_limits<int>::max();
			minVal_c = std::numeric_limits<int>::max();
			medArr_r.resize(blobs[cnt].size());
			medArr_c.resize(blobs[cnt].size());

			// find the center of the lines
			for(int in_cnt =0; in_cnt<blobs[cnt].size(); in_cnt++)  // locate the line coordinates into vector to calculate the median value.
			{
				y = blobs[cnt][in_cnt].y;
				x = blobs[cnt][in_cnt].x;

				mean_r = mean_r + y;
				mean_c = mean_c + x;
				if(y < minVal_r) minVal_r = y;
				if(x < minVal_c) minVal_c = x;
				if(y > maxVal_r) maxVal_r = y;
				if(x > maxVal_c) maxVal_c = x;
			}
			mean_r = mean_r / blobs[cnt].size();
			mean_c = mean_c / blobs[cnt].size();

			// compute the line length
			y = maxVal_r - minVal_r;
			x = maxVal_c - minVal_c;

			line_length = sqrt(x*x+y*y);

			// We are using radius value to determine height and width of ROI, so it should be int type.
			int radius = (line_length*2)/3;

			// check the line location: if some part of the radius is outside of the image border, then ignore the line
			if(mean_c-radius >= 0 && mean_c+radius < LineImg.cols && mean_r-radius >= 0 && mean_r+radius < LineImg.rows)
			{
				REALNUM angle=cSectorAngle*LineIndex;
				switch(LineIndex) // use gradient-dependent 2pi-aware orientation
				{
				case 0: if (SrcImg(mean_r+1, mean_c) < SrcImg(mean_r-1, mean_c)) angle+=180; break;
				case 1: case 2: if (SrcImg(mean_r+1, mean_c+1) < SrcImg(mean_r-1, mean_c-1)) angle+=180; break;
				case 3: if (SrcImg(mean_r, mean_c+1) < SrcImg(mean_r, mean_c-1)) angle+=180; break;
				case 4: case 5: if (SrcImg(mean_r-1, mean_c+1) < SrcImg(mean_r+1, mean_c-1)) angle+=180; break;
				}
				mKeyLines.push_back(KeyLine(mean_c, mean_r, line_length, angle));
			}
		}
	}
}

/// @return convolution of line filters with the edge image
Mat convolve(const Mat & ImgEdge, const Mat & aFilter)
{// TODO: optimize
	FTIMELOG
	Mat Img_Edge = ImgEdge/0xFF; // normalize
	Img_Edge.convertTo(Img_Edge, CV_32FC1);
	Mat LineImg; filter2D(Img_Edge, LineImg, -1, aFilter); // filter2D produces shorter lines than conv. It is better to use conv2() instead.
	LineImg.convertTo(LineImg, CV_32FC1); // TODO: remove, when normalization stops requiring that
	return LineImg;
}

/// Binarize the sFilter output.
void binarize(Mat & LineImg, float thr)
{// TODO: optimize
	FTIMELOG

	if (getVisVerbLevel()>2) imOut("lin", MatUC(LineImg*0xFF));

	//TODO: possible push thresholding to the GPU
	double minval=0, maxval=0;
	int maxIdx[2];
	minMaxIdx(LineImg,&minval,&maxval,0,maxIdx);

	LineImg = (LineImg-minval)/(maxval-minval);
	if (getVisVerbLevel()>2) imOut("exp-lin", MatUC(LineImg*0xFF)), waitKey();

	float val=0;
	MatReal & rfLineImg = (MatReal&)LineImg;
	for(size_t i=0; i<LineImg.rows; ++i)
	{
		for(size_t j=0; j<LineImg.cols; ++j)
		{
			val = rfLineImg(i,j);
			if(val>thr)
				rfLineImg(i,j) = 1;
			else
				rfLineImg(i,j) = 0;
		}
	}
}

void ImgDscRSILC::ApplyLineFilters(Mat &Img)
{
	TIMELOG(getType()+"::ApplyLineFilters");
//--- compute an edge map
	Img = gblur(Img, Size(9, 9), 1.8); // config/param
	if (getVisVerbLevel()>1) imOut("blr", Img);
	Mat Img_Edge;
	Canny(Img, Img_Edge, 35, 120); // 36, 90); // TODO: param/config
	if (getVisVerbLevel()>1) imOut("edg", Img_Edge);
	if (!countNonZero(Img_Edge))
		throw Exception("no edges found for RSILC");
//--- apply line filters to Img_Edge
	for(size_t i=0; i<cLineFilterCount; ++i)
	{
		Mat LineImg = convolve(Img_Edge, sFilter.getLineImage(i));
		if (getVisVerbLevel()>2) imOut(format("conv-lin%d", i), Mat_<byte>(LineImg*0xFF));
		binarize(LineImg, 0.5);
		if (getVisVerbLevel()>2) imOut(format("norm-lin%d", i), Mat_<byte>(LineImg*0xFF));
		ComputeLineCoords(Img, LineImg, i);
	}
}

/// @return 0<=angle<=360
inline REALNUM getAngle(REALNUM y, REALNUM x)
{
	REALNUM angle=atan2(y,x)*R2D;
	return angle<0 ? angle+360 : angle;
}

/// Compute the gradient magnitude and gradient orientation of an image.
void ComputeGradient(const Mat &Img, MatReal &grad_mag, MatReal &grad_angle)
{
	FTIMELOG
	MatReal grad_x, grad_y;
	Sobel(Img, grad_x, grad_x.type(), 1, 0);
	Sobel(Img, grad_y, grad_y.type(), 0, 1);
	for(size_t r=0; r<grad_x.rows; ++r)
		for(size_t c=0; c<grad_x.cols; ++c)
		{
			REALNUM dx=grad_x(r,c), dy=grad_y(r,c);
			grad_mag(r,c) = sqrt(dx*dx+dy*dy);
			grad_angle(r,c) = getAngle(dy,dx);
		}
}

/// Compute the region of interest for MainLine
void cropRoI(const ImgDscRSILC::KeyLine & Main_Line, const Mat &Img_Y, const Mat &Img_Cb, const Mat &Img_Cr, const Mat &Grad_Mag, const Mat &Grad_Angle,
	Mat &crop_Img_Y, Mat &crop_Img_Cb, Mat &crop_Img_Cr, Mat &crop_Grad_Mag, Mat &crop_Grad_Angle, Mat &crop_pixel_angle)
{
	FTIMELOG

	const int radius = 2*int(Main_Line.size)/3; // TODO: remove cast

	// crop Region of Interest
	Rect myRoI(Main_Line.pt.x-radius, Main_Line.pt.y-radius, 2*radius+1, 2*radius+1); // TODO: why +1 ?

	crop_Img_Y = Img_Y(myRoI).clone();
	crop_Img_Cb = Img_Cb(myRoI).clone();
	crop_Img_Cr = Img_Cr(myRoI).clone();
	crop_Grad_Mag = Grad_Mag(myRoI).clone();
	crop_Grad_Angle = Grad_Angle(myRoI).clone();

	// angle values for each pixel in the crop region
	crop_pixel_angle = Mat_<int>::zeros(crop_Img_Y.rows, crop_Img_Y.cols);

	// removing the values outside the circle by assigning -1 to these locations
	const int // TODO: why -1 ? why not use myRoI ?
		rect_center_c = (crop_Img_Y.cols-1)/2,
		rect_center_r = (crop_Img_Y.rows-1)/2;
	// these coordinates will be used to compute the angle array
	for(int r=0; r<crop_Img_Y.rows; ++r)
		for(int c=0; c<crop_Img_Y.cols; ++c)
		{
			// In order to find the pixels inside the circle, measure the distance of each pixel to the center pixel.
			// If the distance is bigger than radius, then, this pixel is out of the circle.
			REALNUM
				x = c-rect_center_c,
				y = rect_center_r-r,
				distance = sqrt(y*y + x*x);
			// in order to remove the values outside of the circle
			if(distance > radius)
			{
				crop_Img_Y.at<int>(r,c) = -1;
				crop_Img_Cb.at<int>(r,c) = -1;
				crop_Img_Cr.at<int>(r,c) = -1;
				crop_Grad_Mag.at<float>(r,c) = -1;
				crop_Grad_Angle.at<float>(r,c) = -1;
			}
			int pixelAngle = getAngle(y,x);
			pixelAngle = pixelAngle-Main_Line.angle;
			crop_pixel_angle.at<int>(r,c) = (pixelAngle<0 ? pixelAngle+360 : pixelAngle);
		}
}

/// Compute the section matrix: the elements of the matrix are the section numbers w.r.t. the main line.
/// This matrix is used during relational feature computation.
void CreateSectionMatrix(MatInt &SectionMatrix, const Mat &Img, const ImgDscRSILC::KeyLine &Main_Line, int numberOfSection)
{
	FTIMELOG
// TODO: do we really need this LUT?
	int pixelAngle=0;
	const REALNUM AngleRange = 360.0/numberOfSection;
	// initialize the crop_pixel_angle matrix as the same size of cropped regions.
	SectionMatrix = MatInt::zeros(Img.rows, Img.cols);
	for(int r=0; r<Img.rows; ++r)
		for(int c=0; c<Img.cols; ++c)
		{
			REALNUM
				x = c - Main_Line.pt.x,
				y = Main_Line.pt.y - r;
			pixelAngle = getAngle(y,x); // atan2(y,x)*R2D;
			pixelAngle = pixelAngle-Main_Line.angle;
			if (pixelAngle<0) pixelAngle+=360;
			SectionMatrix(r, c) = pixelAngle/AngleRange;
		}
}

template<typename NUM>
void computeHistogram(vector<float> &hist, const vector<NUM> &Values, int nof_bins, int maxRange)
{
	FTIMELOG
	hist.assign(nof_bins, 0);
	const size_t VLen=Values.size();
	if (!VLen) return;
	for(size_t i=0; i<VLen; ++i)
	{
		int index = (Values[i]*nof_bins)/maxRange;
		if(index >= nof_bins)
			index = nof_bins -1;
		hist[index] = hist[index] + 1;
	}
	for(size_t i=0; i< hist.size(); i++)
		hist[i] = hist[i]/VLen;
}

/**
* Take ROI as input, then check each pixel located in the observed subsection.
* If it is in the subsection, the pixel value is saved into a vector to create histograms.
*/
void GetValsInSections(vector<float> &h_angle, vector<float> &h_mag,
	vector<float> &h_R, vector<float> &h_G, vector<float> &h_B,
	const Mat &crop_Img_R, const Mat &crop_Img_G, const Mat &crop_Img_B,
	const Mat &crop_Grad_Angle, const Mat &crop_Grad_Mag, const Mat &crop_pixel_angle,
	int sec_no, int numberOfSection, int Nof_local_bins)
{
	FTIMELOG

	float AngleRange = (float)360/numberOfSection;
	float LowerAngle = sec_no*AngleRange;
	float UpperAngle = (sec_no+1)*AngleRange;
	int maxval_forMag = 360; // TODO: find the maximum Magnitute value of Image, OR normalize the Mag of the image between 0-X

	vector<float> Sec_Angle;
	vector<float> Sec_Mag;
	vector<int> Sec_Img_R;
	vector<int> Sec_Img_G;
	vector<int> Sec_Img_B;

	for(size_t r = 0; r<crop_Img_R.rows; r++)
	{
		for(size_t c = 0; c<crop_Img_R.cols; c++)
		{
			int pixelAngleVal = crop_pixel_angle.at<int>(r,c);
			if(pixelAngleVal>=LowerAngle && pixelAngleVal<UpperAngle)
			{
				// outside of the circle has -1 values, skip these values while creating histograms.
				if(crop_Grad_Angle.at<float>(r,c) >= 0)
					Sec_Angle.push_back(crop_Grad_Angle.at<float>(r,c));
				if(crop_Grad_Mag.at<float>(r,c) >= 0)
					Sec_Mag.push_back(crop_Grad_Mag.at<float>(r,c));
				if(crop_Img_R.at<int>(r,c) >= 0)
					Sec_Img_R.push_back(crop_Img_R.at<int>(r,c));
				if(crop_Img_G.at<int>(r,c) >= 0)
					Sec_Img_G.push_back(crop_Img_G.at<int>(r,c));
				if(crop_Img_B.at<int>(r,c) >= 0)
					Sec_Img_B.push_back(crop_Img_B.at<int>(r,c));
			}
		}
	}
	computeHistogram(h_angle, Sec_Angle, Nof_local_bins, 360); // maximum value for gradient angle values is 360
	computeHistogram(h_mag, Sec_Mag,Nof_local_bins,maxval_forMag);
	computeHistogram(h_R, Sec_Img_R,Nof_local_bins,255); // maximum value for intensity values is 255
	computeHistogram(h_G, Sec_Img_G,Nof_local_bins,255);
	computeHistogram(h_B, Sec_Img_B,Nof_local_bins,255);
}

template<typename T>
inline T sqr(const T & a) {return a*a;}

template<typename T>
inline REALNUM norm(const T & a)
{return sqrt(sqr(a.x)+sqr(a.y));}

void ImgDscRSILC::SpatialInfo(vector<float> &h_angdif, vector<float> &h_nofLines, const KeyLine &Main_Line, const MatInt &SectionMatrix, int sec_no, int Nof_spt_bins)
{
	TIMELOG(getType()+"::SpatialInfo");
	const static int MaxAbsAngleDiff=360;

	vector<int> sptDegreeArr; // holds the difference of line direction angles between main line and other lines.
	vector<int> sptNofLinesArr; // holds the angular section values inside the

	// check the location of all lines in this section (in sec_no).
	for(size_t i=0, len=mKeyLines.size(); i<len; i++)
	{
		// is that line in this section?
		const KeyLine & KL=mKeyLines[i];
		if(SectionMatrix(KL.pt.y, KL.pt.x) == sec_no)
		{
			// Spatial Feature 1:  direction angle difference between main line and other lines in the observed section

			int DegreeDiff = abs((int)KL.angle - (int)Main_Line.angle); // TODO: remove cast
			sptDegreeArr.push_back(DegreeDiff);

			// Spatial Feature 2: number of lines in each angular section.
			REALNUM line_distance = norm(Main_Line.pt-KL.pt);
			int line_distance_portion = line_distance/(int)Main_Line.size; // TODO: remove cast // line_distance_portion is the angular section that the observed line (mKeyLines[i]) in

			if(line_distance_portion>3)
				sptNofLinesArr.push_back(3);
			else if(line_distance_portion>2)
				sptNofLinesArr.push_back(2);
			else if(line_distance_portion>1)
				sptNofLinesArr.push_back(1);
			else if(line_distance_portion>=0)
				sptNofLinesArr.push_back(0);
		}
	}

	if(sptDegreeArr.size()==0)
		for(size_t cnt = 0; cnt<Nof_spt_bins; cnt++)
			h_angdif.push_back(0);
	else
		computeHistogram(h_angdif, sptDegreeArr, Nof_spt_bins, MaxAbsAngleDiff);

	if(sptDegreeArr.size()==0)
		for(size_t cnt = 0; cnt<Nof_spt_bins; cnt++)
			h_nofLines.push_back(0);
	else
		computeHistogram(h_nofLines, sptNofLinesArr, Nof_spt_bins, 4); // TODO: config/param
}

template<typename T>
inline void cat(vector<T> & d, const vector<T> & a)
{
	d.insert(d.end(), a.begin(), a.end());
}

static const int // TODO: param/config
	Nof_local_bins = 8,
	Nof_spt_bins = 4;

void ImgDscRSILC::ComputeHistogramROI(VectorReal & hLine,
	const Mat &crop_Img_Y, const Mat &crop_Img_Cb, const Mat &crop_Img_Cr,
	const Mat &crop_Grad_Angle, const Mat &crop_Grad_Mag,
	const Mat &crop_pixel_angle, const Mat &SectionMatrix,
	const KeyLine &Main_Line, int SecCnt)
{
	TIMELOG(getType()+"::ComputeHistogramROI");
	hLine.clear();

	for(size_t sec_no=0; sec_no<SecCnt; ++sec_no)
	{
	//--- local region histograms (histograms computed inside the circle of line for observed section).
		VectorReal h_angle, h_mag, h_Y, h_Cb, h_Cr;
		GetValsInSections(h_angle, h_mag, h_Y, h_Cb, h_Cr, crop_Img_Y, crop_Img_Cb, crop_Img_Cr, crop_Grad_Angle, crop_Grad_Mag, crop_pixel_angle, sec_no, SecCnt, Nof_local_bins);
		cat(hLine, h_angle);
		cat(hLine, h_mag);
		cat(hLine, h_Y);
		cat(hLine, h_Cb);
		cat(hLine, h_Cr);
		if (mUseSpatialInfo)
		{
		//--- spatial info from other lines in this subsection.
			VectorReal h_angdif, h_nofLines;
			SpatialInfo(h_angdif, h_nofLines, Main_Line, SectionMatrix, sec_no, Nof_spt_bins);
			cat(hLine, h_angdif);
			cat(hLine, h_nofLines);
		}
	}
}

REALNUM diff(const ImgDscRSILC::DescriptorRow & a, const ImgDscRSILC::DescriptorRow & b)
{
	// TIMELOG("RSILCdiff");
	REALNUM sm=0; // L1(a,b)
	const int len=a.cols;
	for(int i=0; i<len; ++i) sm+=abs(a(0,i)-b(0,i)); // using proximity caching
	return sm; // TODO: enough?
	// NOTE: computing L1's in separate omp threads is not effective in win7
	REALNUM ms=0; // L1(a, b.reflected); // ensure reflection-invariance
	const int len2=len/2; // reflective the indexing for |ai-bk|, where k=n/2-i if i<n/2, k=n-i, if i<n
	for(int i=0, j=len2; j<len; ++i, ++j)
		// ms+=abs(a(0,i)-b(0,j))+abs(a(0,j)-b(0,i)); // half-swapped
		ms+=abs(a(0,i)-b(0,len2-i-1))+abs(a(0,j)-b(0,len-i-1)); // reflective
	REALNUM d=min(sm, ms);
	if (getVerbLevel()>1 && sm>d) clog<<"sm="<<sm<<" ms="<<ms<<" diff="<<abs(sm-ms)<<endl;
	return d;
}

void print(vector< vector<float> > & dif_val_mat)
{
	std::cout << std::setprecision(2) << std::fixed;
	for (vector< vector<int> >::size_type u = 0; u < dif_val_mat.size(); u++)
	{
		for (vector<int>::size_type v = 0; v < dif_val_mat[u].size(); v++)
			cout << dif_val_mat[u][v] << " ";
		cout << endl;
	}
	cout << "==================================== "<< endl;
}

/// @return output stream after text-serialization of the object
ostream & operator<<(ostream & s, ///< output text stream
	const KeyPoint& kp /**< key-point to print */)
{
	s<<"{"<<kp.pt<<", "<<kp.size<<", "<<kp.angle<<", "<<kp.response<<", "<<kp.octave<<", "<<kp.class_id<<"}";
	return s;
}

ostream & operator<<(ostream & s, const ImgDscRSILC::KeyLines& KL)
{
	for (const auto & kl: KL) s<<kl<<endl;
	return s;
}

void ImgDscRSILC::print(ostream & s, const string & fmt)const
{
	unsigned
		VL=getVerbLevel(),
		KLCnt=mKeyLines.size(),
		DscLen=mDescriptor.cols;
	if (fmt=="ACRD")
	{
		s<<(VL ? DscLen : 1)<<endl
		<<KLCnt<<endl; // http://www.robots.ox.ac.uk/~vgg/research/affine/detectors.html
		// x y a b c d d d ...
		for (unsigned i=0; i<KLCnt; ++i)
		{
			const auto & kln = mKeyLines[i];
			const REALNUM // http://www.maa.org/external_archive/joma/Volume8/Kalman/General.html
				M = kln.size, M2=1./sqr(M),
#if 0 // estimated ellipse: weaker repeatability compared to the circle
				m = M/2, m2=4*M2,
				an = -kln.angle,
				ca=cos(an), ca2=sqr(ca),
				sa=sin(an), sa2=sqr(sa),
				a=ca2*M2+sa2*m2,
				b=ca*sa*(M2-m2),
				c=sa2*M2+ca2*m2;
#else // circle
				a=M2, b=0, c=M2;
#endif
			s<<kln.pt.x<<"\t"<<kln.pt.y
			<<"\t"<<a<<"\t"<<b<<"\t"<<c;
			if (VL) for (unsigned j=0; j<DscLen; ++j) s<<'\t'<<mDescriptor(i,j);
			s<<endl;
		}
	}
	else
	{
		s<<"size="<<mKeyLines.size();
		if (VL>0) s<<endl<<"KeyLines={"<<mKeyLines<<"}";
		if (VL>1) s<<endl<<"Descriptor="<<mDescriptor;
	}
}

static atomic<REALNUM> sDistScale(0.0101562); // TODO: param/config
REALNUM ImgDscRSILC::getDistScale(bool OM)const{ return sDistScale; }
void ImgDscRSILC::setDistScale(REALNUM s, bool OM)const{ sDistScale=s; }

REALNUM ImgDscRSILC::dist(const ImgDscBase & a, Matches * pMatches)const
{
	TIMELOG(getType()+"::dist");
	const ImgDscRSILC &external = (const ImgDscRSILC &) a;
	const Descriptor
		&dsc1 = mDescriptor,
		&dsc2 = external.mDescriptor;
	ParallelErrorStream PES;
	auto LeftDist = [&pMatches, &PES, this](const Descriptor & dsc1, const Descriptor & dsc2)
	{
		const unsigned
			Len1 = dsc1.rows,
			Len2 = dsc2.rows;
		REALNUM TotalDiff=0;
		if (pMatches) pMatches->resize(Len1);
		{ TIMELOG(getType()+"::dist::forLen1");
	#pragma omp parallel for shared (TotalDiff)
		for(int i=0; i<Len1; ++i)
			try
			{
				const DescriptorRow &dsc1row = dsc1.row(i);
				REALNUM minDiff=maxREALNUM, d=maxREALNUM; // NOTE: omp for loop does not appear to be effective on win7
				int minNdx=-1;
				{ TIMELOG(getType()+"::dist::forLen2");
				for(int k=0; k<Len2 && minDiff>0; ++k)
				{
					d=diff(dsc1row, dsc2.row(k));
					if (d>minDiff) continue;
					minDiff = d;
					minNdx = k;
				}}
				if (pMatches) pMatches->at(i)=DMatch(i, minNdx, minDiff);
			#pragma omp critical
				TotalDiff += minDiff;
			}
			PCATCH(PES, format("dsc1 %d", i))
		}
		PES.report("parallel ImgDscRSILC::dist::LeftDist errors");
		return TotalDiff/Len1; // average distance is the similarity of Dist_I1 and Dist_I2.
	};
	REALNUM d12=0,d21=0;
#pragma omp parallel sections
	{
	#pragma omp section
		try	{ d12=LeftDist(dsc1, dsc2); }
		PCATCH(PES, "RSILC dist12 failed")
	#pragma omp section
		try { d21=LeftDist(dsc2, dsc1); }
		PCATCH(PES, "RSILC dist21 failed")
	}
	PES.report("parallel ImgDscRSILC::dist errors");
	REALNUM d = (d12+d21)/2;
	return R2Unit(getDistScale()*d);
}

static const int SecCnt=16;  // TODO: config/param

void ImgDscRSILC::init(const Image & src, unsigned dim)
{
	TIMELOG(getType()+"::init");

	// normalize the source image
	const Image && img = dim ? Image(src, dim) : src;
	const Mat & Img = img.mx();

	Mat Img_YCrCb;
	cvtColor(Img, Img_YCrCb, CV_BGR2YCrCb);

	// Split the image into channels:
	// Intensity values at Y, Cb, Cr channels, gradient magnitude and gradient angles are used as feature for region histograms.
	vector<Mat> Channels(3);
	split(Img_YCrCb, Channels); // channels follow BGR order in OpenCV

	MatInt // making them integer to avoid strange artifacts during histogram computation because of setting some values to -1; TODO: do we need that -1?
		Img_Y = Channels[0],
		Img_Cr = Channels[1],
		Img_Cb = Channels[2];

	Mat Img_Gray = Mat_<byte>(Img_Y);  // TODO: can we use only Img_Y (without conversions) for further computations?

	ApplyLineFilters(Img_Gray);

	const int N = mKeyLines.size();
	if(!N) throw Exception("ImgDscRSILC found no key-lines");

	// Gradient magnitude and gradient angles are used as features for region histograms.
	MatReal Grad_Mag(Img_Y.size()), Grad_Angle(Img_Y.size());
	ComputeGradient(Img_Gray, Grad_Mag, Grad_Angle);

	vector<VectorReal> ObjDsc(N);
	ParallelErrorStream PES;
	atomic<unsigned> M(0);
#pragma omp parallel for shared (PES, Img, Img_Y, Img_Cb, Img_Cr, Grad_Mag, Grad_Angle)
	for(int i=0; i<N; ++i)
		try
		{
			const KeyLine & rLine = mKeyLines[i];
			Mat crop_Img_Y, crop_Img_Cb, crop_Img_Cr, crop_Grad_Mag, crop_Grad_Angle, crop_pixel_angle; // crop the region of interest
			cropRoI(rLine, Img_Y, Img_Cb, Img_Cr, Grad_Mag, Grad_Angle, crop_Img_Y, crop_Img_Cb, crop_Img_Cr, crop_Grad_Mag, crop_Grad_Angle, crop_pixel_angle);

			MatInt SectionMatrix; // part numbers of the whole image: this array is necessary for the spatial information
			CreateSectionMatrix(SectionMatrix, Img, rLine, SecCnt);
			ComputeHistogramROI(ObjDsc[i], crop_Img_Y, crop_Img_Cb, crop_Img_Cr, crop_Grad_Angle, crop_Grad_Mag, crop_pixel_angle, SectionMatrix, rLine, SecCnt);
			M=max<unsigned>(M, ObjDsc[i].size());
		}
		PCATCH(PES, format("ComputeHistogramROI %d problem", i))
	PES.report("parallel ComputeHistogramROI errors");
	mDescriptor = getMatrix(ObjDsc);
	if (getVisVerbLevel()>1) drawKeyLines(src); // TODO: display at the same scale
}
void ImgDscRSILC::drawKeyLines(Image img)
{
	vector<KeyPoint> kpts=mKeyLines; // inverse key-point angles for proper display
	for (auto & kp: kpts) kp.angle=-kp.angle;
	Mat dst; drawKeypoints(img.mx(), kpts, dst, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	static const string title="kLines";
	imOut(title, dst);
	if (waitKey()==ESC) throw ESC;
	destroyWindow(title);
}
ImgDscRSILC::ImgDscRSILC(const FileNode & fn): TBase(fn), mUseSpatialInfo(true)
{
	cv::read(fn["KeyLines"], mKeyLines);
	CHECK(mKeyLines.size(), "no key-lines loaded for "+fn.name());
	cv::read(fn["Descriptor"], mDescriptor);
	CHECK(!mDescriptor.empty(), "no descriptors loaded for "+fn.name());
}
ImgDscRSILC::ImgDscRSILC(const Image & src, unsigned dim, bool bUseSpatialInfo): TBase(), mUseSpatialInfo(bUseSpatialInfo)
{
	init(src, dim);
}
ImgDscRSILC::ImgDscRSILC(const Image * pSrc, unsigned dim, bool bUseSpatialInfo): TBase(), mUseSpatialInfo(bUseSpatialInfo)
{
	if (!pSrc) return;
	init(*pSrc, dim);
}
ImgDscRSILC::ImgDscRSILC(istream & s): TBase(s), mUseSpatialInfo(true)
{
	read(s, mKeyLines);
	read(s, mDescriptor);
	CHECK(!mKeyLines.empty(), "no key-lines loaded for RSILC");
	CHECK(!mDescriptor.empty(), "no descriptors loaded for RSILC");
}
bool ImgDscRSILC::empty()const
{
	return mDescriptor.empty();
}
const Mat ImgDscRSILC::getVectors()const
{
	return mDescriptor;
}
void ImgDscRSILC::write(ostream & s)const
{
	ImgDscBase::write(s);
	cv::write(s, mKeyLines);
	cv::write(s, mDescriptor);
}
void ImgDscRSILC::write(FileStorage& fs)const
{
	TBase::write(fs);
	cv::write(fs, "KeyLines", mKeyLines);
	cv::write(fs, "Descriptor", getVectors());
}

} // namespace FaceMatch
