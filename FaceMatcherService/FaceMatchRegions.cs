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
using System.Text.RegularExpressions;
using System.Diagnostics.Eventing.Reader;
using System.Collections.Concurrent;
using Newtonsoft.Json;

namespace FaceMatcherService
{
    /// <summary>
    /// main class that implements FaceMatch services
    /// </summary>
    [ServiceBehavior(
        ConcurrencyMode = ConcurrencyMode.Multiple,
        InstanceContextMode = InstanceContextMode.Single
        )]
    public partial class FaceMatchRegions : IFaceMatchRegions
    {
        static int filePrefixIngest = 0;
        static int filePrefixQuery = 0;
        static int removeConcurrency = 0;

        public FaceMatcherCoreLib.CoreMatcherFaceRegionsMany imr = null;

        public static int systemLogEventID = 1000;
        public static Object systemLogEventIDlock = new Object();

        public enum FaceMatchError : int
        {
            SUCCESS = 0,//0
            FM_NOT_AVAILABLE, //1
            INVALID_KEY, //2
            KEY_REQUIRED, //3
            ILLEGAL_EVENT_ID,//4
            EVENT_ID_NOT_FOUND,//5
            CANNOT_DOWNLOAD_URL,//6
            URL_IS_EMPTY,//7
            URL_IS_NULL,//8
            URL_BAD_SCHEME,//9
            URL_IS_INVALID,//10
            LOCAL_FILE_URL_NOT_ALLOWED,//11
            NO_FACE_DETECTED,//12
            ILLEGAL_INFLATE_PERCENTAGE,//13
            REGIONS_PARAM_IS_NULL,//14
            DISPLAY_REGS_PARAM_IS_NULL,//15
            RESULT_PARAM_IS_NULL,//16
            BAD_PERFORMANCE_VALUE,//17
            FM_EXCEPTION,//18
            FM_INTERNAL_ERROR,//19
            FM_UNKNOWN,//20
            FM_INVALID_REGIONS//21
        }

        public FaceMatchRegions()
        {
            try
            {
                if (imr == null)
                    imr = new FaceMatcherCoreLib.CoreMatcherFaceRegionsMany();

                //save an interface reference for use by internal clients
                ServiceStateManager.svcFMR = this;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to instantiate CoreMatcherFaceRegionsMany class at com server: " + ex.Message);
            }
        }

