using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.ServiceModel;
using System.ServiceProcess;
using System.Threading;
using System.Threading.Tasks;

namespace FaceMatcherService
{
    public partial class Service1 : ServiceBase
    {
        List<ServiceHost> hosts = new List<ServiceHost>();

        public static ManualResetEvent shutDownTaskCompletion = null;
        public Service1()
        {
            InitializeComponent();
        }

        protected override void OnStart(string[] args)
        {
            try
            {
                Task.Factory.StartNew(FaceMatchWebServices);
                Task.Factory.StartNew(ServiceStateManager.FaceMatchWebServiceLogger);
            }
            catch { }
        }

        private void FaceMatchWebServices()
        {
            try
            {
                //load the facematch index files
                ServiceStateManager.LoadIndexes();

                //open services
                CloseServices();
                StartServices();
            }
            catch(Exception ex)
            {
                Console.WriteLine("Exception occurred while loading the FaceMatch indexes : " + ex.Message);
            }
        }

        protected override void OnStop()
        {
            try
            {
                ServiceStateManager.SaveIndexes();

                CloseServices();
            }
            catch (Exception) { }
            finally
            {
                shutDownTaskCompletion = new ManualResetEvent(false);

                //request logging thread to write out all remaining messages
                ServiceStateManager.logEvent.Set();

                //wait for all log messages to be written out (or a max of 1 minute);
                shutDownTaskCompletion.WaitOne(TimeSpan.FromMinutes(1));
            }
        }

        private void StartServices()
        {
            Type[] serviceTypes = new Type[] {
                    //services
                    typeof(FaceFinder),
                    typeof(FaceFinderCrossDomainService),
                    typeof(FaceMatchRegions),
                    typeof(FaceMatchRegionsCrossDomainService),
                    typeof(WholeImageMatcher),
                    typeof(WholeImageMatcherCrossDomainService),
                    typeof(FaceMatchPerformance),
                    typeof(FaceMatchPerformanceCrossDomainService)
                    /*,
                    //internal services
                    typeof(FaceFinderInternal),
                    typeof(FaceFinderInternalCrossDomainService),
                    typeof(FaceMatchRegionsInternal),
                    typeof(FaceMatchRegionsInternalCrossDomainService),
                    typeof(WholeImageMatcherInternal),
                    typeof(WholeImageMatcherInternalCrossDomainService)*/
                };

            //typeof(FaceMatchSearchResultsCombiner),
            //typeof(FaceMatchSearchResultsCombinerCrossDomainService)

            foreach (Type service in serviceTypes)
            {
                ServiceHost host = new ServiceHost(service);
                host.Open();
                hosts.Add(host);
            }
        }

        private void CloseServices()
        {
            foreach (ServiceHost host in hosts)
                host.Close();

            hosts.Clear();
        }
    }
}
