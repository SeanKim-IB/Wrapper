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
// ThreadPoolMgr.cpp: implementation of the CThreadPoolMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ThreadPoolMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Default constructor - set the pool size to TP_DEF_POOL_SIZE
CThreadPoolMgr::CThreadPoolMgr():
    m_iThreadPoolSize(TP_DEF_POOL_SIZE)   // Init the pool size
{
#if defined(_WINDOWS)
    ::InitializeCriticalSection( &m_csCritSec ); // Initialize critical section
#elif defined(__linux__)
      pthread_mutexattr_t mutexattr;   // Mutex attribute variable
      pthread_mutexattr_init(&mutexattr);
      // Set the mutex as a recursive mutex
      pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
      // create the mutex with the attributes set
      pthread_mutex_init(&m_mutex, &mutexattr);
      //After initializing the mutex, the thread attribute can be destroyed
      pthread_mutexattr_destroy(&mutexattr);
#endif
    InitThreadPool(); // Initialize the thread pool

}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Parametrized constructor - which accepts the pool size
CThreadPoolMgr::CThreadPoolMgr(int p_iPoolSize):
    m_iThreadPoolSize(p_iPoolSize)   // Init the pool size
{
#if defined(_WINDOWS)
    ::InitializeCriticalSection( &m_csCritSec ); // Initialize critical section
#elif defined(__linux__)
    pthread_mutexattr_t mutexattr;   // Mutex attribute variable
    pthread_mutexattr_init(&mutexattr);
    // Set the mutex as a recursive mutex
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    // create the mutex with the attributes set
    pthread_mutex_init(&m_mutex, &mutexattr);
    //After initializing the mutex, the thread attribute can be destroyed
    pthread_mutexattr_destroy(&mutexattr);
#endif
    InitThreadPool(); // Initialize the thread pool

}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Destructor - free all resources - threads, crit secs etc
CThreadPoolMgr::~CThreadPoolMgr()
{
    ClearThreadPool();
#if defined(_WINDOWS)
    ::DeleteCriticalSection( &m_csCritSec ); // Release the critical section
#elif defined(__linux__)
    // Destroy / close the mutex
    pthread_mutex_destroy (&m_mutex);
#endif
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Initialize the thread pool
bool CThreadPoolMgr::InitThreadPool()
{
    bool bRetVal = true;

    for (int i = 0; i < m_iThreadPoolSize; i ++)
    {
        // Create one CWorkerThread per thread in the pool
        CWorkerThread *pWorkerThread = new CWorkerThread;

        if (pWorkerThread)
        {
            // If the thread has been created successfully, add to the Thread Pool
            m_obThreadPool.push_back(pWorkerThread);

        }
        else
        {
            // NOTE 1:	msvcrt new does not throw an exception as in the c++ spec
            //			so you have to check the pointer
            // NOTE 2:  error handling here is for illustration only.
            //			replace the error handling here to be inline with
            //			whatever coding standards you adhere to
            bRetVal = false;
            ClearThreadPool(); // Free all threads in the pool

        }

    }

    m_bIsPoolValid = bRetVal;

    return bRetVal;

}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Here we will need to delete all the threads in the pool
bool CThreadPoolMgr::ClearThreadPool()
{
    bool bRetVal = true;

#if defined(_WINDOWS)
    // Enter the critical region around the pool
    ::EnterCriticalSection( &m_csCritSec );
#elif defined(__linux__)
    // Acquire the mutex to access the shared resource
    pthread_mutex_lock (&m_mutex);
#endif

    // Signal that the pool will not accept any more jobs
    m_bIsPoolValid = false;

    // Start deleting all CWorkerThreads
    // NOTE:	The CWorkerThread destructor posts a WM_QUIT message to the embedded thread,
    //			and then waits on the thread to complete. So each destructor will return only
    //			after the embedded thread has processed all jobs in it's queue
    for (int i = 0; i < m_iThreadPoolSize; i ++)
    {
        try
        {
            // delete calls the destructor - which will return after all
            // the threads jobs are completed
            delete m_obThreadPool[i];
            m_obThreadPool[i] = 0;

        }
        catch (...)
        {
            bRetVal = false ;
        }
    }

#if defined(_WINDOWS)
    // Leave the critical region around the pool
    ::LeaveCriticalSection( &m_csCritSec );
#elif defined(__linux__)
    // Release the mutex  and release the access to shared resource
    pthread_mutex_unlock (&m_mutex);
#endif

    return bRetVal;
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Here we locate a suitable thread from the pool and submit the job
bool CThreadPoolMgr::SubmitJob(CJob *p_pJob)
{
    bool			bRetVal = false;
    CWorkerThread	*pWorkerThread;

    // We will use DCLP (Double checked lock pattern) here, trivial optimization ...
    if (!m_bIsPoolValid)
    {
        bRetVal = false;

    }
    else
    {
#if defined(_WINDOWS)
        // Enter the critical region around the pool
        ::EnterCriticalSection( &m_csCritSec );
#elif defined(__linux__)
        // Acquire the mutex to access the shared resource
        pthread_mutex_lock (&m_mutex);
#endif

        // Check pool validity once more (DCLP remember?)
        if (m_bIsPoolValid)
        {
            // Get the first thread
            pWorkerThread = m_obThreadPool.front();

            // Locate a thread with the lowest number of jobs ...
            // NOTE 1:	This approach assumes all jobs are equal,
            //			which is an obviously wrong assumption.
            //			So you will need to think of weighting jobs - based on complexity,
            //			latency etc etc.
            // NOTE 2:	Each thread is hard at work processing jobs.
            //			So by the time we reach the end of the list,
            //			chances are we have the most loaded thread,
            //			instead of the least loaded thread.
            //			So you will also need to figure out a way for each
            //			thread to return an absolute ETA, and then
            //			use the min of those to assign the current job to.
            for (int i = 0; i < m_iThreadPoolSize; i++)
            {
                if (pWorkerThread->m_iNumJobs > m_obThreadPool[i]->m_iNumJobs)
                {
                    pWorkerThread = m_obThreadPool[i];
                }

            }

            // Submit the job to the least loaded thread
            pWorkerThread->SubmitJob(p_pJob);
        }

#if defined(_WINDOWS)
        // Leave the critical region around the pool
        ::LeaveCriticalSection( &m_csCritSec );
#elif defined(__linux__)
        // Release the mutex  and release the access to shared resource
        pthread_mutex_unlock (&m_mutex);
#endif

    }

    return bRetVal;
}
//////////////////////////////////////////////////////////////////////