        /// <summary>
        /// ingests an image into FaceMatch
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="url"></param>
        /// <param name="IDRegs"></param>
        /// <param name="gender"></param>
        /// <param name="age"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        /// <param name="webServiceMilliseconds"></param>
        /// <param name="imageCoreMilliseconds"></param>
        /// <param name="comMilliseconds"></param>
        /// <param name="urlFetchMilliseconds"></param>
        /// <param name="requestsOutstanding"></param>
        public void ingest(string appKey, int eventID, string url, string IDRegs, string gender, int age, ref string errorString, ref int errorCode, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            try
            {
                long webServiceTicks = Stopwatch.GetTimestamp();

                int comRuntimeWatchTotal = 0;
                int coreOperationTimeTotal = 0;
                int? coreOperationTime_Remove = 0, comRuntimeWatch_Remove = 0;

                //check if system is running
                if (!isFMSystemRunning())
                {
                    errorString = "FaceMatch system is not available";
                    errorCode = (int)FaceMatchError.FM_NOT_AVAILABLE;
                    return;
                }

                try
                {
                    //validate and init
                    if (ingestValidateInit(appKey, eventID, gender, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds) == false)
                        return;

                    //validate face region formatting
                    if (hasBadFormatFaceOrProfile(IDRegs))
                    {
                        errorString = "Invalid regions present, faces were not ingested";
                        return;
                    }

                    //allow 'exact' id-reuse (without face regions)
                    if (IDRegs.Contains("[") == false && user_isPL(appKey) == false)
                    {
                        uint records = 0;
                        int? outstandingRemoveRequests = 0, unused1 = 0;
                        remove(appKey, eventID, IDRegs, ref records, ref errorString, ref errorCode, ref unused1, ref coreOperationTime_Remove, ref comRuntimeWatch_Remove, ref outstandingRemoveRequests);
                    }

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    ServiceStateManager.PeekEvent(appKey, eventID, true);

                    try
                    {
                        Uri uri = null;

                        //check url
                        if (validateUrl(url, ref errorString, ref uri) == false)
                            return;

                        FileInfo fi = null;

                        if (uri.IsFile)
                        {
                            errorString = "local files are not allowed. Please use a url.";
                            return;
                        }

                        try
                        {
                            string file = "";
                            string url_image;
                            string[] split;

                            if (isEmptyUrl(url, ref errorString, out url_image, out split))
                                return;

                            int outstanding = Interlocked.Increment(ref filePrefixIngest);

                            if (requestsOutstanding != null)
                                requestsOutstanding = outstanding;

                            file = @"C:\FaceMatchSL\Facematch\bin\UploadBin\App." + appKey + ".event." + eventID + ".fri." + filePrefixIngest + "." + split[split.Length - 1];

                            WebClient wc = new WebClient();

                            long urlTicks = Stopwatch.GetTimestamp();

                            //download file
                            downloadURLtoFile(file, wc, url_image);

                            //measure download time
                            if (urlFetchMilliseconds != null)
                                urlFetchMilliseconds = Convert.ToInt32(((double)(Stopwatch.GetTimestamp() - urlTicks) / (double)Stopwatch.Frequency) * 1000.0d);

                            //check
                            fi = new FileInfo(file);

                            if (fi.Exists)
                            {
                                List<string> annGroup = convertToNewFormatAndGroupForIngest(IDRegs);

                                if (annGroupsHaveInvalidRegions(annGroup))
                                {
                                    errorString = "Invalid regions present, faces were not ingested";
                                    return;
                                }

                                coreOperationTimeTotal = 0;
                                comRuntimeWatchTotal = 0;

                                foreach (string groupIDRegs in annGroup)
                                {
                                    //parse GT meta data
                                    List<IngestData> ingestList = parseGT(groupIDRegs, ref gender, age, ref errorString);

                                    if (ingestList != null && ingestList.Count > 0)
                                    {
                                        int comMillisecondsRegion = 0;
                                        int coreOperationTimeRegion = 0;

                                        foreach (IngestData ingestRegion in ingestList)
                                        {
                                            comMillisecondsRegion = 0;
                                            coreOperationTimeRegion = 0;

                                            coreIngest(appKey, eventID, ingestRegion.IDRegs, ref errorString, ref comMillisecondsRegion, ref coreOperationTimeRegion, file, ingestRegion.Gender, ingestRegion.Age, url);

                                            //accumulate total time
                                            comRuntimeWatchTotal += comMillisecondsRegion;
                                            coreOperationTimeTotal += coreOperationTimeRegion;
                                        }
                                    }
                                }
                            }

                            //delete file
                            fi = eraseDeleteProcessedFile(fi, file);
                        }
                        catch (Exception ex)
                        {
                            errorString = "Could not download url specified";
                            Console.WriteLine(ex.Message);
                        }
                        finally
                        {
                            Interlocked.Decrement(ref filePrefixIngest);
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Exception occurred: " + ex.Message);
                    }
                }
                finally
                {
                    //account for remove overhead
                    comRuntimeWatchTotal += comRuntimeWatch_Remove.Value;
                    coreOperationTimeTotal += coreOperationTime_Remove.Value;

                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceTicks, comRuntimeWatchTotal, coreOperationTimeTotal, urlFetchMilliseconds);
                }
            }
            finally
            {
                errorCode = setErrorCode(errorString);
                errorString = setSuccessStringOnNoError(errorString, errorCode);
            }
        }

