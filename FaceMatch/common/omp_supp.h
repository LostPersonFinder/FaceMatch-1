
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

#pragma once // 2011-2016 (C) 

#include <omp.h>
#include <thread>
#include <stdexcept>
#include <atomic>
#include <mutex>

using namespace std;

/// OMP style locking interface
struct OMPLock
{
	virtual ~OMPLock(){}
	/// @return	result of the test if the lock has been set, but do not block the thread
	virtual int test()=0;
	/// Set the lock.
	virtual void set()=0;
	/// Release the lock.
	virtual void unset()=0;
	/// @return is lock valid?
	virtual bool valid()const=0;
};

/// OMP style lock template with critical operations for safer static locks
template<typename TLock>
class OMPLockCritical : public OMPLock
{
	TLock mLock;
	void (*mDone)(TLock * pLock);
	void (*mSet)(TLock * pLock);
	void (*mUnSet)(TLock * pLock);
	int (*mTest)(TLock * pLock);
	atomic<bool> mValid;
public:
	/// Instantiate.
	OMPLockCritical(void (*init)(TLock * pLock), ///< function pointer to initialization routine
		void done(TLock * pLock), ///< function pointer to clean-up routine
		void set(TLock * pLock), ///< function pointer to lock set routine
		void unset(TLock * pLock),  ///< function pointer to lock unset routine
		int test(TLock * pLock)  ///< function pointer to lock test routine
	) : mDone(done), mSet(set), mUnSet(unset), mTest(test), mValid(false)
	{
		if (!init) throw logic_error("NULL init");
#pragma omp critical (OMPLockCriticalInit)
		if (!mValid)
		{
			init(&mLock);
			mValid = true;
		}
	}
	virtual void set()override
	{
		if (!mSet) throw logic_error("NULL set");
		if (mValid) mSet(&mLock);
		else throw logic_error("unable to set invalid lock");
	}
	virtual void unset()override
	{
		if (!mUnSet) throw logic_error("NULL unset");
		if (mValid) mUnSet(&mLock);
		else throw logic_error("unable to unset invalid lock");
	}
	virtual int test()override
	{
		if (!mTest) throw logic_error("NULL test");
		int res = 0;
		if (mValid) res = mTest(&mLock);
		else throw logic_error("unable to test invalid lock");
		return res;
	}
	virtual ~OMPLockCritical()
	{
#pragma omp critical (OMPLockCriticalDestroy)
		if (mValid)
		{
			mValid = false;
			if (mDone) mDone(&mLock);
		}
	}
	virtual bool valid()const override { return mValid;	}
};

/// OMP simple lock encapsulation
struct OMPSimpleLock : public OMPLockCritical<omp_lock_t>
{
	OMPSimpleLock() : OMPLockCritical<omp_lock_t>(omp_init_lock, omp_destroy_lock,
		omp_set_lock, omp_unset_lock, omp_test_lock)
	{ }
};

/// OMP nested lock encapsulation
struct OMPNestedLock : public OMPLockCritical<omp_nest_lock_t>
{
	OMPNestedLock() : OMPLockCritical<omp_nest_lock_t>(omp_init_nest_lock, omp_destroy_nest_lock,
		omp_set_nest_lock, omp_unset_nest_lock, omp_test_nest_lock)
	{ }
};

/// OMP style locker
class OMPLocker
{
protected:
	/// OMP style lock reference
	OMPLock & mLock;
	bool mNeed2Clear; ///< need to clear the lock on destruction?
	/// validate the lock in specified number of tries
	void validate(int tries = 1024 /**< number of tries to yield before failing */)const
	{
		while (!mLock.valid() && --tries) this_thread::yield(); // ::sleep_for(chrono::microseconds(4))
		if (!mLock.valid()) throw logic_error("unable to validate lock");
	}
public:
	/**
	 * Instantiate: set the lock.
	 * @param aLock reference to a lock
	 * @param block do we need to block?
	 */
	OMPLocker(OMPLock & aLock, bool block=true): mLock(aLock), mNeed2Clear(block)
	{
		validate();
		if (block) mLock.set();
		else mNeed2Clear=mLock.test();
	}
	/// unset and destroy
	virtual ~OMPLocker()
	{
		validate();
		if (mNeed2Clear) mLock.unset();
	}
};

#include <cv.h>
using namespace cv; // for Ptr, as STL's unique_ptr appears to produce a crash on destruction of a static instance

/// thread safe static construction for singletons
#define OMPStaticLock(TLock) static Ptr<TLock> pLock(nullptr); static once_flag OF; call_once(OF, [&]{ pLock=new TLock(); }); OMPLocker lkr(*pLock);
#define OMPAutoLockSimple OMPStaticLock(OMPSimpleLock)
#define OMPAutoLockNested OMPStaticLock(OMPNestedLock)
#define StaticLkdCtor OMPAutoLockSimple static
