
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

#include "common.h" // 2013-2017 (C) FaceMatch@NIH.gov
#include "MLP.h"

// TODO: consider a way of adjusting matrix elements other than Mat.at<type>(i,j)[k?].

using namespace std;
using namespace cv;
using namespace cv::gpu;

namespace FaceMatch
{

MLP::MLP(bool GPU)
{
	gpu = GPU;
	Gain = 1;
}

void MLP::load(const string & ModelFN)
{
	TBase::load(ModelFN.c_str(), "mlp");
	CHECK(get_layer_count(), "unable to initalize from the model file " + ModelFN);
}

void MLP::calc_activ_func_deriv(CvMat* xf, CvMat* deriv, const double* bias) const
{
	throw FaceMatch::Exception("Not implemented yet, calc_active_fun_deriv");
	cout << "calc active func derivative" << endl;
	cout << "Finished calc act func derv" << endl;
}

void MLP::calc_activ_func(CvMat* xf, const double* bias) const
{

	//part 1, add in the bias
	int n = xf->rows, m = xf->cols;
	Mat mat = xf;
	for (int i = 0; i < n; i++) {
		double* data = mat.ptr<double>(i);
		for (int j = 0; j < m; j++) {
			data[j] += bias[j];
		}
	}

	// part 2 apply activation function
	// S(x) =1/(1+e^(-tx))
	mat *= -Gain; //Gain;
	exp(mat, mat);
	mat += 1.0;
	pow(mat, -1, mat); //1 / mat;
}

void MLP::scale_input(const CvMat* _src, CvMat* _dst) const
{
	int i, j, cols = _src->cols;
	double* dst = _dst->data.db;
	const double* w = weights[0];
	int step = _src->step;

	if ( CV_MAT_TYPE( _src->type ) == CV_32F) {
		const float* src = _src->data.fl;
		step /= sizeof(src[0]);

		for (i = 0; i < _src->rows; i++, src += step, dst += cols)
			for (j = 0; j < cols; j++)
				dst[j] = src[j];	// * w[j * 2] + w[j * 2 + 1];
	} else {
		const double* src = _src->data.db;
		step /= sizeof(src[0]);

		for (i = 0; i < _src->rows; i++, src += step, dst += cols)
			for (j = 0; j < cols; j++)
				dst[j] = src[j];	// * w[j * 2] + w[j * 2 + 1];
	}

}
//override, we don't scale the output
void MLP::scale_output(const CvMat* _src, CvMat* _dst) const
{
	int i, j, cols = _src->cols;
	const double* src = _src->data.db;
	const double* w = weights[layer_sizes->cols];
	int step = _dst->step;

	if ( CV_MAT_TYPE( _dst->type ) == CV_32F) {
		float* dst = _dst->data.fl;
		step /= sizeof(dst[0]);

		for (i = 0; i < _src->rows; i++, src += cols, dst += step)
			for (j = 0; j < cols; j++) {
				dst[j] = (float) (src[j]);	// * w[j * 2] + w[j * 2 + 1]);
			}
	} else {
		double* dst = _dst->data.db;
		step /= sizeof(dst[0]);

		for (i = 0; i < _src->rows; i++, src += cols, dst += step)
			for (j = 0; j < cols; j++) {
				dst[j] = src[j];	// * w[j * 2] + w[j * 2 + 1];
			}
	}
}

float MLP::predict(const MatReal _inputs, MatReal _outputs) const
{
	return CvANN_MLP::predict(_inputs, _outputs);
}

//////////////////////////////////////////////
///////// GPU OPTIMIZATIONS///////////////////
//////////////////////////////////////////////

float MLP::predict(const GpuMat _inputs, GpuMat _outputs, vector<GpuMat> weightsGPU)const
{
// validate input parameters
	if (!layer_sizes)
		CV_Error(CV_StsError, "The network has not been initialized");
	if (_inputs.rows != _outputs.rows || _inputs.type() != _outputs.type())
		CV_Error(CV_StsBadArg, "Both input and output must be floating-point matrices of the same type and have the same number of rows");
	if (_inputs.cols != layer_sizes->data.i[0] + 1)
		CV_Error(CV_StsBadSize, "input matrix must have the same number of columns as the number of neurons in the input layer");
	if (_outputs.cols != layer_sizes->data.i[layer_sizes->cols - 1] + 1)
		CV_Error(CV_StsBadSize, "output matrix must have the same number of columns as the number of neurons in the output layer");

	int j=0, l_count = layer_sizes->cols, type = CV_32F;

	// load network onto GPU
	// gpu temp variables: the input, output, weights, and bias
	GpuMat layer_in = _inputs, layer_out, _w, _b;
	//temp variables for the weights and inputs as mats to be uploaded to the gpu
	for (int j = 1; j < l_count; j++)
	{
		//prep output mat
		layer_out.create(layer_in.rows, layer_sizes->data.i[j] + 1, type);

		_w = weightsGPU[j - 1];
		//multiply
		cv::gpu::gemm(layer_in, _w, 1, GpuMat(), 0, layer_out);

		//activate
		calc_activ_func(layer_out);

		//make output next input
		layer_out.copyTo(layer_in);
	}

	//copy output to _outputs
	layer_out.copyTo(_outputs);

	return 0.f;
}

void MLP::calc_activ_func(GpuMat out) const
{
	// leave the last column untouched
	GpuMat toEval = out.colRange(0, out.cols - 1);
	//activation function
	cv::gpu::multiply(toEval, -Gain, toEval);
	exp(toEval, toEval);
	add(toEval, 1.0f, toEval);
	pow(toEval, -1.0f, toEval);

}

void MLP::calc_activ_func_deriv(GpuMat xf, GpuMat deriv, const GpuMat bias) const
{
	throw FaceMatch::Exception("Not implemented yet, calc_active_fun_deriv");
	cout << "calc active func derivative" << endl;
	cout << "Finished calc act func derv" << endl;

}

vector<GpuMat> MLP::loadGpuWeights() const
{
	vector<GpuMat> w;
	int j, l_count = layer_sizes->cols, type = CV_32F;
	for (int j = 1; j < l_count; j++) {

		//make Mats of the weights
		int inD = layer_sizes->data.i[j - 1];
		int outD = layer_sizes->data.i[j];

		//weights + bias
		Mat wT(inD + 1, outD, CV_64F, this->weights[j]);
		Mat bT = cv::Mat::zeros(inD + 1, 1, CV_64F);
		bT.at<double>(inD, 0) = 1;
		hconcat(wT, bT, wT);

		GpuMat wG(wT);
		wG.convertTo(wG, type);
		w.push_back(wG);
	}
	return w;
}
}
