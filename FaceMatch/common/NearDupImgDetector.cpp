
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
#include "NearDupImgDetector.h"
#include <fstream>
#include <memory>

namespace FaceMatch
{

void NearDupImgDetector::init(const string & ImgInpFN, istream & is, const string & RepoPath, unsigned flags)
{
	ImgFtrHaarWavelet::setDistScale(0.75); // for strict similarity threshold at 0.1
	Ptr<ImgDscHaarWhole> inHaarDscr = ImgInpFN.empty() ? 0 : new ImgDscHaarWhole(ImgInpFN);
	string repoFNLine;
	while(!is.eof())
	{
		getline(is, repoFNLine); if (repoFNLine.empty() || repoFNLine[0]=='#') continue;
		try
		{
			if (inHaarDscr) match(*inHaarDscr, repoFNLine, flags);
			else process(repoFNLine, flags);
		}
		catch(const FaceMatch::Exception & e)
		{
			cerr<<repoFNLine<<": "<<e.what()<<endl;
		}
		catch(const exception & e)
		{
			cerr<<repoFNLine<<": "<<e.what()<<endl;
		}
		catch(...)
		{
			cerr<<repoFNLine<<": unknown error"<<endl;
		}
	}
}

NearDupImgDetector::NearDupImgDetector(const string & ImgInpFN, istream & is, const string & RepoPath, unsigned flags):
	mImgRepoPath(RepoPath)
{
	init(ImgInpFN, is, RepoPath, flags);
}
NearDupImgDetector::NearDupImgDetector(const string & ImgInpFN, const string InpLstFN, const string & RepoPath, unsigned flags):
	mImgRepoPath(RepoPath)
{
	ifstream is(InpLstFN);
	init(ImgInpFN, is, RepoPath, flags);
}
void NearDupImgDetector::process(const string & imgFNLine, unsigned flags)
{
	string imgFN, NearDupFN;
	stringstream strm(imgFNLine);
	getline(strm, imgFN, '\t');
	Image img(mImgRepoPath+imgFN);
	const string & ImgKey = (flags & keepAttributes) ? imgFNLine : imgFN;
	if (flags)
	{
		bool bSave=flags&genSave;
		collect(img, ImgKey);
		if (flags & genCrop)
		{
			crop(imgFN, img, 0.90, bSave);
			crop(imgFN, img, 0.75, bSave);
		}
		if (flags & genScale)
		{
			scale(imgFN, img, 2.0, bSave);
			scale(imgFN, img, 0.5, bSave);
		}
		if (flags & genRotate)
		{
			rotate(imgFN, img, PI/12, bSave);
			rotate(imgFN, img, -PI/12, bSave);
		}
		if (flags & gen90DegreePhases)
		{
			ParallelErrorStream PES;
		#pragma omp parallel for shared(PES, img)
			for (int rp=Image::tphOne; rp<int(Image::tphCount); ++rp)
				try
				{
					Image dst(img, (Image::ETurnPhase)rp);
					Ptr<ImgDscHaarWhole> dstHD(new ImgDscHaarWhole(imgFN, dst));
					const ImgDscHaarWhole & dsc = *dstHD;
					string NearDupFN = mHaarSigIndex.findClosest(dsc);
					if (!NearDupFN.empty())
					#pragma omp critical
						mImgNearDupCollection.update(imgFN, NearDupFN);
				}
				PCATCH(PES, format("phase %d problem", rp))
			PES.report("parallel NearDupImgDetector::process gen90DegreePhases errors");
		}
		if (flags & useDups)
		{
			getline(strm, NearDupFN, '\t');
			collect(img, ImgKey, NearDupFN);
		}
	}
	else collect(img, ImgKey);
}
void NearDupImgDetector::crop(const string & imgFN, const Image & src, REALNUM a, bool bSave)
{
	Mat m(src.mx());
	Rect r(0,0, src.width(), src.height());
	stretch(r, a);
	Image dst(m, r);
	collectMod(dst, imgFN, "crop", a, bSave);
}
void NearDupImgDetector::collectMod(const Image & dst, const string & srcFN, const string & suffix, float param, bool bStore, bool verify)
{
	stringstream dstFNStrm; dstFNStrm<<srcFN<<"."<<suffix<<"."<<param<<".jpg";
	string dstFN = dstFNStrm.str();
	if (bStore) dst.store(mImgRepoPath+dstFN);
	collect(dst, dstFN, srcFN, verify);
}
void NearDupImgDetector::collect(const Image & img, const string & imgFN, string NearDupFN, bool verify)
{
	Ptr<ImgDscHaarWhole> pImgDsc = new ImgDscHaarWhole(img);
	const ImgDscHaarWhole & dsc = *pImgDsc;
	if (NearDupFN.empty())
		NearDupFN = mHaarSigIndex.findClosest(dsc);
	else // perform checks
	{
		if (imgFN == NearDupFN)
			throw Exception("self similarity: "+imgFN+"\t"+NearDupFN);
		if (verify)
			if (!mHaarSigIndex.verifySimilar(NearDupFN, dsc))
				throw Exception(
					format("unable to verify similarity dist(%s,%s)=%f",
						imgFN.c_str(), NearDupFN.c_str(), dsc.dist(ImgDscHaarWhole(mImgRepoPath+NearDupFN))
						));
	}
	mImgNearDupCollection.add(img, imgFN, NearDupFN);
	mHaarSigIndex.insert(imgFN, pImgDsc);
}

void NearDupImgDetector::match(const ImgDscHaarWhole & inHD, const string & repoFNLine, unsigned flags)
{
	stringstream strm(repoFNLine);
	string imgFN; getline(strm, imgFN, '\t');
	string imgPath = mImgRepoPath+imgFN;
	Image img(imgPath);
	Ptr<ImgDscHaarWhole> pHD(new ImgDscHaarWhole(img));
	// TODO: react to flags, e.g. rotation or scale
	if (similar(*pHD, inHD))
	{
		mImgNearDupCollection.add(img, imgFN);
		mHaarSigIndex.insert(imgFN, pHD);
	}
}
string NearDupImgDetector::getNearDups(bool unique)
{
	stringstream strm;
	printNearDups(strm, unique);
	return strm.str();
}

} // namespace FaceMatch
