
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
#include "ObjectDetectorANN.h"
namespace FaceMatch
{
	REALNUM ObjectDetector::Classify(const Mat Input, const unsigned OriginalSize, const unsigned EncodedSize) const
	{
		MatReal RESULT(1, EncodedSize);
		// normalization
		MatReal INPUT = Input.clone().reshape(0, 1) / 0xFF;
		mEncoder.predict(INPUT, RESULT);
		MatReal ENCODED = RESULT.clone();
		RESULT.create(1, 2);
		mClassifier.predict(ENCODED, RESULT);
		REALNUM d = RESULT[0][0] + RESULT[0][1];
		if (d == 0) return 0;
		return RESULT[0][0] < RESULT[0][1] ?
			RESULT[0][1] / d : (1 - RESULT[0][0] / d);
	}
	REALNUM ObjectDetectorANN::detect(const Mat & patch) const
	{
		Size dims = patch.size();
		if (dims.area() == 0)
			return 0;
		if (dims.width < minDim || dims.height < minDim)
			return 0;
		if (dims.width < dims.height * aspect
			|| dims.height < dims.width * aspect)
			return 0;
		Mat np = ::GrayScale(patch);
		resize(np, np, Size(ImgDim, ImgDim));
		MatReal rimg = np; // convert to real
		return mOD.Classify(rimg, InputSize, EncodedSize);
	}
} // namespace FaceMatch