

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 04:14:07 2038
 */
/* Compiler settings for iping.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0622 
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data , no_format_optimization
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __iping_h__
#define __iping_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPing_FWD_DEFINED__
#define __IPing_FWD_DEFINED__
typedef interface IPing IPing;

#endif 	/* __IPing_FWD_DEFINED__ */


/* header files for imported files */
#include "objidl.h"
#include "oaidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IPing_INTERFACE_DEFINED__
#define __IPing_INTERFACE_DEFINED__

/* interface IPing */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IPing;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("decdbeef-d1ac-11d1-96bc-00aa00573fb0")
    IPing : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Ping( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PingToServer( 
            /* [in] */ LPSTR pszString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PingToClient( 
            /* [out] */ LPSTR *ppszString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PingToClientSize( 
            /* [in] */ ULONG cbOut) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IPingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPing * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPing * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPing * This);
        
        HRESULT ( STDMETHODCALLTYPE *Ping )( 
            IPing * This);
        
        HRESULT ( STDMETHODCALLTYPE *PingToServer )( 
            IPing * This,
            /* [in] */ LPSTR pszString);
        
        HRESULT ( STDMETHODCALLTYPE *PingToClient )( 
            IPing * This,
            /* [out] */ LPSTR *ppszString);
        
        HRESULT ( STDMETHODCALLTYPE *PingToClientSize )( 
            IPing * This,
            /* [in] */ ULONG cbOut);
        
        END_INTERFACE
    } IPingVtbl;

    interface IPing
    {
        CONST_VTBL struct IPingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPing_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPing_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPing_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPing_Ping(This)	\
    ( (This)->lpVtbl -> Ping(This) ) 

#define IPing_PingToServer(This,pszString)	\
    ( (This)->lpVtbl -> PingToServer(This,pszString) ) 

#define IPing_PingToClient(This,ppszString)	\
    ( (This)->lpVtbl -> PingToClient(This,ppszString) ) 

#define IPing_PingToClientSize(This,cbOut)	\
    ( (This)->lpVtbl -> PingToClientSize(This,cbOut) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPing_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


