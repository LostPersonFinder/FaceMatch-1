

/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4152 )  /* function/data pointer conversion in expression */

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 475
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif /* __RPCPROXY_H_VERSION__ */


#include "FaceMatcherCore_i.h"

#define TYPE_FORMAT_STRING_SIZE   65                                
#define PROC_FORMAT_STRING_SIZE   847                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _FaceMatcherCore_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } FaceMatcherCore_MIDL_TYPE_FORMAT_STRING;

typedef struct _FaceMatcherCore_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } FaceMatcherCore_MIDL_PROC_FORMAT_STRING;

typedef struct _FaceMatcherCore_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } FaceMatcherCore_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const FaceMatcherCore_MIDL_TYPE_FORMAT_STRING FaceMatcherCore__MIDL_TypeFormatString;
extern const FaceMatcherCore_MIDL_PROC_FORMAT_STRING FaceMatcherCore__MIDL_ProcFormatString;
extern const FaceMatcherCore_MIDL_EXPR_FORMAT_STRING FaceMatcherCore__MIDL_ExprFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICoreMatcherFaceRegionsMany_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICoreMatcherFaceRegionsMany_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICoreImageMatcherWhole_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICoreImageMatcherWhole_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IFaceFinder_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IFaceFinder_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

static const FaceMatcherCore_MIDL_PROC_FORMAT_STRING FaceMatcherCore__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure initialize */


	/* Procedure initialize */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x7 ),	/* 7 */
/*  8 */	NdrFcShort( 0x50 ),	/* X64 Stack size/offset = 80 */
/* 10 */	NdrFcShort( 0x3c ),	/* 60 */
/* 12 */	NdrFcShort( 0x24 ),	/* 36 */
/* 14 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x9,		/* 9 */
/* 16 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 18 */	NdrFcShort( 0x1 ),	/* 1 */
/* 20 */	NdrFcShort( 0x1 ),	/* 1 */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter regKey */


	/* Parameter regKey */

/* 26 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 28 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 30 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */


	/* Parameter eventID */

/* 32 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 34 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 36 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter overflowListing */


	/* Parameter overflowListing */

/* 38 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 40 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 42 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter BUCKET_SIZE */


	/* Parameter BUCKET_SIZE */

/* 44 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 46 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 48 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errorString */


	/* Parameter errorString */

/* 50 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 52 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 54 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */


	/* Parameter time */

/* 56 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 58 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 60 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter useGPU */


	/* Parameter useGPU */

/* 62 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 64 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 66 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter performanceValue */


	/* Parameter performanceValue */

/* 68 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 70 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 72 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */


	/* Return value */

/* 74 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 76 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 78 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ingest */

/* 80 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 82 */	NdrFcLong( 0x0 ),	/* 0 */
/* 86 */	NdrFcShort( 0x8 ),	/* 8 */
/* 88 */	NdrFcShort( 0x78 ),	/* X64 Stack size/offset = 120 */
/* 90 */	NdrFcShort( 0x44 ),	/* 68 */
/* 92 */	NdrFcShort( 0x24 ),	/* 36 */
/* 94 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0xe,		/* 14 */
/* 96 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 98 */	NdrFcShort( 0x1 ),	/* 1 */
/* 100 */	NdrFcShort( 0x1 ),	/* 1 */
/* 102 */	NdrFcShort( 0x0 ),	/* 0 */
/* 104 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter regKey */

/* 106 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 108 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 110 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */

/* 112 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 114 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 116 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter Img */

/* 118 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 120 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 122 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter IDRegs */

/* 124 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 126 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 128 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter bucket */

/* 130 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 132 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 134 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter currentOverflowLevel */

/* 136 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 138 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 140 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter newOverflowValue */

/* 142 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 144 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 146 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter BUCKET_SIZE */

/* 148 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 150 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 152 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errorString */

/* 154 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 156 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 158 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 160 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 162 */	NdrFcShort( 0x50 ),	/* X64 Stack size/offset = 80 */
/* 164 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter debugInfo */

