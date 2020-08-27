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
// ThreadPoolMgr.h: interface for the CThreadPoolMgr class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// STL / Win32 includes for basic functionality
#include <vector>
#ifdef _WINDOWS
#include <windows.h>
#include "WorkerThread.h"
#elif defined(__linux__)
#include <pthread.h>
#include "LinuxPort.h"
#include "WorkerThread_Linux.h"
#endif

// Headers for pooling functionality
#include "../CaptureLib/MultiThreadSync.h"
#include "Job.h"
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constants for use in the pool manager
#define		TP_DEF_POOL_SIZE			5 // Default number of threads in the pool
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Some typedefs for easy use
typedef std::vector<CWorkerThread*>		thread_pool;
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// The thread pool manager is responsible for creating a pool of threads
// and assigning jobs to them
class CThreadPoolMgr
{
public:
	CThreadPoolMgr();
	CThreadPoolMgr(int p_iPoolSize);
	virtual ~CThreadPoolMgr();
	
	bool InitThreadPool(); // Initialize the thread pool
	bool ClearThreadPool(); // Destroy all threads in the pool

	bool SubmitJob( CJob* p_pJob ); // Submit a job to be performed

private:

	int					m_iThreadPoolSize;	// Number of threads in the pool
	thread_pool			m_obThreadPool;		// Vector of threads in the pool
	bool				m_bIsPoolValid;		// Is the thread pool shutting down? 
#if defined(_WINDOWS)
    CRITICAL_SECTION	m_csCritSec;		// Used to synchronize access to the thread pool
#elif defined(__linux__)
   pthread_mutex_t      m_mutex;           // Mutex
#endif
};
//////////////////////////////////////////////////////////////////////
