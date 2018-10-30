using System.Collections.Concurrent;
using System.Collections.Generic;

namespace FaceMatcherService
{
    public interface ICacheUpdateManager
    {
        //given a set of ingest buckets clears the known hits for that image from cache
        void clearCacheHitsOnIngest_OR_Remove(string appKey, int eventID, List<string> in_bucketsModified);
    }
}
