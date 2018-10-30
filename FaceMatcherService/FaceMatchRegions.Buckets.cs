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
using System.Collections.Concurrent;

namespace FaceMatcherService
{
    /// <summary>
    /// implements the bucketing strategy for the FaceMatch server
    /// </summary>
    public class ApplicationBucketManager : ConcurrentDictionary<string, EventBucketManager>, IApplicationBucket
    {
        public ApplicationBucketManager()
        {
        }

        void RegisterApp(string app)
        {
            if(base.ContainsKey(app) == false)
                while(!TryAdd(app, new EventBucketManager()));
        }

        /// <summary>
        /// returns the bucket manager for the given application
        /// </summary>
        /// <param name="app"></param>
        /// <returns></returns>
        EventBucketManager GetEventBucketManagerForApp(string app)
        {
            if (ContainsKey(app))
                return this[app];
            else
                RegisterApp(app);

            return this[app];
        }

        public void syncOverflowBucketCountsForEvent(string app, int eventID, string values)
        {
            OverflowBucketManager obm = GetEventBucketManagerForApp(app).GetOverflowBucketManagerForEvent(eventID);

            if(obm != null)
                obm.parseOverflowValuesForBucket(values);
        }

        public List<string> queryGetBuckets(string app, int eventID, string gender, int age)
        {
            List<string> queryBuckets = new List<string>();

            OverflowBucketManager obm = GetEventBucketManagerForApp(app).GetOverflowBucketManagerForEvent(eventID);
            if (obm != null)
                return obm.queryGetBuckets(gender, age);

            return queryBuckets;
        }

        public List<string> ingestGetBuckets(string app, int eventID, string gender, int age)
        {
            List<string> ingestBuckets = new List<string>();

            OverflowBucketManager obm = GetEventBucketManagerForApp(app).GetOverflowBucketManagerForEvent(eventID);
            if (obm != null)
                return obm.ingestGetBuckets(gender, age);

            return ingestBuckets;
        }

        public bool isPenalizeBucket(string app, int eventID, string gender, int age, string bucketNameWithOverflowNumbering)
        {
            OverflowBucketManager obm = GetEventBucketManagerForApp(app).GetOverflowBucketManagerForEvent(eventID);
            if (obm != null)
                return obm.isPenalizeBucketForGenderandAge(gender, age, bucketNameWithOverflowNumbering);

            return false;
        }

        public void UnRegisterApp(string app)
        {
            if (base.ContainsKey(app) == true)
            {
                EventBucketManager removed = null;
                while (!TryRemove(app, out removed));
            }
        }

        public void UnRegisterEvent(string app, int eventID)
        {
            EventBucketManager ebm = GetEventBucketManagerForApp(app);
            if (ebm != null)
                ebm.UnRegisterEvent(eventID);
        }
    }

    public class EventBucketManager : ConcurrentDictionary<int, OverflowBucketManager>
    {
        public EventBucketManager()
        {
        }

        void RegisterEvent(int eventID)
        {
            if (!base.ContainsKey(eventID))
                while (!TryAdd(eventID, new OverflowBucketManager()));
        }

        public OverflowBucketManager GetOverflowBucketManagerForEvent(int eventID)
        {
            if (ContainsKey(eventID))
                return this[eventID];
            else
                RegisterEvent(eventID);

            return this[eventID];
        }

        public void UnRegisterEvent(int eventID)
        {
            if (base.ContainsKey(eventID))
            {
                OverflowBucketManager removed = null;
                while (!TryRemove(eventID, out removed)) ;
            }
        }
    }

    public partial class OverflowBucketManager : ConcurrentDictionary<string, int>
    {
        //maximum bucketsize of 1000 records
        readonly char[] sepPeriod = { '.' };

        //bucket -> level
        public OverflowBucketManager()
        {
            createTopLevelBuckets();
        }

        private void createTopLevelBuckets()
        {
            string[] gender = { "Male", "Female", "GenderUnknown" };
            string[] age = { "Youth", "Adult", "AgeUnknown" };

            foreach (string g in gender)
                foreach (string a in age)
                    while (!base.TryAdd(g + '.' + a, 1)) ;
        }

        private int getOverflowValueForBucket(string bucket)
        {
            if (ContainsKey(bucket))
                return this[bucket];
            return -1; 
        }

        private void updateOverflowValueForBucket(string bucket, int value)
        {
            int oldVal = getOverflowValueForBucket(bucket);
            if(oldVal != -1 && oldVal < value)
                this[bucket] = value;
        }

        public void parseOverflowValuesForBucket(string values)
        {
            using (StringReader reader = new StringReader(values))
            {
                string line = "";
                while ((line = reader.ReadLine()) != null)
                {
                    //updateOverflowValueForBucket
                    string[] parts = line.Split(sepPeriod, StringSplitOptions.RemoveEmptyEntries);
                    if (parts != null && parts.Length > 1)
                    {
                        string bucket = parts[0] + '.' + parts[1];
                        int value = -1;

                        if (int.TryParse(parts[2], out value))
                            updateOverflowValueForBucket(bucket, value);
                    }
                }
            }
        }
    }
}