/* 166 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 168 */	NdrFcShort( 0x58 ),	/* X64 Stack size/offset = 88 */
/* 170 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter useGPU */

/* 172 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 174 */	NdrFcShort( 0x60 ),	/* X64 Stack size/offset = 96 */
/* 176 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter performanceValue */

/* 178 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 180 */	NdrFcShort( 0x68 ),	/* X64 Stack size/offset = 104 */
/* 182 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 184 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 186 */	NdrFcShort( 0x70 ),	/* X64 Stack size/offset = 112 */
/* 188 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure query */

/* 190 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 192 */	NdrFcLong( 0x0 ),	/* 0 */
/* 196 */	NdrFcShort( 0x9 ),	/* 9 */
/* 198 */	NdrFcShort( 0x68 ),	/* X64 Stack size/offset = 104 */
/* 200 */	NdrFcShort( 0x34 ),	/* 52 */
/* 202 */	NdrFcShort( 0x40 ),	/* 64 */
/* 204 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0xc,		/* 12 */
/* 206 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 208 */	NdrFcShort( 0x1 ),	/* 1 */
/* 210 */	NdrFcShort( 0x1 ),	/* 1 */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	NdrFcShort( 0x4000 ),	/* 16384 */

	/* Parameter regKey */

/* 216 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 218 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 220 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */

/* 222 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 224 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 226 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter result */

/* 228 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 230 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 232 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter imgFNRegs */

/* 234 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 236 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 238 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter bucket */

/* 240 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 242 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 244 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter overflowBucket */

/* 246 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 248 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 250 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter tolerance */

/* 252 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 254 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 256 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter errorString */

/* 258 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 260 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 262 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 264 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 266 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 268 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter debugInfo */

/* 270 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 272 */	NdrFcShort( 0x50 ),	/* X64 Stack size/offset = 80 */
/* 274 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter matches */

/* 276 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 278 */	NdrFcShort( 0x58 ),	/* X64 Stack size/offset = 88 */
/* 280 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 282 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 284 */	NdrFcShort( 0x60 ),	/* X64 Stack size/offset = 96 */
/* 286 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure save */


	/* Procedure save */

/* 288 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 290 */	NdrFcLong( 0x0 ),	/* 0 */
/* 294 */	NdrFcShort( 0xa ),	/* 10 */
/* 296 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 298 */	NdrFcShort( 0x24 ),	/* 36 */
/* 300 */	NdrFcShort( 0x24 ),	/* 36 */
/* 302 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x5,		/* 5 */
/* 304 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 306 */	NdrFcShort( 0x1 ),	/* 1 */
/* 308 */	NdrFcShort( 0x1 ),	/* 1 */
/* 310 */	NdrFcShort( 0x0 ),	/* 0 */
/* 312 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter regKey */


	/* Parameter regKey */

/* 314 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 316 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 318 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */


	/* Parameter eventID */

/* 320 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 322 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 324 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errorString */


	/* Parameter errorString */

/* 326 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 328 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 330 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */


	/* Parameter time */

/* 332 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 334 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 336 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */


	/* Return value */

/* 338 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 340 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 342 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure remove */

/* 344 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 346 */	NdrFcLong( 0x0 ),	/* 0 */
/* 350 */	NdrFcShort( 0xb ),	/* 11 */
/* 352 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 354 */	NdrFcShort( 0x40 ),	/* 64 */
/* 356 */	NdrFcShort( 0x40 ),	/* 64 */
/* 358 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x7,		/* 7 */
/* 360 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 362 */	NdrFcShort( 0x1 ),	/* 1 */
/* 364 */	NdrFcShort( 0x1 ),	/* 1 */
/* 366 */	NdrFcShort( 0x0 ),	/* 0 */
/* 368 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter regKey */

/* 370 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 372 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 374 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */

/* 376 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 378 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 380 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter ID */

/* 382 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 384 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 386 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter removed */

/* 388 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 390 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 392 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errorString */

/* 394 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 396 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 398 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 400 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 402 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 404 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 406 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 408 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 410 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure releaseEventDatabase */

