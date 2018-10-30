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
using System.Text;
using System.IO;
using System.Xml;
using System.Runtime.Serialization;
using System.Timers;
using System.Threading;
using System.Runtime.InteropServices;
using System.Collections.Concurrent;


namespace FaceMatcherService
{
    public static class ServiceStateManager
    {
        static readonly string pathToConfigXML = @"c:\FaceMatchSL\FaceMatch\bin\fms.config.xml";
        static readonly string pathToAppBin = @"c:\FaceMatchSL\FaceMatch\bin\";

        //map of each application and its events
        static Dictionary<string, Dictionary<int, FaceMatchEventSettings>> map = null;

        static FaceMatcherCoreLib.CoreMatcherFaceRegionsMany imr = null;
        static FaceMatcherCoreLib.CoreImageMatcherWhole imw = null;

        static System.Timers.Timer saveTimer = null;
        static System.Timers.Timer logTimer = null;

        //set default index size to 4000 (need to use larger bucket size with FLANN)
        public const int MAX_BUCKET_SIZE = 4000;

        //create a bucket manager
        public static readonly IApplicationBucket bucketManager = new ApplicationBucketManager();

        //keep a pointer to our FaceFinder
        public static FaceFinder faceFinder = null;

        public static IFaceMatchRegions svcFMR = null;

        public static IWholeImageMatcher svcWIM = null;

        public static Object applicationLogWriter = new Object();

        public static ConcurrentQueue<string> logMessagesQueue = new ConcurrentQueue<string>();

        public static AutoResetEvent logEvent = new AutoResetEvent(false);

        //called first (auto) before any members are referenced
        static ServiceStateManager()
        {
            CreateDirectories();
            LoadEventXML();

            //indexes are auto saved every 30 minutes
            saveTimer = new System.Timers.Timer(1800000);
            saveTimer.Elapsed += new ElapsedEventHandler(saveTimer_Elapsed);
            saveTimer.Start();

            //messages are logged every 5 minutes
            logTimer = new System.Timers.Timer(300000);
            logTimer.Elapsed += new ElapsedEventHandler(logTimer_Elapsed);
            logTimer.Start();
        }

        //triggers a log event every 5 minutes
        private static void logTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            logEvent.Set();
        }

        /// <summary>
        /// creates the necessary paths on startup
        /// </summary>
        private static void CreateDirectories()
        {
            String[] dirsNeeded = new String[] { "UploadBin", "Logs", "Problematic", "Data" };
            foreach (String item in dirsNeeded)
            {
                try
                {
                    String dir = pathToAppBin + item;
                    if (!Directory.Exists(dir))
                        Directory.CreateDirectory(dir);
                }
                catch (Exception){
                    //do nothing
                }
            }

            foreach (String value in KeyManager.keys.Values)
            {
                try
                {
                    String dir = pathToAppBin + @"Data\" + value;
                    if (!Directory.Exists(dir))
                        Directory.CreateDirectory(dir);
                }
                catch (Exception){
                    //do nothing
                }
            }
        }

