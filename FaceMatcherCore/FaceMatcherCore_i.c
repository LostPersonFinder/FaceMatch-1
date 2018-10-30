

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Tue Sep 05 11:21:04 2017
 */
/* Compiler settings for FaceMatcherCore.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_ICoreMatcherFaceRegionsMany,0xDDDB75E1,0xC09B,0x47B3,0xB1,0x4F,0x98,0xD6,0xE3,0x42,0xB2,0x18);


MIDL_DEFINE_GUID(IID, IID_ICoreImageMatcherWhole,0x49318CB1,0x9DA9,0x43FA,0xB4,0xAC,0x0C,0x09,0xBF,0xB3,0x89,0xF8);


MIDL_DEFINE_GUID(IID, IID_IFaceFinder,0xB77687D2,0x8150,0x4613,0x82,0x1A,0x0A,0x9E,0xE2,0x9B,0x4D,0x3C);


MIDL_DEFINE_GUID(IID, LIBID_FaceMatcherCoreLib,0x8D053CE6,0xE679,0x44EF,0xA6,0xF2,0xB0,0x5C,0x6D,0xB4,0xBB,0x87);


MIDL_DEFINE_GUID(CLSID, CLSID_CoreMatcherFaceRegionsMany,0x7E06FF04,0x0A05,0x4F39,0x98,0x4C,0x96,0x58,0x6E,0xAA,0x66,0x0B);


MIDL_DEFINE_GUID(CLSID, CLSID_CoreImageMatcherWhole,0x5306C26A,0xEB56,0x4935,0xAF,0x97,0x97,0xF0,0xEF,0xE7,0x3B,0xC5);


MIDL_DEFINE_GUID(CLSID, CLSID_FaceFinder,0xA68516F9,0x3A80,0x4D0A,0x9D,0xE6,0x4E,0x7D,0x3C,0x70,0xFE,0x53);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



