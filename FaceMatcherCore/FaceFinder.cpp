/*
Informational Notice:

This software was developed under contract funded by the National Library of Medicine, which is part of the National Institutes of Health, an agency of the Department of Health and Human Services, United States Government.

The license of this software is an open-source BSD license.  It allows use in both commercial and non-commercial products.

The license does not supersede any applicable United States law.

The license does not indemnify you from any claims brought by third parties whose proprietary rights may be infringed by your usage of this software.

Government usage rights for this software are established by Federal law, which includes, but may not be limited to, Federal Acquisition Regulation (FAR) 48 C.F.R. Part52.227-14, Rights in Data—General.
The license for this software is intended to be expansive, rather than restrictive, in encouraging the use of this software in both commercial and non-commercial products.

LICENSE:

Government Usage Rights Notice:  The U.S. Government retains unlimited, royalty-free usage rights to this software, but not ownership, as provided by Federal law.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above Government Usage Rights Notice, this list of conditions and the following disclaimer.

•	Redistributions in binary form must reproduce the above Government Usage Rights Notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

•	The names,trademarks, and service marks of the National Library of Medicine, the National Cancer Institute, the National Institutes of Health, and the names of any of the software developers shall not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE U.S. GOVERNMENT AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITEDTO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE U.S. GOVERNMENT
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// FaceFinder.cpp : Implementation of CFaceFinder

#include "stdafx.h"
#include "FaceFinder.h"
#include "FaceMatcherCore.h"
#include "common.h"

extern class CFaceMatcherCoreModule _AtlModule;


// CFaceFinder

STDMETHODIMP CFaceFinder::GetFaces(BSTR path, int options, BSTR* faces, BSTR* ErrorString, LONG* time, int useGPU, int performanceOption)
{
	string imgFNLine;

	HRESULT hr = S_OK;
	*time = 0;

	TRY
	{
		HANDLE h = CreateFileA(RBS(path).data(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE){
			*ErrorString = _bstr_t("Cannot open file").Detach();
			return S_OK;
		}
		CloseHandle(h);
		h = INVALID_HANDLE_VALUE;

		::SetCurrentDirectoryA("C:\\FaceMatchSL\\Facematch\\bin\\");

		//both the FRD's not available
		if (_AtlModule.frdFF_GPU_ANN.get() == NULL && _AtlModule.frdFF_CPU_ANN.get() == NULL)
		{
			*time = 0;
			*ErrorString = _bstr_t("unable to localize as both FRD's are not available. Reason: creating the FRD using the CPU failed and creating the FRD using the GPU failed during initialization").Detach();
			return hr;
		}

		//GPU aided FRD not available
		if (useGPU == 1 && _AtlModule.frdFF_GPU_ANN.get() == NULL)
		{
			*time = 0;
			*ErrorString = _bstr_t("GPU aided FRD is not available. Reason: creating the FRD with GPU failed during initialization").Detach();
			return hr;
		}

		//CPU based FRD not available
		if (useGPU == 0 && _AtlModule.frdFF_CPU_ANN.get() == NULL)
		{
			*time = 0;
			*ErrorString = _bstr_t("CPU aided FRD is not available. Reason: creating the FRD with CPU failed during initialization").Detach();
			return hr;
		}

		//fast = selective | HistEQ | cascade | keepCascaded; SkinColorMapperKind = "Hist"; skinColorParmFN = "Lab.all.xyz"; // assume up-right faces/profiles, detect landmarks, use histogram based skin mapper
		//middle = fast | rotation | seekLandmarks; SkinColorMapperKind = "Hist"; skinColorParmFN = "Lab.all.xyz"; // 90-degree rotation robust faces/profiles, detect landmarks in skin blobs, use histogram based skin mapper
		//accurate = tradeoff | rotationMultiway | seekLandmarksColor; SkinColorMapperKind = "ANN"; skinColorParmFN = "NET_PL_9_15_50_90.18.ann.yml"; // 30-degree rotation robust faces/profiles, detect color-aware landmarks in skin blobs, use ANN based skin mapper

		//detect faces
		unsigned const int defaultFaceFinderOptions = FaceMatch::FaceFinder::selective | FaceMatch::FaceFinder::HistEQ | FaceMatch::FaceFinder::rotation;

		unsigned int flags = defaultFaceFinderOptions;

		string logMessage;

		//favor speed
		if (performanceOption == 0)
		{
			flags = FaceMatch::FaceFinder::selective | FaceMatch::FaceFinder::HistEQ | FaceMatch::FaceFinder::cascade | FaceMatch::FaceFinder::keepCascaded;

			logMessage += NEWLINE;
			logMessage += "FaceFinder called on : " + RBS(path);
			logMessage += NEWLINE;
			logMessage += "FaceFinder flags are : FaceMatch::FaceFinder::selective | HistEQ | cascade | keepCascaded";
			logMessage += NEWLINE;
			logMessage += "performance setting : favorSpeed ";
			logMessage += "useGPU value : " + (useGPU == 1) ? " 1 " : " 0 ";
		}
		//favor optimum (checks for rotation)
		else if (performanceOption == 1)
		{
			flags = FaceMatch::FaceFinder::selective | FaceMatch::FaceFinder::HistEQ | FaceMatch::FaceFinder::cascade | FaceMatch::FaceFinder::keepCascaded
				| FaceMatch::FaceFinder::rotation | FaceMatch::FaceFinder::seekLandmarks;

			logMessage += NEWLINE;
			logMessage += "FaceFinder called on : " + RBS(path);
			logMessage += NEWLINE;
			logMessage += "FaceFinder flags are : FaceMatch::FaceFinder::selective | HistEQ | cascade | keepCascaded | rotation | seekLandmarks";
			logMessage += NEWLINE;
			logMessage += "performance setting : optimal ";
			logMessage += "useGPU value : " + (useGPU == 1) ? " 1 " : " 0 ";
		}
		//favor accuracy
		else if (performanceOption == 2)
		{
			flags = FaceMatch::FaceFinder::selective | FaceMatch::FaceFinder::HistEQ | FaceMatch::FaceFinder::cascade | FaceMatch::FaceFinder::keepCascaded
				| FaceMatch::FaceFinder::rotation | FaceMatch::FaceFinder::seekLandmarks
				| FaceMatch::FaceFinder::rotationMultiway | FaceMatch::FaceFinder::seekLandmarksColor;

			logMessage += NEWLINE;
			logMessage += "FaceFinder called on : " + RBS(path);
			logMessage += NEWLINE;
			logMessage += "FaceFinder flags are : FaceMatch::FaceFinder::selective | HistEQ | cascade | keepCascaded | rotation | seekLandmarks | rotationMultiway | seekLandmarksColor";
			logMessage += NEWLINE;
			logMessage += "performance setting : favorAccuracy ";
			logMessage += "useGPU value : " + (useGPU == 1) ? " 1 " : " 0 ";
		}
			
		CTimer t;
		t.Start();

		FaceMatch::FaceFinder ff(_AtlModule.getFRD(useGPU, performanceOption, logMessage), string(), RBS(path), flags);

		logMessage += NEWLINE;

		////default: all faces returned
		if(ff.gotFaces())
		{
			const FaceMatch::FaceRegions& fr(ff.getFaces());
			if(fr.size() > 0)
			{
				ostringstream oss;
				oss << fr;
				if(oss.str().length())
				{
					_bstr_t x(oss.str().c_str());
					*faces = x.Detach();

					logMessage += "Faces found: ff.gotFaces() == true AND oss << ff.getFaces() values are : ";
					logMessage += oss.str().c_str();
				}
				else
				{
					logMessage += "Faces found: ff.gotFaces() == true AND oss << ff.getFaces() has no faces";
				}
			}
			else
			{
				logMessage += "Faces found: ff.gotFaces() == true AND ff.getFaces().size() <= 0";
			}
		}
		else
		{
			logMessage += "No faces found: ff.gotFaces() == false";
		}

		_AtlModule.Log(logMessage);

		//performance : one face only
		//if(ff.hasFaces())
		//{
		//	const FaceMatch::FaceRegions& fr(ff.getFaces());
		//	ostringstream oss;
		//	if(fr.size() > 0)
		//		oss << *fr[0];
		//	if(oss.str().length())
		//	{
		//		_bstr_t x(oss.str().c_str());
		//		*faces = x.Detach();
		//	}
		//}

		*time = t.ElapsedMilliSeconds();

		//no error
		*ErrorString = _bstr_t("SUCCESS").Detach();
	}
	CATCH
	
	return hr;
}



STDMETHODIMP CFaceFinder::usingGPU(BSTR* gpuStatus)
{
	FaceRegionDetector* frd = _AtlModule.frdFF_GPU_ANN.get();

	if(frd != NULL)
	{
		if(frd->usingGPU())
			*gpuStatus = _bstr_t("SUCCESS").Detach();
		else
			*gpuStatus = _bstr_t("unable to utilize GPU for face localization").Detach();
	}
	else
	{
			*gpuStatus = _bstr_t("GPU aided FRD is not available. Reason: creating the FRD with GPU failed during initialization").Detach();
	}

	return S_OK;
}