        private static void LoadEventXML()
        {
            try
            {
                imr = new FaceMatcherCoreLib.CoreMatcherFaceRegionsMany();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception: Failed to get a reference to the CoreMatcherFaceRegionsMany details : " + ex.Message);
                ServiceStateManager.Log("Exception: Failed to get a reference to the CoreMatcherFaceRegionsMany from COM details : " + ex.Message);
            }
            finally
            {
                if (imr == null)
                    ServiceStateManager.Log("the ServiceStateManager.imr reference is null.");
            }

            try
            {
                imw = new FaceMatcherCoreLib.CoreImageMatcherWhole();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception: Failed to get a reference to the CoreImageMatcherWhole details : " + ex.Message);
                ServiceStateManager.Log("Exception: Failed to get a reference to the CoreImageMatcherWhole from COM details : " + ex.Message);
            }
            finally
            {
                if (imw == null)
                    ServiceStateManager.Log("the ServiceStateManager.imw reference is null.");
            }

            try
            {
                map = new Dictionary<string, Dictionary<int, FaceMatchEventSettings>>();

                //deserialize from config file
                FileInfo fi = new FileInfo(pathToConfigXML);

                if (fi.Exists)
                {
                    //perform deserialization
                    XmlReader reader = XmlReader.Create(pathToConfigXML);
                    DataContractSerializer serializer = new DataContractSerializer(map.GetType());
                    map = (Dictionary<string, Dictionary<int, FaceMatchEventSettings>>)serializer.ReadObject(reader);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception occurred in EventManager loading of event information:" + ex.Message);
                ServiceStateManager.Log("Exception occurred in EventManager loading of event information:" + ex.Message);
            }
        }

        public static void SaveEventXML()
        {
            lock (map)
            {
                try
                {
                    //rem
                    //consider using : using

                    //serialize app and event information to disk
                    File.Delete(pathToConfigXML);
                    XmlWriter writer = XmlWriter.Create(pathToConfigXML);
                    DataContractSerializer serializer = new DataContractSerializer(map.GetType());
                    serializer.WriteObject(writer, map);
                    writer.Flush();
                    writer.Close();
                }
                catch (Exception ex)
                {
                    Console.WriteLine("Exception occurred in EventManager saving of event information:" + ex.Message);
                }
            }
        }

        public static void LoadIndexes()
        {
            lock (map)
            {
                //for debug
                //Thread.Sleep(30000);

                foreach (string key in map.Keys)
                {
                    foreach (int eventID in map[key].Keys)
                    {
                        string errorString = "";
                        int loadTime = 0;
                        string overflowListing = "";

                        try
                        {
                            //noGPUFromKey is for debug purposess that will disable GPU when service is called with a special key
                            if(imr != null)
                                imr.initialize(key, eventID, ref overflowListing, MAX_BUCKET_SIZE, ref errorString, ref loadTime, useGPUFromKey(key), map[key][eventID].PerformanceValue);
                            else
                                ServiceStateManager.Log("skipping call to imr.initialize as the imr is null");

                            if (overflowListing.Length > 0)
                                bucketManager.syncOverflowBucketCountsForEvent(key, eventID, overflowListing);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager loading of region indexes :" + ex.Message);
                            ServiceStateManager.Log("Exception occurred in EventManager loading of region indexes :" + ex.Message);
                        }

                        try
                        {
                            overflowListing = "";

                            if(imw != null)
                                imw.initialize(key, eventID, ref overflowListing, MAX_BUCKET_SIZE, ref errorString, ref loadTime, useGPUFromKey(key), map[key][eventID].PerformanceValue);
                            else
                                ServiceStateManager.Log("skipping call to imw.initialize as the imw is null");

                            //ignore report back as there is no de-dup functionality consensus for version 0.1
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager loading of whole image indexes :" + ex.Message);
                            ServiceStateManager.Log("Exception occurred in EventManager loading of whole image indexes :" + ex.Message);
                        }
                    }
                }
            }
        }

        public static bool PeekEvent(string regKey, int eventID, bool isIngest)
        {
            lock (map)
            {
                string app = getAppFromKey(regKey);

                if (!isIngest)
                {
                    return map.ContainsKey(app) && map[app].ContainsKey(eventID);
                }

                if (map.ContainsKey(app))
                {
                    if (map[app].ContainsKey(eventID) == false)
                    {
                        FaceMatchEventSettings evs = new FaceMatchEventSettings();
                        evs.eventID = eventID;
                        evs.PerformanceString = FaceMatchEventSettings.Optimal;

                        map[app][eventID] = evs;

                        //new event id needs to be saved
                        SaveEventXML();
                    }
                }
                else
                {
                    Dictionary<int, FaceMatchEventSettings> evsLookup = new Dictionary<int, FaceMatchEventSettings>();
                    
                    map[app] = evsLookup;

                    FaceMatchEventSettings evs = new FaceMatchEventSettings();
                    evs.eventID = eventID;
                    evs.PerformanceString = FaceMatchEventSettings.Optimal;

                    map[app][eventID] = evs;

                    //new event id needs to be saved
                    SaveEventXML();
                }
            }

            return true;
        }

        public static IEnumerable<int> getCollectionIDs(string regKey)
        {
            lock (map)
            {
                string app = getAppFromKey(regKey);

                if (map.ContainsKey(app))
                {
                    return map[app].Keys;
                }
            }

            return null;
        }

        public static bool modifyCollectionPerformance(string appKey, int collectionID, string performanceValue, ref string errorString)
        {
            lock (map)
            {
                try
                {
                    map[appKey][collectionID].PerformanceString = performanceValue;

                    //save new performance setting
                    SaveEventXML();

                    errorString = "SUCCESS";
                }
                catch (Exception)
                {
                    errorString = "Exception occurred : could not set performance option";

                    return false;
                }
            }

            return true;
        }

        public static int getFaceFinderPerformanceValue(string appKey, int collectionID)
        {
            lock (map)
            {
                try
                {
                    if(map.ContainsKey(appKey) && map[appKey].ContainsKey(collectionID))
                        return map[appKey][collectionID].PerformanceValue;
                }
                catch (Exception)
                {
                }
            }

            //default to optimum performance
            return 1;
        }

        public static bool deleteEvent(string regKey, int eventID, ref string errorString, ref int coreOperationTime)
        {
            lock (map)
            {
                string app = getAppFromKey(regKey);

                if (map.ContainsKey(app))
                {
                    if (map[app].ContainsKey(eventID))
                    {
                        map[app].Remove(eventID);

                        //modified map needs to be saved
                        SaveEventXML();

                        //operation
                        try
                        {
                            if (imr != null)
                                imr.releaseEventDatabase(app, eventID, ref errorString, ref coreOperationTime);
                            else
                                ServiceStateManager.Log("skipping call to releaseEventDatabase as the imr reference is null");
                        }
                        catch (Exception ex)
                        {
                            ServiceStateManager.Log("Exception : the imr.releaseEventDatabase com server call returned an exception : details : " + ex.Message);
                        }

                        //delete all underlying files that match the following name
                        //App.PL.Female.Adult.1.many.eventid.1002.SURF

                        string searchPattern = "App." + app + ".*.eventid." + eventID + ".*";

                        foreach (string file in Directory.EnumerateFiles(pathToAppBin, searchPattern))
                            File.Delete(file);

                        //release associated buckets for eventID
                        bucketManager.UnRegisterEvent(app, eventID);
                    }
                }
            }

            return true;
        }

        private static bool isValidAppKey(string appKey)
        {
            return KeyManager.isValidAppKey(appKey);
        }

        private static string getAppFromKey(string appKey)
        {
            return KeyManager.getAppFromKey(appKey);
        }

        private static int useGPUFromKey(string appKey)
        {
            return KeyManager.useGPUFromKey(appKey);
        }

        static void saveTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            saveTimer.Stop();

            SaveIndexes();
            
            saveTimer.Start();
        }

