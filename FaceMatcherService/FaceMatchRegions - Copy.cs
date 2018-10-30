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

namespace FaceMatcherService
{
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

        public FaceMatchRegions()
        {
            try
            {
                if(imr == null)
                    imr = new FaceMatcherCoreLib.CoreMatcherFaceRegionsMany();

                //save an interface reference for use by internal clients
                ServiceStateManager.svcFMR = this;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to instantiate CoreMatcherFaceRegionsMany class at com server: " + ex.Message);
            }
        }

        public void ingest(string appKey, int eventID, string url, string IDRegs, string gender, int age, ref string errorString, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            Stopwatch webServiceWatch = null;
            Stopwatch comRuntimeWatch = null;

            int coreOperationTime = 0;

            try
            {
                //start web service timer
                webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

                //validate and init
                if (ingestValidateInit(appKey, eventID, gender, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds) == false)
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

                        coreIngest(appKey, eventID, IDRegs, ref errorString, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, localPath, gender, age, url + '\t' + IDRegs);

                        return;
                    }

                    try
                    {
                        string file = "";
                        string url_image;
                        string[] split;

                        if (isEmptyUrl(url, ref errorString, out url_image, out split))
                            return;

                        requestsOutstanding = Interlocked.Increment(ref filePrefixIngest);

                        file = @"C:\FaceMatchSL\Facematch\bin\UploadBin\App." + appKey + ".event." + eventID + ".fri." + filePrefixIngest + "." + split[split.Length - 1];

                        WebClient wc = new WebClient();
                        Stopwatch urlFetch = new Stopwatch();
                        
                        //download file
                        urlFetch.Start();

                        downloadURLtoFile(file, wc, url_image);

                        urlFetch.Stop();

                        //measure download time
                        if (urlFetchMilliseconds != null)
                            urlFetchMilliseconds = Convert.ToInt32(urlFetch.ElapsedMilliseconds);

                        //check
                        fi = new FileInfo(file);

                        if (fi.Exists)
                            coreIngest(appKey, eventID, IDRegs, ref errorString, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, file, gender, age, url + '\t' + IDRegs);

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

        public void query(string appKey, int eventID, ref string result, string urlRegs, string gender, int age, float tolerance, ref string errorString, 
            uint? maxMatchesRequested, ref uint? matches, ref int? webServiceMilliseconds, 
            ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            Stopwatch webServiceWatch = null;
            Stopwatch comRuntimeWatch = null;

            int coreOperationTime = 0;

            try
            {
                //start web service timer
                webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

                //validate and init
                if (queryValidateInit(appKey, eventID, gender, ref result, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding) == false)
                    return;

                //compress tolerance parameter
                if(maxMatchesRequested != null && maxMatchesRequested.HasValue)
                    tolerance += maxMatchesRequested.Value;

                //replace key with app name
                appKey = getAppFromKey(appKey);

                //peek event
                if (!ServiceStateManager.PeekEvent(appKey, eventID, false))
                {
                    //event is not found or has no images ingested yet
                    errorString = "eventID not found or has no images ingested";
                    result = "";
                    if(matches != null)
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
                        queryCore(appKey, eventID, ref result, tolerance, ref errorString, ref matches, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, localPath + optionalROI, gender, age);

                        return;
                    }

                    try
                    {
                        string file = "";
                        string url_image;
                        string[] split;

                        if (isEmptyUrl(url, ref errorString, out url_image, out split))
                            return;

                        requestsOutstanding = Interlocked.Increment(ref filePrefixQuery);

                        file = @"C:\FaceMatchSL\Facematch\bin\UploadBin\App." + appKey + ".event." + eventID + ".frq." + filePrefixQuery + "." + split[split.Length - 1];


                        WebClient wc = new WebClient();
                        Stopwatch urlFetch = new Stopwatch();

                        //download file
                        urlFetch.Start();
                        downloadURLtoFile(file, wc, url_image);
                        urlFetch.Stop();

                        //measure download time
                        if (urlFetchMilliseconds != null)
                            urlFetchMilliseconds = Convert.ToInt32(urlFetch.ElapsedMilliseconds);

                        //check
                        fi = new FileInfo(file);
                        if (fi.Exists)
                            queryCore(appKey, eventID, ref result, tolerance, ref errorString, ref matches, ref comMilliseconds, ref comRuntimeWatch, ref coreOperationTime, file + optionalROI, gender, age);

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

        public void remove(string appKey, int eventID, string ID, ref uint records, ref string errorString, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? requestsOutstanding)
        {
            Stopwatch webServiceWatch = null;
            Stopwatch comRuntimeWatch = null;

            int coreOperationTime = 0;
            int? unused = null;

            try
            {
                //start web service timer
                webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

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
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, unused);
                    return;
                }

                try
                {
                    //start COM timer
                    comRuntimeWatch = startCOMtimer(comMilliseconds, comRuntimeWatch);

                    requestsOutstanding = Interlocked.Increment(ref removeConcurrency);

                    //operation
                    //need to parallelize remove
                    if(imr != null)
                        imr.remove(appKey, eventID, ID, ref records, ref errorString, ref coreOperationTime);
                    else
                        ServiceStateManager.Log("skipping call to imr.remove as the imr is null");


                    //stop COM timer
                    stopCOMtimer(comRuntimeWatch);

                    if (comMilliseconds != null)
                        comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
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
                }
            }
            finally
            {
                stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, unused);
            }
        }

        public void save(string appKey, int eventID, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds)
        {
            Stopwatch webServiceWatch = null;
            Stopwatch comRuntimeWatch = null;

            int coreOperationTime = 0;
            int? unused = null;

            try
            {
                //start web service timer
                webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

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
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, unused);
                    return;
                }

                try
                {
                    //start COM timer
                    comRuntimeWatch = startCOMtimer(comMilliseconds, comRuntimeWatch);

                    //operation
                    if(imr != null)
                        imr.save(appKey, eventID, ref errorString, ref coreOperationTime);
                    else
                        ServiceStateManager.Log("skipping call to imr.save as the imr is null");

                    //stop COM timer
                    stopCOMtimer(comRuntimeWatch);

                    if (comMilliseconds != null)
                        comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Exception occurred: " + ex.Message);
                    errorString = "Exception occurred: " + ex.Message;

                    ServiceStateManager.Log("Exception : exception caught in imr.save call details : " + ex.Message);
                }
            }
            finally
            {
                stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, unused);
            }
        }

        public void deleteEvent(string appKey, int eventID, ref string errorString, ref int? webServiceMilliseconds, ref int? comMilliseconds)
        {
            Stopwatch webServiceWatch = null;
            Stopwatch comRuntimeWatch = null;

            int coreOperationTime = 0;
            int? unused = null;

            try
            {
                //start web service timer
                webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

                //validate
                if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                    return;

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

                    //locked delete
                    ServiceStateManager.deleteEvent(appKey, eventID, ref errorString, ref coreOperationTime);

                    //stop COM timer
                    stopCOMtimer(comRuntimeWatch);

                    if (comMilliseconds != null)
                        comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Exception occurred: " + ex.Message);
                    errorString = "Exception occurred: " + ex.Message;
                }
            }
            finally
            {
                stopTimersandMeasure(ref webServiceMilliseconds, ref unused, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, unused);
            }
        }

        /// <summary>
        /// The version number of the underlying core
        /// </summary>
        /// <returns></returns>
        public string getVersion()
        {
            return "10635";
        }
    }
}
