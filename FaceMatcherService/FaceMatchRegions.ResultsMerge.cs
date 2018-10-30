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
using System.Threading.Tasks;

namespace FaceMatcherService
{
    public partial class FaceMatchRegions : IFaceMatchRegions
    {
        private bool mergeOnEmpty(Queue<Match> q1, Queue<Match> q2, Queue<Match> q3)
        {
            while (q1.Count == 0 && q2.Count > 0)
                q3.Enqueue(q2.Dequeue());

            while (q1.Count > 0 && q2.Count == 0)
                q3.Enqueue(q1.Dequeue());

            return (q1.Count == 0 || q2.Count == 0);
        }

        private Queue<Match> mergePair(Queue<Match> q1, Queue<Match> q2, int limit)
        {
            if (q1 == null || q2 == null)
                return null;

            Queue<Match> q3 = new Queue<Match>();

            int counter = 0;

            while (q1.Count > 0 || q2.Count > 0)
            {
                //if a queue is empty, merge remainder of other.
                if (mergeOnEmpty(q1, q2, q3))
                    break;

                Match m1 = q1.Peek();
                Match m2 = q2.Peek();

                //we know there is at least one element in each queue
                if (m1.score < m2.score)
                    q3.Enqueue(q1.Dequeue());
                else
                    q3.Enqueue(q2.Dequeue());

                //do not merge more than user requested hits
                if (++counter >= limit)
                    break;
            }

            return q3;
        }

        private ConcurrentQueue<Queue<Match>> parallelMergeResults(ConcurrentQueue<Queue<Match>> work, int limit)
        {
            while (work.Count > 1)
            {
                //parallel merge
                Parallel.ForEach(Partitioner.Create(0, work.Count, 2), (range) =>
                {
                    //if we have a single, its merged by default
                    if ((range.Item2 - range.Item1) < 2)
                        return;

                    Queue<Match> q1 = null;
                    Queue<Match> q2 = null;
                    Queue<Match> merged = null;

                    while ((work.Count > 1) && !work.TryDequeue(out q1));
                    while ((work.Count > 0) && !work.TryDequeue(out q2));

                    //merge (stable)
                    merged = mergePair(q1, q2, limit);

                    if (merged != null && merged.Count > 0)
                        work.Enqueue(merged);
                });
            }

            return work;
        }

        private string resultsFromQueue(ConcurrentQueue<Queue<Match>> work, float tolerance, ref uint? matches)
        {
            string results = "";

            int limit = resultsLimit(tolerance);

            if (work.Count == 1)
            {
                Queue<Match> merged = null;

                //get queue
                while (!work.TryDequeue(out merged)) ;

                //ensure distinct results (we favor the best score for a given id)
                HashSet<string> idLookup = new HashSet<string>();

                int counter = 0;
                foreach (Match s in merged)
                {
                    //distinct check
                    if (idLookup.Contains(s.idregn) == false)
                        idLookup.Add(s.idregn);
                    else
                        continue;
                    
                    //accumulate results
                    if(counter++ < limit)
                        results += s.line + "\n";
                }

                if (matches != null)
                    matches = (uint)counter;
            }

            return results.TrimEnd('\n');
        }

        private int resultsLimit(float tolerance)
        {
            int limit = int.MaxValue;
            if (tolerance > 1 && tolerance < int.MaxValue)
                limit = (int)tolerance;
            return limit;
        }
    }
}
