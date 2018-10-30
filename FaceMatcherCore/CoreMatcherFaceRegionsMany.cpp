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
// CoreMatcherFaceRegionsMany.cpp : Implementation of CCoreMatcherFaceRegionsMany

#include "stdafx.h"
#include "CoreMatcherFaceRegionsMany.h"
#include "FaceMatcherCore.h"
#include "common.h"

extern class CFaceMatcherCoreModule _AtlModule;

using namespace FaceMatch;

// CCoreMatcherFaceRegionsMany

STDMETHODIMP CCoreMatcherFaceRegionsMany::initialize(BSTR regKey, int eventID, BSTR* OverflowListing, int BUCKET_SIZE, BSTR* ErrorString, LONG* time, int useGPU, int performanceValue)
{
	CMutexLock lock(hMutexFR);

	//initialize with index file
	HRESULT hr = S_OK;

	*time = 0;
	
	try
	{
		string bucketListing;

		//create FRM for event
		CTimer t;
		t.Start();
		if(_AtlModule.CreateObjectsForEventID(RBS(regKey), eventID, useGPU, performanceValue, bucketListing, BUCKET_SIZE) == FALSE)
			return S_FALSE;
		*time = t.ElapsedMilliSeconds();
		*OverflowListing = _bstr_t(bucketListing.c_str()).Detach();
	}
	catch(const exception & e)
	{
		*time = 0;
		*ErrorString = _bstr_t(e.what()).Detach();
		*OverflowListing = _bstr_t("").Detach();
		hr = S_FALSE;
	}
	catch(...)
	{
		*time = 0;
		*ErrorString = _bstr_t("unknown exception").Detach();
		*OverflowListing = _bstr_t("").Detach();
		hr = S_FALSE;
	}
	
	return hr;
}

