
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

#pragma once // 2011-2013 (C) 

#include "ImgNearDupCollection.h"
#include "SigIndexHaar.h"

using namespace std;

namespace FaceMatch
{

/// near-duplicate detector options/flags
enum ENearDupOptions
{
	genScale = 1, ///< generate scaled near-dups
	genRotate = 1<<1, ///< generate rotated near-dups
	genCrop = 1<<2, ///< generate cropped near-dups
	gen90DegreePhases = 1<<3, ///< generate pi/2-phased near-dups
	genAll = genScale|genRotate|genCrop|gen90DegreePhases, ///< generate all near-dups
	genSave = 1<<4, ///< save generated near-dups
	useDups = 1<<5, ///< use (generated) near-dups in indexing/evaluation
	keepAttributes = 1<<6 ///< keep image faces/attributes
};

/**
 * Encapsulate near duplicate image detection functionality based on
 * Charles E. Jacobs, et al., "Fast Multiresolution Image Querying", SIGGRAPH 1995.
 */
class LIBDCL NearDupImgDetector
{
	const string mImgRepoPath;
	ImgNearDupCollection mImgNearDupCollection;
	SigIndexHaarWhole mHaarSigIndex;
	void init(const string & ImgInpFN, istream & is, const string & RepoPath, unsigned flags);
	void collectMod(const Image & dst, const string & srcFN, const string & suffix, float param, bool bStore, bool verify=true);
	void crop(const string & imgFN, const Image & src, REALNUM a, bool bSave);
	void scale(const string & imgFN, const Image & src, REALNUM a, bool bSave)
	{
		Image dst(src, src.width()*a, src.height()*a);
		collectMod(dst, imgFN, "scale", a, bSave);
	}
	void rotate(const string & imgFN, const Image & src, REALNUM a, bool bSave)
	{
		Image dst(src); dst.rotateR(a);
		collectMod(dst, imgFN, "rotate", a, bSave);
	}
	void collect(const Image & img, const string & imgFN, string NearDupFN = "", bool verify = false);
	void match(const ImgDscHaarWhole & inHD, const string & repoFNLine, unsigned flags);
	void process(const string & imgFNLine, unsigned flags);
public:
	/**
	 * Import image file list, and for each new image, try to find its closest near-duplicate.
	 * @param ImgInpFN input image file to match against the list; if empty, proceed with near-dups in the list
	 * @param is input text stream; each line specifies image file name
	 * @param RepoPath optional image repository path; default is empty
	 * @param genImgXform generate near-duplicates via specified transforms
	 * @see ENearDupOptions
	 */
	NearDupImgDetector(const string & ImgInpFN, istream & is, const string & RepoPath = "", unsigned genImgXform = 0);
	/// Instantiate.
	NearDupImgDetector(const string & ImgInpFN, ///< input image file to match against the list; if empty, proceed with near-dups in the list
		const string InpLstFN, ///< file name of the input list, where each line specifies image file name
		const string & RepoPath = "", ///< optional image repository path; default is empty
		unsigned genImgXform=0 ///< generate near-duplicates via specified transforms as in FaceMatch::ENearDupOptions
		);
	/**
	 * Output detected near-duplicates.
	 * @param os output text stream
	 * @param unique output only unique, de-duplicated image names
	 */
	void printNearDups(ostream & os, bool unique = false)
	{
		mImgNearDupCollection.OutputUnique = unique;
		os<<mImgNearDupCollection;
	}
	/**
	 * Get the output of near-duplicates.
	 * This is an easier but less efficient method for getting the near-dups.
	 * @param unique output only unique, de-duplicated image names
	 */
	string getNearDups(bool unique = false);
};

} // namespace FaceMatch
