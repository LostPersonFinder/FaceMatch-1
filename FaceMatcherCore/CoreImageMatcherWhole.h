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
// CoreImageMatcherWhole.h : Declaration of the CCoreImageMatcherWhole

#pragma once
#include "resource.h"       // main symbols



#include "FaceMatcherCore_i.h"

#include "..\Facematch\common\ImageMatcherWhole.h"

using namespace ATL;

using namespace FaceMatch;

// CCoreImageMatcherWhole

class ATL_NO_VTABLE CCoreImageMatcherWhole :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CCoreImageMatcherWhole, &CLSID_CoreImageMatcherWhole>,
	public IDispatchImpl<ICoreImageMatcherWhole, &IID_ICoreImageMatcherWhole, &LIBID_FaceMatcherCoreLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CCoreImageMatcherWhole()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COREIMAGEMATCHERWHOLE)

DECLARE_NOT_AGGREGATABLE(CCoreImageMatcherWhole)

BEGIN_COM_MAP(CCoreImageMatcherWhole)
	COM_INTERFACE_ENTRY(ICoreImageMatcherWhole)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		char buffer[1024] = {0};
		sprintf_s(buffer, sizeof(buffer), "CCoreImageMatcherWholeMutex0x%p", this);
		hMutexWI = ::CreateMutexA(NULL, FALSE, buffer);
		return S_OK;
	}

	void FinalRelease()
	{
		if(hMutexWI != NULL){
			CloseHandle(hMutexWI);
			hMutexWI = NULL;
		}
	}

public:

	STDMETHOD(initialize)(BSTR regKey, int eventID, BSTR* overflowListing, int BUCKET_SIZE, BSTR* ErrorString, LONG* time, int useGPU, int performanceValue);
	STDMETHOD(ingest)(BSTR regKey, int eventID, BSTR Img, BSTR IDRegs, BSTR* ErrorString, LONG* time, BSTR debugInfo, int useGPU, int performanceValue);
	STDMETHOD(query)(BSTR regKey, int eventID, BSTR* result, BSTR imgFNRegs, FLOAT tolerance, BSTR* ErrorString, LONG* time, BSTR debugInfo, ULONG* matches);
	STDMETHOD(save)(BSTR regKey, int eventID, BSTR* ErrorString, LONG* time);
	STDMETHOD(remove)(BSTR regKey, int eventID, BSTR ID, BSTR* ErrorString, LONG* time);


public:
	HANDLE hMutexWI; 
};

OBJECT_ENTRY_AUTO(__uuidof(CoreImageMatcherWhole), CCoreImageMatcherWhole)