STDMETHODIMP CCoreMatcherFaceRegionsMany::ingest(BSTR regKey, int eventID, BSTR Img, BSTR IDRegs, BSTR bucket, int currentOverflowLevel,  
												 BSTR* newOverflowValue, int BUCKET_SIZE, BSTR* ErrorString, LONG* time, BSTR debugInfo, int useGPU, int performanceValue)
{
	HRESULT hr = S_OK;

	*time = 0;

	TRY
	{
		string bucketListing;

		MatcherFaceRegionsMany* pimFRmr = _AtlModule.GetFaceRegionMatcherForEventID(RBS(regKey), eventID, RBS(bucket), currentOverflowLevel);
		if(pimFRmr == NULL)
		{
			//if level is 1, this could be a new eventID
			if(currentOverflowLevel == 1)
			{
				//create FaceRegionMatcher for event
				if(_AtlModule.CreateObjectsForEventID(RBS(regKey), eventID, useGPU, performanceValue, bucketListing, BUCKET_SIZE) == FALSE)
					return S_FALSE;

				//it is possible to not have the correct FaceRegionMatcher, if 
				//1. we loaded a known but 'lost' event (possibly containing many overflow indexes for this main bucket)
				//2. we are at BUCKET_SIZE
				pimFRmr = _AtlModule.GetFaceRegionMatcherForEventID(RBS(regKey), eventID, RBS(bucket), currentOverflowLevel);

				if(pimFRmr != NULL)
				{
					//solve 1
					//check for overflow FaceRegionMatchers for main bucket
					MatcherFaceRegionsMany* pFRM = pimFRmr;
					int level = currentOverflowLevel;
					while(pFRM != NULL)
					{
						//not reported as this information is already present in *newOverflowValue below (for new event and at BUCKET_SIZE)
						currentOverflowLevel = level;
						pimFRmr = pFRM;
						pFRM = _AtlModule.GetFaceRegionMatcherForEventID(RBS(regKey), eventID, RBS(bucket), ++level);
					}
				}
				else
				{
					*ErrorString = _bstr_t("Bucket Not ready").Detach();
					return S_FALSE;
				}
			}
		}
			
		//ingest should stay within bucket limits
		if(pimFRmr != NULL && 
			pimFRmr->count() >= BUCKET_SIZE)
		{
			//on overflow, create new bucket for ingest
			pimFRmr = _AtlModule.CreateFaceRegionMatcherForEventIDandOverflowBucket(RBS(regKey), eventID, useGPU, performanceValue, RBS(bucket), currentOverflowLevel + 1);
			if(pimFRmr == NULL)
			{
				*ErrorString = _bstr_t("Bucket Not ready").Detach();
				return S_FALSE;
			}
			else
			{
				//inform web service to update map
				stringstream ss;
				ss <<  (currentOverflowLevel + 1);
				string sOverflowNumber;
				ss >> sOverflowNumber;

				string overflowBucketName;
				overflowBucketName = RBS(bucket) + '.' + sOverflowNumber;

				//pass new overflow bucket value
				bucketListing += overflowBucketName + "\n";
			}
		}

		//copied for new event creation, overflow, and in remote cases for a previously known but 'lost' load
		if(bucketListing.length() > 0)
			*newOverflowValue = _bstr_t(bucketListing.c_str()).Detach();

		if(pimFRmr != NULL)
		{
			//ingest
			CTimer t;
			int numFacesIngested = 0;
			t.Start();
			//numFacesIngested = pimFRmr->ingest(RBS(Img), RBS(IDRegs));
			string ImgPathFNExt(RBS(Img) + "\t" + RBS(IDRegs));
			numFacesIngested = pimFRmr->ingest(ImgPathFNExt);
			*time = t.ElapsedMilliSeconds();

			if (numFacesIngested == 0)
			{
				*ErrorString = _bstr_t("No face detected in input image").Detach();
				return hr;
			}

			//LOGGING for INGEST - BEGIN

			stringstream ss;

			string temp;
			ss.clear();
			if (bucketListing.length() > 0)
				ss << currentOverflowLevel + 1;
			else
				ss << currentOverflowLevel;
			ss >> temp;

			string desc_bucket = "bucket: " + RBS(bucket);
			desc_bucket += "." + temp;

			string sRegKey = RBS(regKey);

			string sEventID;
			ss.clear();
			ss << eventID;
			ss >> sEventID;

			string sObjectID;
			ss.clear();
			ss << static_cast<const void*>(pimFRmr);
			ss >> sObjectID;

			string logMessage;

			logMessage = ("ingest called on : " + desc_bucket + " app : " + sRegKey + " eventid: " + sEventID + " object_address: " + sObjectID);
			logMessage += NEWLINE + ("ingest values    : " + ImgPathFNExt);

			ss.clear();
			ss << numFacesIngested;
			ss >> temp;

			logMessage += NEWLINE + ("ingest return value : " + temp);

			_AtlModule.Log(logMessage);

			//LOGGING for INGEST - END

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


STDMETHODIMP CCoreMatcherFaceRegionsMany::query(BSTR regKey, int eventID, BSTR* result, BSTR imgFNRegs, BSTR bucket, int overflowBucket, FLOAT tolerance,  BSTR* ErrorString,  LONG* time, BSTR debugInfo, ULONG* matches)
{
	HRESULT hr = S_OK;
	
	//no results by default
	*result = _bstr_t("").Detach();
	*matches = 0;
	*time = 0;

	TRY
	{
		string bucketName(RBS(bucket).c_str());
		
		MatcherFaceRegionsMany* pimFRmr = _AtlModule.GetFaceRegionMatcherForEventID(RBS(regKey), eventID, RBS(bucket), overflowBucket);
		if(pimFRmr != NULL)
		{
			string results;

			//query
			CTimer t;
			t.Start();
			if (pimFRmr->count() > 0)
			{
				try
				{
					*matches = pimFRmr->query(results, RBS(imgFNRegs), tolerance);

					stringstream ss;

					string temp;
					ss.clear();
					ss << overflowBucket;
					ss >> temp;

					string desc_bucket = "bucket: " + RBS(bucket);
					desc_bucket += "." + temp;

					string sRegKey = RBS(regKey);

					string sEventID;
					ss.clear();
					ss << eventID;
					ss >> sEventID;

					string sObjectID;
					ss.clear();
					ss << static_cast<const void*>(pimFRmr);
					ss >> sObjectID;

					string sTolerance;
					ss.clear();
					ss << tolerance;
					ss >> sTolerance;

					string truncResults = results;
					truncResults.resize(2048);

					string logMessage;
					logMessage =  ("query called on : " + desc_bucket + " app : " + sRegKey + " eventid: " + sEventID + " object_address: " + sObjectID);
					logMessage += NEWLINE + ("query values    : " + RBS(imgFNRegs) + " tolerance: " + sTolerance);
					logMessage += NEWLINE + ("query returned  (2K chars max): " + results);

					_AtlModule.Log(logMessage);
				}
				catch (...)	{}
			}
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
			//given that we could now have a valid collectionID/eventID without any ingested images 
			//(this is to balance EB's multispeed FM requirements, and PL's need for a speed adjusted Face localizer)
			//the notion that a non ready event is an error is no longer the case. So we need to flatten it to
			//a success code and pass empty results back.
			
			//*ErrorString = _bstr_t("Not ready").Detach();
			
			*ErrorString = _bstr_t("SUCCESS").Detach();
			*matches = 0;
			*result = _bstr_t("").Detach();
			*time = 1;

			stringstream ss;

			string temp;
			ss.clear();
			ss << overflowBucket;
			ss >> temp;

			string desc_bucket = "bucket: " + RBS(bucket);
			desc_bucket += "." + temp;

			string sRegKey = RBS(regKey);

			string sEventID;
			ss.clear();
			ss << eventID;
			ss >> sEventID;

			string sTolerance;
			ss.clear();
			ss << tolerance;
			ss >> sTolerance;

			string logMessage;
			logMessage = ("query called on : " + desc_bucket + " app : " + sRegKey + " eventid: " + sEventID);
			logMessage += NEWLINE + ("query values    : " + RBS(imgFNRegs) + " tolerance: " + sTolerance);
			logMessage += NEWLINE + string("query not issued as the bucket is empty");

			_AtlModule.Log(logMessage);
		}
	}
	CATCHDBG

	return hr;
}

string CCoreMatcherFaceRegionsMany::GetFileNameManyForEvent(string appKey, int eventID, string bucket)
{
	static const string sFileNameFormat = "%s\\App.%s.%s.many.eventid.%d.txt";
	const unsigned bufSize = 4096;
	auto_ptr<char> buf(new char[bufSize]);
	sprintf_s(buf.get(), bufSize, sFileNameFormat.c_str(), _AtlModule.GetRootDirectory().c_str(), appKey.c_str(), bucket.c_str(), eventID);
	string fileName = buf.get();
	return fileName;
}

STDMETHODIMP CCoreMatcherFaceRegionsMany::save(BSTR regKey, int eventID, BSTR* ErrorString, LONG* time)
{
	CMutexLock lock(hMutexFR);

	HRESULT hr = S_OK;
	*time = 0;
	TRY
	{
		//elements in the gender and age buckets should be distinct in its array
		string genderBuckets[] = { "Male", "Female", "GenderUnknown" };
		string ageBuckets[] = { "Youth", "Adult", "AgeUnknown" };

		CTimer t;
		t.Start();

		for(int gender = 0; gender < sizeof(genderBuckets)/sizeof(genderBuckets[0]); gender++)
		{
			for(int age = 0; age < sizeof(ageBuckets)/sizeof(ageBuckets[0]); age++)
			{
				//make bucket name
				string bucket = genderBuckets[gender] + "." + ageBuckets[age];

				int overflowBucketNumbering = 1;
				string bucketNameWithOverflow;
				
				while(1)
				{
					//make bucket number
					stringstream ss;
					ss << overflowBucketNumbering;

					string overflowBucket;
					ss >> overflowBucket;

					//make bucket name
					bucketNameWithOverflow = bucket + '.' + overflowBucket;

					//write
					MatcherFaceRegionsMany* pimFRmr = _AtlModule.GetFaceRegionMatcherForEventID(RBS(regKey), eventID, bucket, overflowBucketNumbering);
					if(pimFRmr != NULL)
					{
						string appKey =  RBS(regKey);

						//get at the correct many.txt path name
						string fileName = GetFileNameManyForEvent(appKey, eventID, bucketNameWithOverflow);

						pimFRmr->save(fileName);
					}
					else
					{
						break;
					}

					overflowBucketNumbering++;
				}
			}
		}
	
		*time = t.ElapsedMilliSeconds();

		//no error
		*ErrorString = _bstr_t("SUCCESS").Detach();
	}
	CATCH

	return hr;
}

STDMETHODIMP CCoreMatcherFaceRegionsMany::remove(BSTR regKey, int eventID, BSTR ID, ULONG* removed, BSTR* ErrorString, LONG* time)
{
	CMutexLock lock(hMutexFR);

	HRESULT hr = S_OK;
	*time = 0;
	*removed = 0;
	TRY
	{
		//elements in the gender and age buckets should be distinct in its array
		string genderBuckets[] = { "Male", "Female", "GenderUnknown" };
		string ageBuckets[] = { "Youth", "Adult", "AgeUnknown" };

		CTimer t;
		t.Start();

		for(int gender = 0; gender < sizeof(genderBuckets)/sizeof(genderBuckets[0]); gender++)
		{
			for(int age = 0; age < sizeof(ageBuckets)/sizeof(ageBuckets[0]); age++)
			{
				//make bucket name
				string bucket = genderBuckets[gender] + "." + ageBuckets[age];

				int overflowBucketNumbering = 1;
				
				while(1)
				{
					//write
					MatcherFaceRegionsMany* pimFRmr = _AtlModule.GetFaceRegionMatcherForEventID(RBS(regKey), eventID, bucket, overflowBucketNumbering);
					if(pimFRmr != NULL)
					{
						*removed += pimFRmr->remove(RBS(ID));


						//LOGGING for REMOVE - BEGIN
						
						string desc_bucket = "bucket - " + bucket;
						stringstream ss;
						ss << overflowBucketNumbering;
						string temp;
						ss >> temp;
						desc_bucket += " overflow level - " + temp;

						ss.clear();
						ss << *removed;
						ss >> temp;

						_AtlModule.Log("remove called on : " + desc_bucket);
						_AtlModule.Log("remove values    : " + RBS(ID));
						_AtlModule.Log("remove return value : " + temp);

						//LOGGING for REMOVE - END
					}
					else
					{
						break;
					}

					overflowBucketNumbering++;
				}
			}
		}
	
		*time = t.ElapsedMilliSeconds();

		//no error
		*ErrorString = _bstr_t("SUCCESS").Detach();
	}
	CATCH

	return hr;
}

STDMETHODIMP CCoreMatcherFaceRegionsMany::releaseEventDatabase(BSTR regKey, int eventID, BSTR* ErrorString, LONG* time)
{
	CMutexLock lock(hMutexFR);

	HRESULT hr = S_OK;
	*time = 0;
	TRY
	{
		CTimer t;
		t.Start();
		
		bool released = _AtlModule.ReleaseFaceRegionMatchersForEventID(RBS(regKey), eventID);
		
		*time = t.ElapsedMilliSeconds();

		//no error
		*ErrorString = released ? _bstr_t("SUCCESS").Detach() : _bstr_t("Not ready").Detach();

	}
	CATCH

	return hr;
}

STDMETHODIMP CCoreMatcherFaceRegionsMany::flushLog(void)
{
	//write application log
	_AtlModule.FlushLog();

	return S_OK;
}