/* 412 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 414 */	NdrFcLong( 0x0 ),	/* 0 */
/* 418 */	NdrFcShort( 0xc ),	/* 12 */
/* 420 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 422 */	NdrFcShort( 0x24 ),	/* 36 */
/* 424 */	NdrFcShort( 0x24 ),	/* 36 */
/* 426 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x5,		/* 5 */
/* 428 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 430 */	NdrFcShort( 0x1 ),	/* 1 */
/* 432 */	NdrFcShort( 0x1 ),	/* 1 */
/* 434 */	NdrFcShort( 0x0 ),	/* 0 */
/* 436 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter regKey */

/* 438 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 440 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 442 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */

/* 444 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 446 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 448 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errorString */

/* 450 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 452 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 454 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 456 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 458 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 460 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 462 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 464 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 466 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure flushLog */

/* 468 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 470 */	NdrFcLong( 0x0 ),	/* 0 */
/* 474 */	NdrFcShort( 0xd ),	/* 13 */
/* 476 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 478 */	NdrFcShort( 0x0 ),	/* 0 */
/* 480 */	NdrFcShort( 0x8 ),	/* 8 */
/* 482 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x1,		/* 1 */
/* 484 */	0xa,		/* 10 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 486 */	NdrFcShort( 0x0 ),	/* 0 */
/* 488 */	NdrFcShort( 0x0 ),	/* 0 */
/* 490 */	NdrFcShort( 0x0 ),	/* 0 */
/* 492 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Return value */

/* 494 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 496 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 498 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ingest */

/* 500 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 502 */	NdrFcLong( 0x0 ),	/* 0 */
/* 506 */	NdrFcShort( 0x8 ),	/* 8 */
/* 508 */	NdrFcShort( 0x58 ),	/* X64 Stack size/offset = 88 */
/* 510 */	NdrFcShort( 0x34 ),	/* 52 */
/* 512 */	NdrFcShort( 0x24 ),	/* 36 */
/* 514 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0xa,		/* 10 */
/* 516 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 518 */	NdrFcShort( 0x1 ),	/* 1 */
/* 520 */	NdrFcShort( 0x1 ),	/* 1 */
/* 522 */	NdrFcShort( 0x0 ),	/* 0 */
/* 524 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter regKey */

/* 526 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 528 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 530 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */

/* 532 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 534 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 536 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter Img */

/* 538 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 540 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 542 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter IDRegs */

/* 544 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 546 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 548 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter errorString */

/* 550 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 552 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 554 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 556 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 558 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 560 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter debugInfo */

/* 562 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 564 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 566 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter useGPU */

/* 568 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 570 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 572 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter performanceValue */

/* 574 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 576 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 578 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 580 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 582 */	NdrFcShort( 0x50 ),	/* X64 Stack size/offset = 80 */
/* 584 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure query */

/* 586 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 588 */	NdrFcLong( 0x0 ),	/* 0 */
/* 592 */	NdrFcShort( 0x9 ),	/* 9 */
/* 594 */	NdrFcShort( 0x58 ),	/* X64 Stack size/offset = 88 */
/* 596 */	NdrFcShort( 0x2c ),	/* 44 */
/* 598 */	NdrFcShort( 0x40 ),	/* 64 */
/* 600 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0xa,		/* 10 */
/* 602 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 604 */	NdrFcShort( 0x1 ),	/* 1 */
/* 606 */	NdrFcShort( 0x1 ),	/* 1 */
/* 608 */	NdrFcShort( 0x0 ),	/* 0 */
/* 610 */	NdrFcShort( 0x400 ),	/* 1024 */

	/* Parameter regKey */

/* 612 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 614 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 616 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */

/* 618 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 620 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 622 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter result */

/* 624 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 626 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 628 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter imgFNRegs */

/* 630 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 632 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 634 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter tolerance */

/* 636 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 638 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 640 */	0xa,		/* FC_FLOAT */
			0x0,		/* 0 */

	/* Parameter errorString */

/* 642 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 644 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 646 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 648 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 650 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 652 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter debugInfo */

/* 654 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 656 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 658 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter matches */

