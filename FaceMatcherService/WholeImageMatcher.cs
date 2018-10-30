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
using System.IO;
using System.Net;
using System.Security;
using System.ServiceModel;
using System.Threading;
using System.Diagnostics;

namespace FaceMatcherService
{
    [ServiceBehavior(
        ConcurrencyMode = ConcurrencyMode.Multiple,
        InstanceContextMode = InstanceContextMode.Single
        )]
    public class WholeImageMatcher : IWholeImageMatcher
    {
        FaceMatcherCoreLib.CoreImageMatcherWhole imw = null;

        static int filePrefixIngest = 0;
        static int filePrefixQuery = 0;
        static int removeConcurrency = 0;

        public WholeImageMatcher()
        {
            try
            {
                imw = new FaceMatcherCoreLib.CoreImageMatcherWhole();

                //save an interface reference for use by internal clients
                ServiceStateManager.svcWIM = this;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to instantiate CoreImageMatcherWhole class at com server: " + ex.Message);
            }
        }

        public void ingest(string appKey, int eventID, string url, string ID, ref string errorString, ref int errorCode, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            try
            {
                Stopwatch webServiceWatch = null;
                Stopwatch comRuntimeWatch = null;

                int coreOperationTime = 0;

                try
                {
                    //start web service timer
                    webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

                    //validate and init
                    if (ingestValidateInit(appKey, eventID, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds) == false)
                        return;

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
                        string localPath = "";

                        if (uri.IsFile)
                        {
                            GetLocalFileInfo(url, ref errorString, uri, ref fi, ref localPath);

                            if (errorString.Length != 0)
                                return;

                            coreIngest(appKey, eventID, ID, ref errorString, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, localPath, url);

                            return;
                        }

                        try
                        {
                            string file = "";
                            string url_image;
                            string[] split;

                            if (isEmptyUrl(url, ref errorString, out url_image, out split))
                                return;

                            int oustanding = Interlocked.Increment(ref filePrefixIngest);

                            if (requestsOutstanding != null)
                                requestsOutstanding = oustanding;

                            file = @"C:\FaceMatchSL\Facematch\bin\UploadBin\App." + appKey + ".event." + eventID + ".wii." + filePrefixIngest + "." + split[split.Length - 1];

                            WebClient wc = new WebClient();
                            Stopwatch urlFetch = new Stopwatch();

                            //download file
                            urlFetch.Start();
                            downloadNewFile(file, wc, url_image);
                            urlFetch.Stop();

                            //measure download time
                            if (urlFetchMilliseconds != null)
                                urlFetchMilliseconds = Convert.ToInt32(urlFetch.ElapsedMilliseconds);

                            //check
                            fi = new FileInfo(file);

                            if (fi.Exists)
                            {
                                coreIngest(appKey, eventID, ID, ref errorString, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, file, url);
                            }

                            //delete file
                            fi = eraseDeleteProcessedFile(fi, file);
                        }
                        catch (Exception)
                        {
                            errorString = "Could not download url specified";
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
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, urlFetchMilliseconds);
                }

            }
            finally
            {
                errorCode = FaceMatchRegions.setErrorCode(errorString);
            }
        }

        public void query(string appKey, int eventID, ref string result, string url, float tolerance, ref string errorString, ref int errorCode, 
            uint? maxMatchesRequested, ref uint? matches, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            try
            {
                Stopwatch webServiceWatch = null;
                Stopwatch comRuntimeWatch = null;

                int coreOperationTime = 0;

                try
                {
                    //start web service timer
                    webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

                    //validate and init
                    if (queryValidateInit(appKey, eventID, result, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding) == false)
                        return;

                    //compress tolerance parameter
                    if (maxMatchesRequested != null && maxMatchesRequested.HasValue)
                        tolerance += maxMatchesRequested.Value;

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                    {
                        //event is not found or has no images ingested yet
                        result = "";
                        if (matches != null)
                            matches = 0;
                        return;
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
                            queryCore(appKey, eventID, ref result, tolerance, ref errorString, ref matches, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, localPath, url);

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

                            file = @"C:\FaceMatchSL\Facematch\bin\UploadBin\App." + appKey + ".event." + eventID + ".wiq." + filePrefixQuery + "." + split[split.Length - 1];


                            WebClient wc = new WebClient();
                            Stopwatch urlFetch = new Stopwatch();

                            //download file
                            urlFetch.Start();
                            downloadNewFile(file, wc, url_image);
                            urlFetch.Stop();

                            //measure download time
                            if (urlFetchMilliseconds != null)
                                urlFetchMilliseconds = Convert.ToInt32(urlFetch.ElapsedMilliseconds);

                            //check
                            fi = new FileInfo(file);
                            if (fi.Exists)
                            {
                                //query
                                queryCore(appKey, eventID, ref result, tolerance, ref errorString, ref matches, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, file, url);
                            }

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
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, urlFetchMilliseconds);
                }

            }
            finally
            {
                errorCode = FaceMatchRegions.setErrorCode(errorString);
            }
        }

