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

#include "stdafx.h"
#include "WorkerThread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Initialize members. Then create the thread.
CWorkerThread::CWorkerThread()
{
    m_dwThreadID	= 0;
    m_hThread		= 0;
    m_iNumJobs		= 0;

    m_hEvent = CreateEvent(0, FALSE, FALSE, 0);

    // NOTE:	CreateThread returns NULL in case of failure.
    //			In production code, you MUST handle failures, and pass
    //			it on according to whatever error handling mechanisms you follow
    m_hThread		= ::CreateThread(0,
                                     0,
                                     ThreadFunc,
                                     this,
                                     0,
                                     &m_dwThreadID);

    // Wait for the thread to initialize and get ready ...
    WaitForSingleObject(m_hEvent, INFINITE);

    m_bAliveWorkerThread = FALSE;
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
        // NOTE:	PostThreadMessage can also fail. A null return value
        //			indicates failure. Again, you MUST handle failures
        ::PostThreadMessage(m_dwThreadID,
                            WM_USER_QUIT,
                            0,
							MSG_PARAM ); 

        // Wait on the thread, until it completes
        //		::WaitForSingleObject( m_hThread, INFINITE );
        //		::WaitForSingleObject( m_hThread, 5000 );
        DWORD nRc;
        while (TRUE)
        {
            nRc = ::WaitForSingleObject(m_hThread, 1000);
            if (nRc == WAIT_OBJECT_0)
            {
                break;
            }

            if (nRc == WAIT_TIMEOUT && m_bAliveWorkerThread == FALSE)
            {
                // 2012-09-17 enzyme add - Forced to kill thread due to Visual Basic
//#ifdef __G_DEBUG__
	TRACE(_T("WorkerThread timeout\n"));
//#endif
                DWORD nExitCode = NULL;
                GetExitCodeThread(m_hThread, &nExitCode);
                TerminateThread(m_hThread, nExitCode);
                break;
            }
        }
        m_hThread = 0;

        ::CloseHandle(m_hEvent);
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

    // Post the the job pointer (typecast to a wparam) to the thread
    ::PostThreadMessage(m_dwThreadID,
						WM_USER_SUBMITJOB,
                        (WPARAM) p_pJob,
						MSG_PARAM );

    // Increment the number of jobs in the threads message queue
    ::InterlockedIncrement(&m_iNumJobs);

    return bRetVal;
}
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// This is a static function which is at the heart of the worker thread class.
// This function is used as the start address of each thread created through CWorkerThread.
DWORD CWorkerThread::ThreadFunc(LPVOID lpvThreadParam)
{
    // This function receives a pointer to the CWorkerThread instance as an LPVOID.
    // This is type cast back to a CWorkerThread*.
    // Then the function runs a message loop.

    DWORD dwRetVal = 0;

    // Typecast the thread param to a CWorkerThread*
    CWorkerThread *pWorkerThread = static_cast<CWorkerThread *>(lpvThreadParam);

    pWorkerThread->m_bAliveWorkerThread = TRUE;
    if (!pWorkerThread)
    {
        // Invalid worker thread pointer, so quit
        dwRetVal ++;

    }
    else
    {
        // Run a message loop here
        MSG msg; // MSG struct to hold each message temporarirly

        // Create the message queue with this call ...
        PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

        // Signal the pool manager that we are ready ...
        SetEvent(pWorkerThread->m_hEvent);

        while (::GetMessage(&msg, 0, 0, 0))       // The loop
        {
			if (msg.message == WM_USER_QUIT && msg.lParam == MSG_PARAM)
			{
				break;
			}
			else if (msg.message == WM_USER_SUBMITJOB && msg.lParam == MSG_PARAM)
			{
                try
                {
                    // Cast the wparam back into a CJob*
                    CJob *pJob = (CJob *) msg.wParam;

                    if (pJob)     // If a valid CJob*
                    {
                        // Invoke the job func, with the job param, and store
                        // the results in the m_dwRetVal member of the pJob itself
                        pJob->m_dwRetVal = pJob->m_pJobFunc(pJob->m_pJobParam);

                        // Invoke the notifier, and pass it the pJob
                        // The notifier should clean up by deleting the pJob after
                        // performing notifications
                        pJob->m_pNotifier(pJob);

                    }

                    // Decrement the number of jobs pending for this thread
                    ::InterlockedDecrement(&(pWorkerThread->m_iNumJobs));

                }
                catch (...)
                {
                    // Do something reasonable ...

                }

            }
            else
            {
				TRACE(_T("Invalid Thread message (hwnd=%x, message=%x, wParam=%x, lParam=%x)"),
					msg.hwnd, msg.message, msg.wParam, msg.lParam);
            }

        }

    }

    pWorkerThread->m_bAliveWorkerThread = FALSE;

    return dwRetVal;
}
//////////////////////////////////////////////////////////////////////