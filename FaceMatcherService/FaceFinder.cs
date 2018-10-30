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
using System.ServiceModel;
using System.IO;
using System.Security;
using System.Drawing;
using System.Threading;
using System.Net;
using System.Diagnostics;
using System.Collections.Concurrent;
using System.Windows;

namespace FaceMatcherService
{
    /// <summary>
    /// main class that implements the FaceFinder interface
    /// </summary>
    [ServiceBehavior(
        ConcurrencyMode = ConcurrencyMode.Multiple,
        InstanceContextMode = InstanceContextMode.Single
        )]
    public class FaceFinder : IFaceFinder
    {
        static int filePrefix = 0;

        //0.75 means: if a 75% overlap exists between 2 rectangles, then they are considered
        //to be part of the same face.
        const double overlapTolerance = 0.25;

        //0.15 means: inflate each bounding box by 15%
        //const double inflateBy = 0.15;

        public FaceMatcherCoreLib.FaceFinder ff = null;

        readonly char[] sepForwardSlash = { '/' };
        readonly char[] faceSeps = { '[' };
        readonly char[] regseps = { ' ', '\t', '{', '[', ',', ';', ']', '}' };

        public FaceFinder()
        {
            //save a reference to this instance
            if (ServiceStateManager.faceFinder == null)
                ServiceStateManager.faceFinder = this;

            //obtain a ref to the FF com server
            try
            {
                ff = new FaceMatcherCoreLib.FaceFinder();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Failed to instantiate com server (FaceFinder): " + ex.Message);
                ServiceStateManager.Log("Exception : Failed to instantiate FaceFinder com server : details : " + ex.Message);
            }
            finally
            {
                if (ff == null)
                    ServiceStateManager.Log("Failed to obtain a reference to the FaceFinder com server, the ff reference is null.");
            }
        }

        public void getFaces(string appKey, int eventID, string url, ref string regions, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            //check if system is running
            if (!FaceMatchRegions.isFMSystemRunning())
            {
                errorString = "FaceMatch system is not available";
                return;
            }

            string displayRegions = "";
            double inflatePct = 0;
            getFacesForUI(appKey, eventID, url, ref regions, ref displayRegions, inflatePct, ref errorString, ref errorCode, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);
            return;
        }

