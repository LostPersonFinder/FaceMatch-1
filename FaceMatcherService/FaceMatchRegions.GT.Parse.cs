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
using System.Text.RegularExpressions;
using System.Diagnostics.Eventing.Reader;

namespace FaceMatcherService
{
    public partial class FaceMatchRegions : IFaceMatchRegions
    {
        List<string> convertToNewFormatAndGroupForIngest(string idRegsInMixedFormat)
        {
            List<string> collectOut = new List<string>();

            //check if already in new format if so break into groups and return
            string[] parts = idRegsInMixedFormat.Split(new char[]{'}'}, StringSplitOptions.RemoveEmptyEntries);
            if(parts.Count(n => !n.Contains("d[")) == 0)
            {
                foreach (var item in parts)
                    collectOut.Add(item + "}");

                return collectOut;
            }


            char[] tabSep = new char[] { '\t' };
            String[] splits = idRegsInMixedFormat.Split(tabSep, StringSplitOptions.RemoveEmptyEntries);

            //empty - no id, no faces
            if ((splits == null) || (splits != null && splits.Length == 0))
                return collectOut;

            string id = string.Empty;

            //return old format id, only id, no faces
            if (splits.Length == 1 && splits[0].Contains("[") == false)
            {
                id = splits[0];
                collectOut.Add(id);
                return collectOut;
            }

            //extract id
            if (splits.Length > 1 && splits[0].Contains("[") == false)
                id = splits[0];

            //seperate out id (exists) from faces
            string onlyFacesOrNewFormatString = string.Empty;
            if(id.Length > 0)
                for (int i = 1; i < splits.Length; i++)
                    onlyFacesOrNewFormatString += tabSep[0] + splits[i];

            splits = onlyFacesOrNewFormatString.Split(tabSep, StringSplitOptions.RemoveEmptyEntries);

            for (int i = 0; i < splits.Length; i++)
            {
                //on old format face, we have id : convert to new format face
                if( (id.Length != 0) && 
                    (splits[i].IndexOf("f[") == 0) &&
                    (splits[i].IndexOf("]") == splits[i].Length - 1)
                    )
                {
                    splits[i] = splits[i].Replace("f[", "f{[") + "\td[" + id + "]}";
                    collectOut.Add(splits[i]);
                }

                //on old format profile, we have id : convert to new format profile
                else if ((id.Length != 0) &&
                    (splits[i].IndexOf("p[") == 0) &&
                    (splits[i].IndexOf("]") == splits[i].Length - 1)
                    )
                {
                    splits[i] = splits[i].Replace("p[", "p{[") + "\td[" + id + "]}";
                    collectOut.Add(splits[i]);
                }

                //on annotated face, we have id : process entire annotation, convert to new format if needed
                else if (
                    (splits[i].IndexOf("f{[") == 0)
                    )
                {
                    string temp = string.Empty;

                    for (int j = i; j < splits.Length; j++)
                    {
                        i = j;

                        temp += tabSep[0] + splits[j];

                        if (splits[j].Contains("}"))
                        {
                            temp = temp.Trim(tabSep);
                            break;
                        }
                    }

                    if(temp.Length > 0 && temp.Contains("d[") == false && id.Length > 0)
                    {
                        temp = temp.Replace("}", "\td[" + id + "]}");
                    }

                    collectOut.Add(temp);
                }

                //on annotated profile, we have id : process entire annotation, convert to new format if needed
                else if (
                    (splits[i].IndexOf("p{[") == 0)
                    )
                {
                    string temp = string.Empty;

                    for (int j = i; j < splits.Length; j++)
                    {
                        i = j;

                        temp += tabSep[0] + splits[j];

                        if (splits[j].Contains("}"))
                        {
                            temp = temp.Trim(tabSep);
                            break;
                        }
                    }

                    if (temp.Length > 0 && temp.Contains("d[") == false && id.Length > 0)
                    {
                        temp = temp.Replace("}", "\td[" + id + "]}");
                    }

                    collectOut.Add(temp);
                }
            }

            return collectOut;
        }

        private bool annGroupsHaveInvalidRegions(List<string> annGroup)
        {
            foreach (var item in annGroup)
            {
                if (item.Contains("f[") || item.Contains("p[") || item.Contains("f{[") || item.Contains("p{["))
                {
                    FaceWithLandmarks face = new FaceWithLandmarks(item);
                    if (!face.isValid)
                        return true;
                }
            }
            return false;
        }

        private List<IngestData> parseGT(String groupIDRegsIn, ref String genderIn, int ageIn, ref String errorString)
        {
            List<IngestData> ingestList = new List<IngestData>();

            //no regions, no age, gender or skin
            if (!groupIDRegsIn.Contains('{') && !groupIDRegsIn.Contains('}') &&
                !groupIDRegsIn.Contains('[') && !groupIDRegsIn.Contains(']')
                )
            {
                //just the id, no other information given
                ingestList.Add(new IngestData() { Age = ageIn, Gender = genderIn, IDRegs = groupIDRegsIn });
                return ingestList;
            }

            //make sure that all metadata is well formed
            if (!isExpressionBalanced(ref errorString, groupIDRegsIn))
                return ingestList;

            //seperate on boundaries
            errorString = parseHelperForFaceOrProfileWithRegions(genderIn, ageIn, errorString, ingestList, groupIDRegsIn);

            return ingestList;
        }

