#pragma once

class CIBCriticalSection
{
public:
	CIBCriticalSection(VOID)
	{
		InitializeCriticalSection(&mSync);
//        mSync = CreateMutex(NULL, FALSE, NULL);
	}

	~CIBCriticalSection(VOID)
	{
		DeleteCriticalSection(&mSync);
//        CloseHandle(mSync);
	}

	inline VOID Enter(VOID)
	{
		EnterCriticalSection(&mSync);
//        WaitForSingleObject(mSync, WAIT_ABANDONED);
	//	TRACE("mSync.Enter, LockCount=%d, OwningThread=%x, ThreadID=%x\n", mSync.LockCount, mSync.OwningThread, GetCurrentThreadId());
    }

	inline VOID Leave(VOID)
	{
		LeaveCriticalSection(&mSync);
//        ReleaseMutex(mSync);
	//	TRACE("mSync.Leave, LockCount=%d, OwningThread=%x, ThreadID=%x\n", mSync.LockCount, mSync.OwningThread, GetCurrentThreadId());
	}

private:
	CRITICAL_SECTION	mSync;
//    HANDLE  mSync;
};