        public void remove(string appKey, int eventID, string ID, ref uint records, ref string errorString, ref int errorCode, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            try
            {
                Stopwatch webServiceWatch = null;
                Stopwatch comRuntimeWatch = null;

                int coreOperationTime = 0;

                try
                {
                    //start web service timer
                    webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

                    //validate
                    if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                        return;

                    //zero out timing vars
                    initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds);

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                    {
                        //event is not found or has no images ingested yet
                        return;
                    }

                    try
                    {
                        //start COM timer
                        comRuntimeWatch = startCOMtimer(comMilliseconds, comRuntimeWatch);

                        int concurrency = Interlocked.Increment(ref removeConcurrency);

                        if (requestsOutstanding != null)
                            requestsOutstanding = concurrency;

                        //operation
                        if (imw != null)
                            imw.remove(appKey, eventID, ID, ref errorString, ref coreOperationTime);
                        else
                            ServiceStateManager.Log("skipping call to imw.remove as the imw is null");


                        //stop COM timer
                        stopCOMtimer(comRuntimeWatch);

                        if (comMilliseconds != null)
                            comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Exception occurred: " + ex.Message);
                        errorString = "Exception occurred: " + ex.Message;
                        ServiceStateManager.Log("Exception : exception caught in imw.remove call details : " + ex.Message);
                    }
                    finally
                    {
                        Interlocked.Decrement(ref removeConcurrency);
                    }
                }
                finally
                {
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, urlFetchMilliseconds);
                }

            }
            finally
            {
                errorCode = FaceMatchRegions.setErrorCode(errorString);
            }
        }

        public void save(string appKey, int eventID, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds)
        {
            try
            {
                Stopwatch webServiceWatch = null;
                Stopwatch comRuntimeWatch = null;

                int coreOperationTime = 0;

                try
                {
                    //start web service timer
                    webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

                    //validate
                    if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                        return;

                    //zero out timing vars
                    int? dummy1 = null;
                    initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref dummy1);

                    //replace key with app name
                    appKey = getAppFromKey(appKey);

                    //peek event
                    if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                    {
                        //event is not found or has no images ingested yet
                        return;
                    }

                    try
                    {
                        //start COM timer
                        comRuntimeWatch = startCOMtimer(comMilliseconds, comRuntimeWatch);

                        //operation
                        if (imw != null)
                            imw.save(appKey, eventID, ref errorString, ref coreOperationTime);
                        else
                            ServiceStateManager.Log("skipping call to imw.save as imw is null ");


                        //stop COM timer
                        stopCOMtimer(comRuntimeWatch);

                        if (comMilliseconds != null)
                            comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine("Exception occurred: " + ex.Message);
                        errorString = "Exception occurred: " + ex.Message;
                        ServiceStateManager.Log("Exception : exception caught in imw.save call details : " + ex.Message);
                    }
                }
                finally
                {
                    int? urlFetchMilliseconds = null;
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, urlFetchMilliseconds);
                }

            }
            finally
            {
                errorCode = FaceMatchRegions.setErrorCode(errorString);
            }
        }

        private void downloadNewFile(string file, WebClient wc, string url_image)
        {
            //delete existing copy
            File.Delete(file);

            //download the file
            wc.DownloadFile(url_image, file);
        }

        private void coreIngest(string appKey, int eventID, string ID, ref string errorString, ref int? comMilliseconds, ref Stopwatch comRuntimeWatch, ref int coreOperationTime, string localPath, string url)
        {
            //start COM timer
            comRuntimeWatch = startCOMtimer(comMilliseconds, comRuntimeWatch);

            //operation
            try
            {
                if (imw == null)
                    ServiceStateManager.Log("skipping call to imw.ingest as the imw reference is null");
                else
                {
                    string debugInfoForCoreLog = string.Empty;
                    debugInfoForCoreLog = "\r\nurl: " + url;
                    debugInfoForCoreLog += "\r\nID [in]: " + ID;
                    debugInfoForCoreLog += "\r\neventID [in]: " + eventID;
                    debugInfoForCoreLog += "\r\nclient [in]: " + appKey;

                    imw.ingest(appKey, eventID, localPath, ID, ref errorString, ref coreOperationTime, debugInfoForCoreLog, useGPUFromKey(appKey), 0);
                }
            }
            catch (Exception ex)
            {
                ServiceStateManager.Log("Exception : the call to imw.ingest returned an exception details : " + ex.Message);
            }

            //stop COM timer
            stopCOMtimer(comRuntimeWatch);

            if (comMilliseconds != null)
                comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
        }

        private bool isEmptyUrl(string url, ref string errorString, out string url_image, out string[] split)
        {
            url_image = url;
            char[] sep = new char[] { '/' };
            split = url_image.Split(sep, StringSplitOptions.RemoveEmptyEntries);

            if (split != null && split.Length == 0)
            {
                errorString = "invalid url specified";
                return true;
            }

            return false;
        }

        private void queryCore(string appKey, int eventID, ref string result, float tolerance, ref string errorString, ref uint? matches, ref int? comMilliseconds, ref Stopwatch comRuntimeWatch, ref int coreOperationTime, string localPath, string url)
        {
            //start COM timer
            comRuntimeWatch = startCOMtimer(comMilliseconds, comRuntimeWatch);

            //operation
            uint hits = 0;
            try
            {
                string debugInfoForCoreLog = string.Empty;
                debugInfoForCoreLog = "\r\nurl [in]: " + url;
                debugInfoForCoreLog += "\r\neventID [in]: " + eventID;
                debugInfoForCoreLog += "\r\nclient [in]: " + appKey;
                debugInfoForCoreLog += "\r\nlocalPath value : " + localPath;

                if (imw == null)
                    ServiceStateManager.Log("skipping call to imw.query as the imw reference is null");
                else
                    hits = imw.query(appKey, eventID, ref result, localPath, tolerance, ref errorString, ref coreOperationTime, debugInfoForCoreLog);
            }
            catch (Exception ex)
            {
                ServiceStateManager.Log("Exception : the call to imw.query returned an exception details : " + ex.Message);
            }

            if (matches != null)
                matches = hits;

            //stop COM timer
            stopCOMtimer(comRuntimeWatch);

            if (comMilliseconds != null)
                comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
        }

        private bool validateUrl(string url, ref string errorString, ref Uri uri)
        {
            if (url.Length == 0)
            {
                errorString = "url cannot be empty";
                return false;
            }

            try
            {
                uri = new Uri(url);
            }
            catch (ArgumentNullException)
            {
                errorString = "url cannot be null";
                return false;
            }
            catch (UriFormatException)
            {
                errorString = "url is empty.-or- The scheme specified in uriString is not correctly formed.";
                return false;
            }

            return true;
        }

        private bool queryValidateInit(string appKey, int eventID, string result, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            //validate
            if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                return false;

            //result expected
            if (result == null)
            {
                errorString = "result parameter cannot be null";
                return false;
            }

            //zero out timing vars
            initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds);

            return isValidAppKey(appKey);
        }

        private bool ingestValidateInit(string appKey, int eventID, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds)
        {
            //validate
            if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                return false;

            //zero out timing vars
            initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds);

            return isValidAppKey(appKey);
        }

        private bool validateKeyEventErrorParams(string appKey, int eventID, ref string errorString)
        {
            if (errorString == null)
                return false;

            errorString = "";

            if (appKey.Length == 0)
            {
                errorString = "Application key cannot be null";
                return false;
            }

            if (eventID == -1)
            {
                errorString = "Event ID cannot be less than zero";
                return false;
            }

            return isValidAppKey(appKey);
        }

        private void stopCOMtimer(Stopwatch comRuntimeWatch)
        {
            if (comRuntimeWatch != null)
                comRuntimeWatch.Stop();
        }

        private Stopwatch startCOMtimer(int? comMilliseconds, Stopwatch comRuntimeWatch)
        {
            if (comMilliseconds != null)
            {
                comRuntimeWatch = new Stopwatch();
                comRuntimeWatch.Start();
            }
            return comRuntimeWatch;
        }

        private Stopwatch startWebServiceTimer(int? webServiceMilliseconds, Stopwatch webServiceWatch)
        {
            if (webServiceMilliseconds != null)
            {
                webServiceWatch = new Stopwatch();
                webServiceWatch.Start();
            }
            return webServiceWatch;
        }

        private void stopTimersandMeasure(ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds,
            Stopwatch webServiceWatch, Stopwatch comRuntimeWatch, int coreOperationTime, int? urlFetchMilliseconds)
        {
            //stop timers
            if (webServiceWatch != null &&
                webServiceWatch.IsRunning)
                webServiceWatch.Stop();
            if (comRuntimeWatch != null &&
                comRuntimeWatch.IsRunning)
                comRuntimeWatch.Stop();

            //measure composite intervals
            if (webServiceWatch != null &&
                webServiceMilliseconds != null)
                webServiceMilliseconds = Convert.ToInt32(webServiceWatch.ElapsedMilliseconds);
            if (comRuntimeWatch != null &&
                comMilliseconds != null)
                comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);

            //breakdown components
            if (comMilliseconds != null &&
                comMilliseconds != 0)
                comMilliseconds -= coreOperationTime;

            if (webServiceMilliseconds != null)
            {
                webServiceMilliseconds -= coreOperationTime;
                webServiceMilliseconds -= comMilliseconds;

                //subtract time spent in downloading file
                if (urlFetchMilliseconds != null)
                    webServiceMilliseconds -= urlFetchMilliseconds;
            }
            if (imageCoreMilliseconds != null)
                imageCoreMilliseconds = coreOperationTime;
        }

        private void initTimingVars(ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds)
        {
            if (webServiceMilliseconds != null)
                webServiceMilliseconds = 0;
            if (imageCoreMilliseconds != null)
                imageCoreMilliseconds = 0;
            if (comMilliseconds != null)
                comMilliseconds = 0;
            if (urlFetchMilliseconds != null)
                urlFetchMilliseconds = 0;
        }

        private bool isValidAppKey(string appKey)
        {
            return KeyManager.isValidAppKey(appKey);
        }

        private string getAppFromKey(string appKey)
        {
            return KeyManager.getAppFromKey(appKey);
        }

        private int useGPUFromKey(string appKey)
        {
            return KeyManager.useGPUFromKey(appKey);
        }

        private FileInfo eraseDeleteProcessedFile(FileInfo fi, string file)
        {
            //clean up
            fi = new FileInfo(file);
            if (fi.Exists)
            {
                byte[] zeroBlock = new byte[file.Length];
                for (int i = 0; i < zeroBlock.Length; i++)
                    zeroBlock[i] = 0;
                File.WriteAllBytes(file, zeroBlock);
                File.Delete(file);
            }
            return fi;
        }

        private void GetLocalFileInfo(string url, ref string ErrorString, Uri uri, ref FileInfo fi, ref string localPath)
        {
            try
            {
                localPath = uri.LocalPath;
                fi = new FileInfo(url);
            }
            catch (ArgumentNullException)
            {
                ErrorString = "url cannot be null";
            }
            catch (SecurityException)
            {
                ErrorString = "The caller does not have the required permission.";
            }
            catch (ArgumentException)
            {
                ErrorString = "The file name is empty, contains only white spaces, or contains invalid characters.";
            }
            catch (UnauthorizedAccessException)
            {
                ErrorString = "Access to fileName is denied.";
            }
            catch (PathTooLongException)
            {
                ErrorString = "The specified path, file name, or both exceed the system-defined maximum length. For example, on Windows-based platforms, paths must be less than 248 characters, and file names must be less than 260 characters.";
            }
            catch (NotSupportedException)
            {
                ErrorString = "fileName contains a colon (:) in the middle of the string.";
            }

            if (fi.Exists == false)
            {
                ErrorString = "File does not exist or could not open the file.";
            }
        }
    }
}