        /// <summary>
        /// calls FaceFinder to detect faces on the image and modifies the returned value to merge overlaps / expands the returned rectangle by the given percent
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="url"></param>
        /// <param name="queryRegions"></param>
        /// <param name="displayRegions"></param>
        /// <param name="inflatePct"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        /// <param name="webServiceMilliseconds"></param>
        /// <param name="imageCoreMilliseconds"></param>
        /// <param name="comMilliseconds"></param>
        /// <param name="urlFetchMilliseconds"></param>
        /// <param name="requestsOutstanding"></param>
        /// <param name="useGPU"></param>
        public void getFacesForUI(string appKey, int eventID, string url, ref string queryRegions, ref string displayRegions, double inflatePct, ref string errorString, ref int errorCode, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding,
            bool useGPU = true)
        {
            try
            {
                //validate input
                if (faceFinderValidateInit(appKey, url, ref queryRegions, ref displayRegions, inflatePct, ref errorString,
                    ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds) == false)
                    return;

                Stopwatch webServiceWatch = null;
                Stopwatch comRuntimeWatch = null;

                //create event map if not existant
                ServiceStateManager.PeekEvent(appKey, eventID, true);

                //replace key with app name
                appKey = getAppFromKey(appKey);

                int coreOperationTime = 0;

                int performanceValue = ServiceStateManager.getFaceFinderPerformanceValue(appKey, eventID);

                try
                {
                    //start web service timer
                    webServiceWatch = startWebServiceTimer(webServiceMilliseconds, webServiceWatch);

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

                        findFaces(ref queryRegions, ref displayRegions, inflatePct, ref errorString, ref imageCoreMilliseconds, ref comMilliseconds, ref comRuntimeWatch, localPath, ref coreOperationTime, useGPU, performanceValue);

                        if (urlFetchMilliseconds != null)
                            urlFetchMilliseconds = 0;

                        return;
                    }

                    try
                    {
                        string file = "";
                        string url_image;
                        string[] split;

                        if (isEmptyUrl(url, ref errorString, out url_image, out split))
                            return;

                        int reqs = Interlocked.Increment(ref filePrefix);

                        if (requestsOutstanding != null)
                            requestsOutstanding = reqs;

                        file = @"C:\FaceMatchSL\Facematch\bin\UploadBin\App." + appKey + ".ff." + filePrefix + "." + split[split.Length - 1];

                        fi = new FileInfo(file);

                        //delete existing copy
                        File.Delete(file);

                        //download the file
                        WebClient wc = new WebClient();
                        Stopwatch urlFetch = new Stopwatch();

                        urlFetch.Start();
                        wc.DownloadFile(url_image, file);
                        urlFetch.Stop();

                        //measure download time
                        if (urlFetchMilliseconds != null)
                            urlFetchMilliseconds = Convert.ToInt32(urlFetch.ElapsedMilliseconds);

                        //check
                        fi = new FileInfo(file);
                        if (fi.Exists)
                        {
                            findFaces(ref queryRegions, ref displayRegions, inflatePct, ref errorString, ref imageCoreMilliseconds, ref comMilliseconds, ref comRuntimeWatch, file, ref coreOperationTime, useGPU, performanceValue);
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
                        Interlocked.Decrement(ref filePrefix);

                        string logMessage = string.Empty;
                        logMessage += "\r\nFaceFinder called on eventID : " + eventID + " app : " + appKey + " url : " + url;
                        logMessage += "\r\nperformance value : " + FaceMatchEventSettings.getPerformanceStringForValue(performanceValue);
                        logMessage += "\r\npreferGPU : " + useGPU;
                        logMessage += "\r\nreturn values : query regions " + queryRegions;
                        logMessage += "\r\ninflate pct : " + inflatePct;
                        logMessage += "\r\nreturn values : display regions " + displayRegions;
                        logMessage += "\r\nimageCore (milliseconds) : " + imageCoreMilliseconds;
                        logMessage += "\r\nerrorString : " + errorString;
                        logMessage += "\r\nerrorCode : " + FaceMatchRegions.setErrorCode(errorString);

                        ServiceStateManager.Log(logMessage);
                    }
                }
                finally
                {
                    //convert faces to new format
                    FaceFinderParser parser = new FaceFinderParser();
                    queryRegions = string.Join("\t", parser.getFaceList(queryRegions)).Trim();
                    displayRegions = string.Join("\t", parser.getFaceList(displayRegions)).Trim();
                    
                    stopTimersandMeasure(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, webServiceWatch, comRuntimeWatch, coreOperationTime, urlFetchMilliseconds);
                }

            }
            finally
            {
                errorCode = FaceMatchRegions.setErrorCode(errorString);
            }
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

        private Stopwatch startCOMtimer(int? comMilliseconds, Stopwatch comRuntimeWatch)
        {
            if (comMilliseconds != null)
            {
                comRuntimeWatch = new Stopwatch();
                comRuntimeWatch.Start();
            }
            return comRuntimeWatch;
        }

        private void stopCOMtimer(Stopwatch comRuntimeWatch)
        {
            if (comRuntimeWatch != null)
                comRuntimeWatch.Stop();
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
            {
                comMilliseconds -= coreOperationTime;
                if (comMilliseconds < 0)
                    comMilliseconds = 0;
            }
            
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

        private bool isEmptyUrl(string url, ref string errorString, out string url_image, out string[] split)
        {
            url_image = url;
            split = url_image.Split(sepForwardSlash, StringSplitOptions.RemoveEmptyEntries);

            if (split != null && split.Length == 0)
            {
                errorString = "invalid url specified";
                return true;
            }

            return false;
        }

        private bool isValidAppKey(string appKey)
        {
            //Openi, PL
            return KeyManager.isValidAppKey(appKey);
        }

        private string getAppFromKey(string appKey)
        {
            return KeyManager.getAppFromKey(appKey);
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

        private void findFaces(ref string queryRegions, ref string displayRegions, double inflateBy, ref string errorString, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref Stopwatch comRuntimeWatch, string localPath, ref int coreOperationTime, bool useGPU, int faceFinderPerformanceValue)
        {
            //get bitmap bounds
            Rectangle bBox;
            using (Bitmap b = new Bitmap(localPath))
            {
                bBox = Rectangle.FromLTRB(0, 0, b.Width, b.Height);
            }

            string faceRegions = "";
            string errors = "";

            //find faces
            coreGetFaces(ref comMilliseconds, ref comRuntimeWatch, localPath, ref faceRegions, ref errors, ref coreOperationTime, useGPU, faceFinderPerformanceValue);

            //detected faces
            queryRegions = faceRegions;

            //merge profiles and faces
            displayRegions = mergeProfilesWithFaces(faceRegions, bBox, inflateBy);

            errorString = errors;

            if (faceRegions.Contains('['))
                errorString = "SUCCESS";
            else
                errorString = "No face detected in input image";
        }

        /// <summary>
        /// calls the FaceMatchCore server to find faces on the given image
        /// </summary>
        /// <param name="comMilliseconds"></param>
        /// <param name="comRuntimeWatch"></param>
        /// <param name="localPath"></param>
        /// <param name="faceRegions"></param>
        /// <param name="errorString"></param>
        /// <param name="coreOperationTime"></param>
        /// <param name="useGPU"></param>
        /// <param name="faceFinderPerformanceValue"></param>
        private void coreGetFaces(ref int? comMilliseconds, ref Stopwatch comRuntimeWatch, string localPath, ref string faceRegions, ref string errorString, ref int coreOperationTime, bool useGPU, int faceFinderPerformanceValue)
        {
            //start COM timer
            comRuntimeWatch = startCOMtimer(comMilliseconds, comRuntimeWatch);

            int GPU = 0;
            if (useGPU)
                GPU = 1;

            try
            {
                //add exception handler here.
                if(ff != null)
                    ff.GetFaces(localPath, 1, ref faceRegions, ref errorString, ref coreOperationTime, GPU, faceFinderPerformanceValue);
                else
                    ServiceStateManager.Log("Skipped ff.GetFaces call, as the ff reference is null.");

                //log any FRD issues here
                if (!errorString.Equals("SUCCESS", StringComparison.OrdinalIgnoreCase))
                    ServiceStateManager.Log(errorString);
            }
            catch (Exception ex)
            {
                ServiceStateManager.Log("Exception : the FaceFinder com server call returned an exception : details : " + ex.Message);
            }

            //stop COM timer
            stopCOMtimer(comRuntimeWatch);

            if(comMilliseconds != null)
                comMilliseconds = Convert.ToInt32(comRuntimeWatch.ElapsedMilliseconds);
        }

        private bool faceFinderValidateInit(string appKey, string url, ref string queryRegions, ref string displayRegions, double inflatePct, ref string ErrorString, 
            ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds)
        {
            if (webServiceMilliseconds != null)
                webServiceMilliseconds = 0;
            if (imageCoreMilliseconds != null)
                imageCoreMilliseconds = 0;
            if (comMilliseconds != null)
                comMilliseconds = 0;

            if (ErrorString == null)
                return false;

            if (isValidAppKey(appKey) == false)
            {
                ErrorString = "Invalid application key";
                return false;
            }

            if (queryRegions == null)
            {
                ErrorString = "regions parameter cannot be null";
                return false;
            }

            if (displayRegions == null)
            {
                ErrorString = "display regions parameter cannot be null";
                return false;
            }

            if (url.Length == 0)
            {
                queryRegions = "";
                displayRegions = "";
                ErrorString = "url cannot be empty";
                return false;
            }

            if (inflatePct < -1)
            {
                ErrorString = "inflate percentage < -1 is not valid (please choose a number between 0 and 1, for example 0.15 would mean inflate returned rectangle(s) by fifteen percent)";
                return false;
            }


            queryRegions = "";
            ErrorString = "";
            displayRegions = "";

            return true;
        }

        private static void GetLocalFileInfo(string url, ref string ErrorString, Uri uri, ref FileInfo fi, ref string localPath)
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

        string mergeProfilesWithFaces(string regionsIn, Rectangle bBox, double inflateBy)
        {
            if (regionsIn == null || regionsIn.Length == 0)
                return regionsIn;

            //total number of faces (all regions)
            int numFaces = regionsIn.Split(faceSeps).Length - 1;

            string[] x = regionsIn.Split(regseps, StringSplitOptions.RemoveEmptyEntries);


            char profile;
            Rectangle rect = new Rectangle();
            List<Rectangle> faces = new List<Rectangle>();
            List<Rectangle> profiles = new List<Rectangle>();

            for (int i = 0; i < numFaces; i++)
            {
                profile = x[5 * i][0];

                if (profile == 'f' || profile == 'p')
                {
                    rect.X = Convert.ToInt32(x[1 + 5 * i]);
                    rect.Y = Convert.ToInt32(x[2 + 5 * i]);
                }
                else
                {
                    //merge i|n|m regions as they are wholly contained
                    continue;
                }

                rect.Width = Convert.ToInt32(x[3 + 5 * i]);
                rect.Height = Convert.ToInt32(x[4 + 5 * i]);

                if (profile == 'f')
                    faces.Add(rect);
                else
                    profiles.Add(rect);
            }


            //merge faces with associated profiles
            for (int i = 0; i < faces.Count; i++)
            {
                foreach (Rectangle r in profiles)
                {
                    //if r entirely contained in face, remove
                    if (faces[i].Contains(r))
                    {
                        profiles.Remove(r);
                        break;
                    }

                    //if face entirely contained in r, remove
                    if (r.Contains(faces[i]))
                    {
                        faces[i] = Rectangle.Union(faces[i], r);
                        profiles.Remove(r);
                        break;
                    }

                    //if r intersects within specified tolerance
                    if (overlap(faces[i], r, overlapTolerance))
                    {
                        faces[i] = Rectangle.Union(faces[i], r);
                        profiles.Remove(r);
                        break;
                    }
                }
            }

            //at this point we have faces and profiles that have no overlap 

            //promote all profiles to faces
            faces.AddRange(profiles);

            //clip to bounding box
            for (int i = 0; i < faces.Count; i++)
            {
                int xOffset = (Convert.ToInt32(faces[i].Width * (1 + inflateBy)) - faces[i].Width) / 2;
                int yOffset = (Convert.ToInt32(faces[i].Height * (1 + inflateBy)) - faces[i].Height) / 2;

                int left = Convert.ToInt32(Convert.ToDouble(faces[i].Left) - xOffset);
                int top = Convert.ToInt32(Convert.ToDouble(faces[i].Top) - yOffset);
                int right = Convert.ToInt32(Convert.ToDouble(faces[i].Right) + xOffset);
                int bottom = Convert.ToInt32(Convert.ToDouble(faces[i].Bottom) + yOffset);

                //clip ltrb
                if (left < 0)
                    left = 0;
                if (top < 0)
                    top = 0;
                if (right > bBox.Right)
                    right = bBox.Right;
                if (bottom > bBox.Bottom)
                    bottom = bBox.Bottom;

                faces[i] = Rectangle.FromLTRB(left, top, right, bottom);
            }

            string s = "";
            for (int i = 0; i < faces.Count; i++)
            {
                if (faces[i].Width != 0 &&
                    faces[i].Height != 0)
                {
                    s += '\t';

                    s += "f[";
                    s += faces[i].X;
                    s += ",";
                    s += faces[i].Y;
                    s += ";";
                    s += faces[i].Width;
                    s += ",";
                    s += faces[i].Height;
                    s += "]";
                }
            }

            return s;
        }

        static bool overlap(Rectangle a, Rectangle b, double tol)
        {
            Rectangle c = a;
            c.Intersect(b);
            double A = 0.5 * (area(a) + area(b));
            return ((area(c) / A) > tol);
        }

        static double area(Rectangle a)
        {
            return a.Width * a.Height;
        }


        public void getGPUStatus(ref int isEnabled)
        {
            //for now return 1, will be replaced with a call to the GPU card
            //will return 0 for no GPU used and 1 for GPU use.
            isEnabled = 0;

            if (ff != null)
            {
                string gpuOn = String.Empty;

                ff.usingGPU(ref gpuOn);

                if (gpuOn.Equals("SUCCESS"))
                    isEnabled = 1;
            }
        }
    }

    /// <summary>
    /// converts a face string to a face object
    /// Handles only single face
    /// </summary>
    internal class FaceWithLandmarks
    {
        private readonly string faceFragment;

        //constructor
        public FaceWithLandmarks(string faceFragment)
        {
            this.faceFragment = faceFragment;
            
            fpRect = Rect.Empty;
            iRect = Rect.Empty;
            nRect = Rect.Empty;
            mRect = Rect.Empty;
            
            parseFragment();
        }

        private void parseFragment()
        {
            string[] fragments = faceFragment.Split(new string[] { "\t" }, StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < fragments.Length; i++)
            {
                if (fragments[i].Contains("f"))
                {
                    FP = "f";
                    parseRegion(ref fpRect, fragments[i].Replace("f", string.Empty));
                }
                else if (fragments[i].Contains("p"))
                {
                    FP = "p";
                    parseRegion(ref fpRect, fragments[i].Replace("p", string.Empty));
                }
                else if (fragments[i].Contains("i"))
                    parseRegion(ref iRect, fragments[i].Replace("i", string.Empty));
                else if (fragments[i].Contains("n"))
                    parseRegion(ref nRect, fragments[i].Replace("n", string.Empty));
                else if (fragments[i].Contains("m"))
                    parseRegion(ref mRect, fragments[i].Replace("m", string.Empty));
            }
        }

        private void parseRegion(ref Rect r, string s)
        {
            s = s.Replace("{", string.Empty);
            s = s.Replace("}", string.Empty);

            s = s.Replace("[", string.Empty);
            s = s.Replace("]", string.Empty);

            s = s.Replace(";", ",");

            try
            {
                r = Rect.Parse(s);
            }
            catch (Exception) { }
        }

        public string FP { get; set; }

        private Rect fpRect;
        public Rect FP_rect { get { return fpRect; } }

        private Rect iRect;
        public Rect I_rect { get { return iRect; } }

        private Rect nRect;
        public Rect N_rect { get { return nRect; } }

        private Rect mRect;
        public Rect M_rect { get { return mRect; } }

        public bool isValid
        {
            get
            {
                return isValidRegion(FP_rect) && isValidRegion(iRect) && isValidRegion(nRect) && isValidRegion(mRect);
            }
        }

        private bool isValidRegion(Rect r)
        {
            if (r.Equals(Rect.Empty))
                return true;

            return r.Top >= 0 && r.Left >= 0 && r.Width > 0 && r.Height > 0;
        }

        protected string getRect(string kind, Rect r)
        {
            string s = string.Empty;

            if (isValidRegion(r) && !r.Equals(Rect.Empty))
                s = kind + "[" + r.X + "," + r.Y + ";" + r.Width + "," + r.Height + "]";

            return s.Trim();
        }

        public override string ToString()
        {
            string s = string.Empty;

            if (isValid)
            {
                string ret = FP + "{" + getRect(string.Empty, FP_rect);

                if ((s = getRect("i", I_rect)).Length != 0)
                    ret += "\t" + s;

                if ((s = getRect("n", N_rect)).Length != 0)
                    ret += "\t" + s;

                if ((s = getRect("m", M_rect)).Length != 0)
                    ret += "\t" + s;

                ret += "}";

                return ret;
            }

            return s;
        }
    }

    /// <summary>
    /// pairs an ID to a face
    /// Single face only
    /// </summary>
    internal class FaceWithLandmarksID : FaceWithLandmarks
    {
        public string ID { get; set; }

        public FaceWithLandmarksID(string id, string singleFaceFragment)
            : base(singleFaceFragment)
        {
            this.ID = id.Trim();

            if (id.Length == 0)
                throw new Exception("ID must be specified");
        }

        public override string ToString()
        {
            string s = string.Empty;

            if (ID.Length > 0 && isValid)
            {
                if (ID.Contains("d[") == false)
                    s = base.ToString().Replace("}", "\td[" + ID + "]}");
                else
                    s = base.ToString().Replace("}", "\t" + ID + "}");
            }

            return s;
        }
    }

    /// <summary>
    /// Processes the output of the facefinder and pairs faces with ID.
    /// Can also be used to process the FaceFinder output.
    /// Multiple face capable
    /// </summary>
    public class FaceFinderParser
    {
        private readonly string id = string.Empty;
        private readonly string parse = string.Empty;

        /// <summary>
        /// use this when no id association is needed. in query nad facefinder processing.
        /// </summary>
        public FaceFinderParser()
        {
        }

        /// <summary>
        /// use this to associate an id with a face, during ingest 
        /// </summary>
        /// <param name="id"></param>
        /// <param name="parse"></param>
        public FaceFinderParser(string id, string parse)
        {
            this.id = id;
            this.parse = parse;
        }
        
        /// <summary>
        /// use this when you want multiple faces all associated with an id
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return getDelimitedFaceIDLandmarkStr(parse);
        }

        public string getDelimitedFaceIDLandmarkStr(string parse)
        {
            string filmStr = string.Empty;

            foreach (string face in getFaceList(parse))
            {
                FaceWithLandmarksID flid = new FaceWithLandmarksID(id, face);

                if (flid.isValid)
                    filmStr += "\t" + flid;
            }

            return filmStr.Trim();
        }

        /// <summary>
        /// this is to be used in the the ingest execution path as this pairs an id to the facefinder output
        /// </summary>
        /// <param name="faceFragmentList"></param>
        /// <param name="id"></param>
        /// <returns></returns>
        private List<FaceWithLandmarks> getFaceList(List<string> faceFragmentList, string id)
        {
            List<FaceWithLandmarks> fl = new List<FaceWithLandmarks>();

            foreach (var faceFragment in faceFragmentList)
                fl.Add(new FaceWithLandmarks(faceFragment));

            return fl;
        }

        /// <summary>
        /// this is to be used only to parse the face finder output
        /// multiface parsing capable. use this when you do not want an id association
        /// </summary>
        /// <param name="parseFaces"></param>
        /// <returns>a list of faces possibly with landmarks if any</returns>
        public List<string> getFaceList(string parseFaces)
        {
            bool hasFacesOrProfiles = parseFaces.Contains("f[") || parseFaces.Contains("p[");

            string[] faceFragments = null;

            //new formatting
            if (parseFaces.Contains("}") && !hasFacesOrProfiles)
            {
                faceFragments = getFaceFragments(parseFaces);
                return faceFragments.ToList();
            }

            //old formatting or mixed formatting
            else if (hasFacesOrProfiles)
            {
                //convert old and mixed formatting to new formatting
                parseFaces = parseFaces.Replace("f[", "}\tf{[").Replace("p[", "}\tp{[").Trim(new char[] { '\t', '}' }) + "}";

                //convert remaining mixed formatting to new formatting
                parseFaces = parseFaces.Replace("]\tf", "]}\tf").Replace("]\tp", "]}\tp");

                string[] tokens = parseFaces.Split(new char[] { '\t' }, StringSplitOptions.RemoveEmptyEntries);

                faceFragments = getFaceFragments(string.Join("\t", tokens).Trim());

                return faceFragments.ToList();
            }

            return new List<string>();
        }

        private string[] getFaceFragments(string parseFaces)
        {
            string[] faceFragments = parseFaces.Split(new char[] { '}' }, StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < faceFragments.Length; i++)
                faceFragments[i] = faceFragments[i].Trim() + "}";
            return faceFragments;
        }
    }
}
