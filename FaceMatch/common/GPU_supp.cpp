
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
#include "GPU_supp.h"
#include <thread>

using namespace std;
using namespace gpu;

atomic<bool> sPreferGPU(false);
bool preferGPU(){ return sPreferGPU; }
bool preferGPU(bool val) { return sPreferGPU = val; }

int getGPUCount(bool rethrow)
{
	int res=0;
	try{ res=getCudaEnabledDeviceCount(); }
	catch(const exception & e){ res=0; if (rethrow) throw e; }
	return res;
}
void printGPUInfo() { for (int c=0, CUDACnt=getGPUCount(); c<CUDACnt; ++c) printShortCudaDeviceInfo(c); }
int setGPU(int ID)
{
	setDevice(ID);
	return ID;
}
/// GPU lock array
class GPUs
{
	typedef OMPNestedLock TLock;
	typedef Ptr<TLock> PLock;
	vector<PLock> mGPUs;
	unsigned mNextID;
public:
	/// Instantiate.
	GPUs(): mNextID(0)
	{
		int cnt=getGPUCount();
		if (cnt<0 && getVerbLevel()) clog<<"getCudaEnabledDeviceCount="<<cnt<<endl;
		if (cnt<=0)	return;
		mGPUs.resize(cnt);
		for (unsigned ID=0, size=mGPUs.size(); ID<size; ++ID) mGPUs[ID]=new TLock();
	}
	/// @return locked GPU ID
	unsigned lock(unsigned ID /**< GPU ID to lock*/)
	{
		CHECK(ID>=0 && ID<getCount(), format("provided GPU ID=%d falls outside of the required interval 0<=ID<%d ", ID, getCount()));
		mGPUs[ID]->set();
		return setGPU(ID);
	}
	/// Unlock a specific GPU by ID.
	void unlock(unsigned ID /**< GPU ID to unlock*/)
	{
		if (ID<0 || ID>=getCount()) return;
		mGPUs[ID]->unset();
	}
	/// @return locked ID
	unsigned lockNext()
	{
		CHECK(getCount(), "no GPUs found");
		for (;;)
			for (unsigned ID = 0, size = getCount(); ID<size; ++ID)
				if (mGPUs[ID]->test())
					return setGPU(ID);
				else
					this_thread::yield();
		return 0;
	}
	/// @return GPU count
	unsigned getCount()const{return mGPUs.size();}
};
/// @return static reference to GPU the lock array
static GPUs & getGPUs()
{
	 StaticLkdCtor GPUs sGPUs;
	 return sGPUs;
}
GPULocker::GPULocker(): mID(getGPUs().lockNext()){}
GPULocker::GPULocker(unsigned ID): mID(ID) { getGPUs().lock(mID); }
GPULocker::~GPULocker(){ getGPUs().unlock(mID); }
