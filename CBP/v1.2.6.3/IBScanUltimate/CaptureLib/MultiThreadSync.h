#pragma once

#ifdef _WINDOWS
#include "IBCriticalSection.h"
#elif defined(__linux__)
#include "IBCriticalSection_Linux.h"
#endif

template <class T>
class CMultiThreadSync
{
friend class CThreadSync;
public:
	class CThreadSync
	{
	public:
		CThreadSync(VOID)
		{
			T::mSync.Enter();
		}

		~CThreadSync(VOID)
		{
			T::mSync.Leave();
		}
	};

private:
	static CIBCriticalSection mSync;
};

template <class T>
CIBCriticalSection CMultiThreadSync<T>::mSync;