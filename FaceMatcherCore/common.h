/*
Informational Notice:

This software was developed under contract funded by the National Library of Medicine, which is part of the National Institutes of Health, an agency of the Department of Health and Human Services, United States Government.

The license of this software is an open-source BSD license.  It allows use in both commercial and non-commercial products.

The license does not supersede any applicable United States law.

The license does not indemnify you from any claims brought by third parties whose proprietary rights may be infringed by your usage of this software.

Government usage rights for this software are established by Federal law, which includes, but may not be limited to, Federal Acquisition Regulation (FAR) 48 C.F.R. Part52.227-14, Rights in Data—General.
The license for this software is intended to be expansive, rather than restrictive, in encouraging the use of this software in both commercial and non-commercial products.

LICENSE:

Government Usage Rights Notice:  The U.S. Government retains unlimited, royalty-free usage rights to this software, but not ownership, as provided by Federal law.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above Government Usage Rights Notice, this list of conditions and the following disclaimer.

•	Redistributions in binary form must reproduce the above Government Usage Rights Notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

•	The names,trademarks, and service marks of the National Library of Medicine, the National Cancer Institute, the National Institutes of Health, and the names of any of the software developers shall not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE U.S. GOVERNMENT AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITEDTO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE U.S. GOVERNMENT
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#define NEWLINE "\r\n"

//wrappers
#define RBS(x)	std::string(_bstr_t(x))

//exceptions
#define TRY			try
#define CATCH		catch(const exception & e)\
					{\
						/*return exception details*/\
						*ErrorString = _bstr_t(e.what()).Detach();\
						string message;\
						message += string("\r\nexception: exception caught");\
						message += string("\r\nfile: ") + __FILE__;\
						message += string("\r\nfunction: ") + __FUNCSIG__;\
						message += string("\r\ndetail: ") + e.what();\
						_AtlModule.Log(message);\
						hr = S_FALSE;\
					}\
					catch(...)\
					{\
						/*return generic exception status*/\
						*ErrorString = _bstr_t("unknown exception").Detach();\
						string message;\
						message += string("\r\nexception: unknown exception caught in ");\
						message += string("\r\nfile: ") + __FILE__;\
						message += string("\r\nfunction: ") + __FUNCSIG__;\
						message += string("\r\ndetail: not available");\
						_AtlModule.Log(message);\
						hr = S_FALSE;\
					}

#define CATCHDBG	catch(const exception & e)\
					{\
						/*return exception details*/\
						*ErrorString = _bstr_t(e.what()).Detach();\
						string message;\
						message += string("\r\nexception: exception caught");\
						message += string("\r\nfile: ") + __FILE__;\
						message += string("\r\nfunction: ") + __FUNCSIG__;\
						message += string("\r\ndetail: ") + e.what();\
						message += string("\r\nother info: ") + RBS(debugInfo);\
						_AtlModule.Log(message);\
						hr = S_FALSE;\
					}\
					catch(...)\
					{\
						/*return generic exception status*/\
						*ErrorString = _bstr_t("unknown exception").Detach();\
						string message;\
						message += string("\r\nexception: unknown exception caught in ");\
						message += string("\r\nfile: ") + __FILE__;\
						message += string("\r\nfunction: ") + __FUNCSIG__;\
						message += string("\r\ndetail: not available");\
						message += string("\r\nother info: ") + RBS(debugInfo);\
						_AtlModule.Log(message);\
						hr = S_FALSE;\
					}

class CMutexLock
{
public:
	CMutexLock(HANDLE mutex)
	{
		if(mutex != NULL)
		{
			m_mutex = mutex;
			WaitForSingleObject(m_mutex, INFINITE);
		}
	}
	
	virtual ~CMutexLock()
	{
		if(m_mutex != NULL)
		{
			ReleaseMutex(m_mutex);
			m_mutex = NULL;
		}
	}
private:
	HANDLE m_mutex;
};

class CQueryLock
{
public:
	CQueryLock(HANDLE hEvent) : m_event(NULL)
	{
		if( hEvent != NULL &&
			WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
		{
			m_event = hEvent;
			ResetEvent(m_event);
		}
	}
	
	virtual ~CQueryLock()
	{
		if(m_event != NULL)
		{
			SetEvent(m_event);
			m_event = NULL;
		}
	}
private:
	HANDLE m_event;
};

class CTimer
{
	typedef struct 
	{
		LARGE_INTEGER start;
		LARGE_INTEGER stop;
	}stopWatch;
private:
	stopWatch timer;
    LARGE_INTEGER frequency;

public:
	//void Start(void){
		//begin = clock();
	//}

	//LONG ElapsedMilliSeconds(void){
		//return (((double)clock() - (double)begin)/(double)CLOCKS_PER_SEC) * 1000/*MILLI_SECONDS*/;
	//}

	double LIToMilliSecs(LARGE_INTEGER & L){
		return (((double)L.QuadPart /(double)frequency.QuadPart)*1000);
	}
 
	CTimer()
	{
		timer.start.QuadPart=0;
		timer.stop.QuadPart=0; 
		QueryPerformanceFrequency(&frequency);
	}
 
	void Start(){
		QueryPerformanceCounter(&timer.start);
	}
 
	LONG ElapsedMilliSeconds() 
	{
		QueryPerformanceCounter(&timer.stop);

		LARGE_INTEGER time;
		time.QuadPart = timer.stop.QuadPart - timer.start.QuadPart;
		return (LONG)LIToMilliSecs(time);
	}

//private:
	//clock_t begin;
};

class CAutoLock
{
public:
	CAutoLock(CRITICAL_SECTION* cs){
		m_cs = *cs;
		EnterCriticalSection(&m_cs);
	}
	~CAutoLock(){
		LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION m_cs;
};

class CAutoHandle
{
public:
	HANDLE m_h;

	CAutoHandle(HANDLE h)
	{ 
		if(h == INVALID_HANDLE_VALUE || h == NULL)
			m_h = NULL;
		else
			m_h = h; 
	}
	~CAutoHandle(){ 
		reset(); 
	}
	void set(HANDLE h){ 
		reset(); 
		if(h == INVALID_HANDLE_VALUE || h == NULL)
			m_h = NULL;
		else
			m_h = h; 
	}
	HANDLE get(void){ 
		return m_h; 
	}
	void reset(void){ 
		if (m_h != NULL)
		{
			FlushFileBuffers(m_h);
			CloseHandle(m_h);
		}
		m_h = NULL;
	}
	void operator=(HANDLE h)
	{	
		reset();
		m_h = h;
	}
};

