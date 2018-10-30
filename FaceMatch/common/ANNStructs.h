
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

#pragma once // 2013-2015 (C) FaceMatch@NIH.gov

#define INITIAL_WEIGHT_LOW     -0.3
#define INITIAL_WEIGHT_HIGH     0.3

#define RANDOM_GENERATOR_INIT   12321

#define CONNECTION              1
#define NO_CONNECTION           0

#define BIAS                    1

#define TRAINING                1
#define TESTING                 0

#define ERRORT                   0.001

#define EPOCHS                  1000

#define NETWORK_CREATED         1
#define NETWORK_NOT_CREATED     0

#define INPUT_LAYER_SIZE        9
#define OUTPUT_LAYER_SIZE       2
#define LAYERS_NUMBER           3

#define RANDOMIZE_YES			1
#define RANDOMIZE_NO			0

#define BATCHSIZE				2048

/// represent a layer in the network
typedef struct _Layer
{
	int nUnits;                /**< number of units in the layer*/
	double * Outputs;          /**< the outputs of the neurons from this layer*/
	double * Errors;           /**< the errors of the neurons from this layer*/
	double ** Weights;         /**< the connections weights of the neurons with the next layer*/
	double ** SaveWeights;     /**< the mirror of the last best Weights for stopped training*/
	double ** dWeights;        /**< last weights deltas for momentum*/
	char ** Links;             /**< the connections between the layer and the next layer*/
// fields for GPU
	double *d_output; /**< Outputs stored on GPU */
	double *d_errors; /**< the errors of the neurons from this layer on the GPU */
	double *d_weights; /**< weights stored on GPU */
	double *d_saveweights; /**< the mirror of the last best weights for stopped training on GPU */
	double *d_dweights; /**< last weights deltas for momentum on GPU */
	char *d_link; /**< the connections between the layer and the next layer */
} Layer;

//**********************************************************
// structure to model the MLP
//**********************************************************


/// represent a complete network
typedef struct _Network
{
	int nLayers; /**< number of layers of this network*/
	Layer ** Layers; /**< the layers of the network*/
	Layer * InputLayer; /**< the input layer*/
	Layer * OutputLayer; /**< the output layer*/
	double Alpha; /**< the momentum factor*/
	double Eta; /**< the learning rate*/
	double Gain; /**< the gain of the sigmoid function*/
	double TotalError; /**< the network total error*/
	double *d_totalError; /**< a tmp variable on the GPU used storing the components of the total error*/
	double *d_Targs; /**< a tmp variable on the GPU used for storing target output values of the Network while training*/
} Network;
