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

namespace FaceMatcherService
{
    public partial class FaceMatchRegions : IFaceMatchRegions
    {
        #region URL, Validation and Download

        private void downloadURLtoFile(string file, WebClient wc, string url_image)
        {
            //delete existing copy
            File.Delete(file);

            //download the file
            wc.DownloadFile(url_image, file);
        }

        private bool isEmptyUrl(string url, ref string errorString, out string url_image, out string[] split)
        {
            url_image = url;
            char[] sep = { '/' };
            split = url_image.Split(sep, StringSplitOptions.RemoveEmptyEntries);

            if (split != null && split.Length == 0)
            {
                errorString = "invalid url specified";
                return true;
            }

            return false;
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

        #endregion

        #region Query and Ingest parameter validation

        private bool queryValidateInit(string appKey, int eventID, string gender, ref string result, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            //zero out timing vars
            initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds);

            //validate
            if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                return false;

            //result expected
            if (result == null)
            {
                errorString = "result parameter cannot be null";
                return false;
            }

            return isValidAppKey(appKey);
        }

        private bool ingestValidateInit(string appKey, int eventID, string gender, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds)
        {
            //zero out timing vars
            initTimingVars(ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds);

            //validate
            if (validateKeyEventErrorParams(appKey, eventID, ref errorString) == false)
                return false;

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

        #endregion

        #region Low level local file management functions

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

        #endregion
    }
}