        private string parseHelperForFaceOrProfileWithRegions(String genderIn, int ageIn, String errorString, List<IngestData> ingestList, String annotation)
        {
            char[] tabSep = new char[] { '\t' };

            const String rxFaceWithAnn = @"(p|f)\{\[[0-9]+,[0-9]+;[0-9]+,[0-9]+\](\}{0,1})";
            const String rxFaceOnly = @"(p|f)\[[0-9]+,[0-9]+;[0-9]+,[0-9]+\]";
            const String rxINME = @"([inme])\[[0-9]+,[0-9]+;[0-9]+,[0-9]+\](\}{0,1})";
            const String rxSkin = @"t\[(dark|light)\](\}{0,1})";
            const String rxAge = @"a\[(youth|adult)\](\}{0,1})";
            const String rxGender = @"g\[(male|female)\](\}{0,1})";
            const String rxDescID = @"d\[[a-zA-Z0-9_\-\/.\\]+\](\}{0,1})";

            Regex regExFaceWithAnn = new Regex(rxFaceWithAnn);
            Regex regExFaceOnly = new Regex(rxFaceOnly);
            Regex regExINME = new Regex(rxINME);
            Regex regExSkin = new Regex(rxSkin);
            Regex regExAge = new Regex(rxAge);
            Regex regExGender = new Regex(rxGender);
            Regex regExDescID = new Regex(rxDescID);

            Random rand = new Random();

            String[] items = annotation.Split(tabSep, StringSplitOptions.RemoveEmptyEntries);

            String ingestRegion = String.Empty;
            int numFaces = 0;
            int numRegions = 0;
            int numSkin = 0;
            int numAge = 0;
            int numGender = 0;

            IngestData data = new IngestData() { Age = ageIn, Gender = genderIn };

            HashSet<String> gtErrors = new HashSet<string>();

            foreach (String item in items)
            {
                //begin 
                if (regExFaceWithAnn.IsMatch(item))
                {
                    ingestRegion = item;
                    numFaces++;
                }
                else if (regExFaceOnly.IsMatch(item))
                {
                    ingestRegion = item;
                    numFaces = 1;
                }

                //save region
                else if (regExINME.IsMatch(item))
                {
                    ingestRegion += "\t" + item;
                    numRegions++;
                }

                //save ID
                else if (regExDescID.IsMatch(item))
                {
                    ingestRegion += "\t" + item;
                }

                //filter 
                else if (regExSkin.IsMatch(item))
                {
                    numSkin++;
                    continue;
                }

                //filter and update age
                else if (regExAge.IsMatch(item))
                {
                    //unknown age, GT data overrides
                    if (ageIn == -1)
                    {
                        if (item.Equals("a[youth]", StringComparison.OrdinalIgnoreCase))
                            data.Age = 18;
                        else
                            data.Age = rand.Next(27, 65);
                    }

                    numAge++;
                }

                //filter and update gender
                else if (regExGender.IsMatch(item))
                {
                    //unkown gender, GT data overrides
                    if (genderIn.Equals("GenderUnknown", StringComparison.OrdinalIgnoreCase))
                    {
                        if (item.Equals("g[male]", StringComparison.OrdinalIgnoreCase))
                            data.Gender = "Male";
                        else
                            data.Gender = "Female";
                    }

                    numGender++;
                }
                else
                {
                    //skip unknown classes
                    continue;
                }

                if (numFaces > 1)
                    gtErrors.Add("\nThere can be only one face or profile region within a primary region. Skipping ingest.");
                if (numRegions > 6)
                    gtErrors.Add("\nThere are too many regions specified as eyes, nose, ear and mouth. Skipping ingest.");
                if (numAge > 1)
                    gtErrors.Add("\nMutliple age classes specified for a single person. Skipping ingest.");
                if (numGender > 1)
                    gtErrors.Add("\nMultiple gender classes specified for a single person. Skipping ingest.");
                if (numSkin > 1)
                    gtErrors.Add("\nMultiple skin tone classes specified for a single person. Skipping ingest.");
            }

            //close region expression
            if (gtErrors.Count == 0)
            {
                if (ingestRegion.Contains('{') && !ingestRegion.Contains('}'))
                    ingestRegion += "}";

                data.IDRegs = ingestRegion;
                
                ingestList.Add(data);
            }
            else
            {
                //cancel ingest
                String collate = String.Empty;
                foreach (String error in gtErrors)
                    collate += error;
                errorString += collate.TrimStart();

                ingestList.Clear();
            }

            return errorString;
        }

        private bool isExpressionBalanced(ref String errorString, String sGT)
        {
            int stack = 0;
            foreach (char c in sGT.ToCharArray())
            {
                if (c == '{')
                {
                    if (stack != 0)
                    {
                        errorString += "The supplied metadata is incorrectly formatted. Skipping ingest.";
                        return false;
                    }

                    Interlocked.Increment(ref stack);
                }

                if (c == '[')
                    Interlocked.Increment(ref stack);

                if (c == ']')
                    Interlocked.Decrement(ref stack);

                if (c == '}')
                {
                    Interlocked.Decrement(ref stack);

                    if (stack != 0)
                    {
                        errorString += "The supplied metadata is incorrectly formatted. Skipping ingest.";
                        return false;
                    }
                }
            }

            return true;
        }

        class IngestData
        {
            //public String ID { get; set; }
            //public String Regs { get; set; }
            public String IDRegs { get; set; }

            //public String IDRegs
            //{
            //    get { return (ID.Trim() + "\t" + Regs.TrimStart()).Trim(); }
            //}

            private int age;
            public int Age
            {
                get { return age; }
                set { age = value; }
            }

            private String gender;
            public String Gender
            {
                get { return gender; }
                set { gender = value; }
            }
        }
    }
}