/* 660 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 662 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 664 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 666 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 668 */	NdrFcShort( 0x50 ),	/* X64 Stack size/offset = 80 */
/* 670 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure remove */

/* 672 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 674 */	NdrFcLong( 0x0 ),	/* 0 */
/* 678 */	NdrFcShort( 0xb ),	/* 11 */
/* 680 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 682 */	NdrFcShort( 0x24 ),	/* 36 */
/* 684 */	NdrFcShort( 0x24 ),	/* 36 */
/* 686 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x6,		/* 6 */
/* 688 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 690 */	NdrFcShort( 0x1 ),	/* 1 */
/* 692 */	NdrFcShort( 0x1 ),	/* 1 */
/* 694 */	NdrFcShort( 0x0 ),	/* 0 */
/* 696 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter regKey */

/* 698 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 700 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 702 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter eventID */

/* 704 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 706 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 708 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter ID */

/* 710 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 712 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 714 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter errorString */

/* 716 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 718 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 720 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 722 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 724 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 726 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 728 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 730 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 732 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetFaces */

/* 734 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 736 */	NdrFcLong( 0x0 ),	/* 0 */
/* 740 */	NdrFcShort( 0x7 ),	/* 7 */
/* 742 */	NdrFcShort( 0x48 ),	/* X64 Stack size/offset = 72 */
/* 744 */	NdrFcShort( 0x34 ),	/* 52 */
/* 746 */	NdrFcShort( 0x24 ),	/* 36 */
/* 748 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x8,		/* 8 */
/* 750 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 752 */	NdrFcShort( 0x1 ),	/* 1 */
/* 754 */	NdrFcShort( 0x1 ),	/* 1 */
/* 756 */	NdrFcShort( 0x0 ),	/* 0 */
/* 758 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter path */

/* 760 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 762 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 764 */	NdrFcShort( 0x1c ),	/* Type Offset=28 */

	/* Parameter options */

/* 766 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 768 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 770 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter faces */

/* 772 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 774 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 776 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter errorString */

/* 778 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 780 */	NdrFcShort( 0x20 ),	/* X64 Stack size/offset = 32 */
/* 782 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Parameter time */

/* 784 */	NdrFcShort( 0x158 ),	/* Flags:  in, out, base type, simple ref, */
/* 786 */	NdrFcShort( 0x28 ),	/* X64 Stack size/offset = 40 */
/* 788 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter useGPU */

/* 790 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 792 */	NdrFcShort( 0x30 ),	/* X64 Stack size/offset = 48 */
/* 794 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter performanceOption */

/* 796 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 798 */	NdrFcShort( 0x38 ),	/* X64 Stack size/offset = 56 */
/* 800 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 802 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 804 */	NdrFcShort( 0x40 ),	/* X64 Stack size/offset = 64 */
/* 806 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure usingGPU */

/* 808 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 810 */	NdrFcLong( 0x0 ),	/* 0 */
/* 814 */	NdrFcShort( 0x8 ),	/* 8 */
/* 816 */	NdrFcShort( 0x18 ),	/* X64 Stack size/offset = 24 */
/* 818 */	NdrFcShort( 0x0 ),	/* 0 */
/* 820 */	NdrFcShort( 0x8 ),	/* 8 */
/* 822 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 824 */	0xa,		/* 10 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 826 */	NdrFcShort( 0x1 ),	/* 1 */
/* 828 */	NdrFcShort( 0x1 ),	/* 1 */
/* 830 */	NdrFcShort( 0x0 ),	/* 0 */
/* 832 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter gpuStatus */

/* 834 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 836 */	NdrFcShort( 0x8 ),	/* X64 Stack size/offset = 8 */
/* 838 */	NdrFcShort( 0x2e ),	/* Type Offset=46 */

	/* Return value */

