#pragma once // 2013-2016 (C) Eugene.Borovikov@NIH.gov

#include "dcl.h"
#include <string>
using namespace std;

#ifdef _WIN32
#include <direct.h>
inline int mkdir(const string & DN){ return mkdir(DN.c_str()); }
#else
#include <sys/stat.h>
inline int mkdir(const string & DN){ return mkdir(DN.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH); }
#endif

LIBDCL void makeDirectory(const string & DN);
