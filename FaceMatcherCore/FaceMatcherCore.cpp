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
// FaceMatcherCore.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "FaceMatcherCore.h"
#include "CoreMatcherFaceRegionsMany.h"
#include "common.h"
#include "..\Facematch\common\FaceFinder.h"
#include <algorithm>
#include <memory>
#include <iostream>
#include <sstream>

CFaceMatcherCoreModule _AtlModule;


//application entry
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
								LPTSTR lpCmdLine, int nShowCmd)
{
	static bool oneTimeInit = false;

	if(oneTimeInit == false)
	{
		::SetCurrentDirectoryA("C:\\FacematchSL\\FaceMatch\\bin");
		oneTimeInit = true;
	}

	return _AtlModule.WinMain(nShowCmd);
}

CFaceMatcherCoreModule::CFaceMatcherCoreModule()
{
	InitializeApp();

	static const string FaceModelFN = "haarcascade_frontalface_alt2.xml";
	static const string ProfileModelFN = "haarcascade_profileface.xml";

	//use the new skin model
	static const string SkinColorMapperKind_ANN = "ANN", SkinColorParmFN_NET = "NET_PL_9_15_50_90.18.ann.yml";

	::SetCurrentDirectoryA(rootDirectory.c_str());

	int count = 4;
	while(count-- > 0)
	{
		try
		{
			if(frdFF_GPU_ANN.get() == NULL)
			{
				Log("creating ANN based FRD using GPU");

				frdFF_GPU_ANN.reset(new FaceRegionDetector("FFModels/", 
					FaceModelFN, ProfileModelFN,
					SkinColorMapperKind_ANN, SkinColorParmFN_NET, 16U, 512U, 0.25F, 0.5F, 0.5F, true));
					//SkinColorMapperKind, SkinColorParmFN, 16U, 900U, 0.25F, 0.5F, 0.5F, true));

				Log("creating ANN based FRD using GPU succeeded with no exceptions");
			}
		}
		catch(exception& ex)
		{
			frdFF_GPU_ANN.reset(NULL);

			OutputDebugString(_T("exception:"));
			OutputDebugStringA(ex.what());
			Log("exception : exception caught while instantiating ANN based FRD with GPU, the application failed to initialize correctly.");
			string except = "exception: ";
			except += ex.what();
			Log(except);
		}
		catch(...)
		{
			frdFF_GPU_ANN.reset(NULL);

			OutputDebugString(_T("exception caught in ANN based FRD"));
			Log("unknown exception caught while instantiating ANN based FRD with GPU, the application failed to initialize correctly.");
		}

		try
		{
			if(frdFF_CPU_ANN.get() == NULL)
			{
				Log("creating ANN based FRD using CPU");

				frdFF_CPU_ANN.reset(new FaceRegionDetector("FFModels/", 
					FaceModelFN, ProfileModelFN,
					SkinColorMapperKind_ANN, SkinColorParmFN_NET, 16U, 512U, 0.25F, 0.5F, 0.5F, false));

				Log("creating ANN based FRD using CPU succeeded with no exceptions");
			}
		}
		catch(exception& ex)
		{
			frdFF_CPU_ANN.reset(NULL);

			OutputDebugString(_T("exception:"));
			OutputDebugStringA(ex.what());
			Log("exception : exception caught while instantiating ANN based FRD with CPU, the application failed to initialize correctly.");
			string except = "exception: ";
			except += ex.what();
			Log(except);
		}
		catch(...)
		{
			frdFF_CPU_ANN.reset(NULL);

			OutputDebugString(_T("exception caught in ANN based FRD"));
			Log("unknown exception caught while instantiating ANN based FRD with CPU, the application failed to initialize correctly.");
		}

		//break on success
		if(frdFF_GPU_ANN.get() != NULL && frdFF_CPU_ANN.get() != NULL)
		{
			break;
		}
		else
		{
			Log("entering ANN based FRD instantiation retry after 1 second");
			//sleep 1 second
			Sleep(1000);
		}
	}
	
	static const string SkinColorMapperKind_HIST = "Hist", SkinColorParmFN_Lab = "Lab.all.xyz";

	count = 4;

	while (count-- > 0)
	{
		try
		{
			if (frdFF_GPU_HIST.get() == NULL)
			{
				Log("creating HIST based FRD using GPU");

				frdFF_GPU_HIST.reset(new FaceRegionDetector("FFModels/",
					FaceModelFN, ProfileModelFN,
					SkinColorMapperKind_HIST, SkinColorParmFN_Lab, 16U, 512U, 0.25F, 0.5F, 0.5F, true));
				//SkinColorMapperKind, SkinColorParmFN, 16U, 900U, 0.25F, 0.5F, 0.5F, true));

				Log("creating HIST based FRD using GPU succeeded with no exceptions");
			}
		}
		catch (exception& ex)
		{
			frdFF_GPU_HIST.reset(NULL);

			OutputDebugString(_T("exception:"));
			OutputDebugStringA(ex.what());
			Log("exception : exception caught while instantiating HIST based FRD with GPU, the application failed to initialize correctly.");
			string except = "exception: ";
			except += ex.what();
			Log(except);
		}
		catch (...)
		{
			frdFF_GPU_HIST.reset(NULL);

			OutputDebugString(_T("exception caught in HIST based FRD"));
			Log("unknown exception caught while instantiating HIST based FRD with GPU, the application failed to initialize correctly.");
		}

		try
		{
			if (frdFF_CPU_HIST.get() == NULL)
			{
				Log("creating HIST based FRD using CPU");

				frdFF_CPU_HIST.reset(new FaceRegionDetector("FFModels/",
					FaceModelFN, ProfileModelFN,
					SkinColorMapperKind_HIST, SkinColorParmFN_Lab, 16U, 512U, 0.25F, 0.5F, 0.5F, false));

				Log("creating HIST based FRD using CPU succeeded with no exceptions");
			}
		}
		catch (exception& ex)
		{
			frdFF_CPU_HIST.reset(NULL);

			OutputDebugString(_T("exception:"));
			OutputDebugStringA(ex.what());
			Log("exception : exception caught while instantiating HIST based FRD with CPU, the application failed to initialize correctly.");
			string except = "exception: ";
			except += ex.what();
			Log(except);
		}
		catch (...)
		{
			frdFF_CPU_HIST.reset(NULL);

			OutputDebugString(_T("exception caught in HIST based FRD"));
			Log("unknown exception caught while instantiating HIST based FRD with CPU, the application failed to initialize correctly.");
		}

		//break on success
		if (frdFF_GPU_HIST.get() != NULL && frdFF_CPU_HIST.get() != NULL)
		{
			break;
		}
		else
		{
			Log("entering HIST based FRD instantiation retry after 1 second");
			//sleep 1 second
			Sleep(1000);
		}
	}

	//set zero verbosity level for services
	setVerbLevel(0);
}

