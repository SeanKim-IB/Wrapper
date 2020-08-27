/*
 * Copyright (c) 2005, Raghupathy Srinivasan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 *     * Redistributions of source code must retain the above 
 *       copyright notice, this list of conditions and the 
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the 
 *       above copyright notice, this list of conditions and 
 *       the following disclaimer in the documentation and/or 
 *       other materials provided with the distribution.
 *     * The names of contributors to this software may not be 
 *       used to endorse or promote products derived from this 
 *       software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
 * DAMAGE.
 * 
 *
 * CONTRIBUTORS
 *  Raghupathy Srinivasan <raghupathys@hotmail.com>
 *
 */ 


//////////////////////////////////////////////////////////////////////
// WorkerThread.h: interface for the CWorkerThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// Win32 includes for basic functionality
#include <windows.h>

// Headers for pooling functionality
#include "job.h"
//////////////////////////////////////////////////////////////////////


#define WM_USER_SUBMITJOB	WM_USER + 1
#define WM_USER_QUIT		WM_USER + 2

#define MSG_PARAM			9292

//////////////////////////////////////////////////////////////////////
// This is a wrapper around a Win32 thread.
// It has a static method which runs a message loop.
// So technically, it is not a worker thread, but a UI thread.
// However, just to emphasize the fact that this thread runs jobs,
// i have labelled it as a worker thread
class CWorkerThread  
{
public:
	CWorkerThread();
	virtual ~CWorkerThread();
	bool SubmitJob( CJob* p_pJob ); // Submits a job to the embedded thread
	
	static DWORD WINAPI ThreadFunc( LPVOID lpvThreadParam ); // Static thread proc
	
	LONG	m_iNumJobs; // Current number of jobs in the thread's queue
	BOOL	m_bAliveWorkerThread;
private:
	HANDLE	m_hThread; // The embedded thread handle
	DWORD   m_dwThreadID; // The embedded thread id
	HANDLE	m_hEvent; // Event used to signal readiness of thread ...
};
//////////////////////////////////////////////////////////////////////
