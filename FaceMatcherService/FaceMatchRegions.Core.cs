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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.Text;
using System.IO;
using System.Security;
using System.Net;
using System.Threading;
using System.Diagnostics;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using System.Diagnostics.Eventing.Reader;

namespace FaceMatcherService
{
    public partial class FaceMatchRegions : IFaceMatchRegions
    {
        char[] sepPeriod = new char[] { '.' };
        Object writeLock = new Object();

        /// <summary>
        /// calls the FaceMatchCore server to ingest the image
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="IDRegs"></param>
        /// <param name="errorString"></param>
        /// <param name="comMilliseconds"></param>
        /// <param name="coreOperationTime"></param>
        /// <param name="localPath"></param>
        /// <param name="gender"></param>
        /// <param name="age"></param>
        /// <param name="url"></param>
        private void coreIngest(string appKey, int eventID, string IDRegs, ref string errorString, ref int comMilliseconds, ref int coreOperationTime,
            string localPath, string gender, int age, string url)
        {
            int performanceValue = ServiceStateManager.getFaceFinderPerformanceValue(appKey, eventID);

            double ticksCOM_ff = Stopwatch.GetTimestamp();

            comMilliseconds = 0;
            coreOperationTime = 0;

            //make ID with region information, for faster ingest
            int faceFinderCoreOperationTime = 0;

            //if no face present in imgest image there is nothing to do, return.
            if (detectFaceRegionForIngest(ref IDRegs, localPath, ref faceFinderCoreOperationTime, ref comMilliseconds, performanceValue) == false)
            {
                coreOperationTime = faceFinderCoreOperationTime;
                errorString = "No face detected in input image";
                return;
            }

            //reformat the idregs to newer if needed
            //ID = reformatIDRegsToNewStandard(ID);

            //if the IDregs parameter is in old format then we need to reformat it to new standard

            //operation
            List<string> ingestBuckets = ServiceStateManager.bucketManager.ingestGetBuckets(appKey, eventID, gender, age);

            ConcurrentBag<long> ticksIngest = new ConcurrentBag<long>();
            ConcurrentBag<int> comTimes = new ConcurrentBag<int>();

            ConcurrentBag<string> bucketErrorStrings = new ConcurrentBag<string>();
            ConcurrentBag<string> applicationErrorStrings = new ConcurrentBag<string>();
            ConcurrentBag<string> debugInfoServiceLogCollection = new ConcurrentBag<string>();


            //parallel execute ingest
            Parallel.ForEach(Partitioner.Create(ingestBuckets), (bucket) =>
            {
                string _errorString = "";
                int _coreOperationTime = 0;
                string newOverflowValue = "";

                int currentOverflowLevel = getTopLevelBucketAndOverflowBucketNumber(ref bucket);

                string debugInfoForServiceLog = string.Empty;
                debugInfoForServiceLog = "\r\n---------------------------bucketed ingest------------------------";
                debugInfoForServiceLog += "\r\nurl [in]: " + url;
                debugInfoForServiceLog += "\r\nIDRegs [in]: " + IDRegs;
                debugInfoForServiceLog += "\r\neventID [in]: " + eventID;
                debugInfoForServiceLog += "\r\nclient [in]: " + appKey;
                debugInfoForServiceLog += "\r\nlocalPath value : " + localPath;
                debugInfoForServiceLog += "\r\nbucket: " + bucket;
                debugInfoForServiceLog += "\r\noverflow level: " + currentOverflowLevel;

                try
                {
                    if (imr != null)
                    {
                        //measure time taken to ingest on this bucket
                        ticksIngest.Add(Stopwatch.GetTimestamp());

                        double ticksCOM = Stopwatch.GetTimestamp();

                        imr.ingest(appKey, eventID, localPath, IDRegs, bucket, currentOverflowLevel, ref newOverflowValue, ServiceStateManager.MAX_BUCKET_SIZE, ref _errorString, ref _coreOperationTime, debugInfoForServiceLog, useGPUFromKey(appKey), performanceValue);

                        ticksIngest.Add(Stopwatch.GetTimestamp());

                        double timeCOM = ((double)(Stopwatch.GetTimestamp() - ticksCOM) / (double)Stopwatch.Frequency) * 1000.0d;

                        comTimes.Add(Convert.ToInt32(timeCOM - _coreOperationTime));

                        //update overflow values if changed
                        if (newOverflowValue.Length > 0)
                            ServiceStateManager.bucketManager.syncOverflowBucketCountsForEvent(appKey, eventID, newOverflowValue);
                    }
                    else
                    {
                        ServiceStateManager.Log("skipping call to imr.ingest as the imr reference is null");
                    }

                    debugInfoForServiceLog += "\r\nresults:";
                    debugInfoForServiceLog += "\r\nsucceeded? :" + _errorString;
                    debugInfoForServiceLog += "\r\n_coreOperationTime (milliseconds): " + _coreOperationTime;
                    debugInfoServiceLogCollection.Add(debugInfoForServiceLog);
                }
                catch (Exception e)
                {
                    //do nothing
                    _errorString = "Exception : " + e.Message;
                    _errorString += debugInfoForServiceLog;

                    ServiceStateManager.Log("Exception : exception caught in imr.ingest call details : " + e.Message + debugInfoForServiceLog);

                    logApplicationErrorsIfAny(localPath, url, applicationErrorStrings);
                }
                finally
                {
                    if (_errorString.Equals("SUCCESS", StringComparison.OrdinalIgnoreCase) == false)
                    {
                        bucketErrorStrings.Add(_errorString);

                        //do not check for application errors on success
                        //logApplicationErrorsIfAny(localPath, url, applicationErrorStrings);
                    }
                }
            });

            //measure total ingest time across all buckets per second, convert to milliseconds
            double ingestTimeInSecs = 0;
            int ingestTimeInMilliseconds = 0;
            
            if (ticksIngest.Count > 0)
            {
                ingestTimeInSecs = (double)(ticksIngest.Max() - ticksIngest.Min()) / (double)Stopwatch.Frequency;
                ingestTimeInMilliseconds = Convert.ToInt32(ingestTimeInSecs * 1000.0d);
            }

            coreOperationTime = ingestTimeInMilliseconds;
            coreOperationTime += faceFinderCoreOperationTime;

            if (comTimes.Count > 0)
                comMilliseconds += Convert.ToInt32(comTimes.Average());


            string collatedIngestLog = string.Join(string.Empty, debugInfoServiceLogCollection);
            collatedIngestLog += "\r\nexception messages (if any): " + string.Join(", ", bucketErrorStrings.Distinct()).Trim();
           
            ServiceStateManager.Log(collatedIngestLog);

            //success
            if (bucketErrorStrings.IsEmpty)
                errorString = "SUCCESS";
            else
            {
                errorString = String.Empty;

                foreach (string errStr in bucketErrorStrings.Distinct())
                    errorString += errStr;
            }
        }