/* 840 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 842 */	NdrFcShort( 0x10 ),	/* X64 Stack size/offset = 16 */
/* 844 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const FaceMatcherCore_MIDL_TYPE_FORMAT_STRING FaceMatcherCore__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x12, 0x0,	/* FC_UP */
/*  4 */	NdrFcShort( 0xe ),	/* Offset= 14 (18) */
/*  6 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/*  8 */	NdrFcShort( 0x2 ),	/* 2 */
/* 10 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 12 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 14 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 16 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 18 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 20 */	NdrFcShort( 0x8 ),	/* 8 */
/* 22 */	NdrFcShort( 0xfff0 ),	/* Offset= -16 (6) */
/* 24 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 26 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 28 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 30 */	NdrFcShort( 0x0 ),	/* 0 */
/* 32 */	NdrFcShort( 0x8 ),	/* 8 */
/* 34 */	NdrFcShort( 0x0 ),	/* 0 */
/* 36 */	NdrFcShort( 0xffde ),	/* Offset= -34 (2) */
/* 38 */	
			0x11, 0x0,	/* FC_RP */
/* 40 */	NdrFcShort( 0x6 ),	/* Offset= 6 (46) */
/* 42 */	
			0x13, 0x0,	/* FC_OP */
/* 44 */	NdrFcShort( 0xffe6 ),	/* Offset= -26 (18) */
/* 46 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 48 */	NdrFcShort( 0x0 ),	/* 0 */
/* 50 */	NdrFcShort( 0x8 ),	/* 8 */
/* 52 */	NdrFcShort( 0x0 ),	/* 0 */
/* 54 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (42) */
/* 56 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 58 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 60 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 62 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: ICoreMatcherFaceRegionsMany, ver. 0.0,
   GUID={0xDDDB75E1,0xC09B,0x47B3,{0xB1,0x4F,0x98,0xD6,0xE3,0x42,0xB2,0x18}} */

#pragma code_seg(".orpc")
static const unsigned short ICoreMatcherFaceRegionsMany_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    80,
    190,
    288,
    344,
    412,
    468
    };

