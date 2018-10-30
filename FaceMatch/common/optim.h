
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

#pragma once // 2014-2016 (C)  adapted from OpenCV-3

#include "dcl.h"
#include <cv.h>

using namespace cv;

namespace FaceMatch
{

/// objective function optimizer interface
class LIBDCL Solver: public Algorithm
{
public:
	/// objective function encapuslator
	class LIBDCL Function
	{
	public:
		virtual ~Function() {}
		/// compute the instance value at the given argument
		virtual double calc(const double* x) const=0;
		/// Get objective function gradient at the given location
		virtual void getGradient(const double* /**< argument */, double* /**< gradient at the argument */) {}
	};
	/// @return smart pointer to the objective function
	virtual Ptr<Function> getFunction() const = 0;
	/// set a pointer to the objective function
	virtual void setFunction(const Ptr<Function>& f) = 0;
	/// @return termination criteria structure
	virtual TermCriteria getTermCriteria() const = 0;
	/// set termination criteria structure
	virtual void setTermCriteria(const TermCriteria& termcrit) = 0;
	/**
	 * Minimize the objective function.
	 * x contains the initial point before the call and the minima position (if algorithm converged) after.
	 * x is assumed to be (something that getMat() returns) row-vector or column-vector.
	 * It's size and should be consisted with previous dimensionality data given, if any (otherwise, it determines dimensionality)
	 */
	virtual double minimize(InputOutputArray x /**< argument */) = 0;
};

//! downhill simplex class
class LIBDCL DownhillSolver : public Solver
{
public:
	//! returns row-vector, even if the column-vector was given
	virtual void getInitStep(OutputArray step) const=0;
	//!This should be called at least once before the first call to minimize() and step is assumed to be (something that
	//! after getMat() will return) row-vector or column-vector. *It's dimensionality determines the dimensionality of a problem.*
	virtual void setInitStep(InputArray step)=0;
};

/// @return a smart pointer to a new down-hill solver instance
LIBDCL Ptr<DownhillSolver> createDownhillSolver(const Ptr<Solver::Function>& f=Ptr<Solver::Function>(), ///< objective function
	InputArray initStep=Mat_<double>(1,1,0.0), ///< initial step
	TermCriteria termcrit=TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS,5000,0.000001) ///< termination criteria: both minRange & minError are specified by termcrit.epsilon; In addition, user may specify the number of iterations that the algorithm does
	);

//! conjugate gradient method
class LIBDCL ConjGradSolver : public Solver{};

LIBDCL Ptr<ConjGradSolver> createConjGradSolver(const Ptr<Solver::Function>& f=Ptr<ConjGradSolver::Function>(),
	TermCriteria termcrit=TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS,5000,0.000001));

//!the return codes for solveLP() function
enum
{
	SOLVELP_UNBOUNDED    = -2, //problem is unbounded (target function can achieve arbitrary high values)
	SOLVELP_UNFEASIBLE    = -1, //problem is unfeasible (there are no points that satisfy all the constraints imposed)
	SOLVELP_SINGLE    = 0, //there is only one maximum for target function
	SOLVELP_MULTI    = 1 //there are multiple maxima for target function - the arbitrary one is returned
};

LIBDCL int solveLP(const Mat& Func, const Mat& Constr, Mat& z);
LIBDCL void denoise_TVL1(const std::vector<Mat>& observations,Mat& result, double lambda=1.0, int niters=30);

} // namespace FaceMatch
