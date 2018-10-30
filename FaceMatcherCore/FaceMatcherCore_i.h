

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __FaceMatcherCore_i_h__
#define __FaceMatcherCore_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ICoreMatcherFaceRegionsMany_FWD_DEFINED__
#define __ICoreMatcherFaceRegionsMany_FWD_DEFINED__
typedef interface ICoreMatcherFaceRegionsMany ICoreMatcherFaceRegionsMany;

#endif 	/* __ICoreMatcherFaceRegionsMany_FWD_DEFINED__ */


#ifndef __ICoreImageMatcherWhole_FWD_DEFINED__
#define __ICoreImageMatcherWhole_FWD_DEFINED__
typedef interface ICoreImageMatcherWhole ICoreImageMatcherWhole;

#endif 	/* __ICoreImageMatcherWhole_FWD_DEFINED__ */


#ifndef __IFaceFinder_FWD_DEFINED__
#define __IFaceFinder_FWD_DEFINED__
typedef interface IFaceFinder IFaceFinder;

#endif 	/* __IFaceFinder_FWD_DEFINED__ */


#ifndef __CoreMatcherFaceRegionsMany_FWD_DEFINED__
#define __CoreMatcherFaceRegionsMany_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoreMatcherFaceRegionsMany CoreMatcherFaceRegionsMany;
#else
typedef struct CoreMatcherFaceRegionsMany CoreMatcherFaceRegionsMany;
#endif /* __cplusplus */

#endif 	/* __CoreMatcherFaceRegionsMany_FWD_DEFINED__ */


#ifndef __CoreImageMatcherWhole_FWD_DEFINED__
#define __CoreImageMatcherWhole_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoreImageMatcherWhole CoreImageMatcherWhole;
#else
typedef struct CoreImageMatcherWhole CoreImageMatcherWhole;
#endif /* __cplusplus */

#endif 	/* __CoreImageMatcherWhole_FWD_DEFINED__ */


#ifndef __FaceFinder_FWD_DEFINED__
#define __FaceFinder_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaceFinder FaceFinder;
#else
typedef struct FaceFinder FaceFinder;
#endif /* __cplusplus */

#endif 	/* __FaceFinder_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ICoreMatcherFaceRegionsMany_INTERFACE_DEFINED__
#define __ICoreMatcherFaceRegionsMany_INTERFACE_DEFINED__