static const MIDL_STUBLESS_PROXY_INFO ICoreMatcherFaceRegionsMany_ProxyInfo =
    {
    &Object_StubDesc,
    FaceMatcherCore__MIDL_ProcFormatString.Format,
    &ICoreMatcherFaceRegionsMany_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICoreMatcherFaceRegionsMany_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    FaceMatcherCore__MIDL_ProcFormatString.Format,
    &ICoreMatcherFaceRegionsMany_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(14) _ICoreMatcherFaceRegionsManyProxyVtbl = 
{
    &ICoreMatcherFaceRegionsMany_ProxyInfo,
    &IID_ICoreMatcherFaceRegionsMany,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* ICoreMatcherFaceRegionsMany::initialize */ ,
    (void *) (INT_PTR) -1 /* ICoreMatcherFaceRegionsMany::ingest */ ,
    (void *) (INT_PTR) -1 /* ICoreMatcherFaceRegionsMany::query */ ,
    (void *) (INT_PTR) -1 /* ICoreMatcherFaceRegionsMany::save */ ,
    (void *) (INT_PTR) -1 /* ICoreMatcherFaceRegionsMany::remove */ ,
    (void *) (INT_PTR) -1 /* ICoreMatcherFaceRegionsMany::releaseEventDatabase */ ,
    (void *) (INT_PTR) -1 /* ICoreMatcherFaceRegionsMany::flushLog */
};


static const PRPC_STUB_FUNCTION ICoreMatcherFaceRegionsMany_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ICoreMatcherFaceRegionsManyStubVtbl =
{
    &IID_ICoreMatcherFaceRegionsMany,
    &ICoreMatcherFaceRegionsMany_ServerInfo,
    14,
    &ICoreMatcherFaceRegionsMany_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: ICoreImageMatcherWhole, ver. 0.0,
   GUID={0x49318CB1,0x9DA9,0x43FA,{0xB4,0xAC,0x0C,0x09,0xBF,0xB3,0x89,0xF8}} */

#pragma code_seg(".orpc")
static const unsigned short ICoreImageMatcherWhole_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    500,
    586,
    288,
    672
    };

static const MIDL_STUBLESS_PROXY_INFO ICoreImageMatcherWhole_ProxyInfo =
    {
    &Object_StubDesc,
    FaceMatcherCore__MIDL_ProcFormatString.Format,
    &ICoreImageMatcherWhole_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICoreImageMatcherWhole_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    FaceMatcherCore__MIDL_ProcFormatString.Format,
    &ICoreImageMatcherWhole_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(12) _ICoreImageMatcherWholeProxyVtbl = 
{
    &ICoreImageMatcherWhole_ProxyInfo,
    &IID_ICoreImageMatcherWhole,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* ICoreImageMatcherWhole::initialize */ ,
    (void *) (INT_PTR) -1 /* ICoreImageMatcherWhole::ingest */ ,
    (void *) (INT_PTR) -1 /* ICoreImageMatcherWhole::query */ ,
    (void *) (INT_PTR) -1 /* ICoreImageMatcherWhole::save */ ,
    (void *) (INT_PTR) -1 /* ICoreImageMatcherWhole::remove */
};


static const PRPC_STUB_FUNCTION ICoreImageMatcherWhole_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _ICoreImageMatcherWholeStubVtbl =
{
    &IID_ICoreImageMatcherWhole,
    &ICoreImageMatcherWhole_ServerInfo,
    12,
    &ICoreImageMatcherWhole_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IFaceFinder, ver. 0.0,
   GUID={0xB77687D2,0x8150,0x4613,{0x82,0x1A,0x0A,0x9E,0xE2,0x9B,0x4D,0x3C}} */

#pragma code_seg(".orpc")
static const unsigned short IFaceFinder_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    734,
    808
    };

static const MIDL_STUBLESS_PROXY_INFO IFaceFinder_ProxyInfo =
    {
    &Object_StubDesc,
    FaceMatcherCore__MIDL_ProcFormatString.Format,
    &IFaceFinder_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IFaceFinder_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    FaceMatcherCore__MIDL_ProcFormatString.Format,
    &IFaceFinder_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(9) _IFaceFinderProxyVtbl = 
{
    &IFaceFinder_ProxyInfo,
    &IID_IFaceFinder,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* IDispatch::GetTypeInfoCount */ ,
    0 /* IDispatch::GetTypeInfo */ ,
    0 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IFaceFinder::GetFaces */ ,
    (void *) (INT_PTR) -1 /* IFaceFinder::usingGPU */
};


static const PRPC_STUB_FUNCTION IFaceFinder_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IFaceFinderStubVtbl =
{
    &IID_IFaceFinder,
    &IFaceFinder_ServerInfo,
    9,
    &IFaceFinder_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    FaceMatcherCore__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x800025b, /* MIDL Version 8.0.603 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

const CInterfaceProxyVtbl * const _FaceMatcherCore_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICoreImageMatcherWholeProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IFaceFinderProxyVtbl,
    ( CInterfaceProxyVtbl *) &_ICoreMatcherFaceRegionsManyProxyVtbl,
    0
};

const CInterfaceStubVtbl * const _FaceMatcherCore_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICoreImageMatcherWholeStubVtbl,
    ( CInterfaceStubVtbl *) &_IFaceFinderStubVtbl,
    ( CInterfaceStubVtbl *) &_ICoreMatcherFaceRegionsManyStubVtbl,
    0
};

PCInterfaceName const _FaceMatcherCore_InterfaceNamesList[] = 
{
    "ICoreImageMatcherWhole",
    "IFaceFinder",
    "ICoreMatcherFaceRegionsMany",
    0
};

const IID *  const _FaceMatcherCore_BaseIIDList[] = 
{
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    0
};


#define _FaceMatcherCore_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _FaceMatcherCore, pIID, n)

int __stdcall _FaceMatcherCore_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _FaceMatcherCore, 3, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _FaceMatcherCore, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _FaceMatcherCore, 3, *pIndex )
    
}

const ExtendedProxyFileInfo FaceMatcherCore_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _FaceMatcherCore_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _FaceMatcherCore_StubVtblList,
    (const PCInterfaceName * ) & _FaceMatcherCore_InterfaceNamesList,
    (const IID ** ) & _FaceMatcherCore_BaseIIDList,
    & _FaceMatcherCore_IID_Lookup, 
    3,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* defined(_M_AMD64)*/