        /// <summary>
        /// queries an image from the facematch database.
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="result"></param>
        /// <param name="urlRegs"></param>
        /// <param name="gender"></param>
        /// <param name="age"></param>
        /// <param name="tolerance"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        /// <param name="maxMatchesRequested"></param>
        /// <param name="matches"></param>
        /// <param name="webServiceMilliseconds"></param>
        /// <param name="imageCoreMilliseconds"></param>
        /// <param name="comMilliseconds"></param>
        /// <param name="urlFetchMilliseconds"></param>
        /// <param name="requestsOutstanding"></param>
        public void query(string appKey, int eventID, ref string result, string urlRegs, string gender, int age, float tolerance, ref string errorString, ref int errorCode, 
            uint? maxMatchesRequested, ref uint? matches, ref int? webServiceMilliseconds,
            ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            try
            {
                long webServiceTicks = Stopwatch.GetTimestamp();

                int comRuntimeWatchTotal = 0;
                int coreOperationTimeTotal = 0;

                //check if system is running
                if (!isFMSystemRunning())
                {
                    errorString = "FaceMatch system is not available";
                    return;
                }

                try
                {
                    //validate and init
                    if (queryValidateInit(appKey, eventID, gender, ref result, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding) == false)
                        return;

                    //validate face region formatting
                    if (hasBadFormatFaceOrProfile(urlRegs))
                    {
                        errorString = "url is empty.-or- The scheme specified in uriString is not correctly formed.";
                        return;
                    }

                    //compress tolerance parameter
                    if (maxMatchesRequested != null && maxMatchesRequested.HasValue)
                        tolerance += maxMatchesRequested.Value;

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                    {
                        //event is not found or has no images ingested yet
                        errorString = "eventID not found or has no images ingested";
                        result = "";
                        if (matches != null)
                            matches = 0;
                        return;
                    }

                    char[] sep = { '\t' };
                    string[] parts = urlRegs.Split(sep);

                    string url = parts[0];
                    string optionalROI = "";

                    if (parts != null && parts.Length > 1)
                    {
                        optionalROI = "\t";
                        for (int i = 1; i < parts.Length; i++)
                            optionalROI = optionalROI + parts[i] + "\t";
                        optionalROI = optionalROI.TrimEnd();
                    }

                    try
                    {
                        Uri uri = null;

                        //check url
                        if (validateUrl(url, ref errorString, ref uri) == false)
                            return;

                        FileInfo fi = null;

                        if (uri.IsFile)
                        {
                            string localPath = "";

                            GetLocalFileInfo(url, ref errorString, uri, ref fi, ref localPath);

                            if (errorString.Length != 0)
                                return;

                            //query
                            queryCore(appKey, eventID, ref result, tolerance, ref errorString, ref matches, ref comRuntimeWatchTotal, ref coreOperationTimeTotal, localPath + optionalROI, gender, age, urlRegs);

                            return;
                        }

                        try
                        {
                            string file = "";
                            string url_image;
                            string[] split;

                            if (isEmptyUrl(url, ref errorString, out url_image, out split))
                                return;

                            int outstanding = Interlocked.Increment(ref filePrefixQuery);

                            if (requestsOutstanding != null)
                                requestsOutstanding = outstanding;

                            file = @"C:\FaceMatchSL\Facematch\bin\UploadBin\App." + appKey + ".event." + eventID + ".frq." + filePrefixQuery + "." + split[split.Length - 1];


                            WebClient wc = new WebClient();

                            long urlTicks = Stopwatch.GetTimestamp();

                            //download file
                            downloadURLtoFile(file, wc, url_image);

                            //measure download time
                            if (urlFetchMilliseconds != null)
                                urlFetchMilliseconds = Convert.ToInt32(((double)(Stopwatch.GetTimestamp() - urlTicks) / (double)Stopwatch.Frequency) * 1000.0d);

                            //check
                            fi = new FileInfo(file);
                            if (fi.Exists)
                                queryCore(appKey, eventID, ref result, tolerance, ref errorString, ref matches, ref comRuntimeWatchTotal, ref coreOperationTimeTotal, file + optionalROI, gender, age, urlRegs);

                            //delete file
                            fi = eraseDeleteProcessedFile(fi, file);
                        }
                        catch (Exception)
                        {
                            errorString = "Could not download url specified";
                        }
                        finally
                        {
                            Interlocked.Decrement(ref filePrefixQuery);
                        }
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Exception occurred: " + ex.Message);
                    }
                }
                finally
                {
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceTicks, comRuntimeWatchTotal, coreOperationTimeTotal, urlFetchMilliseconds);
                }
            }
            finally
            {
                errorCode = setErrorCode(errorString);
                errorString = setSuccessStringOnNoError(errorString, errorCode);
            }
        }

