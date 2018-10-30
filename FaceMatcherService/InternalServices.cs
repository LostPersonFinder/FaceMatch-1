using System.IO;
using System.ServiceModel;
using System.ServiceModel.Channels;
using System.Xml;

namespace FaceMatcherService
{
    /// <summary>
    /// pass through service for IFaceMatchRegions (internal)
    /// </summary>
    [ServiceBehavior(
        ConcurrencyMode = ConcurrencyMode.Multiple,
        InstanceContextMode = InstanceContextMode.Single
        )]
    public partial class FaceMatchRegionsInternal : IFaceMatchRegions
    {
        public FaceMatchRegionsInternal()
        {
        }

        public void ingest(string appKey, int eventID, string url, string IDRegs, string gender, int age, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            if(ServiceStateManager.svcFMR != null)
                ServiceStateManager.svcFMR.ingest(appKey, eventID, url, IDRegs, gender, age, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);
        }

        public void query(string appKey, int eventID, ref string result, string urlRegs, string gender, int age, float tolerance, ref string errorString, uint? maxMatchesRequested, ref uint? matches, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            if (ServiceStateManager.svcFMR != null)
                ServiceStateManager.svcFMR.query(appKey, eventID, ref result, urlRegs, gender, age, tolerance, ref errorString, maxMatchesRequested, ref matches, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);
        }

        public void remove(string appKey, int eventID, string ID, ref uint records, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? requestsOutstanding)
        {
            if (ServiceStateManager.svcFMR != null)
                remove(appKey, eventID, ID, ref records, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref requestsOutstanding);
        }

        public void save(string appKey, int eventID, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds)
        {
            if (ServiceStateManager.svcFMR != null)
                save(appKey, eventID, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds);
        }

        public void deleteEvent(string appKey, int eventID, ref string errorString, ref int? webServiceMilliseconds, ref int? comMilliseconds)
        {
            if (ServiceStateManager.svcFMR != null)
                deleteEvent(appKey, eventID, ref errorString, ref webServiceMilliseconds, ref comMilliseconds);
        }
    }

    /// <summary>
    /// pass through service for IFaceFinder (internal)
    /// </summary>
    [ServiceBehavior(
    ConcurrencyMode = ConcurrencyMode.Multiple,
    InstanceContextMode = InstanceContextMode.Single
    )]
    public class FaceFinderInternal : IFaceFinder
    {
        public FaceFinderInternal()
        {
        }

        public void getFacesForUI(string appKey, string url, ref string queryRegions, ref string displayRegions, double inflatePct, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            IFaceFinder ff = ServiceStateManager.faceFinder;
            if(ff != null)
                ff.getFacesForUI(appKey, url, ref queryRegions, ref displayRegions, inflatePct, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);
        }
    }

    /// <summary>
    /// pass through service for IWholeImageMatcher (internal)
    /// </summary>
    [ServiceBehavior(
    ConcurrencyMode = ConcurrencyMode.Multiple,
    InstanceContextMode = InstanceContextMode.Single
    )]
    public class WholeImageMatcherInternal : IWholeImageMatcher
    {
        public void ingest(string appKey, int eventID, string url, string ID, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            if (ServiceStateManager.svcWIM != null)
                ServiceStateManager.svcWIM.ingest(appKey, eventID, url, ID, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);
        }

        public void query(string appKey, int eventID, ref string result, string url, float tolerance, ref string errorString, uint? maxMatchesRequested, ref uint? matches, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            if (ServiceStateManager.svcWIM != null)
                ServiceStateManager.svcWIM.query(appKey, eventID, ref result, url, tolerance, ref errorString, maxMatchesRequested, ref matches, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);
        }

        public void remove(string appKey, int eventID, string ID, ref uint records, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds, ref int? urlFetchMilliseconds, ref int? requestsOutstanding)
        {
            if(ServiceStateManager.svcWIM != null)
                ServiceStateManager.svcWIM.remove(appKey, eventID, ID, ref records, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds, ref urlFetchMilliseconds, ref requestsOutstanding);
        }

        public void save(string appKey, int eventID, ref string errorString, ref int? webServiceMilliseconds, ref int? imageCoreMilliseconds, ref int? comMilliseconds)
        {
            if (ServiceStateManager.svcWIM != null)
                ServiceStateManager.svcWIM.save(appKey, eventID, ref errorString, ref webServiceMilliseconds, ref imageCoreMilliseconds, ref comMilliseconds);
        }
    }

    [ServiceBehavior(AddressFilterMode = AddressFilterMode.Any,
        InstanceContextMode = InstanceContextMode.Single)]
    public class FaceFinderInternalCrossDomainService : IFaceFinderCrossDomainService
    {
        public System.ServiceModel.Channels.Message ProvidePolicyFile()
        {
            TextReader reader = new StringReader(@"<?xml version='1.0' encoding='utf-8'?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from http-request-headers='*'>
        <domain uri='*'/>
      </allow-from>
      <grant-to>
        <resource path='/' include-subpaths='true'/>
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>");
            XmlReader xmlReader = XmlReader.Create(reader);
            return Message.CreateMessage(MessageVersion.None, "", xmlReader);
        }
    }

    [ServiceBehavior(AddressFilterMode = AddressFilterMode.Any,
        InstanceContextMode = InstanceContextMode.Single)]
    public class FaceMatchRegionsInternalCrossDomainService : IFaceMatchRegionsCrossDomainService
    {
        public System.ServiceModel.Channels.Message ProvidePolicyFile()
        {
            TextReader reader = new StringReader(@"<?xml version='1.0' encoding='utf-8'?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from http-request-headers='*'>
        <domain uri='*'/>
      </allow-from>
      <grant-to>
        <resource path='/' include-subpaths='true'/>
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>");
            XmlReader xmlReader = XmlReader.Create(reader);
            return Message.CreateMessage(MessageVersion.None, "", xmlReader);
        }
    }

    [ServiceBehavior(AddressFilterMode = AddressFilterMode.Any,
        InstanceContextMode = InstanceContextMode.Single)]
    public class WholeImageMatcherInternalCrossDomainService : IWholeImageMatcherCrossDomainService
    {
        public System.ServiceModel.Channels.Message ProvidePolicyFile()
        {
            TextReader reader = new StringReader(@"<?xml version='1.0' encoding='utf-8'?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from http-request-headers='*'>
        <domain uri='*'/>
      </allow-from>
      <grant-to>
        <resource path='/' include-subpaths='true'/>
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>");
            XmlReader xmlReader = XmlReader.Create(reader);
            return Message.CreateMessage(MessageVersion.None, "", xmlReader);
        }
    }

}
