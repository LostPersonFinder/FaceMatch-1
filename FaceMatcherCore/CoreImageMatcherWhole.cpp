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
// CoreImageMatcherWhole.cpp : Implementation of CCoreImageMatcherWhole

#include "stdafx.h"
#include "CoreImageMatcherWhole.h"
#include "FaceMatcherCore.h"
#include "common.h"

extern CFaceMatcherCoreModule _AtlModule;

// CCoreImageMatcherWhole

STDMETHODIMP CCoreImageMatcherWhole::initialize(BSTR regKey, int eventID, BSTR* OverflowListing, int BUCKET_SIZE, BSTR* ErrorString, LONG* time, int useGPU, int performanceValue)
{
	CMutexLock lock(hMutexWI);

	//initialize with index file
	HRESULT hr = S_OK;
	
	*time = 0;

	try
	{
		string bucketListing;

		//create WIM for event
		CTimer t;
		t.Start();
		if(_AtlModule.CreateObjectsForEventID(RBS(regKey), eventID, useGPU, performanceValue, bucketListing, BUCKET_SIZE) == FALSE)
			return S_FALSE;
		*time = t.ElapsedMilliSeconds();
		*OverflowListing = _bstr_t(bucketListing.c_str()).Detach();
	}
	catch(const exception & e)
	{
		*ErrorString = _bstr_t(e.what()).Detach();
		hr = S_FALSE;
	}
	catch(...)
	{
		*ErrorString = _bstr_t("unknown exception").Detach();
		hr = S_FALSE;
	}
	
	return hr;
}

STDMETHODIMP CCoreImageMatcherWhole::ingest(BSTR regKey, int eventID, BSTR Img, BSTR IDRegs, BSTR* ErrorString, LONG* time, BSTR debugInfo, int useGPU, int performanceValue)
{
	HRESULT hr = S_OK;
	
	*time = 0;
	
	TRY
	{
		ImgMchWhole* pimWI = _AtlModule.GetWholeImageMatcherForEventID(RBS(regKey), eventID);
		if(pimWI == NULL)
		{
			//these are not passed back to the server as there is no consensus yet on de-deup implementation
			string newOverflowListing;
			const int DEFAULT_BUCKET_SIZE = 1000;
			
			//create WIM for event
			if(_AtlModule.CreateObjectsForEventID(RBS(regKey), eventID, useGPU, performanceValue, newOverflowListing, DEFAULT_BUCKET_SIZE) == FALSE)
				return S_FALSE;
		
			//fetch object
			pimWI = _AtlModule.GetWholeImageMatcherForEventID(RBS(regKey), eventID);
		}

		if(pimWI != NULL)
		{
			//ingest
			CTimer t;
			t.Start();
			pimWI->ingest(RBS(Img), RBS(IDRegs));
			*time = t.ElapsedMilliSeconds();

#ifndef DEBUG_PARAMS
			//no error
			*ErrorString = _bstr_t("SUCCESS").Detach();
#else
			_bstr_t str;
			str = "ingested:\nfile: ";
			str += Img;
			str += "\nkey:\n";
			str += IDRegs;
			*ErrorString = str.Detach();
#endif
		}
		else
		{
			*ErrorString = _bstr_t("Not ready").Detach();
		}
	}
	CATCHDBG

	return hr;
}


STDMETHODIMP CCoreImageMatcherWhole::query(BSTR regKey, int eventID, BSTR* result, BSTR imgFNRegs, FLOAT tolerance,  BSTR* ErrorString, LONG* time, BSTR debugInfo, ULONG* matches)
{
	HRESULT hr = S_OK;
	
	//no results by default
	*result = _bstr_t("").Detach();
	*matches = 0;
	*time = 0;

	TRY
	{
		ImgMchWhole* pimWI = _AtlModule.GetWholeImageMatcherForEventID(RBS(regKey), eventID);

		if(pimWI != NULL)
		{
			string results;

			//query
			CTimer t;
			t.Start();
			*matches = pimWI->query(results, RBS(imgFNRegs), tolerance);
			*time = t.ElapsedMilliSeconds();

			_bstr_t val = "";
			if(*matches > 0)
				val = results.data();

			//return hits
			*result = val.Detach();

			//no error
			*ErrorString = _bstr_t("SUCCESS").Detach();
		}
		else
		{
			*ErrorString = _bstr_t("Not ready").Detach();
		}
	}
	CATCHDBG

	return hr;
}

STDMETHODIMP CCoreImageMatcherWhole::save(BSTR regKey, int eventID, BSTR* ErrorString, LONG* time)
{
	CMutexLock lock(hMutexWI);

	HRESULT hr = S_OK;
	
	*time = 0;

	TRY
	{
		//write
		ImgMchWhole* pimWI = _AtlModule.GetWholeImageMatcherForEventID(RBS(regKey), eventID);
		if(pimWI != NULL)
		{
			string appKey =  RBS(regKey);

			//create file name for whole image matcher
			const unsigned bufSize = 4096;
			auto_ptr<char> buf(new char[bufSize]);
			string fileFormat = "%s\\App.%s.eventid.%d.WI.HAAR.ndx";
			sprintf_s(buf.get(), bufSize, fileFormat.c_str(), _AtlModule.GetRootDirectory().c_str(), appKey.c_str(), eventID);
			string wiName = buf.get();

			CTimer t;
			t.Start();
			pimWI->save(wiName);
			*time = t.ElapsedMilliSeconds();

			//no error
			*ErrorString = _bstr_t("SUCCESS").Detach();
		}
		else
		{
			*ErrorString = _bstr_t("Not ready").Detach();
		}
	}
	CATCH

	return hr;
}

STDMETHODIMP CCoreImageMatcherWhole::remove(BSTR regKey, int eventID, BSTR ID, BSTR* ErrorString, LONG* time)
{
	CMutexLock lock(hMutexWI);

	HRESULT hr = S_OK;

	*time = 0;

	TRY
	{
		//write
		ImgMchWhole* pimWI = _AtlModule.GetWholeImageMatcherForEventID(RBS(regKey), eventID);
		if(pimWI != NULL)
		{
			CTimer t;
			t.Start();
			pimWI->remove(RBS(ID));
			*time = t.ElapsedMilliSeconds();

			//no error
			*ErrorString = _bstr_t("SUCCESS").Detach();
		}
		else
		{
			*ErrorString = _bstr_t("Not ready").Detach();
		}
	}
	CATCH

	return hr;
}

