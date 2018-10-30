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
#include "stdafx.h"
#include "resource.h"
#include "FaceMatcherCore.h"
#include "common.h"


extern CFaceMatcherCoreModule _AtlModule;

ImgMchWhole* CFaceMatcherCoreModule::CreateImageMatcherWholeForEvent(string& IndexFN)
{
	CMutexLock lock(hMutex);
	CQueryLock block(hReady);

	auto_ptr<ImgMchWhole> p;

	//initialize with index file
	HRESULT hr = S_OK;
	try
	{
		//replace default
		::SetCurrentDirectoryA(rootDirectory.c_str());

		HANDLE h = CreateFileA(IndexFN.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
			h = NULL;

			p.reset(new ImgMchWhole(IndexFN));
		}
		else
		{
			p.reset(new ImgMchWhole());

			//save this to disk immediately so the server may have a record of this event
			p->save(IndexFN);
			p.reset(new ImgMchWhole(IndexFN));
		}
	}
	catch(const exception & e)
	{
		//perhaps an event specific logger at some time
		return NULL;
	}
	catch(...)
	{
		return NULL;
	}
	
	return p.release();
}

MatcherFaceRegionsMany* CFaceMatcherCoreModule::CreateImageMatcherFaceRegionsMany(string& IndexFN, FaceRegionDetector& FRD)
{
	CMutexLock lock(hMutex);
	CQueryLock block(hReady);

	auto_ptr<MatcherFaceRegionsMany> p;

	static const unsigned int ffFlags = FaceMatch::FaceFinder::selective|FaceMatch::FaceFinder::HistEQ|FaceMatch::FaceFinder::rotation;
	static const unsigned int imFlags = FaceMatch::EImgDscNdxOptions::dmOM;


	//initialize with index file
	HRESULT hr = S_OK;
	try
	{
		//replace default
		::SetCurrentDirectoryA(rootDirectory.c_str());

		HANDLE h = CreateFileA(IndexFN.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(h != INVALID_HANDLE_VALUE)
		{
			CloseHandle(h);
			h = NULL;

			p.reset(new MatcherFaceRegionsMany(IndexFN, FRD, ffFlags, FaceMatch::DefaultFacePatchDim, imFlags));
		}
		else
		{
			p.reset(new MatcherFaceRegionsMany("", FRD, ffFlags, FaceMatch::DefaultFacePatchDim, imFlags));
			//save this template to disk immediately so the server may have a record of this event
			p->save(IndexFN);
			p.reset(new MatcherFaceRegionsMany(IndexFN, FRD, ffFlags, FaceMatch::DefaultFacePatchDim, imFlags));
		}
	}
	catch(const exception & e)
	{
		//perhaps an event specific logger at some time
		return NULL;
	}
	catch(...)
	{
		return NULL;
	}

	return p.release();
}

