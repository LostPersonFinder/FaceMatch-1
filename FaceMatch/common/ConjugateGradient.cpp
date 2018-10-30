
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

#include "common.h" // 2014 (C)  adapted from OpenCV-3

namespace FaceMatch
{

#define SEC_METHOD_ITERATIONS 4
#define INITIAL_SEC_METHOD_SIGMA 0.1

/// a conjugate gradient implementation
class ConjGradSolverImpl : public ConjGradSolver
{
public:
	/// Instantiate.
	ConjGradSolverImpl();
	/// @return a smart pointer to the objective function
	Ptr<Function> getFunction() const;
	/// Set a smart pointer to the objective function
	void setFunction(const Ptr<Function>& f ///< a smart pointer to the objective function
		);
	/// @return termination criteria
	TermCriteria getTermCriteria() const;
	/// Set termination criteria
	void setTermCriteria(const TermCriteria& termcrit ///< a termination criteria reference
		);
	/// @return the minimal value of the objective function
	double minimize(InputOutputArray x ///< objective function argument
		);
protected:
	/// an objective function smart pointer
	Ptr<Solver::Function> _Function;
	/// minimization termination criteria
	TermCriteria _termcrit;
private:
	Mat_<double> d, r, buf_x, r_old;
	Mat_<double> minimizeOnTheLine_buf1, minimizeOnTheLine_buf2;
	static void minimizeOnTheLine(Ptr<Solver::Function> _f,Mat_<double>& x,const Mat_<double>& d,Mat_<double>& buf1,Mat_<double>& buf2);
};

void ConjGradSolverImpl::minimizeOnTheLine(Ptr<Solver::Function> _f,Mat_<double>& x,const Mat_<double>& d,Mat_<double>& buf1,
	Mat_<double>& buf2){
		double sigma=INITIAL_SEC_METHOD_SIGMA;
		buf1=0.0;
		buf2=0.0;

//		TRACE("before minimizeOnTheLine\n");
//		TRACE("x:\n");
		dmp(x);
//		TRACE("d:\n");
		dmp(d);

		for(int i=0;i<SEC_METHOD_ITERATIONS;i++){
			_f->getGradient((double*)x.data,(double*)buf1.data);
//			TRACE("buf1:\n");
			dmp(buf1);
			x=x+sigma*d;
			_f->getGradient((double*)x.data,(double*)buf2.data);
//			TRACE("buf2:\n");
			dmp(buf2);
			double d1=buf1.dot(d), d2=buf2.dot(d);
			if((d1-d2)==0){
				break;
			}
			double alpha=-sigma*d1/(d2-d1);
//			TRACE("(buf2.dot(d)-buf1.dot(d))=%f\nalpha=%f\n",(buf2.dot(d)-buf1.dot(d)),alpha);
			x=x+(alpha-sigma)*d;
			sigma=-alpha;
		}

//		TRACE("after minimizeOnTheLine\n");
		dmp(x);
}

double ConjGradSolverImpl::minimize(InputOutputArray x){
	CV_Assert(_Function.empty()==false);
//	TRACE("termcrit:\n\ttype: %d\n\tmaxCount: %d\n\tEPS: %g\n",_termcrit.type,_termcrit.maxCount,_termcrit.epsilon);

	Mat x_mat=x.getMat();
	CV_Assert(MIN(x_mat.rows,x_mat.cols)==1);
	int ndim=MAX(x_mat.rows,x_mat.cols);
	CV_Assert(x_mat.type()==CV_64FC1);

	if(d.cols!=ndim){
		d.create(1,ndim);
		r.create(1,ndim);
		r_old.create(1,ndim);
		minimizeOnTheLine_buf1.create(1,ndim);
		minimizeOnTheLine_buf2.create(1,ndim);
	}

	Mat_<double> proxy_x;
	if(x_mat.rows>1){
		buf_x.create(1,ndim);
		Mat_<double> proxy(ndim,1,(double*)buf_x.data);
		x_mat.copyTo(proxy);
		proxy_x=buf_x;
	}else{
		proxy_x=x_mat;
	}
	_Function->getGradient((double*)proxy_x.data,(double*)d.data);
	d*=-1.0;
	d.copyTo(r);

//	TRACE("proxy_x\n");dmp(proxy_x);
//	TRACE("d first time\n");dmp(d);
//	TRACE("r\n");dmp(r);

	double beta=0;
	for(int count=0;count<_termcrit.maxCount;count++){
		minimizeOnTheLine(_Function,proxy_x,d,minimizeOnTheLine_buf1,minimizeOnTheLine_buf2);
		r.copyTo(r_old);
		_Function->getGradient((double*)proxy_x.data,(double*)r.data);
		r*=-1.0;
		double r_norm_sq=norm(r);
		if(_termcrit.type==(TermCriteria::MAX_ITER+TermCriteria::EPS) && r_norm_sq<_termcrit.epsilon){
			break;
		}
		r_norm_sq=r_norm_sq*r_norm_sq;
		beta=MAX(0.0,(r_norm_sq-r.dot(r_old))/r_norm_sq);
		d=r+beta*d;
	}



	if(x_mat.rows>1){
		Mat(ndim, 1, CV_64F, (double*)proxy_x.data).copyTo(x);
	}
	return _Function->calc((double*)proxy_x.data);
}

ConjGradSolverImpl::ConjGradSolverImpl(){
	_Function=Ptr<Function>();
}
Ptr<Solver::Function> ConjGradSolverImpl::getFunction()const{
	return _Function;
}
void ConjGradSolverImpl::setFunction(const Ptr<Function>& f){
	_Function=f;
}
TermCriteria ConjGradSolverImpl::getTermCriteria()const{
	return _termcrit;
}
void ConjGradSolverImpl::setTermCriteria(const TermCriteria& termcrit){
	CV_Assert((termcrit.type==(TermCriteria::MAX_ITER+TermCriteria::EPS) && termcrit.epsilon>0 && termcrit.maxCount>0) ||
		((termcrit.type==TermCriteria::MAX_ITER) && termcrit.maxCount>0));
	_termcrit=termcrit;
}
/// @return a smart pointer to a new instance of the conjugate gradient solver
Ptr<ConjGradSolver> createConjGradSolver(const Ptr<Solver::Function>& f, ///< smart pointer reference to an objective function object
	TermCriteria termcrit ///< a termination criteria: both minRange & minError are specified by termcrit.epsilon; In addition, user may specify the number of iterations that the algorithm does.
	)
{
	ConjGradSolver *CG=new ConjGradSolverImpl();
	CG->setFunction(f);
	CG->setTermCriteria(termcrit);
	return Ptr<ConjGradSolver>(CG);
}

} // namespace FaceMatch
