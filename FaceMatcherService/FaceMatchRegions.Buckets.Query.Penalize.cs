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
    public partial class OverflowBucketManager
    {
        //Query : Penalize buckets
        List<string> getPenalizeBucketsForGenderandAge(string gender, int age)
        {
            List<string> penalizeGenderList = new List<string>();

            if (gender.Equals("male", StringComparison.OrdinalIgnoreCase))
            {
                penalizeGenderList.Add("GenderUnknown");
            }
            else if (gender.Equals("female", StringComparison.OrdinalIgnoreCase))
            {
                penalizeGenderList.Add("GenderUnknown");
            }
            else
            {
                penalizeGenderList.Add("Male");
                penalizeGenderList.Add("Female");
            }

            List<string> ageList = new List<string>();
            List<string> penalizeAgeList = new List<string>();

            if (age >= 0 && age <= 15)
            {
                penalizeAgeList.Add("AgeUnknown");
            }
            else if (age > 15 && age < 21)
            {
                penalizeAgeList.Add("AgeUnknown");
            }
            else if (age >= 21)
            {
                penalizeAgeList.Add("AgeUnknown");
            }

            List<string> penalizeBuckets = new List<string>();

            foreach (string _gender in penalizeGenderList)
                foreach (string _age in penalizeAgeList)
                    penalizeBuckets.Add(_gender + "." + _age);

            return penalizeBuckets;
        }

        virtual public bool isPenalizeBucketForGenderandAge(string gender, int age, string bucketNameWithOverflowNumbering)
        {
            List<string> penalizeBuckets = getPenalizeBucketsForGenderandAge(gender, age);

            string[] parts = bucketNameWithOverflowNumbering.Split(sepPeriod, StringSplitOptions.RemoveEmptyEntries);

            if (parts != null && parts.Length > 1)
                return penalizeBuckets.Contains(parts[0] + '.' + parts[1]);

            return false;
        }
    }
}

//private AgeGroup[] getGroupFromAge(int age)
//{
//    if (age <= 15)
//        return new [] { AgeGroup.Youth };

//    if (age > 15 && age != -1)
//        return new [] { AgeGroup.Youth, AgeGroup.Adult };

//    return new [] { AgeGroup.Unknown };
//}
