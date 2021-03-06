
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

#include <climits>
#include <algorithm>
#include <cstdarg>

namespace FaceMatch
{
using std::vector;

#ifdef ALEX_DEBUG
	static void print_simplex_state(const Mat& c,const Mat& b,double v,const std::vector<int> N,const std::vector<int> B){
		printf("\tprint simplex state\n");

		printf("v=%g\n",v);

		printf("here c goes\n");
		dmp(c);

		printf("non-basic: ");
		print(Mat(N));
		printf("\n");

		printf("here b goes\n");
		dmp(b);
		printf("basic: ");

		print(Mat(B));
		printf("\n");
	}
#else
#define print_simplex_state(c,b,v,N,B)
#endif

/**Due to technical considerations, the format of input b and c is somewhat special:
*both b and c should be one column bigger than corresponding b and c of linear problem and the leftmost column will be used internally
by this procedure - it should not be cleaned before the call to procedure and may contain mess after
it also initializes N and B and does not make any assumptions about their init values
* @return SOLVELP_UNFEASIBLE if problem is unfeasible, 0 if feasible.
*/
static int initialize_simplex(Mat_<double>& c, Mat_<double>& b,double& v,vector<int>& N,vector<int>& B,vector<unsigned int>& indexToRow);
static inline void pivot(Mat_<double>& c,Mat_<double>& b,double& v,vector<int>& N,vector<int>& B,int leaving_index,
	int entering_index,vector<unsigned int>& indexToRow);
/**@return SOLVELP_UNBOUNDED means the problem is unbdd, SOLVELP_MULTI means multiple solutions, SOLVELP_SINGLE means one solution.
*/
static int inner_simplex(Mat_<double>& c, Mat_<double>& b,double& v,vector<int>& N,vector<int>& B,vector<unsigned int>& indexToRow);
static void swap_columns(Mat_<double>& A,int col1,int col2);

#define SWAP(type,a,b) {type tmp=(a);(a)=(b);(b)=tmp;}

//return codes:-2 (no_sol - unbdd),-1(no_sol - unfsbl), 0(single_sol), 1(multiple_sol=>least_l2_norm)
int solveLP(const Mat& Func, const Mat& Constr, Mat& z)
{
	TRACE("call to solveLP\n");

	//sanity check (size, type, no. of channels)
	CV_Assert(Func.type()==CV_64FC1 || Func.type()==CV_32FC1);
	CV_Assert(Constr.type()==CV_64FC1 || Constr.type()==CV_32FC1);
	CV_Assert((Func.rows==1 && (Constr.cols-Func.cols==1))||
		(Func.cols==1 && (Constr.cols-Func.rows==1)));

	//copy arguments for we will shall modify them
	Mat_<double> bigC=Mat_<double>(1,(Func.rows==1?Func.cols:Func.rows)+1),
		bigB=Mat_<double>(Constr.rows,Constr.cols+1);
	if(Func.rows==1){
		Func.convertTo(bigC.colRange(1,bigC.cols),CV_64FC1);
	}else{
		Mat FuncT=Func.t();
		FuncT.convertTo(bigC.colRange(1,bigC.cols),CV_64FC1);
	}
	Constr.convertTo(bigB.colRange(1,bigB.cols),CV_64FC1);
	double v=0;
	vector<int> N,B;
	vector<unsigned int> indexToRow;

	if(initialize_simplex(bigC,bigB,v,N,B,indexToRow)==SOLVELP_UNFEASIBLE){
		return SOLVELP_UNFEASIBLE;
	}
	Mat_<double> c=bigC.colRange(1,bigC.cols),
		b=bigB.colRange(1,bigB.cols);

	int res=0;
	if((res=inner_simplex(c,b,v,N,B,indexToRow))==SOLVELP_UNBOUNDED){
		return SOLVELP_UNBOUNDED;
	}

	//return the optimal solution
	z.create(c.cols,1,CV_64FC1);
	MatIterator_<double> it=z.begin<double>();
	unsigned int nsize = (unsigned int)N.size();
	for(int i=1;i<=c.cols;i++,it++){
		if(indexToRow[i]<nsize){
			*it=0;
		}else{
			*it=b.at<double>(indexToRow[i]-nsize,b.cols-1);
		}
	}

	return res;
}

static int initialize_simplex(Mat_<double>& c, Mat_<double>& b,double& v,vector<int>& N,vector<int>& B,vector<unsigned int>& indexToRow){
	N.resize(c.cols);
	N[0]=0;
	for (std::vector<int>::iterator it = N.begin()+1 ; it != N.end(); ++it){
		*it=it[-1]+1;
	}
	B.resize(b.rows);
	B[0]=(int)N.size();
	for (std::vector<int>::iterator it = B.begin()+1 ; it != B.end(); ++it){
		*it=it[-1]+1;
	}
	indexToRow.resize(c.cols+b.rows);
	indexToRow[0]=0;
	for (std::vector<unsigned int>::iterator it = indexToRow.begin()+1 ; it != indexToRow.end(); ++it){
		*it=it[-1]+1;
	}
	v=0;

	int k=0;
	{
		double min=DBL_MAX;
		for(int i=0;i<b.rows;i++){
			if(b(i,b.cols-1)<min){
				min=b(i,b.cols-1);
				k=i;
			}
		}
	}

	if(b(k,b.cols-1)>=0){
		N.erase(N.begin());
		for (std::vector<unsigned int>::iterator it = indexToRow.begin()+1 ; it != indexToRow.end(); ++it){
			--(*it);
		}
		return 0;
	}

	Mat_<double> old_c=c.clone();
	c=0;
	c(0,0)=-1;
	for(int i=0;i<b.rows;i++){
		b(i,0)=-1;
	}

	print_simplex_state(c,b,v,N,B);

	TRACE("\tWE MAKE PIVOT\n");
	pivot(c,b,v,N,B,k,0,indexToRow);

	print_simplex_state(c,b,v,N,B);

	inner_simplex(c,b,v,N,B,indexToRow);

	TRACE("\tAFTER INNER_SIMPLEX\n");
	print_simplex_state(c,b,v,N,B);

	unsigned int nsize = (unsigned int)N.size();
	if(indexToRow[0]>=nsize){
		int iterator_offset=indexToRow[0]-nsize;
		if(b(iterator_offset,b.cols-1)>0){
			return SOLVELP_UNFEASIBLE;
		}
		pivot(c,b,v,N,B,iterator_offset,0,indexToRow);
	}

	vector<int>::iterator iterator;
	{
		int iterator_offset=indexToRow[0];
		iterator=N.begin()+iterator_offset;
		std::iter_swap(iterator,N.begin());
		SWAP(int,indexToRow[*iterator],indexToRow[0]);
		swap_columns(c,iterator_offset,0);
		swap_columns(b,iterator_offset,0);
	}

	TRACE("after swaps\n");
	print_simplex_state(c,b,v,N,B);

	//start from 1, because we ignore x_0
	c=0;
	v=0;
	for(int I=1;I<old_c.cols;I++){
		if(indexToRow[I]<nsize){
			TRACE("I=%d from nonbasic\n",I);
			int iterator_offset=indexToRow[I];
			c(0,iterator_offset)+=old_c(0,I);
			dmp(c);
		}else{
			TRACE("I=%d from basic\n",I);
			int iterator_offset=indexToRow[I]-nsize;
			c-=old_c(0,I)*b.row(iterator_offset).colRange(0,b.cols-1);
			v+=old_c(0,I)*b(iterator_offset,b.cols-1);
			dmp(c);
		}
	}

	TRACE("after restore\n");
	print_simplex_state(c,b,v,N,B);

	N.erase(N.begin());
	for (std::vector<unsigned int>::iterator it = indexToRow.begin()+1 ; it != indexToRow.end(); ++it){
		--(*it);
	}
	return 0;
}

static int inner_simplex(Mat_<double>& c, Mat_<double>& b, double& v, vector<int>& N, vector<int>& B, vector<unsigned int>& indexToRow)
{
	int count=0;
	for(;;)
	{
		TRACE("iteration #%d\n",count);
		count++;

		static MatIterator_<double> pos_ptr;
		int e=-1,pos_ctr=0,min_var=INT_MAX;
		bool all_nonzero=true;
		for(pos_ptr=c.begin();pos_ptr!=c.end();pos_ptr++,pos_ctr++){
			if(*pos_ptr==0){
				all_nonzero=false;
			}
			if(*pos_ptr>0){
				if(N[pos_ctr]<min_var){
					e=pos_ctr;
					min_var=N[pos_ctr];
				}
			}
		}
		if(e==-1){
			TRACE("hello from e==-1\n");
			dmp(c);
			if(all_nonzero==true){
				return SOLVELP_SINGLE;
			}else{
				return SOLVELP_MULTI;
			}
		}

		int l=-1;
		min_var=INT_MAX;
		double min=DBL_MAX;
		int row_it=0;
		MatIterator_<double> min_row_ptr=b.begin();
		for(MatIterator_<double> it=b.begin();it!=b.end();it+=b.cols,row_it++){
			double myite=0;
			//check constraints, select the tightest one, reinforcing Bland's rule
			if((myite=it[e])>0){
				double val=it[b.cols-1]/myite;
				if(val<min || (val==min && B[row_it]<min_var)){
					min_var=B[row_it];
					min_row_ptr=it;
					min=val;
					l=row_it;
				}
			}
		}
		if(l==-1){
			return SOLVELP_UNBOUNDED;
		}
		TRACE("the tightest constraint is in row %d with %g\n",l,min);

		pivot(c,b,v,N,B,l,e,indexToRow);

		TRACE("objective, v=%g\n",v);
		dmp(c);
		TRACE("constraints\n");
		dmp(b);
		TRACE("non-basic: ");
		dmp(Mat(N));
		TRACE("basic: ");
		dmp(Mat(B));
	}
	return 0;
}

static inline void pivot(Mat_<double>& c,Mat_<double>& b,double& v,vector<int>& N,vector<int>& B,
	int leaving_index,int entering_index,vector<unsigned int>& indexToRow){
		double Coef=b(leaving_index,entering_index);
		for(int i=0;i<b.cols;i++){
			if(i==entering_index){
				b(leaving_index,i)=1/Coef;
			}else{
				b(leaving_index,i)/=Coef;
			}
		}

		for(int i=0;i<b.rows;i++){
			if(i!=leaving_index){
				double coef=b(i,entering_index);
				for(int j=0;j<b.cols;j++){
					if(j==entering_index){
						b(i,j)=-coef*b(leaving_index,j);
					}else{
						b(i,j)-=(coef*b(leaving_index,j));
					}
				}
			}
		}

		//objective function
		Coef=c(0,entering_index);
		for(int i=0;i<(b.cols-1);i++){
			if(i==entering_index){
				c(0,i)=-Coef*b(leaving_index,i);
			}else{
				c(0,i)-=Coef*b(leaving_index,i);
			}
		}
		TRACE("v was %g\n",v);
		v+=Coef*b(leaving_index,b.cols-1);

		SWAP(int,N[entering_index],B[leaving_index]);
		SWAP(int,indexToRow[N[entering_index]],indexToRow[B[leaving_index]]);
}

static inline void swap_columns(Mat_<double>& A,int col1,int col2){
	for(int i=0;i<A.rows;i++){
		double tmp=A(i,col1);
		A(i,col1)=A(i,col2);
		A(i,col2)=tmp;
	}
}
} // namespace FaceMatch
