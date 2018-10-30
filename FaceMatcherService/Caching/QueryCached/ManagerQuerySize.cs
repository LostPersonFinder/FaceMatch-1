using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.Concurrent;

namespace FaceMatcherService
{
    /// <summary>
    /// class that attempts to reduce the size of the query neccessary
    /// </summary>
    public class QuerySizeManager : IQuerySizeManager
    {
        IApplicationCacheManager iCM = null;

        public QuerySizeManager()
        {
            iCM = null; 
        }


        /// <summary>
        ///given a set of query buckets and the localized image to find, the interface, attempts to lookup the known hits for the cache.
        ///each bucket for which the cache returns hits are removed from the input list leaving only the buckets for which the cache lookup was empty.
        ///only the remaining buckets in the input list are sent to the core for query processing, if no buckets remain then we are done.
        /// </summary>
        /// <param name="appKey"></param>
        /// <param name="eventID"></param>
        /// <param name="in_bucketsToQuery"></param>
        /// <param name="urlregs"></param>
        /// <param name="out_cacheHitsForQueryBuckets"></param>
        /// <returns>a dictionary of bucket hits for the urlregs</returns>
        public bool tryReduceQuerySizeFromCacheLookup(string appKey, int eventID, List<string> in_bucketsToQuery, string urlregs, out ConcurrentDictionary<string, string> out_cacheHitsForQueryBuckets)
        {
            bool reduced = false;

            out_cacheHitsForQueryBuckets = new ConcurrentDictionary<string,string>();

            string hits = string.Empty;
            string[] buckets = in_bucketsToQuery.ToArray();

            foreach (var bucket in buckets)
            {
                //see if there are known hits for this urlregs
                if (iCM.getCacheHits(appKey, eventID, urlregs, bucket, ref hits))
                {
                    //add
                    while (!out_cacheHitsForQueryBuckets.TryAdd(bucket, hits)) ;
                    
                    //remove from processing
                    in_bucketsToQuery.Remove(bucket);

                    reduced = true;
                }
            }

            return reduced;
        }
    }
}
