using System.Collections.Concurrent;
using System.Collections.Generic;

namespace FaceMatcherService
{
    class CacheUpdateManager : ICacheUpdateManager
    {
        IApplicationCacheManager appCacheManager = null;

        public CacheUpdateManager()
        {
            appCacheManager = null;
        }

        //given a set of ingest buckets clears the known hits for that image from cache
        public void clearCacheHitsOnIngest_OR_Remove(string appKey, int eventID, List<string> in_bucketsModified)
        {
            foreach (var bucket in in_bucketsModified)
            {
                //remove known hits for this bucket
                appCacheManager.clearCacheHitsOnIngest_OR_Remove(appKey, eventID, bucket);
            }
        }
    }
}
