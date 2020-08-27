/* CE USB KWrapper - a USB kernel driver and user-space library
 * Copyright (C) 2012-2013 RealVNC Ltd.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

// MutexLocker.cpp : RAII wrapper for mutexes.

#include "StdAfx.h"
#include "MutexLocker.h"

MutexLocker::MutexLocker(HANDLE& aMutex)
: mMutex(aMutex),
  mLocked(true)
{
	WaitForSingleObject(mMutex, INFINITE);
}

MutexLocker::~MutexLocker()
{
	unlock();
}

void MutexLocker::unlock()
{
	if (mLocked)
	{
		ReleaseMutex(mMutex);
		mLocked = false;
	}
}

void MutexLocker::relock()
{
	if (!mLocked)
	{
		WaitForSingleObject(mMutex, INFINITE);
		mLocked = true;
	}
}