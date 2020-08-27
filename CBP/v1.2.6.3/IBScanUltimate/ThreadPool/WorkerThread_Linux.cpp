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
// WorkerThread.cpp: implementation of the CWorkerThread class.
//
//////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <stdio.h>
#include <deque>
#include "LinuxPort.h"
#include "WorkerThread_Linux.h"

#ifdef __android__
// --------------------------------------------------------------
// Android Debug code
#include <android/log.h> 
#define LOG_DEBUG_TAG "IBSCAN" 


// End Android Debug code
// ------------------------------------------------------------#endif
#endif

/* NOTE: GCC doesn't instantiate static class data members */
static sem_t    m_sem;  // Event semaphore
typedef std::deque<CJob *> JobQueue;
static JobQueue m_jobQueue;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Initialize members. Then create the thread.
CWorkerThread::CWorkerThread()
{
    m_hThread		= 0;
    m_iNumJobs		= 0;
    m_bThreadStarted = false;
    m_bThreadEnded = false;

    int retCode = sem_init(
                      &m_sem,   // handle to the event semaphore
                      0,     // not shared
                      0);    // initially set to non signaled state

    if (retCode == -1)
    {
        perror("Failed to initialize semaphore sem");
    }

    pthread_attr_t threadAttr;

    //struct sched_param pThreadParam;  // scheduling priority

    // initialize the thread attribute
    pthread_attr_init(&threadAttr);

    // Set the stack size of the thread
    pthread_attr_setstacksize(&threadAttr, 240 * 1024);

    // Set thread to detached state. No need for pthread_join
//#ifdef __android__
//    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
//#else
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
//#endif

    // Create the threads
    int threadRC = pthread_create(&m_hThread, &threadAttr, ThreadFunc, this);

    // Destroy the thread attributes
    pthread_attr_destroy(&threadAttr);

    if (threadRC != 0)
    {
        //			In production code, you MUST handle failures, and pass
        //			it on according to whatever error handling mechanisms you follow
        m_hThread = 0;
        return;
    }

    // Wait for the thread to initialize and get ready ...
    int tries = 0;
    do
    {
        Sleep(10);
        tries++;
    }
    while (!m_bThreadStarted && (tries < 10));
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// The destructor signals the thread to complete
// and then waits for it to complete
CWorkerThread::~CWorkerThread()
{
    if (m_hThread)
    {
        // Post a message to the thread, asking it to wrap up

        {
            CThreadSync Sync;
#ifdef __android__
//            m_jobQueue.push_back(NULL);    // NULL to quit
#endif
        }
        sem_post(&m_sem);

        // Wait on the thread, until it completes

        int rc = 0, status;
#ifdef __android__
        //  ** Android doesn't implement this -> rc = pthread_cancel(m_hThread); // Forced kill
        if ((status = pthread_kill(m_hThread, SIGUSR1)) != 0)
        {
		    __android_log_print(ANDROID_LOG_DEBUG, LOG_DEBUG_TAG, "Error cancelling error = %d\r\n", status);
            // printf("Error cancelling thread %d, error = %d (%s)", pthread_id, status, strerror status));
        }
#else
        rc = pthread_cancel(m_hThread); // Forced kill
#endif
        if (rc == 0 && pthread_join(m_hThread, (void **)&status) == 0)
        {
            //			printf("Forced kill thread (%d)\n", status);
#ifdef __android__
		    __android_log_print(ANDROID_LOG_DEBUG, LOG_DEBUG_TAG, "Forced kill thread (%d)\r\n", status);
#endif
        }

        m_hThread = 0;

    }

}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Submits a job to the thread's queue.
// The jobs are sent via a PostThreadMessage
// So in effect we are using the thread's message queue to hold jobs queued up for the thread
bool CWorkerThread::SubmitJob(CJob *p_pJob)
{
    bool bRetVal = true;

    // Post the the job pointer to the thread
    {
        CThreadSync Sync;
        m_jobQueue.push_back(p_pJob);
    }

    // Increment the number of jobs in the threads message queue
    __sync_fetch_and_add(&m_iNumJobs , 1);
    //::InterlockedIncrement( &m_iNumJobs );

    sem_post(&m_sem);
    return bRetVal;
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// This is a static function which is at the heart of the worker thread class.
// This function is used as the start address of each thread created through CWorkerThread.
void *CWorkerThread::ThreadFunc(LPVOID lpvThreadParam)
{
    // This function receives a pointer to the CWorkerThread instance as an LPVOID.
    // This is type cast back to a CWorkerThread*.
    // Then the function runs a message loop.

    DWORD dwRetVal = 0;

    // Typecast the thread param to a CWorkerThread*
    CWorkerThread *pWorkerThread = static_cast<CWorkerThread *>(lpvThreadParam);

    if (!pWorkerThread)
    {
        // Invalid worker thread pointer, so quit
        dwRetVal ++;

    }
    else
    {
        // Run a message loop here

        // Signal the pool manager that we are ready ...
        pWorkerThread->m_bThreadStarted = true;

        while (TRUE)    // The loop
        {
            int rc = sem_wait(&m_sem);

            if (rc != 0)
            {
                //				int exitReason = errno;
                break;
            }

            CJob *pJob = NULL;
            {
                CThreadSync Sync;
                // Check for multiple thread wakeups (shouldn't happen)
                if (m_jobQueue.size() == 0)
                {
                    continue;
                }
                pJob = m_jobQueue.front();
                m_jobQueue.pop_front();  // Calls destructor for the pointer, not the CJob
            }
            if (pJob == NULL)
            {
                // Terminate
                pWorkerThread->m_bThreadStarted = false;
                break;
            }

            try
            {
                // Invoke the job func, with the job param, and store
                // the results in the m_dwRetVal member of the pJob itself
                pJob->m_dwRetVal = pJob->m_pJobFunc(pJob->m_pJobParam);

                // Invoke the notifier, and pass it the pJob
                // The notifier should clean up by deleting the pJob after
                // performing notifications
                //
                pJob->m_pNotifier(pJob);


                // Decrement the number of jobs pending for this thread
                __sync_fetch_and_sub(&(pWorkerThread->m_iNumJobs) , 1);
                //::InterlockedDecrement( &( pWorkerThread->m_iNumJobs ) );

            }
            catch (...)
            {
                // Do something reasonable ...

            }

        }

    }

    return NULL;
}
//////////////////////////////////////////////////////////////////////
