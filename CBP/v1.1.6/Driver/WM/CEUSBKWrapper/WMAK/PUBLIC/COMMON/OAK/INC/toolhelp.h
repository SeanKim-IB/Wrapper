//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
#ifndef __TOOLHELP_H__
#define __TOOLHELP_H__

#include "tlhelp32.h"

#ifdef __cplusplus
extern "C" {
#endif

// upping this value from 1MB to 1.5MB. If we make it 2Meg, then VirtualAlloc() will 
// allocate it in shared upper memory, potentially creating a security hole.
// We cannot do 1.5*1024*1024 since we are doing integer math.
// 1M*3/2 done this way so 3/2 does not become 1.
#define THSNAP_RESERVE (3*512*1024)                  


typedef struct TH32HEAPENTRY {
    HEAPENTRY32 heapentry;
    struct TH32HEAPENTRY *pNext;
} TH32HEAPENTRY;

typedef struct TH32HEAPLIST {
    HEAPLIST32 heaplist;
    TH32HEAPENTRY *pHeapEntry;
    struct TH32HEAPLIST *pNext;
} TH32HEAPLIST, *PTH32HEAPLIST;

typedef struct TH32PROC {
    PROCESSENTRY32 procentry;
    TH32HEAPENTRY *pMainHeapEntry;
    struct TH32PROC *pNext;
} TH32PROC;

typedef struct TH32MOD {
    MODULEENTRY32 modentry;
    struct TH32MOD *pNext;
} TH32MOD;

typedef struct TH32THREAD {
    THREADENTRY32 threadentry;
    struct TH32THREAD *pNext;
} TH32THREAD;

typedef struct THSNAP {
    LPBYTE pNextFree;
    LPBYTE pHighCommit;
    LPBYTE pHighReserve;
    TH32PROC *pProc;
    TH32MOD *pMod;
    TH32THREAD *pThread;
    TH32HEAPLIST *pHeap;

    /* Keep track of the location of snapshot data. */
    TH32HEAPENTRY *pCurHeapEntry;
    TH32HEAPLIST *pCurHeapList;
} THSNAP;

#ifdef __cplusplus
}
#endif

#endif