CFaceMatcherCoreModule::~CFaceMatcherCoreModule()
{
	//nothing to deallocate as the tree of unique_ptrs will deallocate its contained objects
}

void CFaceMatcherCoreModule::InitializeApp(void)
{
	InitializeCriticalSection(&csLogger);

	rootDirectory = "C:\\FacematchSL\\FaceMatch\\bin";
	hMutex = INVALID_HANDLE_VALUE;
	hReady = INVALID_HANDLE_VALUE;

	char buffer[1024] = { 0 };

	sprintf_s(buffer, sizeof(buffer), "CCoreMatcherApplicationMutex0x%p", this);
	hMutex = ::CreateMutexA(NULL, FALSE, buffer);

	sprintf_s(buffer, sizeof(buffer), "CCoreMatcherApplicationReady0x%p", this);
	hReady = ::CreateEventA(NULL, TRUE, FALSE, buffer);
}

void CFaceMatcherCoreModule::Log(string logMessage)
{
	if (logMessage.length() == 0)
		return;

	time_t tTime = time(NULL);
	char tsBuffer[1024] = { 0 };
	sprintf_s(tsBuffer, sizeof(tsBuffer), "\r\n%s", asctime(localtime(&tTime)));
	tsBuffer[strlen(tsBuffer) - 1] = NULL;

	string message(tsBuffer);
	message += " : ";
	message += logMessage;

	//async logging eanbled, so add to concurrent queue container
	logMessages.push(message);
}

