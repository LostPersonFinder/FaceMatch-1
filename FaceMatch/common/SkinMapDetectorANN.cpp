
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
#include "SkinMapDetectorANN.h"

#include <vector>
#include <sstream>

using namespace cv::gpu;

namespace FaceMatch
{

SkinMapDetectorANN::SkinMapDetectorANN(const string & ModelFN, bool gpu): MLP(gpu)
{
	colorConversionPack.push_back(CV_BGR2RGB);
	colorConversionPack.push_back(CV_BGR2HSV);
	colorConversionPack.push_back(CV_BGR2Lab);
	load(ModelFN);
}

float SkinMapDetectorANN::extractResult(float a, float b) const
{
	float sum = a + b;
	a /= sum;
	b /= sum;
	if (a > b) // TODO: check if this function picks the same value regardless of a>b
		return 1 - a;
	else
		return b;
}

GpuMat SkinMapDetectorANN::extractResult(cv::gpu::GpuMat m) const {
	//normalize output
	GpuMat sum(m.rows, 1, CV_32F);
	GpuMat col0 = m.col(0);
	GpuMat col1 = m.col(1);

	add(col0, col1, sum);
	divide(col0, sum, col0);
	divide(col1, sum, col1);
	multiply(col0, -1, col0);
	add(col0, 1, col0);

	//extract output
	max(col0, col1, sum);

	return sum;

}

//-------------------------------------------------------
// get the likelihood 0 [non skin] 1[skin]
//-------------------------------------------------------
REALNUM SkinMapDetectorANN::getSkinLikelihood(const Vec3b pixil) const {
//clog << "Skin Mapping pixil" << endl;
	MatReal inputSignal = Vec2InputSignal(pixil);
	MatReal ret(1, 2);
	predict(inputSignal, ret);
	return extractResult(ret(0, 0), ret(0, 1));
}

MatReal SkinMapDetectorANN::getSkinLikelihoodMap(const Image3b src) const
{
	if (gpu && getGPUCount()) return getSkinLikelihoodMapGPU(src);

	MatReal inputSignal = Image2InputSignal(src);
	MatReal temp(src.rows * src.cols, 2);
	MatReal colorMap(src.rows, src.cols);

	predict(inputSignal, temp);

// normalize output, and build output image
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			int offset = i * src.cols + j;
			colorMap(i, j) = extractResult(temp(offset, 0), temp(offset, 1));
		}
	return colorMap;
}

// run the image through the network on the gpu
MatReal SkinMapDetectorANN::getSkinLikelihoodMapGPU(const Image3b src) const
{
	CHECK(getGPUCount(), "no GPUs found");
	MatReal colorMap; // the output, a map of skin likelihood of the image

	// vectorize the image
	Mat imageCopy = src.reshape(0, src.rows * src.cols);

	// output to be processed into the colorMap
	MatReal nomredOutput(src.rows * src.cols, 1);

	// number of rows to process at one time
	int BATCH = 8192 * 2 * 2 * 2 * 2 * 2;
	if (BATCH > src.rows * src.cols)
		BATCH = src.rows * src.cols;

	//start of the section of rows to process
	int offsetMajor;

	//code to run on GPU, lock GPU here.
	{
		GPULocker lock;

		//the weights
		vector<GpuMat> weightsGpu = loadGpuWeights();

		//process the rows in BATCH sized chunks
		for (offsetMajor = 0; offsetMajor < imageCopy.rows - BATCH;
				offsetMajor += BATCH)
			processChunk(offsetMajor, offsetMajor + BATCH, imageCopy,
					nomredOutput, weightsGpu);

		//process the remaining rows the loop may have missed
		if (offsetMajor != imageCopy.rows)
			processChunk(offsetMajor, imageCopy.rows, imageCopy, nomredOutput,
					weightsGpu);
	} //release GPU Lock

	colorMap = nomredOutput.reshape(0, src.rows);

	return colorMap;
}

