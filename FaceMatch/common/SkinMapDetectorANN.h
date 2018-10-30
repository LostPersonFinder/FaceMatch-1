
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

#pragma once // 2012-2016 (C) FaceMatch@NIH.gov

#define FEATURE_DESCRIPTION_HEADER_SIZE 128

#include "MLP.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <list>

using namespace std;

namespace FaceMatch
{
/// artificial neural net (ANN) model the skin map
class LIBDCL SkinMapDetectorANN: MLP
{
private:
	/**
	 * Get the Skin Color Map of the given image, by processing it through the network on a GPU
	 * @param image the image to process
	 * @return the color map of the image, indicating skin likehood, 0 [non skin] 1[skin]
	 */
	MatReal getSkinLikelihoodMapGPU(const Image3b image) const;
	/**
	 * Convert an image to an vector of input signals for the network (convert all BGR pixels to RGB|HSV|Lab vectors).
	 *	@param image the image to convert
	 *	@return a vector of input signals for the network, each row of the GpuMat is an input
  	 */
	cv::gpu::GpuMat Image2InputSignal(const cv::gpu::GpuMat image) const;
	/**
	 * Convert an image to an vector of input signals for the network (convert all BGR pixels to RGB|HSV|Lab vectors).
	 *	@param image the image to convert
	 *	@return a vector of input signals for the network, each row of the Mat is an input
	 */
	MatReal Image2InputSignal(const Image3b in) const;
	/**
	 * Converts a pixel into an input signal for the network (convert a BGR pixel to an RGB|HSV|Lab vector).
     *	@param vin the vector to convert
     *	@return the vector converted to an input signal for the network
	 */
	MatReal Vec2InputSignal(const Vec3b vin) const;
	/**
	 * The network outputs 2 values for every pixel it evaluates, this normalizes those values, and selects which one indicates the 
	 * the likelihood of a pixel being skin.
     * @param a the first value the network returns for a pixel
	 * @param b the second value the netork returns for a pixel
     * 
 	 */
	float extractResult(float a, float b) const;
	/***
	 * The network outputs 2 values for every pixel it evaluates, this normalizes those, and selects which one indicates the 
	 * the likelihood of a pixel being skin, and stores the results for all pixels in the GpuMat/vector it returns.
	 *	@param mat the output of the network when run on the GPU.
	 *	@return the final restult of the network on the GPU to download to CPU.
     */
	cv::gpu::GpuMat extractResult(cv::gpu::GpuMat mat) const;
	/**
	 *	On the GPU, coompute the skin likelihood of pixels i through j from the original image
	 *	@param i the ith pixel to start evaluation at
	 *	@param j the ith pixel to end evaluation at
	 * 	@param in the image to evaluate vectorized (ie: pixels i to j are rows i to j of this)
	 *	@param out where the output of the network is stored, 
	 *	@param w the weights of the network uploaded to the GPU
	 */
	void processChunk(int i, int j, Mat in, MatReal out, vector<cv::gpu::GpuMat> w) const;

	/// Skin Detector converts BRG pixels to a vector of RGB|HSV|Lab values; it tells the network what order to organize the values in.
	vector<int> colorConversionPack;
public:
	SkinMapDetectorANN(const string & ModelFN, bool gpu = false);
	virtual ~SkinMapDetectorANN(){}
	/**
	 * Get the likelihood 0 [non skin] 1[skin].
	 * @param pixel	the BRG pixel value to evaluate
	 * @return a REALNUM int eh range [0, 1] indicating likelihood of being skin.
	 */
	REALNUM getSkinLikelihood(const Vec3b pixel) const;
	/**
	 * Get the likelihood map for an entire image, pixel values of 0 [non skin] 1 [skin]
	 * @param image the BRG image to evaluate
	 * @return a mat of REALNUMs ranging from [0, 1] indicating the likelihood that a pixel from the input was skin
	 */
	MatReal getSkinLikelihoodMap(const Image3b image) const;
	/// @return ture if the network is empty, else false (empty here means the network has no layers)
	bool empty() { return 0 == get_layer_count(); }
};

} // namespace FaceMatch
