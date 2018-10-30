using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;
using System.Runtime.Serialization;
using System.Timers;


namespace FaceMatcherService
{
    public static class EventManager
    {
        static string pathToConfigXML = @"c:\FaceMatchSL\FaceMatch\bin\fms.config.xml";

        //map of each application and its events
        static Dictionary<string, List<int>> map = null;

        static FaceMatcherCoreLib.CoreMatcherFaceRegionsMany imr = null;
        static FaceMatcherCoreLib.CoreImageMatcherWhole imw = null;

        static Timer saveTimer = null;

        //called first (auto) before any members are referenced
        static EventManager()
        {
            LoadEventXML();

            //indexes are auto saved every 30 minutes
            saveTimer = new Timer(1800000);
            saveTimer.Elapsed += new ElapsedEventHandler(saveTimer_Elapsed);
            saveTimer.Start();
        }

        private static void LoadEventXML()
        {
            try
            {
                map = new Dictionary<string, List<int>>();
                imr = new FaceMatcherCoreLib.CoreMatcherFaceRegionsMany();
                imw = new FaceMatcherCoreLib.CoreImageMatcherWhole();

                //deserialize from config file
                FileInfo fi = new FileInfo(pathToConfigXML);

                if (fi.Exists)
                {
                    //perform deserialization
                    XmlReader reader = XmlReader.Create(pathToConfigXML);
                    DataContractSerializer serializer = new DataContractSerializer(typeof(Dictionary<string, List<int>>));
                    map = (Dictionary<string, List<int>>)serializer.ReadObject(reader);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Exception occurred in EventManager loading of event information:" + ex.Message);
            }
        }

        public static void SaveEventXML()
        {
            lock (map)
            {
                try
                {
                    //serialize app and event information to disk
                    File.Delete(pathToConfigXML);
                    XmlWriter writer = XmlWriter.Create(pathToConfigXML);
                    DataContractSerializer serializer = new DataContractSerializer(typeof(Dictionary<string, List<int>>));
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
                foreach (string key in map.Keys)
                {
                    foreach (int eventid in map[key])
                    {
                        string errorString = "";
                        int loadTime = 0;

                        try
                        {
                            imr.initialize(key, eventid, ref errorString, ref loadTime, noGPUFromKey(key));
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager loading of region indexes :" + ex.Message);
                        }

                        try
                        {
                            imw.initialize(key, eventid, ref errorString, ref loadTime, noGPUFromKey(key));
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager loading of whole image indexes :" + ex.Message);
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
                    return map.ContainsKey(app) && map[app].Contains(eventID);
                }

                if (map.ContainsKey(app))
                {
                    if (map[app].Contains(eventID) == false)
                    {
                        map[app].Add(eventID);

                        //new event id needs to be saved
                        SaveEventXML();
                    }
                }
                else
                {
                    List<int> events = new List<int>();
                    events.Add(eventID);
                    map.Add(app, events);

                    //new event id needs to be saved
                    SaveEventXML();
                }
            }

            return true;
        }

        private static string getAppFromKey(string appKey)
        {
            if (appKey == "LMIr8vWeARADGRkTRbSybIjaBss=" || appKey.Equals("Openi", StringComparison.OrdinalIgnoreCase) == true)
                return "Openi";
            if (appKey == "bHV0h4pIchrogYUcW4FG98SwNeI=" || appKey.Equals("PL", StringComparison.OrdinalIgnoreCase) == true)
                return "PL";
            if (appKey == "noGPU4pIchrogYUcW4FG98SwNeI=" || appKey.Equals("NOGPU", StringComparison.OrdinalIgnoreCase) == true)
                return "NOGPU";

            return "Test";
        }

        private static int noGPUFromKey(string appKey)
        {
            if (appKey == "noGPU4pIchrogYUcW4FG98SwNeI=" || appKey.Equals("NOGPU", StringComparison.OrdinalIgnoreCase) == true)
                return 1;

            return 0;
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
                    foreach (int eventid in map[key])
                    {
                        string errorString = "";
                        int loadTime = 0;

                        try
                        {
                            imr.save(key, eventid, ref errorString, ref loadTime);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager saving of region indexes :" + ex.Message);
                        }

                        try
                        {
                            imw.save(key, eventid, ref errorString, ref loadTime);
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine("Exception occurred in EventManager saving of whole image indexes :" + ex.Message);
                        }
                    }
                }
            }
        }
    }
}