void CFaceMatcherCoreModule::FlushLog(void)
{
	CAutoLock _crit(&csLogger);

	try
	{

		//open file, write protect prevents changes
		string logFileName("C:\\FaceMatchSL\\FaceMatch\\bin\\Logs\\facematchercore.log.txt");

		CAutoHandle hFile = CreateFileA((LPCSTR)logFileName.c_str(),
			GENERIC_READ | GENERIC_WRITE, 0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (hFile.get() == NULL)
			return;

		const __int64 TEN_MB = 10485760;
		const __int64 EIGHT_MB = 8388608;

		//if file exceeds 10 MB, keep last 2 MB
		LARGE_INTEGER dwFileSize = { 0 };
		if (GetFileSizeEx(hFile.get(), &dwFileSize))
		{
			if (dwFileSize.QuadPart > TEN_MB)
			{
				const char* logLines = new char[dwFileSize.QuadPart];
				const char* logBufferEnd = logLines + dwFileSize.QuadPart;

				//read data
				DWORD dwBytesRead = 0;
				char* pBuffer = (char*)logLines;
				DWORD dwBytesToRead = dwFileSize.QuadPart;

				do
				{
					if (ReadFile(hFile.get(), pBuffer, dwBytesToRead, &dwBytesRead, NULL))
					{
						pBuffer += dwBytesRead;
						dwBytesToRead -= dwBytesRead;
						dwBytesRead = 0;
					}

				} while (dwBytesToRead > 0);

				//we know file is > 10 MB, take last 2 MB
				char* truncLog = (char*)logBufferEnd;
				truncLog -= (TEN_MB - EIGHT_MB);

				//move to the next line
				while (truncLog < logBufferEnd && *truncLog != '\n')
					truncLog++;

				//Calculate the number of bytes to write
				DWORD dwBytesToWrite = logBufferEnd - truncLog;
				DWORD dwBytesWritten = 0;
				const char* pByte = truncLog;

				//seek to begin of file, set length to zero
				SetFilePointer(hFile.get(), 0, NULL, FILE_BEGIN);
				SetEndOfFile(hFile.get());

				//write out the most recent 2 MB worth of log lines
				do
				{
					if (WriteFile(hFile.get(), pByte, dwBytesToWrite, &dwBytesWritten, NULL))
					{
						dwBytesToWrite -= dwBytesWritten;
						pByte += dwBytesWritten;
						dwBytesWritten = 0;
					}
				} while (dwBytesToWrite > 0);

				delete[] logLines;
				logLines = NULL;
				logBufferEnd = NULL;
			}
		}

		string message;

		//seek to end of file
		SetFilePointer(hFile.get(), 0, NULL, FILE_END);

		//write out all messages in concurrent queue
		while (!logMessages.empty())
		{
			message.clear();

			//de-queue a message
			if (logMessages.try_pop(message))
			{
				DWORD dwBytesWritten = 0;
				DWORD dwBytes = message.length();
				LPCSTR pByte = message.c_str();

				//write data
				do
				{
					if (WriteFile(hFile.get(), pByte, dwBytes, &dwBytesWritten, NULL))
					{
						dwBytes -= dwBytesWritten;
						pByte += dwBytesWritten;
						dwBytesWritten = 0;
					}

				} while (dwBytes > 0);
			}
			else
			{
				break;
			}
		}
	}
	catch (const exception & e){}
	catch (...){ }
}

//APP[KEY].EVENT[ID1...ID2...ID3...]
//maps Application KEYS to ALL its EVENTS - top level
//typedef map<string, unique_ptr<EventInfoMap>> AppsToEvents;
//
//EVENT[ID].EVENT_INFO
//maps each EVENT in EVENTS to its data
//typedef map<int, unique_ptr<EVENT_INFO>> EventInfoMap;
//
//EVENT_INFO[bucket].FaceRegionMatcher[1..2..3..4..]
//a map of main buckets within an event
//typedef map<string, unique_ptr<OverflowMap>> BucketMap;
//
//FaceRegionMatcher[1].FaceRegionMatcher
//a map of overflow buckets within an main bucket
//typedef map<int, unique_ptr<MatcherFaceRegionsMany>> OverflowMap;

BOOL CFaceMatcherCoreModule::CreateObjectsForEventID(string regKey, int eventID, int useGPU, int performanceValue, string& bucketListing, int BUCKET_SIZE)
{
	CMutexLock lock(hMutex);
	CQueryLock block(hReady);

	if(eventID == -1 || regKey.length() == 0)
		return FALSE;

	//if eventID already exists, nothing to create
	AppsToEvents::iterator ik = mapEvents.find(regKey);
	if(ik != mapEvents.end())
	{
		EventInfoMap& eim = *(ik->second.get());

		EventInfoMap::iterator eimIt = eim.find(eventID);
		if(eimIt != eim.end())
			return TRUE;
	}

	try
	{
		//elements in the gender and age buckets should be distinct in its array
		const string genderBuckets[] = { "Male", "Female", "GenderUnknown" };
		const string ageBuckets[] = { "Youth", "Adult", "AgeUnknown" };

		unique_ptr<EVENT_INFO> eod(new EVENT_INFO());

		//set root before instantiating a dedicated FRD for this event
		::SetCurrentDirectoryA(rootDirectory.c_str());

		string FaceModelFN = "haarcascade_frontalface_alt2.xml";
		string ProfileModelFN = "haarcascade_profileface.xml";

		//use the new skin model
		string SkinColorMapperKind="Hist", SkinColorParmFN="Lab.all.xyz";

		bucketListing.clear();

		//create many.txt with event specific index file names (HAAR, SIFT, SURF, ORB) for MxN buckets
		for(int gender = 0; gender < sizeof(genderBuckets)/sizeof(genderBuckets[0]); gender++)
		{
			for(int age = 0; age < sizeof(ageBuckets)/sizeof(ageBuckets[0]); age++)
			{
				//make bucket name
				string bucket = genderBuckets[gender] + "." + ageBuckets[age];

				unique_ptr<OverflowMap> overflowMap(new OverflowMap());
				
				int overflowBucketNumbering = 1, indexSize = 0;
				string bucketNameWithOverflow;
				
				while(overflowBucketNumbering < 100)
				{
					//make bucket number
					stringstream ss;
					ss << overflowBucketNumbering;

					string overflowBucket;
					ss >> overflowBucket;

					//make bucket name
					bucketNameWithOverflow = bucket + '.' + overflowBucket;

					//make 'many' bucket file name
					string manyForEventID = CCoreMatcherFaceRegionsMany::GetFileNameManyForEvent(regKey, eventID, bucketNameWithOverflow);

					//create the index. if GPU aided FRD not available fall back to CPU 
					string ignore;
					unique_ptr<MatcherFaceRegionsMany> many(CreateImageMatcherFaceRegionsMany(manyForEventID, getFRD(useGPU, performanceValue, ignore)));

					if (many.get() == NULL)
						return FALSE;

					indexSize = many->count();
					
					//insert the FaceRegionMatcher
					overflowMap->insert(OverflowMap::value_type(overflowBucketNumbering, std::move(many)));

					//if there are BUCKET_SIZE records, load the next overflow bucket
					if(indexSize < BUCKET_SIZE)
						break;

					overflowBucketNumbering++;
				}

				bucketListing += bucketNameWithOverflow + "\n";

				//insert
				eod->bucketMapRegions.insert(BucketMap::value_type(bucket, std::move(overflowMap)));
			}
		}

		//create file name for whole image matcher
		const unsigned bufSize = 4096;
		auto_ptr<char> buf(new char[bufSize]);
		string fileFormat = "%s\\App.%s.eventid.%d.WI.HAAR.ndx";
		sprintf_s(buf.get(), bufSize, fileFormat.c_str(), rootDirectory.c_str(), regKey.c_str(), eventID);
		string wiName = buf.get();

		eod->pimWI.reset(CreateImageMatcherWholeForEvent(wiName));
		if(eod->pimWI.get() == NULL)
			return FALSE;

//APP[KEY].EVENT[ID1...ID2...ID3...]
//maps Application KEYS to ALL its EVENTS - top level
//typedef map<string, unique_ptr<EventInfoMap>> AppsToEvents;
//
//EVENT[ID].EVENT_INFO
//maps each EVENT in EVENTS to its data
//typedef map<int, unique_ptr<EVENT_INFO>> EventInfoMap;

		//insert into map
		if(ik == mapEvents.end())
		{
			unique_ptr<EventInfoMap> eventMap(new EventInfoMap());
			eventMap->insert(EventInfoMap::value_type(eventID, std::move(eod)));

			//new app entity, new event id
			mapEvents.insert(AppsToEvents::value_type(regKey, std::move(eventMap)));
		}
		else
		{
			//known app entity, new event id
			EventInfoMap& eim = *(ik->second.get());
			eim.insert(EventInfoMap::value_type(eventID, std::move(eod)));
		}

		SignalReady(true);
	}
	catch(exception ex)
	{
		OutputDebugString(_T("exception occured in CreateObjectsForEventID:"));
		OutputDebugStringA(ex.what());
		return FALSE;
	}
	catch(...)
	{
		OutputDebugString(_T("Unknown exception caught in CreateObjectsForEventID"));
		return FALSE;
	}

	return TRUE;
}

HRESULT CFaceMatcherCoreModule::PreMessageLoop(int nShowCmd)
{
	// TODO: Add your specialized code here and/or call the base class
	return CAtlExeModuleT::PreMessageLoop(nShowCmd);
}
