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
#pragma once
#pragma warning(disable : 4503)

//resources
#include "resource.h"

//MIDL specific
#include "FaceMatcherCore_i.h"

#include "common.h"
#include "concurrent_queue.h"

//FaceMatch core 
#include "..\Facematch\common\FaceFinder.h"
#include "..\Facematch\common\ImageMatcherWhole.h"
#include "..\Facematch\common\ImageMatcherFaceRegions.h"
#include "..\Facematch\common\SigIndexMany.h"

//local includes
#include "CoreMatcherFaceRegionsMany.h"

//namespaces
using namespace FaceMatch;
using namespace std;
using namespace concurrency;

//typedefs for face regions
typedef ImageMatcherFaceRegionsBase<SigIndexSIFT> MatcherFaceRegionsSift;
typedef ImageMatcherFaceRegionsBase<SigIndexSURF> MatcherFaceRegionsSurf;
typedef ImageMatcherFaceRegionsBase<SigIndexORB> MatcherFaceRegionsOrb;
typedef ImageMatcherFaceRegionsBase<SigIndexHaarFace> MatcherFaceRegionsHaar;
typedef ImageMatcherFaceRegionsBase<SigIndexManyDist> MatcherFaceRegionsMany;

//typedef for whole image matching
typedef ImageMatcherWhole<SigIndexHaarWhole> ImgMchWhole;

//FaceRegionMatcher[1].FaceRegionMatcher
//a map of overflow buckets within an main bucket
typedef map<int, unique_ptr<MatcherFaceRegionsMany>> OverflowMap;

//EVENT_INFO[bucket].FaceRegionMatcher[1..2..3..4..]
//a map of main buckets within an event
typedef map<string, unique_ptr<OverflowMap>> BucketMap;

//event management
class EVENT_INFO
{
public:
	EVENT_INFO(){}

	//face region detector
	//unique_ptr<FaceRegionDetector> frd;

	//a map of main buckets within an event
	//EVENT_INFO[bucket].FaceRegionMatcher[1..2..3..4..]
	BucketMap bucketMapRegions;

	//whole image matcher
	unique_ptr<ImgMchWhole> pimWI;
};

//EVENT[ID].EVENT_INFO
//maps each EVENT in EVENTS to its data
typedef map<int, unique_ptr<EVENT_INFO>> EventInfoMap;

//APP[KEY].EVENT[ID1...ID2...ID3...]
//maps Application KEYS to ALL its EVENTS - top level
typedef map<string, unique_ptr<EventInfoMap>> AppsToEvents;

class CFaceMatcherCoreModule : public ATL::CAtlExeModuleT< CFaceMatcherCoreModule >
{
public :
	DECLARE_LIBID(LIBID_FaceMatcherCoreLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_FACEMATCHERCORE, "{94988D5D-942F-44F2-A049-BAB41D9BFF8A}")

public:
	CFaceMatcherCoreModule();
	~CFaceMatcherCoreModule();

	//behavior
	BOOL CreateObjectsForEventID(string regKey, int eventID, int useGPU, int performanceValue, string& bucketListing, int BUCKET_SIZE);
	
	ImgMchWhole* GetWholeImageMatcherForEventID(string regKey, int eventID){
		CMutexLock lock(hMutex);
		AppsToEvents::iterator ik = mapEvents.find(regKey);
		if(ik != mapEvents.end())
		{
			EventInfoMap& eim = *(ik->second.get());
			EventInfoMap::iterator eimIt = eim.find(eventID);
			if(eimIt != eim.end())
			{
				EVENT_INFO* eim = eimIt->second.get();
				if(eim != NULL)
					return eim->pimWI.get();
			}

		}
		return NULL;
	}
	
	//EVENT_INFO[bucket].FaceRegionMatcher[1..2..3..4..]
	//typedef map<string, unique_ptr<OverflowMap>> BucketMap;
	//
	//FaceRegionMatcher[1].FaceRegionMatcher
	//typedef map<int, unique_ptr<MatcherFaceRegionsMany>> OverflowMap;

	MatcherFaceRegionsMany* GetFaceRegionMatcherForEventID(string regKey, int eventID, string bucket, int overflowBucket = 1){
		CMutexLock lock(hMutex);
		AppsToEvents::iterator ik = mapEvents.find(regKey);
		if(ik != mapEvents.end())
		{
			EventInfoMap& eim = *(ik->second.get());
			EventInfoMap::iterator eimIt = eim.find(eventID);
			if(eimIt != eim.end())
			{
				EVENT_INFO* eim = eimIt->second.get();
				if(eim != NULL)
				{
					BucketMap::iterator bmIt = eim->bucketMapRegions.find(bucket);
					if(bmIt != eim->bucketMapRegions.end())
					{
						OverflowMap& overflowMap = *(bmIt->second.get());
						OverflowMap::iterator ofmIt = overflowMap.find(overflowBucket);
						if(ofmIt != overflowMap.end())
						{
							return ofmIt->second.get();
						}
					}
				}
			}
		}
		return NULL;
	}

