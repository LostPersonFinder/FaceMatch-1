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
        public struct Match
        {
            public double score;
            public string id;
            public string line;
            public string idregn;
        }

        public Queue<Match> parseMatches(string result, bool penalize)
        {
            Queue<Match> listMatches = new Queue<Match>();

            StringReader sr = new StringReader(result);

            char[] sep = { '\t' };
            string line = "";

            while ((line = sr.ReadLine()) != null)
            {
                //skip to hits
                if (line.Length == 0 || line[0] != '0')
                    continue;

                //process hit
                string[] hits = line.Split(sep, StringSplitOptions.RemoveEmptyEntries);

                if (hits != null && hits.Length > 1)
                {
                    Match match = new Match();

                    if (double.TryParse(hits[0], out match.score))
                    {
                        //apply penalization, selective
                        if (penalize == true)
                            match.score = Math.Sqrt(match.score);

                        //this processing needs to change because of change in FM.lib formatting
                        //match.id = hits[1];
                        //match.line = line;

                        ////save id region combo
                        //match.idregn = line.Replace(hits[0], String.Empty);

                        /*
0.909028	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.78326184__1219711185.png	f{[156,6;90,90]	d[2013-yolanda.personfinder.google.orgSLASHperson.78326184__1219711185.png]}
0.909049	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.79716237__1880551399.png	f{[76,121;34,34]	d[2013-yolanda.personfinder.google.orgSLASHperson.79716237__1880551399.png]}
0.910013	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.76316148__1888149248.png	f{[67,1;45,45]	d[2013-yolanda.personfinder.google.orgSLASHperson.76316148__1888149248.png]}
0.910031	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.79196013__1099623760.png	f{[96,102;28,28]	d[2013-yolanda.personfinder.google.orgSLASHperson.79196013__1099623760.png]}
0.911006	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.79736212__1087000331.png	f{[180,92;69,69]	d[2013-yolanda.personfinder.google.orgSLASHperson.79736212__1087000331.png]}
0.920399	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.76126760__1802134166.png	f{[101,86;120,120]	d[2013-yolanda.personfinder.google.orgSLASHperson.76126760__1802134166.png]}
0.920472	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.76606035__1759384145.png	f{[152,134;104,104]	d[2013-yolanda.personfinder.google.orgSLASHperson.76606035__1759384145.png]}
0.920722	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.78476260__1456853522.png	f{[65,53;70,70]	d[2013-yolanda.personfinder.google.orgSLASHperson.78476260__1456853522.png]}
0.921307	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.76246683__1446144056.png	f{[12,130;62,62]	d[2013-yolanda.personfinder.google.orgSLASHperson.76246683__1446144056.png]}
0.921308	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.76386073__1512615731.png	f{[31,42;40,40]	d[2013-yolanda.personfinder.google.orgSLASHperson.76386073__1512615731.png]}
0.921486	C:\FaceMatchSL\Facematch\bin\UploadBin\App.PLS.event.9999.fri.1.2013-yolanda.personfinder.google.orgSLASHperson.78476109__1316319601.png	f{[71,80;60,60]	d[2013-yolanda.personfinder.google.orgSLASHperson.78476109__1316319601.png]}
                         */

                        try
                        {
                            match.id = hits[3].Replace("d[", String.Empty).Replace("]}", String.Empty);
                            match.idregn = match.id + sep[0] + hits[2].Replace("{", String.Empty);
                            match.line = match.score.ToString() + sep[0] + match.idregn;
                        }
                        catch (Exception){}

                        //enqueue
                        listMatches.Enqueue(match);
                    }
                }
            }

            return listMatches;
        }
    }
}
 