        public static void SaveIndexes()
        {
            lock (map)
            {
                foreach (string key in map.Keys)
                {
                    foreach (int eventid in map[key].Keys)
                    {
                        string errorString = "";
                        int loadTime = 0;

                        try
                        {
                            if(imr != null)
                                imr.save(key, eventid, ref errorString, ref loadTime);
                            else
                                ServiceStateManager.Log("skipping call to imr.save as the imr is null");
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager saving of region indexes :" + ex.Message);
                            ServiceStateManager.Log("Exception : exception caught in imr.save call details : " + ex.Message);
                        }

                        try
                        {
                            if(imw != null)
                                imw.save(key, eventid, ref errorString, ref loadTime);
                            else
                                ServiceStateManager.Log("skipping call to imw.save as the imw is null");
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager saving of whole image indexes :" + ex.Message);
                            ServiceStateManager.Log("Exception : exception caught in imw.save call details : " + ex.Message);
                        }
                    }
                }
            }
        }

        public static void Log(string message)
        {
            //string logFileName = @"C:\FaceMatchSL\FaceMatch\bin\Logs\facematcherservice.log.txt";
            //lock (applicationLogWriter)
            //{
            //    using (StreamWriter writer = File.AppendText(logFileName))
            //    {
            //        writer.WriteLine("FaceMatchWebServices : " + DateTime.Now + " : " + message);
            //        writer.Flush();
            //    }

            //    try
            //    {
            //        FileInfo fi = new FileInfo(logFileName);
            //        //10MB
            //        if (fi.Length > 10485760)
            //        {
            //            //truncate to last 2 MB or so worth of data lines
            //            List<string> lines = new List<string>(File.ReadAllLines(logFileName));
            //            int skipLines = Convert.ToInt32((double)lines.Count * 0.8d);
            //            File.WriteAllLines(logFileName, lines.Skip(skipLines));
            //        }
            //    }
            //    catch (Exception) { }
            //    finally{ }
            //}

            string logMessage = string.Empty;

            if (message.Length > 0)
                logMessage = "FaceMatchWebServices : " + DateTime.Now + " : " + message;

            logMessagesQueue.Enqueue(logMessage);
        }

        public static void FaceMatchWebServiceLogger()
        {
            while(true)
            {
                try
                {
                    //write out log every 5 minutes or when the service is shutting down
                    logEvent.WaitOne();

                    const string logFileName = @"C:\FaceMatchSL\FaceMatch\bin\Logs\facematcherservice.log.txt";
                    
                    using (StreamWriter writer = File.AppendText(logFileName))
                    {
                        string message = string.Empty;

                        while (logMessagesQueue.TryDequeue(out message))
                        {
                            writer.WriteLine(message);
                            message = string.Empty;
                        }

                        writer.Flush();
                    }

                    try
                    {
                        FileInfo fi = new FileInfo(logFileName);
                        //10MB
                        if (fi.Length > 10485760)
                        {
                            //truncate to last 2 MB or so worth of data lines
                            List<string> lines = new List<string>(File.ReadAllLines(logFileName));
                            int skipLines = Convert.ToInt32((double)lines.Count * 0.8d);
                            File.WriteAllLines(logFileName, lines.Skip(skipLines));
                        }
                    }
                    catch (Exception) { }
                    finally { }

                    try
                    {
                        //next write out the image core log messages
                        if (imr != null)
                            imr.flushLog();
                    }
                    finally { }

                    //signal service host that we can safely exit
                    if (Service1.shutDownTaskCompletion != null)
                    {
                        Service1.shutDownTaskCompletion.Set();
                        break;
                    }
                }
                catch (Exception){ }
                finally{ }
            }

        }

    }
}