        /// <summary>
        /// provides logging services
        /// </summary>
        /// <param name="localPath"></param>
        /// <param name="url"></param>
        /// <param name="applicationErrorStrings"></param>
        private void logApplicationErrorsIfAny(string localPath, string url, ConcurrentBag<string> applicationErrorStrings)
        {
            //check to see if there was an access violation
            //wait for 1 second to allow for any delays in reporting to system by core
            Thread.Sleep(1000);
            EventLogQuery eventsQuery = new EventLogQuery("Application", PathType.LogName);
            using (System.Diagnostics.Eventing.Reader.EventLogReader logReader = new System.Diagnostics.Eventing.Reader.EventLogReader(eventsQuery))
            {
                for (EventRecord ev = logReader.ReadEvent(); ev != null; ev = logReader.ReadEvent())
                {
                    if (ev.ProviderName != null &&
                        ev.ProviderName.Equals("Application Error", StringComparison.OrdinalIgnoreCase))
                    {
                        if (ev.TimeCreated > DateTime.Now.AddMinutes(-5.0))
                        {
                            String eventDescription = ev.FormatDescription();
                            if (eventDescription.Contains("Face"))
                            {
                                lock (writeLock)
                                {
                                    String destFile = String.Empty;
                                    try
                                    {
                                        destFile = @"c:\FaceMatchSL\FaceMatch\bin\Problematic\";
                                        destFile += Path.GetFileName(localPath);
                                        FileInfo fi = new FileInfo(destFile);
                                        if (!fi.Exists)
                                            File.Copy(localPath, destFile);
                                    }
                                    catch (Exception)
                                    {
                                        ServiceStateManager.Log("could not copy file : " + destFile);
                                    }
                                    finally
                                    {
                                        String logMessage = String.Empty;
                                        logMessage += "\nApplication error detected in the last 5 minutes : ";
                                        logMessage += "\nurl and idregs values: " + url;
                                        logMessage += "\nfile being ingested : " + localPath;
                                        logMessage += "\ndetails:" + eventDescription;
                                        if (!applicationErrorStrings.Contains(logMessage))
                                        {
                                            //add to bag
                                            applicationErrorStrings.Add(logMessage);
                                            //add to log
                                            ServiceStateManager.Log(logMessage);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        private int getTopLevelBucketAndOverflowBucketNumber(ref string bucket)
        {
            int currentBucketValue = 1;
            string[] parts = bucket.Split(sepPeriod, StringSplitOptions.RemoveEmptyEntries);
            if (parts != null && parts.Length > 2)
            {
                if (int.TryParse(parts[2], out currentBucketValue))
                    bucket = parts[0] + '.' + parts[1];
            }
            return currentBucketValue;
        }
        /// <summary>
        /// calls the FaceMatchCore server for querying the image. 
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="result"></param>
        /// <param name="tolerance"></param>
        /// <param name="errorString"></param>
        /// <param name="matches"></param>
        /// <param name="comMilliseconds"></param>
        /// <param name="coreOperationTime"></param>
        /// <param name="localPath"></param>
        /// <param name="gender"></param>
        /// <param name="age"></param>
        /// <param name="url"></param>

        private void queryCore(string appKey, int eventID, ref string result, float tolerance, ref string errorString, ref uint? matches, ref int comMilliseconds, ref int coreOperationTime, 
            string localPath, string gender, int age, string url)
        {
            //get facefinder performance flag
            int performanceValue = ServiceStateManager.getFaceFinderPerformanceValue(appKey, eventID);

            double ticksCOM_ff = Stopwatch.GetTimestamp();

            comMilliseconds = 0;
            coreOperationTime = 0;

            //find region once and pass this across all buckets for faster query
            int faceFinderCoreOperationTime = 0;

            ConcurrentBag<long> ticksQuery = new ConcurrentBag<long>();
            ConcurrentBag<int> comTimes = new ConcurrentBag<int>();

            ConcurrentBag<string> bucketErrorStrings = new ConcurrentBag<string>();
            ConcurrentBag<uint> bucketHits = new ConcurrentBag<uint>();
            ConcurrentQueue<Queue<Match>> bucketResults = new ConcurrentQueue<Queue<Match>>();
            ConcurrentBag<string> debugInfoServiceLogCollection = new ConcurrentBag<string>();

            //if no face present in query image there is nothing to do, return.
            if (detectFaceRegionForQuery(ref localPath, localPath, ref faceFinderCoreOperationTime, ref comMilliseconds, performanceValue) == false)
            {
                coreOperationTime = faceFinderCoreOperationTime;
                errorString = "No face detected in input image";
                result = "";
                return;
            }

            //operation
            List<string> queryBuckets = ServiceStateManager.bucketManager.queryGetBuckets(appKey, eventID, gender, age);

            //results limit
            int limit = resultsLimit(tolerance);

            //parallel execute ingest
            Parallel.ForEach(Partitioner.Create(queryBuckets), (bucket) =>
            {
                string _errorString = "";
                int _coreOperationTime = 0;
                string _result = "";

                string bucketNameWithOverflow = bucket;

                int currentOverflowLevel = getTopLevelBucketAndOverflowBucketNumber(ref bucket);

                string debugInfoForServiceLog = string.Empty;
                debugInfoForServiceLog =  "\r\n---------------------------bucketed query------------------------";
                debugInfoForServiceLog += "\r\nurl [in]: " + url;
                debugInfoForServiceLog += "\r\neventID [in]: " + eventID;
                debugInfoForServiceLog += "\r\nclient [in]: " + appKey;
                debugInfoForServiceLog += "\r\nlocalPath value : " + localPath;
                debugInfoForServiceLog += "\r\nbucket: " + bucket;
                debugInfoForServiceLog += "\r\noverflow level: " + currentOverflowLevel;

                try
                {
                    //operation
                    uint hits = 0;
                    if (imr != null)
                    {
                        ticksQuery.Add(Stopwatch.GetTimestamp());

                        double ticksCOM = Stopwatch.GetTimestamp();

                        hits = imr.query(appKey, eventID, ref _result, localPath, bucket, currentOverflowLevel, tolerance, ref _errorString, ref _coreOperationTime, debugInfoForServiceLog);

                        ticksQuery.Add(Stopwatch.GetTimestamp());

                        double timeCOM = ((double)(Stopwatch.GetTimestamp() - ticksCOM) / (double)Stopwatch.Frequency) * 1000.0d;

                        comTimes.Add(Convert.ToInt32(timeCOM - _coreOperationTime));
                    }
                    else
                        ServiceStateManager.Log("skipping call to imr.query as the imr reference is null");

                    //extended debugging for FM-94 resolution
                    debugInfoForServiceLog += "\r\nresults:";
                    debugInfoForServiceLog += new string(_result.Take(2048).ToArray());
                    debugInfoForServiceLog += "\r\n_coreOperationTime (milliseconds): " + _coreOperationTime;
                    debugInfoServiceLogCollection.Add(debugInfoForServiceLog);
                    
                    //save the results for this bucket
                    bucketHits.Add(hits);

                    //check if we need to penalize the scores for this bucket
                    bool penalize = ServiceStateManager.bucketManager.isPenalizeBucket(appKey, eventID, gender, age, bucketNameWithOverflow);

                    //parse results
                    Queue<Match> queueHits = parseMatches(_result, penalize);
                    Queue<Match> q2 = null;
                    Queue<Match> merged = null;

                    //merge on idle
                    if ((bucketResults.Count > 0) && bucketResults.TryDequeue(out q2))
                    {
                        //merge (stable)
                        merged = mergePair(queueHits, q2, limit);

                        if (merged != null && merged.Count > 0)
                            bucketResults.Enqueue(merged);
                    }
                    //enqueue 
                    else if (queueHits != null && queueHits.Count > 0)
                        bucketResults.Enqueue(queueHits);

                }
                catch (Exception e)
                {
                    //do nothing
                    _errorString = "Exception : " + e.Message;
                    _errorString += debugInfoForServiceLog;

                    ServiceStateManager.Log("Exception : exception caught in imr.query call details : " + e.Message + debugInfoForServiceLog);
                }
                finally
                {
                    //do not add SUCCESS to error string
                    if (_errorString.Equals("SUCCESS", StringComparison.OrdinalIgnoreCase) == false)
                        bucketErrorStrings.Add(_errorString);
                }
            });

            //measure total query time across all buckets per second, convert to milliseconds
            double queryTimeInSecs = 0;
            int queryTimeInMilliseconds = 0;

            if (ticksQuery.Count > 0)
            {
                queryTimeInSecs = (double)(ticksQuery.Max() - ticksQuery.Min()) / (double)Stopwatch.Frequency;
                queryTimeInMilliseconds = Convert.ToInt32(queryTimeInSecs * 1000.0d);
            }

            coreOperationTime = queryTimeInMilliseconds;
            coreOperationTime += faceFinderCoreOperationTime;

            if (comTimes.Count > 0)
                comMilliseconds += Convert.ToInt32(comTimes.Average());

            //parallel merge query results 
            ConcurrentQueue<Queue<Match>> mergedResultQueue = parallelMergeResults(bucketResults, limit);

            //results
            result = resultsFromQueue(mergedResultQueue, tolerance, ref matches);

            //log query for debugging
            string truncatedResult = new string(result.Take(2048).ToArray());
            string mergedQueryLog = string.Join(string.Empty, debugInfoServiceLogCollection);
            mergedQueryLog +=  "\r\n----------------------------------merged results--------------------------------------";
            mergedQueryLog += "\r\nurl [in]: " + url;
            mergedQueryLog += "\r\neventID [in]: " + eventID;
            mergedQueryLog += "\r\nclient [in]: " + appKey;
            mergedQueryLog += "\r\nlocalPath value : " + localPath;
            mergedQueryLog += "\r\nbuckets queried : " + string.Join(", ", queryBuckets);
            mergedQueryLog += "\r\nquery returned (limited to 2K chars max): " + truncatedResult;
            mergedQueryLog += "\r\nexception messages (if any): " + string.Join(", ", bucketErrorStrings.Distinct()).Trim();
            ServiceStateManager.Log(mergedQueryLog);

            //success
            if (bucketErrorStrings.IsEmpty)
                errorString = "SUCCESS";
            else
            {
                if (result.Length == 0)
                {
                    foreach (string errStr in bucketErrorStrings.Distinct())
                        errorString += errStr;
                }
            }
        }

        /// <summary>
        /// calls the FaceMatchCore to find faces on the given image
        /// </summary>
        /// <param name="s"></param>
        /// <param name="localPath"></param>
        /// <param name="faceFinderCoreOperationTime"></param>
        /// <param name="comMilliseconds"></param>
        /// <param name="faceFinderPerformanceValue"></param>
        /// <returns></returns>
        private bool detectFaceRegionForQuery(ref string s, string localPath, ref int faceFinderCoreOperationTime, ref int comMilliseconds, int faceFinderPerformanceValue)
        {
            bool hasFace = s.Contains('[');

            string faceRegions = string.Empty;

            faceFinderCoreOperationTime = 0;
            comMilliseconds = 0;

            if (hasFace == false)
            {
                string errors = "";

                try
                {
                    FaceFinder faceFinder = ServiceStateManager.faceFinder;
                    if (faceFinder != null && faceFinder.ff != null)
                    {
                        double ticksCOM = Stopwatch.GetTimestamp();

                        faceFinder.ff.GetFaces(localPath, 1, ref faceRegions, ref errors, ref faceFinderCoreOperationTime, 1, faceFinderPerformanceValue);

                        double timeCOM = ((double)(Stopwatch.GetTimestamp() - ticksCOM) / (double)Stopwatch.Frequency) * 1000.0d;

                        comMilliseconds = Convert.ToInt32(timeCOM - faceFinderCoreOperationTime);
                    }
                    else if (faceFinder == null)
                        ServiceStateManager.Log("skipping faceFinder.ff.GetFaces call as the faceFinder interface reference is null");
                    else if (faceFinder.ff == null)
                        ServiceStateManager.Log("skipping faceFinder.ff.GetFaces call as the faceFinder.ff reference is null");
                }
                catch (Exception ex)
                {
                    ServiceStateManager.Log("Exception : the faceFinder.ff.GetFaces call returned an exception : details : " + ex.Message);
                }
                finally
                {
                    hasFace = faceRegions.Contains('[');
                    if (hasFace)
                    {
                        //if multiple faces, issue a multiface query at once
                        FaceFinderParser parser = new FaceFinderParser();
                        s = localPath.Trim() + "\t";
                        s += string.Join("\t", parser.getFaceList(faceRegions));
                    }
                }
            }

            return hasFace;
        }

        private bool detectFaceRegionForIngest(ref string s, string localPath, ref int faceFinderCoreOperationTime, ref int comMilliseconds, int faceFinderPerformanceValue)
        {
            bool hasFace = hasFaceOrProfile(s);

            string faceRegions = string.Empty;

            faceFinderCoreOperationTime = 0;
            comMilliseconds = 0;

            if (hasFace == false)
            {
                string errors = "";
                string ID = s;

                s = string.Empty;

                try
                {
                    FaceFinder faceFinder = ServiceStateManager.faceFinder;
                    if (faceFinder != null && faceFinder.ff != null)
                    {
                        double ticksCOM = Stopwatch.GetTimestamp();
                        
                        faceFinder.ff.GetFaces(localPath, 1, ref faceRegions, ref errors, ref faceFinderCoreOperationTime, 1, faceFinderPerformanceValue);

                        double timeCOM = ((double)(Stopwatch.GetTimestamp() - ticksCOM) / (double)Stopwatch.Frequency) * 1000.0d;

                        comMilliseconds = Convert.ToInt32(timeCOM - faceFinderCoreOperationTime);
                    }
                    else if (faceFinder == null)
                        ServiceStateManager.Log("skipping faceFinder.ff.GetFaces call as the faceFinder interface reference is null");
                    else if (faceFinder.ff == null)
                        ServiceStateManager.Log("skipping faceFinder.ff.GetFaces call as the faceFinder.ff reference is null");
                }
                catch (Exception ex)
                {
                    ServiceStateManager.Log("Exception : the faceFinder.ff.GetFaces call returned an exception : details : " + ex.Message);
                }
                finally
                {
                    //attach to ID for ingest across all buckets
                    hasFace = faceRegions.Contains('[');
                    if (hasFace)
                    {
                        var parser = new FaceFinderParser(ID, faceRegions);
                        
                        //this returns all faces collated together so they are ingested at once
                        s = parser.ToString();
                    }
                    else
                    {
                        s = ID;
                    }
                }
            }

            return hasFace;
        }
        
        private static bool hasFaceOrProfile(string s)
        {
            return s.Contains("f[") || s.Contains("p[") || s.Contains("f{[") || s.Contains("p{[");
        }

        private static bool hasBadFormatFaceOrProfile(string s)
        {
            return s.Contains(" f[") || s.Contains(" p[") || s.Contains(" f{[") || s.Contains(" p{[");
        }

    }
}