//run rows i-j of the image input signals through the network.
void SkinMapDetectorANN::processChunk(int i, int j, Mat in, MatReal out,
		vector<GpuMat> w) const {
	Mat subImage = in.rowRange(i, j);
	Mat outChunk = out.rowRange(i, j);

	GpuMat subImageGpu(subImage);
	GpuMat inputSignal = Image2InputSignal(subImageGpu);

	GpuMat gpuOutput(j - i, 3, CV_32F);

	predict(inputSignal, gpuOutput, w);

	GpuMat nomred = extractResult(gpuOutput);

	// download output and copy it to the normalized output
	nomred.download(outChunk);
	outChunk.copyTo(out);

}

///////////////////////////////
/// IMAGE CONVERSION METHODS///
///////////////////////////////

// image conversion on gpu
GpuMat SkinMapDetectorANN::Image2InputSignal(const cv::gpu::GpuMat image) const
{
	FTIMELOG

	if (image.type() != CV_8UC3)
		throw FaceMatch::Exception(
				"Expecting an 8UC3 input image, but didn't get one");

	// buffer to hold converted image
	GpuMat converted(image.rows, 1, CV_8UC3);
	// the output
	GpuMat out(image.rows, 10, CV_8UC1);
	// pointer to specific columns in the output
	GpuMat tempCols;
	// start range of columns to look at
	int i = 0;

	//convert image to signal
	for (int conversion : colorConversionPack) {

		//convert the image
		cvtColor(image, converted, conversion);

		//copy the converted image to the output
		converted = converted.reshape(1);
		tempCols = out.colRange(i, i + 3);
		converted.copyTo(tempCols);
		converted = converted.reshape(3);
		//move to the next range
		i += 3;
	}

	// convert the Unsigned Chars to Floats
	out.convertTo(out, CV_32F);

	//normalize
	divide(out, 0xFF, out);

	// the bias col is always 1
	tempCols = out.col(9);
	tempCols.setTo(1);
	return out;

}

// image conversion on cpu
MatReal SkinMapDetectorANN::Image2InputSignal(const Image3b image) const
{
	FTIMELOG
	//input better be unsigned character Mat
	if (image.type() != CV_8UC3)
		throw FaceMatch::Exception(
				"Expecting an 8UC3 input image, but didn't get one");

	// the input vectorized
	Mat imageVector = image.reshape(0, image.rows * image.cols);
	// buffer holding the image converted
	Mat converted(imageVector.rows, 1, CV_8UC3);
	// the output, the signal
	Mat out(imageVector.rows, 9, CV_8UC1);
	// pointer to specific columns in the output
	Mat tempCols;

	// start index of columns to point to
	int i = 0;

	//convert image to output signal
	for (int conversion : colorConversionPack) {

		//convert image
		cvtColor(imageVector, converted, conversion);

		//copy into the output
		converted = converted.reshape(1);
		tempCols = out.colRange(i, i + 3);
		converted.copyTo(tempCols);
		converted = converted.reshape(3);

		//move to next section of output
		i += 3;
	}

	// the output is currently unsigned characters, we want floats
	out.convertTo(out, CV_32F);

	//normalize the out to a [0,1] range.
	divide(out, 0xFF, out);

	return out;
}

//vector conversion on cpu
MatReal SkinMapDetectorANN::Vec2InputSignal(const Vec3b vin) const {
//clog << "converting pixel to signal" << endl;
	MatReal retValue(1, 9);
	Mat temp(1, 1, CV_8UC3);
	Mat src(1, 1, CV_8UC3);
	src.at<Vec3b>(0, 0) = vin;
	cvtColor(src, temp, CV_BGR2RGB);
	retValue(0, 0) = temp.at<Vec3b>(0)[0];
	retValue(0, 1) = temp.at<Vec3b>(0)[1];
	retValue(0, 2) = temp.at<Vec3b>(0)[2];
	cvtColor(src, temp, CV_BGR2HSV);
	retValue(0, 3) = temp.at<Vec3b>(0)[0];
	retValue(0, 4) = temp.at<Vec3b>(0)[1];
	retValue(0, 5) = temp.at<Vec3b>(0)[2];
	cvtColor(src, temp, CV_BGR2Lab);
	retValue(0, 6) = temp.at<Vec3b>(0)[0];
	retValue(0, 7) = temp.at<Vec3b>(0)[1];
	retValue(0, 8) = temp.at<Vec3b>(0)[2];
	retValue /= 0xFF;
	return retValue;
}

}
// namespace FaceMatch