	MatcherFaceRegionsMany* CreateFaceRegionMatcherForEventIDandOverflowBucket(string regKey, int eventID, int useGPU, int performanceValue, string bucket, int overflowBucketNumber = 1){
		CMutexLock lock(hMutex);
		AppsToEvents::iterator ik = mapEvents.find(regKey);
		if(ik != mapEvents.end())
		{
			EventInfoMap& eim = *(ik->second.get());
			EventInfoMap::iterator eimIt = eim.find(eventID);
			if(eimIt != eim.end())
			{
				EVENT_INFO* eim = eimIt->second.get();
				if(eim != NULL)
				{
					BucketMap::iterator bmIt = eim->bucketMapRegions.find(bucket);
					if(bmIt != eim->bucketMapRegions.end())
					{
						OverflowMap& overflowMap = *(bmIt->second.get());
						OverflowMap::iterator ofmIt = overflowMap.find(overflowBucketNumber);
						if(ofmIt == overflowMap.end())
						{
							try
							{
								//make bucket number
								stringstream ss;
								ss << overflowBucketNumber;

								string overflowBucket;
								ss >> overflowBucket;

								//make bucket name
								string bucketNameWithOverflow = bucket + '.' + overflowBucket;

								//make 'many' bucket file name
								string manyForEventID = CCoreMatcherFaceRegionsMany::GetFileNameManyForEvent(regKey, eventID, bucketNameWithOverflow);

								//create the index. if GPU aided FRD is not available, fall back to CPU based FRD.
								string ignore;
								unique_ptr<MatcherFaceRegionsMany> many(CreateImageMatcherFaceRegionsMany(manyForEventID, getFRD(useGPU, performanceValue, ignore)));

								if(many.get() == NULL)
									return NULL;

								//insert the FaceRegionMatcher
								std::pair<OverflowMap::iterator, bool> _pair = overflowMap.insert(OverflowMap::value_type(overflowBucketNumber, std::move(many)));

								return _pair.first->second.get();
							}
							catch(exception ex)
							{
								OutputDebugString(_T("exception occured in CreateFaceRegionMatcherForEventIDandOverflowBucket:"));
								OutputDebugStringA(ex.what());
								return NULL;
							}
							catch(...)
							{
								OutputDebugString(_T("Unknown exception caught in CreateFaceRegionMatcherForEventIDandOverflowBucket"));
								return NULL;
							}
						}
						else
						{
							return ofmIt->second.get();
						}
					}
				}
			}
		}
		return NULL;
	}

	bool ReleaseFaceRegionMatchersForEventID(string regKey, int eventID){
		CMutexLock lock(hMutex);
		AppsToEvents::iterator ik = mapEvents.find(regKey);
		if(ik != mapEvents.end())
		{
			EventInfoMap& eim = *(ik->second.get());
			EventInfoMap::iterator eimIt = eim.find(eventID);
			if(eimIt != eim.end())
			{
				//erase recursively releases all children
				eim.erase(eimIt);
				return true;
			}
		}
		return false;
	}

	FaceMatch::FaceRegionDetector& getFRD(int useGPU, int performanceValue, string& logMessage)
	{
		CMutexLock lock(hMutex);

		logMessage += NEWLINE;

		if (useGPU == 1)
		{
			//neural network based FRD
			if (performanceValue == 2) //multiwayRotation
			{
				if (frdFF_GPU_ANN.get() != NULL)
				{
					logMessage += "FRD used : frdFF_GPU_ANN";

					//if GPU is available, use acceleration
					return *frdFF_GPU_ANN.get();
				}
				else
				{
					logMessage += "FRD used : frdFF_CPU_ANN";

					//no gpu available, fall back to CPU based FRD
					return *frdFF_CPU_ANN.get();
				}
			}

			//histogram based FRD
			else   //no rotation or 90 degree rotation
			{
				if (frdFF_GPU_HIST.get() != NULL)
				{
					logMessage += "FRD used : frdFF_GPU_HIST";

					//if GPU is available, use acceleration
					return *frdFF_GPU_HIST.get();
				}
				else
				{
					logMessage += "FRD used : frdFF_CPU_HIST";

					//no gpu available, fall back to CPU based FRD
					return *frdFF_CPU_HIST.get();
				}
			}
		}
		else
		{
			//neural network based FRD
			if (performanceValue == 2) //multiwayRotation
			{
				logMessage += "FRD used : frdFF_CPU_ANN";

				//CPU : ANN based FRD requested
				return *frdFF_CPU_ANN.get();
			}

			//histogram based FRD
			else   //no rotation or 90 degree rotation
			{
				logMessage += "FRD used : frdFF_CPU_HIST";

				//CPU : HIST based FRD requested
				return *frdFF_CPU_HIST.get();
			}
		}
	}

	bool IsReady(){
		return WaitForSingleObject(hReady, 0) == WAIT_OBJECT_0;
	}

	string GetRootDirectory(void){
		return rootDirectory;
	}

private:

	ImgMchWhole* CreateImageMatcherWholeForEvent(string& IndexFN);
	MatcherFaceRegionsMany* CreateImageMatcherFaceRegionsMany(string& IndexFN, FaceRegionDetector& FRD);
	
	void SignalReady(bool ready){
		ready ? SetEvent(hReady) : ResetEvent(hReady);
	}

	//data
public:

	unique_ptr<FaceRegionDetector> frdFF_GPU_ANN;
	unique_ptr<FaceRegionDetector> frdFF_CPU_ANN;
	unique_ptr<FaceRegionDetector> frdFF_GPU_HIST;
	unique_ptr<FaceRegionDetector> frdFF_CPU_HIST;

	AppsToEvents mapEvents;

	void Log(string message);

	void FlushLog(void);

private:
	HANDLE hMutex; 
	HANDLE hReady;
	string rootDirectory;
	CRITICAL_SECTION csLogger;

	concurrent_queue<string> logMessages;

	void InitializeApp(void);
public:
	HRESULT PreMessageLoop(int nShowCmd);
};