        /// <summary>
        /// removea an image from the facematch database
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="ID"></param>
        /// <param name="records"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        /// <param name="webServiceMilliseconds"></param>
        /// <param name="imageCoreMilliseconds"></param>
        /// <param name="comMilliseconds"></param>
        /// <param name="requestsOutstanding"></param>
        public void remove(string appKey, int eventID, string ID, ref uint records, ref string errorString, ref int errorCode, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? requestsOutstanding)
        {
            try
            {
                long webServiceTicks = Stopwatch.GetTimestamp();

                int comRuntimeWatchTotal = 0;
                int coreOperationTimeTotal = 0;

                int? unused = null;

                //check if system is running
                if (!isFMSystemRunning())
                {
                    errorString = "FaceMatch system is not available";
                    return;
                }

                try
                {
                    //validate
                    if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                        return;

                    //zero out timing vars
                    initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref unused);

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                    {
                        //event is not found or has no images ingested yet
                        return;
                    }

                    ConcurrentBag<long> ticksRemove = new ConcurrentBag<long>();
                    ConcurrentBag<int> comTimes = new ConcurrentBag<int>();

                    try
                    {
                        int concurrency = Interlocked.Increment(ref removeConcurrency);

                        if (requestsOutstanding != null)
                            requestsOutstanding = concurrency;

                        //operation
                        //need to parallelize remove
                        if (imr != null)
                            foreach (var itemID in ID.Split(new string[] { "$fmremove$" }, StringSplitOptions.RemoveEmptyEntries))
                            {
                                int _coreOperationTime = 0;

                                ticksRemove.Add(Stopwatch.GetTimestamp());

                                double ticksCOM = Stopwatch.GetTimestamp();

                                imr.remove(appKey, eventID, itemID, ref records, ref errorString, ref _coreOperationTime);

                                ticksRemove.Add(Stopwatch.GetTimestamp());

                                double timeCOM = ((double)(Stopwatch.GetTimestamp() - ticksCOM) / (double)Stopwatch.Frequency) * 1000.0d;

                                comTimes.Add(Convert.ToInt32(timeCOM - _coreOperationTime));
                            }
                        else
                            ServiceStateManager.Log("skipping call to imr.remove as the imr is null");
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Exception occurred: " + ex.Message);
                        errorString = "Exception occurred: " + ex.Message;

                        ServiceStateManager.Log("Exception : exception caught in imr.remove call details : " + ex.Message);
                    }
                    finally
                    {
                        Interlocked.Decrement(ref removeConcurrency);

                        //measure total remove time across all ID's per second, convert to milliseconds
                        double removeTimeInSecs = 0;
                        int removeTimeInMilliseconds = 0;

                        if (ticksRemove.Count > 0)
                        {
                            removeTimeInSecs = (double)(ticksRemove.Max() - ticksRemove.Min()) / (double)Stopwatch.Frequency;
                            removeTimeInMilliseconds = Convert.ToInt32(removeTimeInSecs * 1000.0d);
                        }

                        coreOperationTimeTotal = removeTimeInMilliseconds;

                        if (comTimes.Count > 0)
                            comRuntimeWatchTotal = Convert.ToInt32(comTimes.Average());
                    }
                }
                finally
                {
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceTicks, comRuntimeWatchTotal, coreOperationTimeTotal, null);
                }
            }
            finally
            {
                errorCode = setErrorCode(errorString);
                errorString = setSuccessStringOnNoError(errorString, errorCode);
            }
        }

        /// <summary>
        /// saves an image into the facemach database
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        /// <param name="webServiceMilliseconds"></param>
        /// <param name="imageCoreMilliseconds"></param>
        /// <param name="comMilliseconds"></param>
        public void save(string appKey, int eventID, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds)
        {
            try
            {
                long webServiceTicks = Stopwatch.GetTimestamp();

                int comRuntimeWatchTotal = 0;
                int coreOperationTimeTotal = 0;
                int? unused = null;

                //check if system is running
                if (!isFMSystemRunning())
                {
                    errorString = "FaceMatch system is not available";
                    return;
                }

                try
                {
                    //validate
                    if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                        return;

                    //zero out timing vars
                    initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref unused);

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                    {
                        //event is not found or has no images ingested yet
                        return;
                    }

                    ConcurrentBag<long> ticksSave = new ConcurrentBag<long>();
                    ConcurrentBag<int> comTimes = new ConcurrentBag<int>();

                    try
                    {
                        //operation
                        if (imr != null)
                        {
                            int _coreOperationTime = 0;

                            ticksSave.Add(Stopwatch.GetTimestamp());

                            double ticksCOM = Stopwatch.GetTimestamp();

                            imr.save(appKey, eventID, ref errorString, ref _coreOperationTime);

                            ticksSave.Add(Stopwatch.GetTimestamp());

                            double timeCOM = ((double)(Stopwatch.GetTimestamp() - ticksCOM) / (double)Stopwatch.Frequency) * 1000.0d;

                            comTimes.Add(Convert.ToInt32(timeCOM - _coreOperationTime));
                        }
                        else
                            ServiceStateManager.Log("skipping call to imr.save as the imr is null");
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Exception occurred: " + ex.Message);
                        errorString = "Exception occurred: " + ex.Message;

                        ServiceStateManager.Log("Exception : exception caught in imr.save call details : " + ex.Message);
                    }
                    finally
                    {
                        //measure total remove time across all ID's per second, convert to milliseconds
                        double saveTimeInSecs = 0;
                        int saveTimeInMilliseconds = 0;

                        if (ticksSave.Count > 0)
                        {
                            saveTimeInSecs = (double)(ticksSave.Max() - ticksSave.Min()) / (double)Stopwatch.Frequency;
                            saveTimeInMilliseconds = Convert.ToInt32(saveTimeInSecs * 1000.0d);
                        }

                        coreOperationTimeTotal = saveTimeInMilliseconds;

                        if (comTimes.Count > 0)
                            comRuntimeWatchTotal = Convert.ToInt32(comTimes.Average());
                    }
                }
                finally
                {
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceTicks, comRuntimeWatchTotal, coreOperationTimeTotal, null);
                }
            }
            finally
            {
                errorCode = setErrorCode(errorString);
                errorString = setSuccessStringOnNoError(errorString, errorCode);
            }
        }

        private static string setSuccessStringOnNoError(string errorString, int errorCode)
        {
            if (errorCode == (int)FaceMatchError.SUCCESS)
                errorString = "SUCCESS";
            return errorString;
        }

        /// <summary>
        /// deletes an image collection from the facematch database
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        /// <param name="webServiceMilliseconds"></param>
        /// <param name="comMilliseconds"></param>
        public void deleteEvent(string appKey, int eventID, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? comMilliseconds)
        {
            try
            {
                long webServiceTicks = Stopwatch.GetTimestamp();

                int comRuntimeWatchTotal = 0;
                int coreOperationTimeTotal = 0;
                int? unused = null;

                //check if system is running
                if (!isFMSystemRunning())
                {
                    errorString = "FaceMatch system is not available";
                    return;
                }

                ConcurrentBag<long> ticksDelete = new ConcurrentBag<long>();
                ConcurrentBag<int> comTimes = new ConcurrentBag<int>();

                try
                {
                    //validate
                    if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                        return;

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                    {
                        //event is not found or has no images ingested yet
                        errorString = "SUCCESS";
                        return;
                    }

                    try
                    {
                        //locked delete
                        int _coreOperationTime = 0;

                        ticksDelete.Add(Stopwatch.GetTimestamp());

                        double ticksCOM = Stopwatch.GetTimestamp();

                        ServiceStateManager.deleteEvent(appKey, eventID, ref errorString, ref _coreOperationTime);

                        ticksDelete.Add(Stopwatch.GetTimestamp());

                        double timeCOM = ((double)(Stopwatch.GetTimestamp() - ticksCOM) / (double)Stopwatch.Frequency) * 1000.0d;

                        comTimes.Add(Convert.ToInt32(timeCOM - _coreOperationTime));
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Exception occurred: " + ex.Message);
                        errorString = "Exception occurred: " + ex.Message;
                    }
                    finally
                    {
                        //measure total remove time across all ID's per second, convert to milliseconds
                        double saveTimeInSecs = 0;
                        int saveTimeInMilliseconds = 0;

                        if (ticksDelete.Count > 0)
                        {
                            saveTimeInSecs = (double)(ticksDelete.Max() - ticksDelete.Min()) / (double)Stopwatch.Frequency;
                            saveTimeInMilliseconds = Convert.ToInt32(saveTimeInSecs * 1000.0d);
                        }

                        coreOperationTimeTotal = saveTimeInMilliseconds;

                        if (comTimes.Count > 0)
                            comRuntimeWatchTotal = Convert.ToInt32(comTimes.Average());
                    }
                }
                finally
                {
                    stopTimersandMeasure(ref webServiceMilliseconds, ref unused, ref comMilliseconds, webServiceTicks, comRuntimeWatchTotal, coreOperationTimeTotal, null);
                }
            }
            finally
            {
                errorCode = setErrorCode(errorString);
                errorString = setSuccessStringOnNoError(errorString, errorCode);
            }
        }

        /// <summary>
        /// The version number of the underlying core
        /// </summary>
        /// <returns></returns>
        public string getVersion()
        {
            return "13434";
        }

        /// <summary>
        /// system status 
        /// </summary>
        public static bool isFMSystemRunning()
        {
            bool noExecutionFaults = true;
            bool isCoreRunning = false;
            bool isFMRunning = false;
            try
            {
                DateTime since = default(DateTime);
                String executionFaultMessage = String.Empty;

                //check if we have execution faults
                isCoreRunning = areFaceMatchProcessesRunning(ref since);
            }
            catch (Exception ex)
            {
                ServiceStateManager.Log("Exception in isFMSystemRunning() : " + ex.Message);
                Console.WriteLine(ex.Message);
            }
            finally
            {
                isFMRunning = isCoreRunning && noExecutionFaults;
            }

            return isFMRunning;
        }

        /// <summary>
        /// check if the FM service and the core are executing
        /// </summary>
        /// <param name="since"></param>
        /// <returns></returns>
        public static bool areFaceMatchProcessesRunning(ref DateTime since)
        {
            bool processesAreRunning = false;

            int serviceRunning = 0;
            int coreRunning = 0;

            DateTime dtService = default(DateTime);
            DateTime dtCore = default(DateTime);
            DateTime dtFirstProcess = default(DateTime);

            try
            {
                Process[] runningProcesses = Process.GetProcesses();
                if (runningProcesses != null)
                {
                    foreach (Process p in runningProcesses)
                    {
                        if (p != null && 
                            p.ProcessName != null &&
                            p.ProcessName.Equals("FaceMatcherService", StringComparison.OrdinalIgnoreCase))
                        {
                            serviceRunning = p.HasExited ? 0 : 1;
                            dtService = p.StartTime;
                        }

                        if (p != null &&
                            p.ProcessName != null &&
                            p.ProcessName.Equals("FaceMatcherCore", StringComparison.OrdinalIgnoreCase))
                        {
                            coreRunning = p.HasExited ? 0 : 1;
                            dtCore = p.StartTime;
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                ServiceStateManager.Log("Exception in areFaceMatchProcessesRunning() : " + ex.Message);
                Console.WriteLine(ex.Message);
            }
            finally
            {
                if (serviceRunning == 1 && coreRunning == 1)
                    processesAreRunning = true;

                //capture the earlier process start date
                if (dtService > dtCore)
                    dtFirstProcess = dtCore;
                else
                    dtFirstProcess = dtService;

                since = dtFirstProcess;
            }

            return processesAreRunning;
        }

        public static int setErrorCode(string errorString)
        {
            errorString = errorString.Trim();

            FaceMatchError errorCode = FaceMatchError.FM_UNKNOWN;

            if (errorString.Length == 0)
                errorCode = FaceMatchError.SUCCESS;
            else if (errorString.Equals("SUCCESS", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.SUCCESS;
            else if (errorString.Equals("FaceMatch system is not available", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.FM_NOT_AVAILABLE;
            else if (errorString.Equals("Invalid application key", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.INVALID_KEY;
            else if (errorString.Equals("Application key cannot be null", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.KEY_REQUIRED;
            else if (errorString.Equals("Could not download url specified", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.CANNOT_DOWNLOAD_URL;
            else if (errorString.Equals("url cannot be empty", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.URL_IS_EMPTY;
            else if (errorString.Equals("url cannot be null", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.URL_IS_NULL;
            else if (errorString.Equals("url is empty.-or- The scheme specified in uriString is not correctly formed.", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.URL_BAD_SCHEME;
            else if (errorString.Equals("invalid url specified", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.URL_IS_INVALID;
            else if (errorString.Equals("local files are not allowed. Please use a url.", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.LOCAL_FILE_URL_NOT_ALLOWED;
            else if (errorString.Equals("No face detected in input image", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.NO_FACE_DETECTED;
            else if (errorString.Equals("regions parameter cannot be null", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.REGIONS_PARAM_IS_NULL;
            else if (errorString.Equals("display regions parameter cannot be null", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.DISPLAY_REGS_PARAM_IS_NULL;
            else if (errorString.Equals("result parameter cannot be null", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.RESULT_PARAM_IS_NULL;
            else if (errorString.Equals("Unknown performance value string", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.BAD_PERFORMANCE_VALUE;
            else if (errorString.Equals("Event ID cannot be less than zero", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.ILLEGAL_EVENT_ID;
            else if (errorString.Equals("eventID not found or has no images ingested", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.EVENT_ID_NOT_FOUND;
            else if (errorString.Equals("inflate percentage < -1 is not valid (please choose a number between 0 and 1, for example 0.15 would mean inflate returned rectangle(s) by fifteen percent)", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.ILLEGAL_INFLATE_PERCENTAGE;
            else if (errorString.ToUpper().Contains("EXCEPTION"))
                errorCode = FaceMatchError.FM_EXCEPTION;
            else if (errorString.Equals("Invalid regions present, faces were not ingested", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.FM_INVALID_REGIONS;
            else if (
                errorString.Equals("The caller does not have the required permission.", StringComparison.OrdinalIgnoreCase) ||
                errorString.Equals("The file name is empty, contains only white spaces, or contains invalid characters.", StringComparison.OrdinalIgnoreCase) ||
                errorString.Equals("Access to fileName is denied.", StringComparison.OrdinalIgnoreCase) ||
                errorString.Equals("The specified path, file name, or both exceed the system-defined maximum length. For example, on Windows-based platforms, paths must be less than 248 characters, and file names must be less than 260 characters.", StringComparison.OrdinalIgnoreCase) ||
                errorString.Equals("fileName contains a colon (:) in the middle of the string.", StringComparison.OrdinalIgnoreCase) ||
                errorString.Equals("File does not exist or could not open the file.", StringComparison.OrdinalIgnoreCase))
                errorCode = FaceMatchError.FM_INTERNAL_ERROR;
            else
                errorCode = FaceMatchError.FM_UNKNOWN;

            return (int)errorCode;
        }

        public static bool user_isPL(string key)
        {
            return (key.Equals("PLV0h4pIchrogYUcW4FG98SwNeI=") ||
                key.Equals("PL") ||
                key.Equals("bHV0h4pIchrogYUcW4FG98SwNeI=") ||
                key.Equals("PLS"));
        }


        /// <summary>
        /// queries all databases for the image
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventIDs"></param>
        /// <param name="jsonResult"></param>
        /// <param name="urlRegs"></param>
        /// <param name="gender"></param>
        /// <param name="age"></param>
        /// <param name="tolerance"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        /// <param name="maxMatchesRequestedperEvent"></param>
        public void queryall(string appKey, int[] eventIDs, ref string jsonResult, string urlRegs, string gender, int age, float tolerance, ref string errorString, ref int errorCode, uint? maxMatchesRequestedperEvent)
        {
            if(jsonResult != null)
                jsonResult = string.Empty;

            IEnumerable<int> collections = new List<int>();
            if(eventIDs == null)
                collections = ServiceStateManager.getCollectionIDs(appKey);
            else
                collections = eventIDs;

            List<MultiEventQueryResult> queryResults = new List<MultiEventQueryResult>();
            
            foreach (var eventid in collections)
            {
                var r = new MultiEventQueryResult();
                
                string result = string.Empty;
                uint? matches = 0;
                int? webServiceMilliseconds = 0;
                int? imageCoreMilliseconds = 0;
                int? comMilliseconds = 0;
                int? urlFetchMilliseconds = 0;
                int? requestsOutstanding = 0;

                query(appKey, eventid, ref result, urlRegs, gender, age, tolerance, ref errorString, ref errorCode, maxMatchesRequestedperEvent, ref matches, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);

                if(errorCode == (int)FaceMatchError.SUCCESS)
                {
                    r.eventid = eventid;
                    r.result = result;
                    if(matches.HasValue)
                        r.matches = matches.Value;
                    if (webServiceMilliseconds.HasValue)
                        r.webServiceMilliseconds = webServiceMilliseconds.Value;
                    if (imageCoreMilliseconds.HasValue)
                        r.imageCoreMilliseconds = imageCoreMilliseconds.Value;
                    if (comMilliseconds.HasValue)
                        r.comMilliseconds = comMilliseconds.Value;
                    if (urlFetchMilliseconds.HasValue)
                        r.urlFetchMilliseconds = urlFetchMilliseconds.Value;
                    if (requestsOutstanding.HasValue)
                        r.requestsOutstanding = requestsOutstanding.Value;

                    queryResults.Add(r);
                }
                else
                {
                    return;
                }
            }

            if(queryResults.Count > 0 && jsonResult != null)
            {
                try
                {
                    jsonResult = JsonConvert.SerializeObject(queryResults, Formatting.Indented);
                }
                catch (Exception){}
            }
        }
    }

    public class MultiEventQueryResult
    {
        public int eventid { get; set; }
        public string result { get; set; }
        public uint matches { get; set; }
        public int webServiceMilliseconds { get; set; }
        public int imageCoreMilliseconds { get; set; }
        public int comMilliseconds { get; set; }
        public int urlFetchMilliseconds { get; set; }
        public int requestsOutstanding { get; set; }
    }
}
