
//Informational Notice:
//
//This software was developed under contract funded by the National Library of Medicine, which is part of the National Institutes of Health, an agency of the Department of Health and Human Services, United States Government.
//
//The license of this software is an open-source BSD license.  It allows use in both commercial and non-commercial products.
//
//The license does not supersede any applicable United States law.
//
//The license does not indemnify you from any claims brought by third parties whose proprietary rights may be infringed by your usage of this software.
//
//Government usage rights for this software are established by Federal law, which includes, but may not be limited to, Federal Acquisition Regulation (FAR) 48 C.F.R. Part52.227-14, Rights in Data?General.
//The license for this software is intended to be expansive, rather than restrictive, in encouraging the use of this software in both commercial and non-commercial products.
//
//LICENSE:
//
//Government Usage Rights Notice:  The U.S. Government retains unlimited, royalty-free usage rights to this software, but not ownership, as provided by Federal law.
//
//Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//?	Redistributions of source code must retain the above Government Usage Rights Notice, this list of conditions and the following disclaimer.
//
//?	Redistributions in binary form must reproduce the above Government Usage Rights Notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
//?	The names,trademarks, and service marks of the National Library of Medicine, the National Cancer Institute, the National Institutes of Health, and the names of any of the software developers shall not be used to endorse or promote products derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE U.S. GOVERNMENT AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITEDTO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE U.S. GOVERNMENT
//OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "common.h" // 2012-2017 (C) 
#include "Diagnostics.h"
#include "Query.h"
#include "ImageCollage.h"

using namespace std;

GLBDEFA(unsigned, VerbLevel, 0)

namespace FaceMatch
{
string getVersion()
{
	return CV_VERSION ".20170719";
}
GLBDEFA(bool, HistEQ, false)

GLBDEF(string, RepoPath)

static atomic<bool> gVisual(false);
unsigned getVisVerbLevel(){ return gVisual ? getVerbLevel() : 0; }
void setVisVerbLevel(unsigned VerbLevel){ gVisual=VerbLevel; setVerbLevel(VerbLevel); }

/**
 * Wait for a key press, then exit or throw.
 * \param delay in milliseconds, \see cv::waitKey
 * \return key pressed on loop termination (by space=nNext or oOutput)
 * \throw ESC on Esc, Quit or eXit
 */
int waitKeyThrow(int delay)
{
	int k=-1;
	for (; !strchr(" nNoO", k); k=waitKey(delay))
		if (SpecialChar(k)==ESC || strchr("qQxX", k)) throw ESC;
	return k;
}

using namespace FaceMatch;

/// Show query results in order of the increasing distance from the query image.
void displayQueryResults(const string & QryRes)
{
	QueryResults QR(QryRes);
	QR.display();
}
/// Display detailed query matches for all descriptors.
void displayQueryMatches(const string & RepoPath, const string & QueryResult, const string & ComboKind)
{
	typedef Ptr<ImageCollage> PImageCollage;
	typedef list<PImageCollage> TICPack;
	TICPack ICPack;
	PImageCollage pIC;
	string QryRgnID;
	for (stringstream StrmQR(QueryResult); !StrmQR.eof(); )
	{
		string ln; getline(StrmQR, ln); if (ln.empty()) continue;
		if (!isdigit(ln[0])) // query region
		{
			QryRgnID=ln;
			pIC=new ImageCollage(QryRgnID, RepoPath);
			ICPack.push_back(pIC);
		}
		if (!QryRgnID.empty() && pIC && QryRgnID!=ln)
			pIC->addMatch(QryRgnID, ln, ComboKind);
	}
	for (auto p: ICPack) p->show();
	waitKeyThrow();
}
void dmp(const Mat & m)
{
	if (getVerbLevel()) cout<<m<<endl;
}
} // namespace FaceMatch
