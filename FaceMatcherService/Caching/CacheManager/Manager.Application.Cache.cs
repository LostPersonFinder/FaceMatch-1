
namespace FaceMatcherService
{
    internal partial class ApplicationCacheManager : IApplicationCacheManager
    {
        readonly IApplicationCache appCache = new ApplicationCache();

        public bool getCacheHits(string appkey, int eventID, string urlregs, string bucket, ref string hits)
        {
            hits = string.Empty;

            IEventCache eventCache = appCache.getORCreateEventCache(appkey);

            if (eventCache != null)
            {
                IBucketCache bucketCache = eventCache.getORCreateBucketCache(eventID);

                if (bucketCache != null)
                {
                    IQueryCache queryCache = bucketCache.getORCreateBucketQueryCache(bucket);

                    if (queryCache != null)
                    {
                        if (queryCache.getQuery(urlregs, ref hits))
                            return true;
                    }
                }
            }

            return false;
        }

        public void setCacheHits(string appkey, int eventID, string urlregs, string bucket, string hits)
        {
            IEventCache eventCache = appCache.getORCreateEventCache(appkey);

            if (eventCache != null)
            {
                IBucketCache bucketCache = eventCache.getORCreateBucketCache(eventID);

                if (bucketCache != null)
                {
                    IQueryCache queryCache = bucketCache.getORCreateBucketQueryCache(bucket);

                    if (queryCache != null)
                        queryCache.saveQuery(urlregs, hits);
                }
            }
        }

        public void clearCacheHitsOnIngest_OR_Remove(string appkey, int eventID, string bucket)
        {
            IEventCache eventCache = appCache.getORCreateEventCache(appkey);

            if (eventCache != null)
            {
                IBucketCache bucketCache = eventCache.getORCreateBucketCache(eventID);

                if (bucketCache != null)
                {
                    //clear query cache on this bucket
                    bucketCache.removeBucketQueryCache(bucket);
                }
            }
        }
    }
}