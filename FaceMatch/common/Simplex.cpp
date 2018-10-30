
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
#include "optim.h"
#include "Diagnostics.h"

using namespace std;
using namespace cv;

namespace FaceMatch
{
/// a down-hill optimizer implementation
class DownhillSolverImpl: public DownhillSolver
{
public:
	void getInitStep(OutputArray step) const;
	void setInitStep(InputArray step);
	Ptr<Function> getFunction() const;
	void setFunction(const Ptr<Function>& f);
	TermCriteria getTermCriteria() const;
	DownhillSolverImpl();
	void setTermCriteria(const TermCriteria& termcrit);
	double minimize(InputOutputArray x);
protected:
	Ptr<Solver::Function> _Function;
	TermCriteria _termcrit;
	Mat _step;
	Mat_<double> buf_x;
private:
	void createInitialSimplex(Mat_<double>& simplex, const Mat& step);
	double innerDownhillSimplex(cv::Mat_<double>& p,double MinRange,double MinError,int& nfunk,
		const Ptr<Solver::Function>& f,int nmax);
	double tryNewPoint(Mat_<double>& p,Mat_<double>& y,Mat_<double>& coord_sum,const Ptr<Solver::Function>& f,int ihi,
		double fac,Mat_<double>& ptry);
};

double DownhillSolverImpl::tryNewPoint(
	Mat_<double>& p,
	Mat_<double>& y,
	Mat_<double>&  coord_sum,
	const Ptr<Solver::Function>& f,
	int      ihi,
	double   fac,
	Mat_<double>& ptry
)
{
	int ndim=p.cols;
	int j;
	double fac1,fac2,ytry;

	fac1=(1.0-fac)/ndim;
	fac2=fac1-fac;
	for (j=0;j<ndim;j++)
	{
		ptry(j)=coord_sum(j)*fac1-p(ihi,j)*fac2;
	}
	ytry=f->calc((double*)ptry.data);
	if (ytry < y(ihi))
	{
		y(ihi)=ytry;
		for (j=0;j<ndim;j++)
		{
			coord_sum(j) += ptry(j)-p(ihi,j);
			p(ihi,j)=ptry(j);
		}
	}
	return ytry;
}

/*
Minimize Function f.
Matrix p[ndim+1][1..ndim] gives ndim+1 vertices that form a simplex, each row is an ndim vector.
nfunk accumulates the # of function evaluations.
*/
double DownhillSolverImpl::innerDownhillSimplex(
	cv::Mat_<double>&   p,
	double     MinRange,
	double     MinError,
	int&       nfunk,
	const Ptr<Solver::Function>& f,
	int nmax
	)
{
	int ndim=p.cols;
	double res=0;
	int i,ihi,ilo,inhi,j,mpts=ndim+1;
	double error, range, ysave, ytry;
	Mat_<double>
		coord_sum(1, ndim, 0.0),
		buf(1, ndim, 0.0),
		y(1, ndim+1, 0.0);

	for(i=0; i<ndim+1; ++i)
		y(i) = f->calc(p[i]);

	nfunk = ndim+1;

	reduce(p,coord_sum,0,CV_REDUCE_SUM);

	for (;;)
	{
		ilo=0;
		/*  find highest (worst), next-to-worst, and lowest (best) points by going through all of them. */
		ihi = y(0)>y(1) ? (inhi=1,0) : (inhi=0,1);
		for (i=0;i<mpts;i++)
		{
			if (y(i) <= y(ilo))
				ilo=i;
			if (y(i) > y(ihi))
			{
				inhi=ihi;
				ihi=i;
			}
			else if (y(i) > y(inhi) && i != ihi)
				inhi=i;
		}

		/* check stop criterion */
		error=fabs(y(ihi)-y(ilo));
		range=0;
		for(i=0;i<ndim;++i)
		{
			double min = p(0,i), max = p(0,i), d=0;
			for(j=1;j<=ndim;++j)
			{
				if( min > p(j,i) ) min = p(j,i);
				if( max < p(j,i) ) max = p(j,i);
			}
			d = fabs(max-min);
			if(range < d) range = d;
		}

		if(range <= MinRange || error <= MinError)
		{ /* Put best point and value in first slot. */
			std::swap(y(0),y(ilo));
			for (i=0;i<ndim;i++)
				std::swap(p(0,i),p(ilo,i));
			break;
		}

		if (nfunk >= nmax){
			TRACE("nmax exceeded\n");
			return y(ilo);
		}
		nfunk += 2;
		/*Begin a new iteration. First, reflect the worst point about the centroid of others */
		ytry = tryNewPoint(p,y,coord_sum,f,ihi,-1.0,buf);
		if (ytry <= y(ilo))
		{ /*If that's better than the best point, go twice as far in that direction*/
			ytry = tryNewPoint(p,y,coord_sum,f,ihi,2.0,buf);
		}
		else if (ytry >= y(inhi))
		{   /* The new point is worse than the second-highest, but better
			than the worst so do not go so far in that direction */
			ysave = y(ihi);
			ytry = tryNewPoint(p,y,coord_sum,f,ihi,0.5,buf);
			if (ytry >= ysave)
			{ /* Can't seem to improve things. Contract the simplex to good point
				in hope to find a simplex landscape. */
				for (i=0;i<mpts;i++)
				{
					if (i != ilo)
					{
						for (j=0;j<ndim;j++)
						{
							p(i,j) = coord_sum(j) = 0.5*(p(i,j)+p(ilo,j));
						}
						y(i)=f->calc((double*)coord_sum.data);
					}
				}
				nfunk += ndim;
				reduce(p,coord_sum,0,CV_REDUCE_SUM);
			}
		}
		else --(nfunk); /* correct nfunk */
		TRACE("iteration %d, simplex=", nfunk); dmp(p);
		TRACE(" y= "); dmp(y);
	} /* go to next iteration. */
	res = y(0);

	return res;
}
void DownhillSolverImpl::createInitialSimplex(Mat_<double> & simplex, const Mat & step)
{
	for(int i=1; i<=step.cols; ++i)
	{
		simplex.row(0).copyTo(simplex.row(i));
		simplex(i, i-1)+= 0.5*step.at<double>(0, i-1);
	}
	simplex.row(0) -= 0.5*step;
	TRACE("init simplex="); dmp(simplex);
}
double DownhillSolverImpl::minimize(InputOutputArray x)
{
	TRACE("minimizing\n");
	CV_Assert(_Function.empty()==false);
	TRACE("termcrit:\n\ttype: %d\n\tmaxCount: %d\n\tEPS: %g\n",_termcrit.type,_termcrit.maxCount,_termcrit.epsilon);
	TRACE("step\n"); dmp(_step);

	Mat x_mat=x.getMat();
	TRACE("starting at "); dmp(x_mat);
	CV_Assert(MIN(x_mat.rows,x_mat.cols)==1);
	CV_Assert(MAX(x_mat.rows,x_mat.cols)==_step.cols);
	CV_Assert(x_mat.type()==CV_64FC1);

	Mat_<double> proxy_x;

	if(x_mat.rows>1)
	{
		buf_x.create(1,_step.cols);
		Mat_<double> proxy(_step.cols,1,(double*)buf_x.data);
		x_mat.copyTo(proxy);
		proxy_x=buf_x;
	}
	else proxy_x=x_mat;

	int count=0;
	int ndim=_step.cols;
	Mat_<double> simplex=Mat_<double>(ndim+1, ndim, 0.0);

	// simplex.row(0).copyTo(proxy_x);
	proxy_x.copyTo(simplex.row(0));
	createInitialSimplex(simplex, _step);
	double res = innerDownhillSimplex(simplex, _termcrit.epsilon, _termcrit.epsilon, count, _Function, _termcrit.maxCount);
	simplex.row(0).copyTo(proxy_x); // TODO: a median point

	TRACE("%d iterations done, res=%f\n", count, res);

	if(x_mat.rows>1)
		Mat(x_mat.rows, 1, CV_64F, (double*)proxy_x.data).copyTo(x);
	return res;
}
DownhillSolverImpl::DownhillSolverImpl(){
	_Function=Ptr<Function>();
	_step=Mat_<double>();
}
Ptr<Solver::Function> DownhillSolverImpl::getFunction()const{
	return _Function;
}
void DownhillSolverImpl::setFunction(const Ptr<Function>& f){
	_Function=f;
}
TermCriteria DownhillSolverImpl::getTermCriteria()const{
	return _termcrit;
}
void DownhillSolverImpl::setTermCriteria(const TermCriteria& termcrit){
	CV_Assert(termcrit.type==(TermCriteria::MAX_ITER+TermCriteria::EPS) && termcrit.epsilon>0 && termcrit.maxCount>0);
	_termcrit=termcrit;
}
// both minRange & minError are specified by termcrit.epsilon; In addition, user may specify the number of iterations that the algorithm does.
Ptr<DownhillSolver> createDownhillSolver(const Ptr<Solver::Function>& f, InputArray initStep, TermCriteria termcrit)
{
	Ptr<DownhillSolver> DS=new DownhillSolverImpl();
	DS->setFunction(f);
	DS->setInitStep(initStep);
	DS->setTermCriteria(termcrit);
	return DS;
}
void DownhillSolverImpl::getInitStep(OutputArray step)const
{
	_step.copyTo(step);
}
void DownhillSolverImpl::setInitStep(InputArray step)
{
	//set dimensionality and make a deep copy of step
	Mat m=step.getMat();
	TRACE("m.cols=%d\nm.rows=%d\n", m.cols, m.rows);
	CV_Assert(MIN(m.cols,m.rows)==1 && m.type()==CV_64FC1);
	if(m.rows==1){
		m.copyTo(_step);
	}else{
		transpose(m,_step);
	}
}

} // namespace FaceMatch
