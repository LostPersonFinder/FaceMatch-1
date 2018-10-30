
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

#pragma once // 2013-2017 (C) FaceMatch@NIH.gov

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ml.h>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/gpu/gpumat.hpp>

#include "common.h"
#include "GPU_supp.h"

using namespace cv::gpu;

/**
\brief Multi-layer <a href="https://en.wikipedia.org/wiki/Perceptron">perceptron</a> (MLP)
using the <a href="https://en.wikipedia.org/wiki/Sigmoid_function">Sigmoid</a> activation function.

This derivative of CvANN_MLP overrides the original class's activation function
ignoring the options of the IDENTITY, SIGMOID_SYM, and GUASSIAN
functions provided by the parent class in lieu of the SIGMOID function.
*/
namespace FaceMatch
{
class LIBDCL MLP: public CvANN_MLP
{
	typedef CvANN_MLP TBase;
	float Gain;
protected:
	bool gpu;
	virtual void calc_activ_func(CvMat* xf, const double* bias) const override;
	virtual void calc_activ_func(GpuMat xf) const;
	// we don't scale the input
	virtual void scale_input(const CvMat* _src, CvMat* _dst) const override;
	// we don't scale the output
	virtual void scale_output(const CvMat* _src, CvMat* _dst) const override;
public:
	/// Instantiate.
	MLP(bool g = false /**< use GPU? */);
	/// Load model
	void load(const string & ModelFN /**< input file name */);
	virtual void calc_activ_func_deriv(CvMat* xf, CvMat* deriv, const double* bias) const override;
	virtual void calc_activ_func_deriv(GpuMat xf, GpuMat deriv, const GpuMat bias) const;
	virtual float predict(const MatReal in, MatReal out)const;
	virtual float predict(const GpuMat inputs, GpuMat outputs, vector<GpuMat> weights)const;
	virtual vector<GpuMat> loadGpuWeights() const;
};
} // namespace FaceMatch
