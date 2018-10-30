
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

#ifndef commonH
#define commonH

#include "dcl.h"

/// definition of a byte
typedef unsigned char byte;

/// major running modes
enum ERunMode
{
	rmExec, ///< execute
	rmTest, ///< test
	rmEval, ///< evaluate
	rmOptm ///< optimize
};

// standard
#include <string>
#include <iostream>
#include <atomic>
#include <vector>

namespace std
{
template<typename TEL>
inline ostream & operator<<(ostream & s, const vector<TEL> & c)
{
	s<<"["; for (const auto & e: c) s<<" "<<e; s<<"]";
	return s;
}
}

using namespace std;

// generic
#include "ios_supp.h"
#include "math_supp.h"
#include "string_supp.h"
#include "omp_supp.h"
#include "sys_supp.h"
#include "cv_supp.h"

using namespace cv;

// common
#include "Exception.h"
#include "FaceRegions.h"
#include "ImgDscBase.h"
#include "Timing.h"
#include "Diagnostics.h"
#include "GPU_supp.h"
#include "optim.h"

namespace FaceMatch
{
/// query options/flags
enum eQueryOptions
{
	qoNone=0, ///< default queries
	qoSkipSelf=1, ///< skip self match
	qoRotationPhases=1<<1 ///< use pi/2 rotation phases
};

/// weighted evaluation methods
enum EWeighedEvaluation
{
	weNonWeighed, ///< use non-waited evaluation
	weAntiDist, ///< use anti-distance weighting
	weAntiRank, ///< use anti-rank weighting
	weInvRank, ///< use inverse rank weighting
	weEvalKindMask=0xFF, ///< evaluation kind mask
// flags
	weSelfExclude=1<<8, ///< exclude self match in queries
	weQueryNonVariant=1<<9 ///< ignore image variants (rotation/scale/crop) in queries
};

/// maximum source image diameter (e.g. for display purposes)
const unsigned cMaxScrImgDim=900; // TODO: config

static const int maxInteger = numeric_limits<int>::max();
static const REALNUM maxREALNUM = numeric_limits<REALNUM>::max();
static const string nemo; // an empty string for convenience of returning refs

/// \return integer closest to the given real number
inline int round(REALNUM a /**< [in] real to be rounded*/){ return a < 0 ? ceil(a - 0.5) : floor(a + 0.5); }

/**
 * Determine if the input character is "white space".
 * @param c	input character
 * @return is it white space?
 */
inline bool isWhiteSpace(int c){ return strchr("\t\r\n ", c); }
/// face region kinds
static const string cFaceKind="fpu";
/// rectangular top level region kinds
static const string cRectKindTop=cFaceKind+"s";
/// @return is it a rectangular face region kind (face, profile, skin, user)?
inline bool isRectKindTop(int c /**< region kind */){ return contains(cRectKindTop, c); }
/// rectangular face sub-region kinds
static const string cRectKindSub="inme";
/// @return is it a rectangular face sub-region kind (eye, nose, mouth, ear)?
inline bool isRectKindSub(int c /**< region kind */){ return contains(cRectKindSub, c); }
/// @return k is a face sub-region (eye, nose, mouth, ear)?
inline bool isRectKindSub(const string & k /**< region kind*/){ return isRectKindSub(k[0]); }
/// \return is it a rectangular (sub-)region kind?
inline bool isRectKind(int c /**< face region prefix*/){ return isRectKindTop(c) || isRectKindSub(c); }
/// \return is it a rectangular top-level face region: face, profile, skin, user?
inline bool isFRRectKind(const string & k /**< face region*/){ return isRectKind(k[0]); }
/// \return is region face/profile/user kind?
inline bool isFaceKind(const string & rgn /**< face region*/){ return strchr(cFaceKind.c_str(), rgn[0]); }
/// \return does prefix correspond to a rectangular face/profile (but not skin or user) region kind?
inline bool isFaceOrProfileKind(int c /**< region prefix*/){ return strchr("fp", c); }
/// \return is region a rectangular face/profile (but not skin or user) region kind?
inline bool isFaceOrProfileKind(const string & rgn /**< face region*/){ return isFaceOrProfileKind(rgn[0]); }
/// @return can the region kind be counted for evaluation purposes?
inline bool isEvalKind(int c /**< region kind */){ return strchr("fpinm", c); }
/// @return can the region kind be counted for evaluation purposes?
inline bool isEvalKind(const string & RgnKind/**< region kind */){ return isEvalKind(RgnKind[0]); }
/// @return does extension represent a Mark-up Language file: .xml or .yml
inline bool isMLE(const string & fext /**< file extension */)
{
	string ext = toUC(fext);
	return ext==".XML" || ext==".YML" || ext==".YAML" || ext==".GZ";
}

static const REALNUM
	PIo2=PI/2,
	invPIo2=1/PIo2;

/**
 * Define a monotone increasing, differentiable map (-inf,inf)->(-1,1),
 * which can be conveniently used in distance mapping, because
 * a) it maps [0,inf)->[0,1)
 * b) f(0) = 0, f'(0) = 1.
 * @param  d value in (-inf, inf)
 * @return value in (-1,1)
 */
inline REALNUM R2Unit(REALNUM d)
{
	return invPIo2*atan(PIo2*d);
}

/**
 * Define a monotone increasing, differentiable map (-1,1)->(-inf,inf),
 * which can be conveniently used in distance mapping, because
 * a) it maps [0,1)->[0,inf)
 * b) f(0) = 0, f'(0) = 1.
 * @param  d value in (-1, 1)
 * @return value in (-inf,inf)
 */
inline REALNUM Unit2R(REALNUM d)
{
	return invPIo2*tan(PIo2*d);
}

/**
 * Define a monotone increasing, differentiable map (-inf,inf)->(0,1),
 * which can be conveniently used in distance mapping.
 * @param  d value in (-inf, inf)
 * @return value in (0,1)
 */
inline REALNUM R2Unit01(REALNUM d)
{
	return 0.5+atan(d)/PI;
}

const unsigned gDefaultTopN=1<<10; ///< practical top-N result count for kNN queries
const REALNUM
	gDefaultDistQueryT=0.5, ///< practical distance query threshold in [0,1] for radius queries
	gDefaultMaxQueryT=gDefaultTopN+gDefaultDistQueryT; ///< practical limit on query threshold: default top-N plus default distance threshold

/// @return a reasonable maximum query threshold, balancing speed and accuracy
inline REALNUM getMaxQueryT(REALNUM MatchT, ///< input query threshold
	REALNUM MaxQueryT=gDefaultMaxQueryT ///< max query threshold
	)
{
	return max(MatchT, MaxQueryT);
}

} // namespace FaceMatch

#endif // commonH
