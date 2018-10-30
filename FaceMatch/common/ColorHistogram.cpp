
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

#include "common.h"
#include "ColorHistogram.h"
#include <fstream>
#include <math.h>

using namespace std;

namespace FaceMatch
{

unsigned ColorHistogram::loadDataBuildHist(const string & FN, unsigned nMaxItemsToBeRead)
{
	ifstream f(FN.c_str());	if (!f.is_open()) throw FaceMatch::Exception("unable to open "+FN);
	unsigned nLineCounter=0;
	while (!f.eof())
	{
		string ln; getline(f, ln); if (ln.empty()) continue;
		stringstream strmItem(ln);
		const int
			dim=getDimensionCount(),
			bins=getBinCount();
		vector<REALNUM> item(dim);
		for (int j=0; j<dim; ++j) strmItem>>item[j];
		int
			i1=(int)(floor(item[0]*(bins-1))),
			i2=(int)(floor(item[1]*(bins-1))),
			i3=(int)(floor(item[2]*(bins-1)));

		++Cube[i1][i2][i3];
		++nLineCounter;
		if (nMaxItemsToBeRead && nLineCounter>=nMaxItemsToBeRead) break;
	}
	normalizeCube();
	return nLineCounter;
}
void ColorHistogram::write(FileStorage & fs)const
{
	if (!mColorSpace.empty()) fs<<"ColorSpace"<<mColorSpace;
	fs << "nBins" << nBins;
	fs << "Cube" << "[:";
	for(int i=0;i<nBins;i++)
		for(int j=0;j<nBins;j++)
			for(int k=0;k<nBins;k++)
				fs << Cube[i][j][k];
	fs << "]"; // cube
}
void ColorHistogram::read(FileStorage & fs)
{
	fs["ColorSpace"]>>mColorSpace;
	nBins=fs["nBins"];
	FileNode cube = fs["Cube"];
	FileNodeIterator it = cube.begin();
	for(int i=0;i<nBins;i++)
		for(int j=0;j<nBins;j++)
			for(int k=0;k<nBins;k++)
				Cube[i][j][k]=(*it++);
}
/// print color histogram
ostream & operator<<(ostream & s, const ColorHistogram & hst)
{
	if (!hst.mColorSpace.empty()) s<<"ColorSpace="<<hst.mColorSpace<<endl;
	s<<"nBins="<<hst.nBins <<endl;
	s<<"Cube="<<endl
	<<"["<<endl;
	for(int i=0;i<hst.nBins;i++)
	{
		s<<" ["<<endl;
		for(int j=0;j<hst.nBins;j++)
		{
			s<<"  [";
			for(int k=0;k<hst.nBins;k++) s<<" "<<hst.Cube[i][j][k];
			s<<"]"<<endl;
		}
		s<<" ]"<<endl;
	}
	s << "]"; // cube
	return s;
}
void ColorHistogram::outXYZV(const string & FN)const
{
	ofstream s(FN.c_str());
	if (!mColorSpace.empty()) s<<"# ColorSpace="<<mColorSpace<<endl;
	for(int i=0;i<nBins;i++)
		for(int j=0;j<nBins;j++)
			for(int k=0;k<nBins;k++)
			{
				double v=Cube[i][j][k];
				if (v) s<<i<<"\t"<<j<<"\t"<<k<<"\t"<<v<<endl;
			}
}
void ColorHistogram::inXYZV(const string & FN)
{
	reAllocateCube();
	ifstream s(FN.c_str());
	if (!s.is_open()) throw Exception("unable to open "+FN);
	while (s.good())
	{
		string line; getline(s, line); trim(line);
		if (line.empty()) continue;
		const string
			CSLabel="# ColorSpace=",
			BCLabel="# BinCnt=";
		if (line.find(CSLabel)==0)
			mColorSpace=line.substr(CSLabel.length());
		else if (line.find(BCLabel)==0)
			reAllocateCube(atoi(line.substr(BCLabel.length()).c_str()));
		else if (line[0]=='#') continue;
		stringstream strmLn(line);
		int i=0, j=0, k=0;
		double v=0;
		strmLn>>i>>j>>k>>v;
		if (i<nBins && j<nBins && k<nBins) Cube[i][j][k]=v;
	}
}
REALNUM ColorHistogram::getSkinLikelihood(const Vec3b & p)const // assume 0:255 range
{
	Vec3d q=p; q/=0xFF;
	REALNUM v=getSkinLikelihoodCubeHistogram(q.val, q.channels);
//	clog<<"v="<<v<<endl; // TODO: remove
	return v;
}

}; // namespace FaceMatch
