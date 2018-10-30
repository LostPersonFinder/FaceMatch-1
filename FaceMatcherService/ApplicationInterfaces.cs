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
using System.ServiceModel;

namespace FaceMatcherService
{
    [ServiceContract]
    public interface IFaceMatchRegions
    {
        [OperationContract]
        void ingest(string appKey, int eventID, string url, string IDRegs, string gender, int age, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding);
        [OperationContract]
        void query(string appKey, int eventID, ref string result, string urlRegs, string gender, int age, float tolerance, ref string errorString, ref int errorCode, uint? maxMatchesRequested, ref uint? matches, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding);
        [OperationContract]
        void remove(string appKey, int eventID, string ID, ref uint records, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? requestsOutstanding);
        [OperationContract]
        void save(string appKey, int eventID, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds);
        [OperationContract]
        void deleteEvent(string appKey, int eventID, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? comMilliseconds);
        [OperationContract]
        string getVersion();
        [OperationContract]
        void queryall(string appKey, int[] eventID, ref string jsonResult, string urlRegs, string gender, int age, float tolerance, ref string errorString, ref int errorCode, uint? maxMatchesRequestedperEvent);
    }

    [ServiceContract]
    public interface IWholeImageMatcher
    {
        [OperationContract]
        void ingest(string appKey, int eventID, string url, string ID, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding);
        [OperationContract]
        void query(string appKey, int eventID, ref string result, string url, float tolerance, ref string errorString, ref int errorCode, uint? maxMatchesRequested, ref uint? matches, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding);
        [OperationContract]
        void remove(string appKey, int eventID, string ID, ref uint records, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding);
        [OperationContract]
        void save(string appKey, int eventID, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds);
    }

    [ServiceContract]
    public interface IFaceFinder
    {
        [OperationContract]
        void getFacesForUI(string appKey, int eventID, string url, ref string queryRegions, ref string displayRegions, double inflatePct, ref string errorString, ref int errorCode, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding, bool useGPU = true);
        [OperationContract]
        void getGPUStatus(ref int isEnabled);
    }

    [ServiceContract]
    public interface IFaceMatchSearchResultsCombiner
    {
        [OperationContract]
        void combine(string results1, string results2, ref string combinedResults, ref string errorString, ref int? webServiceMilliseconds);
    }
    
    /// <summary>
    /// Interface controls the speed of the FaceMatchSystem. 
    /// If this interface is not implemented the FaceMatchSystem uses an optimal performance setting as a default value for a given collection/event.
    /// </summary>
    [ServiceContract]
    public interface IFaceMatchPerformance
    {
        /// <summary>
        /// Assigns a performance setting for a collection (aka event). If this method is not called the FaceMatchSystem uses
        /// an optimal performance setting as a default value for a given collection/event.
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="collectionID"></param>
        /// <param name="performanceValue"></param>
        /// <param name="errorString"></param>
        /// <param name="errorCode"></param>
        [OperationContract]
        void setPerformanceForCollection(string appKey, int collectionID, string performanceValue, ref string errorString, ref int errorCode);
    }
}
