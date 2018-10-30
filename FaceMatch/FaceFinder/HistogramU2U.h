#pragma once // 2012 (C) Eugene.Borovikov@NIH.gov

#include <map>
using namespace std;

/**
 * a simple histogram of unsigned values in unsigned bins
 */
class HistogramU2U
{
	typedef map<unsigned, unsigned> Hist;
	Hist mHist;
	unsigned mSum;
public:
	HistogramU2U(): mSum(0) {}
	/**
	 * Increment a histogram bin.
	 * @param bin	bin to be incremented
	 * @param val	increment delta
	 */
	unsigned inc(unsigned bin, unsigned val=1){ mSum+=val; return mHist[bin]+=val; }
	/**
	 * Output the histogram instance to a text output stream.
	 * @param s	output text stream
	 * @param h	histogram instance
	 * @return	output text stream
	 */
	friend ostream & operator<<(ostream & s, const HistogramU2U & h)
	{
		s<<"[sum]="<<h.mSum;
		if (!h.mSum) return s;
		s<<endl;
		unsigned BinSum=0, BinSum2=0, BinCnt=h.mHist.size();
		for (auto it=h.mHist.begin(); it!=h.mHist.end(); ++it)
		{
			unsigned b=it->first, c=it->second;
			s<<"["<<b<<"]="<<c<<"\t"<<REALNUM(c)/h.mSum<<endl;
			unsigned v=b*c;
			BinSum+=v;
			BinSum2+=b*v;
		}
		REALNUM m=REALNUM(BinSum)/h.mSum;
		s<<"[mean]="<<m<<endl
		<<"[std]="<<sqrt(REALNUM(BinSum2)/h.mSum-m*m);
		return s;
	}
};
