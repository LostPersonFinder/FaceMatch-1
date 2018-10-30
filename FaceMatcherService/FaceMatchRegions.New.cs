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

            int comRuntimeWatchTotal = 0;
            int coreOperationTimeTotal = 0;

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
                        {
                            //add the GT parsing logic here
                            List<IngestData> ingestList = parseGT(IDRegs, ref gender, age, ref errorString);
                            if (ingestList != null)
                            {
                                comRuntimeWatchTotal = 0;
                                coreOperationTimeTotal = 0;

                                foreach (IngestData ingestRegion in ingestList)
                                {
                                    int? comMillisecondsRegion = 0; 
                                    int  coreOperationTimeRegion = 0;

                                    coreIngest(appKey, eventID, ingestRegion.IDRegs, ref errorString, ref comMillisecondsRegion, ref comRuntimeWatch, ref coreOperationTimeRegion, file, ingestRegion.Gender, ingestRegion.Age);

                                    //added this for GT ingest
                                    if (comRuntimeWatch != null)
                                    {
                                        if(comRuntimeWatch.IsRunning)
                                            comRuntimeWatch.Stop();
                                        
                                        //accumulate total time
                                        comRuntimeWatchTotal += Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
                                        comRuntimeWatch = null;
                                    }

                                    coreOperationTimeTotal += coreOperationTimeRegion;
                                }
                            }
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
                stopTimersandMeasure_Ingest(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatchTotal, coreOperationTimeTotal, urlFetchMilliseconds);
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

        private List<IngestData> parseGT(String idRegsIn, ref String genderIn, int ageIn, ref String errorString)
        {
            String errorsIn = errorString;
            List<IngestData> ingestList = new List<IngestData>();
            try
            {
                char[] tabSep = new char[] { '\t' };
                char[] closingBrace = new char[] { '}' };

                //check idRegs type
                //base
                //1. no regions 
                //2. no age, gender or skin
                if (!idRegsIn.Contains('{') && !idRegsIn.Contains('}') &&
                    !idRegsIn.Contains('[') && !idRegsIn.Contains(']')
                    )
                {
                    //just the id, no other information given
                    ingestList.Add(new IngestData() { Age = ageIn, Gender = genderIn, ID = idRegsIn, Regs = String.Empty });
                    return ingestList;
                }

                String[] splits = idRegsIn.Split(tabSep, StringSplitOptions.RemoveEmptyEntries);
                String _ID = splits[0];

                const String rxFace = @"(p|f)\{\[[0-9]+,[0-9]+;[0-9]+,[0-9]+\]";
                const String rxINME = @"([inme])\[[0-9]+,[0-9]+;[0-9]+,[0-9]+\]";
                const String rxSkin = @"t\[(dark|light)]";
                const String rxAge = @"a\[(youth|adult)]";
                const String rxGender = @"g\[(male|female)]";

                Regex regExFace = new Regex(rxFace);
                Regex regExINME = new Regex(rxINME);
                Regex regExSkin = new Regex(rxSkin);
                Regex regExAge = new Regex(rxAge);
                Regex regExGender = new Regex(rxGender);

                Random rand = new Random();

                //check idRegs type
                //base + attributes
                //1. no regions 
                //2. has age, gender or skin
                if (!idRegsIn.Contains('{') && !idRegsIn.Contains('}') &&
                    idRegsIn.Contains('[') && idRegsIn.Contains(']')
                    )
                {
                    //just the id, no other information given
                    IngestData data = new IngestData() { Age = ageIn, Gender = genderIn, ID = _ID, Regs = String.Empty };

                    HashSet<String> gtErrors = new HashSet<string>();

                    int numRegions = 0;
                    int numSkin = 0;
                    int numAge = 0;
                    int numGender = 0;

                    for (int i = 1; i < splits.Length; i++)
                    {
                        //ignore white spaces
                        if (splits[i].Trim().Length == 0)
                            continue;

                        //update region
                        if (regExINME.IsMatch(splits[i]))
                        {
                            numRegions++;
                            break;
                        }

                        //filter skin
                        else if (regExSkin.IsMatch(splits[i]))
                        {
                            numSkin++;
                            continue;
                        }

                        //save age
                        else if (regExAge.IsMatch(splits[i]))
                        {
                            //unknown age, GT data overrides
                            if (ageIn == -1)
                            {
                                if (splits[i].Equals("a[youth]", StringComparison.OrdinalIgnoreCase))
                                    data.Age = 18;
                                else
                                    data.Age = rand.Next(27, 65);
                            }

                            numAge++;
                        }

                        //save gender
                        else if (regExGender.IsMatch(splits[i]))
                        {
                            //unkown gender, GT data overrides
                            if (genderIn.Equals("GenderUnknown", StringComparison.OrdinalIgnoreCase))
                            {
                                if (splits[i].Equals("g[male]", StringComparison.OrdinalIgnoreCase))
                                    data.Gender = "Male";
                                else
                                    data.Gender = "Female";
                            }

                            numGender++;
                        }
                        else
                        {
                            gtErrors.Add("\nThe metadata information supplied contains an unknown class OR is incorrectly scoped -> " + splits[i] + " . Skipping ingest.");
                        }

                        if (numRegions > 0)
                            gtErrors.Add("\nA region was specified outside enclosing braces. Skipping ingest.");
                        if (numAge > 1)
                            gtErrors.Add("\nMutliple age classes specified for a single person. Skipping ingest.");
                        if (numGender > 1)
                            gtErrors.Add("\nMultiple gender classes specified for a single person. Skipping ingest.");
                        if (numSkin > 1)
                            gtErrors.Add("\nMultiple skin tone classes specified for a single person. Skipping ingest.");

                        if (gtErrors.Count == 0)
                        {
                            ingestList.Add(data);
                        }
                        else
                        {
                            //append malformed error
                            String collate = String.Empty;
                            foreach (String error in gtErrors)
                                collate += error;
                            errorString += collate.TrimStart();
                        }
                    }

                    return ingestList;
                }

                //check idRegs type
                //base + attributes + regions
                //1. has regions 
                //2. has age, gender or skin

                String sGT = String.Empty;
                for (int i = 1; i < splits.Length; i++)
                    sGT += "\t" + splits[i];
                sGT = sGT.TrimStart();

                //make sure that all metadata is well formed
                if (!isExpressionBalanced(ref errorString, sGT))
                    return ingestList;

                //seperate on boundaries
                String[] annotations = sGT.Split(closingBrace, StringSplitOptions.RemoveEmptyEntries);
                foreach (String annotation in annotations)
                {
                    String[] regions = annotation.Split(tabSep, StringSplitOptions.RemoveEmptyEntries);

                    String ingestRegion = String.Empty;
                    int numFaces = 0;
                    int numRegions = 0;
                    int numSkin = 0;
                    int numAge = 0;
                    int numGender = 0;

                    IngestData data = new IngestData() { Age = ageIn, Gender = genderIn, ID = _ID, Regs = String.Empty };

                    HashSet<String> gtErrors = new HashSet<string>();

                    foreach (String region in regions)
                    {
                        //ignore white spaces
                        if (region.Trim().Length == 0)
                            continue;

                        //begin region
                        if (regExFace.IsMatch(region))
                        {
                            ingestRegion = region;
                            numFaces++;
                        }

                        //update region
                        else if (regExINME.IsMatch(region))
                        {
                            ingestRegion += "\t" + region;
                            numRegions++;
                        }

                        //filter skin
                        else if (regExSkin.IsMatch(region))
                        {
                            numSkin++;
                            continue;
                        }

                        //save age
                        else if (regExAge.IsMatch(region))
                        {
                            //unknown age, GT data overrides
                            if (ageIn == -1)
                            {
                                if (region.Equals("a[youth]", StringComparison.OrdinalIgnoreCase))
                                    data.Age = 18;
                                else
                                    data.Age = rand.Next(27, 65);
                            }

                            numAge++;
                        }

                        //save gender
                        else if (regExGender.IsMatch(region))
                        {
                            //unkown gender, GT data overrides
                            if (genderIn.Equals("GenderUnknown", StringComparison.OrdinalIgnoreCase))
                            {
                                if (region.Equals("g[male]", StringComparison.OrdinalIgnoreCase))
                                    data.Gender = "Male";
                                else
                                    data.Gender = "Female";
                            }

                            numGender++;
                        }
                        else
                        {
                            gtErrors.Add("\nThe metadata information supplied contains an unknown class OR is incorrectly scoped -> " + region + " . Skipping ingest.");
                        }

                        if (numFaces > 1)
                            gtErrors.Add("\nThere can be only one face or profile region within a primary region. Skipping ingest.");
                        if (numRegions > 6)
                            gtErrors.Add("\nThere are too many regions specified as eyes, nose, ear and mouth. Skipping ingest.");
                        if (numAge > 1)
                            gtErrors.Add("\nMutliple age classes specified for a single person. Skipping ingest.");
                        if (numGender > 1)
                            gtErrors.Add("\nMultiple gender classes specified for a single person. Skipping ingest.");
                        if (numSkin > 1)
                            gtErrors.Add("\nMultiple skin tone classes specified for a single person. Skipping ingest.");
                    }

                    //close region expression
                    if (gtErrors.Count == 0)
                    {
                        ingestRegion += "}";
                        data.Regs = ingestRegion;

                        //append
                        ingestList.Add(data);
                    }
                    else
                    {
                        String collate = String.Empty;
                        foreach (String error in gtErrors)
                            collate += error;
                        errorString += collate.TrimStart();
                    }
                }
            }
            finally
            {
                //do not direct to console
                //if (!errorsIn.Equals(errorString))
                //Console.WriteLine(errorString);
            }

            return ingestList;
        }

        private bool isExpressionBalanced(ref String errorString, String sGT)
        {
            int stack = 0;
            foreach (char c in sGT.ToCharArray())
            {
                if (c == '{')
                {
                    if (stack != 0)
                    {
                        errorString += "The supplied metadata is incorrectly formatted. Skipping ingest.";
                        return false;
                    }

                    Interlocked.Increment(ref stack);
                }

                if (c == '[')
                    Interlocked.Increment(ref stack);

                if (c == ']')
                    Interlocked.Decrement(ref stack);

                if (c == '}')
                {
                    Interlocked.Decrement(ref stack);

                    if (stack != 0)
                    {
                        errorString += "The supplied metadata is incorrectly formatted. Skipping ingest.";
                        return false;
                    }
                }
            }

            return true;
        }

    }

    class IngestData
    {
        public String ID { get; set; }
        public String Regs { get; set; }

        public String IDRegs
        {
            get { return ID.Trim() + "\t" + Regs.TrimStart(); }
        }

        private int age;
        public int Age
        {
            get { return age; }
            set { age = value; }
        }

        private String gender;
        public String Gender
        {
            get { return gender; }
            set { gender = value; }
        }
    }

}