/* interface ICoreMatcherFaceRegionsMany */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICoreMatcherFaceRegionsMany;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DDDB75E1-C09B-47B3-B14F-98D6E342B218")
    ICoreMatcherFaceRegionsMany : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE initialize( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *overflowListing,
            /* [in] */ int BUCKET_SIZE,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ingest( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR Img,
            /* [in] */ BSTR IDRegs,
            /* [in] */ BSTR bucket,
            /* [in] */ int currentOverflowLevel,
            /* [out][in] */ BSTR *newOverflowValue,
            /* [in] */ int BUCKET_SIZE,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE query( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *result,
            /* [in] */ BSTR imgFNRegs,
            /* [in] */ BSTR bucket,
            /* [in] */ int overflowBucket,
            /* [in] */ FLOAT tolerance,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [retval][out] */ ULONG *matches) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE save( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE remove( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR ID,
            /* [out][in] */ ULONG *removed,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE releaseEventDatabase( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE flushLog( void) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ICoreMatcherFaceRegionsManyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICoreMatcherFaceRegionsMany * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICoreMatcherFaceRegionsMany * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *initialize )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *overflowListing,
            /* [in] */ int BUCKET_SIZE,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ingest )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR Img,
            /* [in] */ BSTR IDRegs,
            /* [in] */ BSTR bucket,
            /* [in] */ int currentOverflowLevel,
            /* [out][in] */ BSTR *newOverflowValue,
            /* [in] */ int BUCKET_SIZE,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *query )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *result,
            /* [in] */ BSTR imgFNRegs,
            /* [in] */ BSTR bucket,
            /* [in] */ int overflowBucket,
            /* [in] */ FLOAT tolerance,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [retval][out] */ ULONG *matches);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *save )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *remove )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR ID,
            /* [out][in] */ ULONG *removed,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *releaseEventDatabase )( 
            ICoreMatcherFaceRegionsMany * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *flushLog )( 
            ICoreMatcherFaceRegionsMany * This);
        
        END_INTERFACE
    } ICoreMatcherFaceRegionsManyVtbl;

    interface ICoreMatcherFaceRegionsMany
    {
        CONST_VTBL struct ICoreMatcherFaceRegionsManyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICoreMatcherFaceRegionsMany_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICoreMatcherFaceRegionsMany_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICoreMatcherFaceRegionsMany_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICoreMatcherFaceRegionsMany_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICoreMatcherFaceRegionsMany_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICoreMatcherFaceRegionsMany_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICoreMatcherFaceRegionsMany_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICoreMatcherFaceRegionsMany_initialize(This,regKey,eventID,overflowListing,BUCKET_SIZE,errorString,time,useGPU,performanceValue)	\
    ( (This)->lpVtbl -> initialize(This,regKey,eventID,overflowListing,BUCKET_SIZE,errorString,time,useGPU,performanceValue) ) 

#define ICoreMatcherFaceRegionsMany_ingest(This,regKey,eventID,Img,IDRegs,bucket,currentOverflowLevel,newOverflowValue,BUCKET_SIZE,errorString,time,debugInfo,useGPU,performanceValue)	\
    ( (This)->lpVtbl -> ingest(This,regKey,eventID,Img,IDRegs,bucket,currentOverflowLevel,newOverflowValue,BUCKET_SIZE,errorString,time,debugInfo,useGPU,performanceValue) ) 

#define ICoreMatcherFaceRegionsMany_query(This,regKey,eventID,result,imgFNRegs,bucket,overflowBucket,tolerance,errorString,time,debugInfo,matches)	\
    ( (This)->lpVtbl -> query(This,regKey,eventID,result,imgFNRegs,bucket,overflowBucket,tolerance,errorString,time,debugInfo,matches) ) 

#define ICoreMatcherFaceRegionsMany_save(This,regKey,eventID,errorString,time)	\
    ( (This)->lpVtbl -> save(This,regKey,eventID,errorString,time) ) 

#define ICoreMatcherFaceRegionsMany_remove(This,regKey,eventID,ID,removed,errorString,time)	\
    ( (This)->lpVtbl -> remove(This,regKey,eventID,ID,removed,errorString,time) ) 

#define ICoreMatcherFaceRegionsMany_releaseEventDatabase(This,regKey,eventID,errorString,time)	\
    ( (This)->lpVtbl -> releaseEventDatabase(This,regKey,eventID,errorString,time) ) 

#define ICoreMatcherFaceRegionsMany_flushLog(This)	\
    ( (This)->lpVtbl -> flushLog(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICoreMatcherFaceRegionsMany_INTERFACE_DEFINED__ */


#ifndef __ICoreImageMatcherWhole_INTERFACE_DEFINED__
#define __ICoreImageMatcherWhole_INTERFACE_DEFINED__

/* interface ICoreImageMatcherWhole */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ICoreImageMatcherWhole;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49318CB1-9DA9-43FA-B4AC-0C09BFB389F8")
    ICoreImageMatcherWhole : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE initialize( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *overflowListing,
            /* [in] */ int BUCKET_SIZE,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ingest( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR Img,
            /* [in] */ BSTR IDRegs,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE query( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *result,
            /* [in] */ BSTR imgFNRegs,
            /* [in] */ FLOAT tolerance,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [retval][out] */ ULONG *matches) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE save( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE remove( 
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR ID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct ICoreImageMatcherWholeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICoreImageMatcherWhole * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICoreImageMatcherWhole * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICoreImageMatcherWhole * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICoreImageMatcherWhole * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *initialize )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *overflowListing,
            /* [in] */ int BUCKET_SIZE,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ingest )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR Img,
            /* [in] */ BSTR IDRegs,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [in] */ int useGPU,
            /* [in] */ int performanceValue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *query )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *result,
            /* [in] */ BSTR imgFNRegs,
            /* [in] */ FLOAT tolerance,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ BSTR debugInfo,
            /* [retval][out] */ ULONG *matches);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *save )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *remove )( 
            ICoreImageMatcherWhole * This,
            /* [in] */ BSTR regKey,
            /* [in] */ int eventID,
            /* [in] */ BSTR ID,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time);
        
        END_INTERFACE
    } ICoreImageMatcherWholeVtbl;

    interface ICoreImageMatcherWhole
    {
        CONST_VTBL struct ICoreImageMatcherWholeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICoreImageMatcherWhole_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ICoreImageMatcherWhole_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ICoreImageMatcherWhole_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ICoreImageMatcherWhole_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ICoreImageMatcherWhole_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ICoreImageMatcherWhole_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ICoreImageMatcherWhole_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ICoreImageMatcherWhole_initialize(This,regKey,eventID,overflowListing,BUCKET_SIZE,errorString,time,useGPU,performanceValue)	\
    ( (This)->lpVtbl -> initialize(This,regKey,eventID,overflowListing,BUCKET_SIZE,errorString,time,useGPU,performanceValue) ) 

#define ICoreImageMatcherWhole_ingest(This,regKey,eventID,Img,IDRegs,errorString,time,debugInfo,useGPU,performanceValue)	\
    ( (This)->lpVtbl -> ingest(This,regKey,eventID,Img,IDRegs,errorString,time,debugInfo,useGPU,performanceValue) ) 

#define ICoreImageMatcherWhole_query(This,regKey,eventID,result,imgFNRegs,tolerance,errorString,time,debugInfo,matches)	\
    ( (This)->lpVtbl -> query(This,regKey,eventID,result,imgFNRegs,tolerance,errorString,time,debugInfo,matches) ) 

#define ICoreImageMatcherWhole_save(This,regKey,eventID,errorString,time)	\
    ( (This)->lpVtbl -> save(This,regKey,eventID,errorString,time) ) 

#define ICoreImageMatcherWhole_remove(This,regKey,eventID,ID,errorString,time)	\
    ( (This)->lpVtbl -> remove(This,regKey,eventID,ID,errorString,time) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ICoreImageMatcherWhole_INTERFACE_DEFINED__ */


#ifndef __IFaceFinder_INTERFACE_DEFINED__
#define __IFaceFinder_INTERFACE_DEFINED__

/* interface IFaceFinder */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IFaceFinder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B77687D2-8150-4613-821A-0A9EE29B4D3C")
    IFaceFinder : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFaces( 
            /* [in] */ BSTR path,
            /* [in] */ int options,
            /* [out][in] */ BSTR *faces,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ int useGPU,
            /* [in] */ int performanceOption) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE usingGPU( 
            /* [out][in] */ BSTR *gpuStatus) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFaceFinderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaceFinder * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaceFinder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaceFinder * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaceFinder * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaceFinder * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaceFinder * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaceFinder * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetFaces )( 
            IFaceFinder * This,
            /* [in] */ BSTR path,
            /* [in] */ int options,
            /* [out][in] */ BSTR *faces,
            /* [out][in] */ BSTR *errorString,
            /* [out][in] */ LONG *time,
            /* [in] */ int useGPU,
            /* [in] */ int performanceOption);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *usingGPU )( 
            IFaceFinder * This,
            /* [out][in] */ BSTR *gpuStatus);
        
        END_INTERFACE
    } IFaceFinderVtbl;

    interface IFaceFinder
    {
        CONST_VTBL struct IFaceFinderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaceFinder_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFaceFinder_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFaceFinder_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFaceFinder_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFaceFinder_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFaceFinder_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFaceFinder_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IFaceFinder_GetFaces(This,path,options,faces,errorString,time,useGPU,performanceOption)	\
    ( (This)->lpVtbl -> GetFaces(This,path,options,faces,errorString,time,useGPU,performanceOption) ) 

#define IFaceFinder_usingGPU(This,gpuStatus)	\
    ( (This)->lpVtbl -> usingGPU(This,gpuStatus) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFaceFinder_INTERFACE_DEFINED__ */



#ifndef __FaceMatcherCoreLib_LIBRARY_DEFINED__
#define __FaceMatcherCoreLib_LIBRARY_DEFINED__

/* library FaceMatcherCoreLib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_FaceMatcherCoreLib;

EXTERN_C const CLSID CLSID_CoreMatcherFaceRegionsMany;

#ifdef __cplusplus

class DECLSPEC_UUID("7E06FF04-0A05-4F39-984C-96586EAA660B")
CoreMatcherFaceRegionsMany;
#endif

EXTERN_C const CLSID CLSID_CoreImageMatcherWhole;

#ifdef __cplusplus

class DECLSPEC_UUID("5306C26A-EB56-4935-AF97-97F0EFE73BC5")
CoreImageMatcherWhole;
#endif

EXTERN_C const CLSID CLSID_FaceFinder;

#ifdef __cplusplus

class DECLSPEC_UUID("A68516F9-3A80-4D0A-9DE6-4E7D3C70FE53")
FaceFinder;
#endif
#endif /* __FaceMatcherCoreLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


