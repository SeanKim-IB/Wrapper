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

// UsbDevice.cpp : Represents a particular USB device or interface for a device.

#include "StdAfx.h"
#include "UsbDevice.h"
#include "UsbDeviceList.h"
#include "Transfer.h"
#include "drvdbg.h"
#include "UserBuffer.h"
#include "MutexLocker.h"
#include "EndianUtils.h"
#include "InterfaceClaimers.h"
#include <Usbclient.h>

#include <new>

#ifdef DEBUG
void UsbDeviceDumpOutput( char *szTitle, UCHAR *pBuffer, size_t Count )
{

	if( ZONE_DUMPBUFFER )
	{
		WCHAR OutBuffer[512];
		int LoopCount = 0;
		Count = min( Count, 128 );  // Limit output to 128 bytes max
		while( Count )
		{
			WCHAR *pwchPos = OutBuffer;
			wsprintfW(pwchPos, TEXT("%S:%08x:%02x:"), szTitle, pBuffer,(LoopCount++)*8);
			pwchPos += wcslen(pwchPos);

			for( int i = 0; i < 7 && Count > 1; i++, Count-- )
			{
				wsprintfW(pwchPos, TEXT("%02x,"), *pBuffer++);
				pwchPos += wcslen(pwchPos); 
			}
			if( Count )
			{
				wsprintfW(pwchPos, TEXT("%02x"), *pBuffer++);
				Count--;
				pwchPos += wcslen(pwchPos); 
			}
			DUMPBUFFER_MSG((TEXT("%s\r\n"), OutBuffer));
		}
	}
}
#endif



extern "C" BOOL UsbDeviceNotifyRoutine(
	LPVOID lpvNotifyParameter,
	DWORD dwCode,
	LPDWORD* dwInfo1,
	LPDWORD* dwInfo2,
	LPDWORD* dwInfo3,
	LPDWORD* dwInfo4
	)
{
	UsbDevice* dev = static_cast<UsbDevice*>(lpvNotifyParameter);
	switch (dwCode) {
#if _WIN32_WCE >= 0x700
		case USB_RESUMED_DEVICE:
		case USB_SUSPENDED_DEVICE:
			return FALSE;
#endif
		case USB_CLOSE_DEVICE:
		{
			dev->Close();
			UsbDeviceList* list = UsbDeviceList::Get();
			if (list && dev) {
				list->PutDevice(dev);
			}
			return TRUE;
		}
		default:
			WARN_MSG((TEXT("USBKWrapperDrv!UsbDeviceNotifyRoutine")
				TEXT(" received unknown code %d for dev 0x%08x\r\n"),
				dwCode, dev));
			return FALSE;
	}
	return FALSE;
}

UsbDevice::UsbDevice(
		unsigned char Bus,
		unsigned char Address,
		unsigned long SessionId)
: mRefCount(1),
  mDevice(NULL),
  mUsbFuncs(NULL),
  mUsbInterface(NULL),
  mRegistered(FALSE),
  mBus(Bus),
  mAddress(Address),
  mSessionId(SessionId),
  mInterfaceClaimers(NULL),
  mInterfaceClaimersCount(0),
  mConfigDescriptors(NULL),
  mNumConfigurations(0),
  mAttachKernelDriverCount(0),
  mAttachKernelDriverEvent(NULL)
{

}

UsbDevice::~UsbDevice()
{
	Close();

	if (mInterfaceClaimers)
		delete [] mInterfaceClaimers;

	if (mAttachKernelDriverEvent)
		CloseHandle(mAttachKernelDriverEvent);

	DestroyAllConfigDescriptors();

	DEVLIFETIME_MSG((
		TEXT("USBKWrapperDrv!UsbDevice::~UsbDevice() mDevice: 0x%08x mBus: %d mAddress: %d)\r\n"),
		mDevice, mBus, mAddress));
}

void UsbDevice::Close()
{
	WriteLocker lock(mCloseMutex);
	if (mRegistered && mUsbFuncs &&	mDevice) {
		DEVLIFETIME_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::Close() mDevice: 0x%08x mBus: %d mAddress: %d\r\n"),
			mDevice, mBus, mAddress));

		// Ensure all kernel driver attach operations have completed before closing.
		while (mAttachKernelDriverCount > 0) {
			lock.unlock();
			WaitForSingleObject(mAttachKernelDriverEvent, INFINITE);
			lock.relock();
		}

		// Provide notification to userland that this USB device has disappeared
		AdvertiseDevice(FALSE);

		mUsbFuncs->lpUnRegisterNotificationRoutine(
			mDevice, UsbDeviceNotifyRoutine, this);
		// Close any pipes in, and release all, claimed interfaces.
		for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
			ClosePipes(mInterfaceClaimers[i]);
			mInterfaceClaimers[i].ReleaseAll();
		}
		mRegistered = FALSE;
		mUsbFuncs = NULL;
		mDevice = NULL;
	}
}

BOOL UsbDevice::Closed() const
{
	return (!mRegistered) || mUsbFuncs == NULL || mDevice == NULL;
}

BOOL UsbDevice::Init(USB_HANDLE hDevice,
	LPCUSB_FUNCS lpUsbFuncs,
	LPCUSB_INTERFACE lpInterface)
{
	if (!hDevice || !lpUsbFuncs) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::Init(...) - passed null device or functions\r\n")));
		return FALSE;
	}

	if (!mCloseMutex.Init()) {
		ERROR_MSG((TEXT("USBKWrapperDrv!UsbDevice::Init() - failed to create mutex\r\n")));
		return FALSE;
	}

	mAttachKernelDriverEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!mAttachKernelDriverEvent) {
		ERROR_MSG((TEXT("USBKWrapperDrv!UsbDevice::Init() - failed to create event\r\n")));
		return FALSE;
	}

	mDevice = hDevice;
	mUsbFuncs = lpUsbFuncs;
	mUsbInterface = lpInterface;

	// Need to register for notifications
	// so that it can be noticed when devices disconnect
	mRegistered = mUsbFuncs->lpRegisterNotificationRoutine(
		mDevice, UsbDeviceNotifyRoutine, this);
	if (!mRegistered) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::Init(...) - failed to register for device notification\r\n")));
		return FALSE;
	}

	WriteLocker lock(mCloseMutex);
	if (Closed()) {
		// Check that the device hasn't been disconnected in the meantime.
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::Init(...) - device closed\r\n")));
		return FALSE;
	}
	// Allocate any structures needed
	if (!AllocateInterfaceClaimers()) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::Init(...) - failed to allocate interface claimers\r\n")));
		return FALSE;
	}

	// Fetch and cache raw configuration descriptor bytes
	if (!FetchAllConfigDescriptors()) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::Init(...) - failed to fetch configuration descriptors\r\n")));
		return FALSE;
	}

	// Provide notification to userland that this USB device has appeared
	AdvertiseDevice(TRUE);

	DEVLIFETIME_MSG((
		TEXT("USBKWrapperDrv!UsbDevice::Init(0x%08x, ...) mBus: %d mAddress: %d\r\n"),
		mDevice, mBus, mAddress));
	return TRUE;
}

BOOL UsbDevice::IsSameDevice(USB_HANDLE hDevice) {
	return (hDevice == mDevice);
}

UKWD_USB_DEVICE UsbDevice::GetIdentifier()
{
	return this;
}

void UsbDevice::IncRef()
{
	// This is done without a lock as this should only be called by UsbDeviceList
	++mRefCount;
	DEVLIFETIME_MSG((
		TEXT("USBKWrapperDrv!UsbDevice::IncRef() mDevice: %d mBus: %d mAddress: %d mRefCount: %d\r\n"),
		mDevice, mBus, mAddress, mRefCount));
}

DWORD UsbDevice::DecRef()
{
	// This is done without a lock as this should only be called by UsbDeviceList
	--mRefCount;
	DEVLIFETIME_MSG((
		TEXT("USBKWrapperDrv!UsbDevice::DecRef() mDevice: %d mBus: %d mAddress: %d mRefCount: %d\r\n"),
		mDevice, mBus, mAddress, mRefCount));
	return mRefCount;
}

void UsbDevice::ReleaseAllInterfaces(LPVOID Context)
{
	WriteLocker lock(mCloseMutex);
	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		mInterfaceClaimers[i].ReleaseAll(Context);
	}
}

BOOL UsbDevice::ClaimInterface(DWORD dwInterfaceValue, LPVOID Context)
{
	WriteLocker lock(mCloseMutex);
	if (Closed()) {
		// Don't allow closed devices to be claimed
		FUNCTION_MSG((	TEXT("USBKWrapperDrv!UsbDevice::ClaimInterface() error: attempt to claim interface on closed device.\r\n")));
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		if (mInterfaceClaimers[i].InterfaceValue() == dwInterfaceValue) {
			BOOL IsFirst = FALSE;
			if (!mInterfaceClaimers[i].Claim(Context, IsFirst)) {
				FUNCTION_MSG((	TEXT("USBKWrapperDrv!UsbDevice::ClaimInterface() error: couldn't claim interface.\r\n")));
				return FALSE;
			}
			// If this was the first claim then need to open pipes
			if (IsFirst) {
				if (!OpenPipes(mInterfaceClaimers[i])) {
					FUNCTION_MSG((	TEXT("USBKWrapperDrv!UsbDevice::ClaimInterface() error: couldn't open Pipes.\r\n")));
					ClosePipes(mInterfaceClaimers[i]);
					SetLastError(ERROR_INTERNAL_ERROR);
					return FALSE;
				}
			}
			FUNCTION_MSG((TEXT("USBKWrapperDrv!UsbDevice::ClaimInterface() succeeded\r\n")));	
			return TRUE;
		}
	}
	ERROR_MSG((TEXT("USBKWrapperDrv!UsbDevice::ClaimInterface() error: Interface not found in mInterfaceClaimers\r\n")));
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
}

BOOL UsbDevice::ReleaseInterface(DWORD dwInterfaceValue, LPVOID Context)
{
	WriteLocker lock(mCloseMutex);
	if (Closed()) {
		// Closed devices have already had their interfaces released
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		if (mInterfaceClaimers[i].InterfaceValue() == dwInterfaceValue) {
			if (!mInterfaceClaimers[i].Release(Context)) {
				return FALSE;
			}
			return TRUE;
		}
	}
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
}

BOOL UsbDevice::AnyInterfacesClaimed() const
{
	ReadLocker lock(mCloseMutex);
	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		if (mInterfaceClaimers[i].AnyClaimed())
			return TRUE;
	}
	return FALSE;
}

BOOL UsbDevice::InterfaceClaimed(DWORD dwInterfaceValue, LPVOID Context) const
{
	ReadLocker lock(mCloseMutex);
	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		if (mInterfaceClaimers[i].InterfaceValue() == dwInterfaceValue)
			return mInterfaceClaimers[i].IsClaimed(Context);
	}
	return FALSE;
}

BOOL UsbDevice::FindInterface(UCHAR Endpoint, DWORD& dwInterfaceValue)
{
	ReadLocker lock(mCloseMutex);
	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		if (mInterfaceClaimers[i].HasEndpoint(Endpoint)) {
			dwInterfaceValue = mInterfaceClaimers[i].InterfaceValue();
			return TRUE;
		}
	}
	return FALSE;
}

unsigned char UsbDevice::Bus() const
{
	return mBus;
}

unsigned char UsbDevice::Address() const
{
	return mAddress;
}

unsigned long UsbDevice::SessionId() const
{
	return mSessionId;
}


BOOL UsbDevice::GetDeviceDescriptor(LPUSB_DEVICE_DESCRIPTOR lpDeviceDescriptor)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	LPCUSB_DEVICE devInfo = mUsbFuncs->lpGetDeviceInfo(mDevice);
	memcpy(lpDeviceDescriptor, &devInfo->Descriptor, sizeof(devInfo->Descriptor));
	return TRUE;
}

BOOL UsbDevice::GetActiveConfigDescriptor(UserBuffer<LPVOID>& buffer, LPDWORD lpSize)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	LPCUSB_DEVICE devInfo = mUsbFuncs->lpGetDeviceInfo(mDevice);
	// Find configuration whose bConfigurationValue matches the active configuration
	UCHAR index;
	for (index = 0; index < mNumConfigurations; index++) {
		if (mConfigDescriptors[index].bConfigurationValue == devInfo->lpActiveConfig->Descriptor.bConfigurationValue) {
			break;
		}
	}
	if (index == mNumConfigurations) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::GetActiveConfigDescriptor() - failed to find cached descriptor with bConfigurationValue=%i\r\n"), 
			devInfo->lpActiveConfig->Descriptor.bConfigurationValue));
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	return CopyConfigDescriptor(index, buffer, lpSize);
}

BOOL UsbDevice::GetConfigDescriptor(DWORD dwConfigurationIndex, UserBuffer<LPVOID>& buffer, LPDWORD lpSize)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	if (dwConfigurationIndex < 0 ||
		dwConfigurationIndex >= mNumConfigurations) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	return CopyConfigDescriptor(dwConfigurationIndex, buffer, lpSize);
}

BOOL UsbDevice::GetActiveConfigValue(PUCHAR pConfigurationValue)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	LPCUSB_DEVICE devInfo = mUsbFuncs->lpGetDeviceInfo(mDevice);
	(*pConfigurationValue) = devInfo->lpActiveConfig->Descriptor.bConfigurationValue;
	return TRUE;
}

BOOL UsbDevice::SetActiveConfigValue(UCHAR pConfigurationValue)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	/* Until Windows CE provides an API for this (or we receive guidance from MS
	 * on a better way to do this), we need to send this control request by manually
	 * issuing a synchronous transfer. IssueVendorTransfer() can be used for this
	 * (even though SET_CONFIGURATION is not a vendor-specific command).
	 */
	USB_DEVICE_REQUEST setConfiguration;
	setConfiguration.bmRequestType = 0x00;
	setConfiguration.bRequest = 0x09;		// SET_CONFIGURATION
	setConfiguration.wValue = pConfigurationValue;
	setConfiguration.wIndex = 0;
	setConfiguration.wLength = 0;
	return (mUsbFuncs->lpIssueVendorTransfer(
		mDevice, NULL, NULL, USB_OUT_TRANSFER, &setConfiguration, NULL, NULL) != NULL);
}

BOOL UsbDevice::SetAltSetting(DWORD dwInterface, DWORD dwAlternateSetting)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	if (dwInterface > UCHAR_MAX || dwAlternateSetting > UCHAR_MAX) {
		
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	USB_TRANSFER transfer = mUsbFuncs->lpSetInterface(mDevice, NULL, NULL, 0, 
		static_cast<UCHAR>(dwInterface), static_cast<UCHAR>(dwAlternateSetting));
	if (!transfer) {
		return FALSE;
	}
	DWORD dwError = 0;
	if(mUsbFuncs->lpGetTransferStatus(transfer, NULL, &dwError)) {
		SetLastError(Transfer::TranslateError(dwError, 0, FALSE));
		return dwError == USB_NO_ERROR;
	} else {
		return FALSE;
	}
}

BOOL UsbDevice::IsEndpointHalted(DWORD dwInterface, UCHAR Endpoint, BOOL& halted)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	if (Endpoint == 0) {
		// This could use mUsbFuncs->lpIsDefaultPipeHalted,
		// but that API is deprecated and always returns false.
		halted = FALSE;
		return TRUE;
	}
	USB_PIPE epPipe = GetPipeForEndpoint(dwInterface, Endpoint);
	if (!epPipe) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	// First check the halt on the host side
	halted = FALSE;
	if(!mUsbFuncs->lpIsPipeHalted(epPipe, &halted)) {
		return FALSE;
	}
	return TRUE;
}

BOOL UsbDevice::ClearHaltHost(DWORD dwInterface, UCHAR Endpoint)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	if (Endpoint == 0) {
		if(!mUsbFuncs->lpResetDefaultPipe(mDevice)) {
			return false;
		}
	} else {
		USB_PIPE epPipe = GetPipeForEndpoint(dwInterface, Endpoint);
		if (!epPipe) {
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}
		// First clear the halt on the host side
		BOOL halted = FALSE;
		if(!mUsbFuncs->lpIsPipeHalted(epPipe, &halted)) {
			return FALSE;
		}
		// Only count failure to reset as an error if halted
		if(!mUsbFuncs->lpResetPipe(epPipe) && halted) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL UsbDevice::ClearHaltDevice(DWORD dwInterface, UCHAR Endpoint)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	USB_TRANSFER transfer = mUsbFuncs->lpClearFeature(
		mDevice, NULL, NULL, USB_SEND_TO_ENDPOINT, USB_FEATURE_ENDPOINT_STALL, Endpoint);
	if (!transfer) {
		return FALSE;
	}
	DWORD dwError = 0;
	if(mUsbFuncs->lpGetTransferStatus(transfer, NULL, &dwError)) {
		SetLastError(Transfer::TranslateError(dwError, 0, FALSE));
		return dwError == USB_NO_ERROR;
	} else {
		return FALSE;
	}
}

BOOL UsbDevice::GetTransferStatus(
	USB_TRANSFER hTransfer,
	LPDWORD lpdwBytesTransferred,
	LPDWORD lpdwError)
{
	ReadLocker lock(mCloseMutex);
	return GetTransferStatusNoLock(hTransfer, lpdwBytesTransferred, lpdwError);
}

BOOL UsbDevice::GetTransferStatusNoLock(
	USB_TRANSFER hTransfer,
	LPDWORD lpdwBytesTransferred,
	LPDWORD lpdwError)
{
	// This doesn't hold the lock as it's intended to only be called from a transfer completion callback, during
	// which the device can't change status.
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	return mUsbFuncs->lpGetTransferStatus(hTransfer, lpdwBytesTransferred, lpdwError);
}

BOOL UsbDevice::CancelTransfer(USB_TRANSFER hTransfer, DWORD dwFlags)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	return mUsbFuncs->lpAbortTransfer(hTransfer, dwFlags);
}

BOOL UsbDevice::CloseTransfer(USB_TRANSFER hTransfer)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	return mUsbFuncs->lpCloseTransfer(hTransfer);
}

extern "C" DWORD StaticTransferNotifyRoutine(LPVOID lpvNotifyParameter)
{
	Transfer* tc = static_cast<Transfer*>(lpvNotifyParameter);
	return tc->TransferComplete();
}

USB_TRANSFER UsbDevice::IssueVendorTransfer(
	Transfer* callback,
	DWORD dwFlags,
	LPCUSB_DEVICE_REQUEST lpControlHeader,
	LPVOID lpvBuffer)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return NULL;
	}

	return mUsbFuncs->lpIssueVendorTransfer(
		mDevice,
		callback ? &StaticTransferNotifyRoutine : NULL, callback,
		dwFlags, lpControlHeader, lpvBuffer, NULL);
}

USB_TRANSFER UsbDevice::IssueBulkTransfer(
	Transfer* callback,
	DWORD dwInterface,
	UCHAR Endpoint,
	DWORD dwFlags,
	DWORD dwDataBufferSize,
	LPVOID lpvBuffer)
{
	FUNCTION_MSG((TEXT("USBKWrapperDrv!UsbDevice::IssueBulkTransfer() - Entry\r\n")));
	USB_TRANSFER ret;
	
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::IssueBulkTransfer() - Error: Closed() returned True\r\n")));
		SetLastError(ERROR_INVALID_HANDLE);
		return NULL;
	}

	USB_PIPE epPipe = GetPipeForEndpoint(dwInterface, Endpoint);
	if (!epPipe) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::IssueBulkTransfer() - Error: Couln't get pipe for endpoint. Interface %08x, EndPoint %08x\r\n"), dwInterface, Endpoint));

		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	FUNCTION_MSG((TEXT("USBKWrapperDrv!UsbDevice::IssueBulkTransfer calling lpIssueBulkTransfer(ePipe=0x%08x, notifyroutine=0x%08x, context=0x%08x, flags=0x%x, buffersize=0x%x, buffer=0x%08x, NULL\r\n"),
		epPipe, callback ? &StaticTransferNotifyRoutine : NULL, callback, dwFlags, dwDataBufferSize, lpvBuffer, NULL));

#ifdef DEBUG
	UsbDeviceDumpOutput( "USBKWrapperDrv!UsbDevice::IssueBulkTransfer Buffer", (UCHAR *) lpvBuffer, min(dwDataBufferSize, 128) );
#endif

//	DebugBreak(); // ###1
	ret = mUsbFuncs->lpIssueBulkTransfer(
		epPipe, callback ? &StaticTransferNotifyRoutine : NULL, callback,
		dwFlags, dwDataBufferSize, lpvBuffer, NULL);

	FUNCTION_MSG((TEXT("USBKWrapperDrv!UsbDevice::IssueBulkTransfer lpIssueBulkTransfer returned 0x%x\r\n"), ret));
	return ret;
}

void UsbDevice::AdvertiseDevice(BOOL isAttached)
{
	// Allow an additional 9 bytes with prefix: 
	// Two '_' seperators, 6 digit characters, NULL terminator
	size_t length = wcslen(DEVCLASS_CEUSBKWRAPPER_NAME_PREFIX) + 9;
	wchar_t *buf = new wchar_t[sizeof(wchar_t)*length];
	_snwprintf(buf, length, L"%s_%.03i_%.03i", DEVCLASS_CEUSBKWRAPPER_NAME_PREFIX, mBus, mAddress);

	DISCOVERY_MSG((
		TEXT("USBKWrapperDrv: Advertising device %s, isAttached = %i\r\n"), buf, isAttached));
	if (!AdvertiseInterface(&ceusbkwrapper_guid, buf, isAttached)) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::AdvertiseDevice() - failed to advertise device: %i\r\n"), GetLastError()));
	}
	delete[] buf;
}

BOOL UsbDevice::FetchAllConfigDescriptors() {
	// We fetch configuration descriptors ourselves because our clients need access to
	// raw descriptor bytes for parsing. WinCE appears not to provide the complete configuration
	// descriptor contents in the USB_CONFIGURATION instances it provides us.
	LPCUSB_DEVICE devInfo = mUsbFuncs->lpGetDeviceInfo(mDevice);	
	DestroyAllConfigDescriptors();

	mConfigDescriptors = new (std::nothrow) USBDEVICE_CONFIG_DESCRIPTOR[devInfo->Descriptor.bNumConfigurations];
	if (!mConfigDescriptors) {
		ERROR_MSG((
			TEXT("USBKWrapperDrv!UsbDevice::FetchAllConfigDescriptors() - failed to allocate memory for descriptor set\r\n")));
		return FALSE;
	}

	// Use mNumConfigurations to track the number of descriptors we've allocated memory for
	// so far (so we can safely destroy a partial descriptor set).
	mNumConfigurations = 0;
	for (UCHAR i = 0; i < devInfo->Descriptor.bNumConfigurations; i++) {
		mConfigDescriptors[i].bConfigurationValue = devInfo->lpConfigs[i].Descriptor.bConfigurationValue;
		mConfigDescriptors[i].wTotalLength = devInfo->lpConfigs[i].Descriptor.wTotalLength;
		mConfigDescriptors[i].pDescriptor = new (std::nothrow) UCHAR[mConfigDescriptors[i].wTotalLength];
		if (!mConfigDescriptors[i].pDescriptor) {
			ERROR_MSG((
				TEXT("USBKWrapperDrv!UsbDevice::FetchAllConfigDescriptors() - failed to allocate memory for descriptor\r\n")));
			DestroyAllConfigDescriptors();
			return FALSE;
		}
		mNumConfigurations++;

		// Synchronously fetch the raw descriptor bytes
		if (!mUsbFuncs->lpGetDescriptor(mDevice, NULL, NULL, 0, USB_CONFIGURATION_DESCRIPTOR_TYPE, 
			i, 0, mConfigDescriptors[i].wTotalLength, mConfigDescriptors[i].pDescriptor)) {
			ERROR_MSG((
				TEXT("USBKWrapperDrv!UsbDevice::FetchAllConfigDescriptors() - failed to fetch descriptor\r\n")));
			DestroyAllConfigDescriptors();
			return FALSE;
		}
	}
	return TRUE;
}

void UsbDevice::DestroyAllConfigDescriptors() {
	for (UCHAR i = 0; i < mNumConfigurations; i++) {
		delete[] mConfigDescriptors[i].pDescriptor;
	}
	delete[] mConfigDescriptors;
	mConfigDescriptors = NULL;
	mNumConfigurations = 0;
}

BOOL UsbDevice::CopyConfigDescriptor(DWORD dwIndex, UserBuffer<LPVOID>& buffer, LPDWORD lpSize)
{
	// Fill the buffer with up to wTotalLength bytes of descriptor.
	// As we're dealing with raw descriptor bytes, endianness will be bus order.
	DWORD toCopy = (buffer.Size() < mConfigDescriptors[dwIndex].wTotalLength) ? 
		buffer.Size() : mConfigDescriptors[dwIndex].wTotalLength;
	memcpy(buffer.Ptr(), mConfigDescriptors[dwIndex].pDescriptor, toCopy);

	// Report back the copied size if provided
	if (lpSize)
		*lpSize = toCopy;
	return TRUE;
}

BOOL UsbDevice::AllocateInterfaceClaimers()
{
	// Callers should already hold mCloseMutex.
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}
	// Work out how many interfaces are needed for this configuration
	LPCUSB_DEVICE devInfo = mUsbFuncs->lpGetDeviceInfo(mDevice);
	LPCUSB_CONFIGURATION activeConfig = devInfo->lpActiveConfig;
	const DWORD newIfaceCount = activeConfig->dwNumInterfaces;
	InterfaceClaimers* newClaimers = NULL;
	if (newIfaceCount > 0) {
		// Allocate the new interface claim tracking objects
		// and initialise them.
		newClaimers = new (std::nothrow) InterfaceClaimers[newIfaceCount];
		for (DWORD i = 0; i < newIfaceCount; ++i) {
			const USB_INTERFACE& iface = activeConfig->lpInterfaces[i];
			if (!newClaimers[i].Init(iface)) {
				delete [] newClaimers;
				return FALSE;
			}
		}
	}
	if (mInterfaceClaimers) {
		delete [] mInterfaceClaimers;
		mInterfaceClaimers = NULL;
		mInterfaceClaimersCount = 0;
	}
	mInterfaceClaimers = newClaimers;
	mInterfaceClaimersCount = newIfaceCount;
	return TRUE;
}

BOOL UsbDevice::Reset()
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	// Only reset if we're controlling the entire device, and not just a single interface.
	if (mUsbInterface) {
		ERROR_MSG((TEXT("USBKWrapperDrv!UsbDevice::Reset(...) - can't reset non-exclusive single interface devices\r\n")));
		SetLastError(ERROR_NOT_SUPPORTED);
		return FALSE;
	}
	// Currently reset isn't supported for any configuration
	SetLastError(ERROR_NOT_SUPPORTED);
	return FALSE;
}

BOOL UsbDevice::Reenumerate()
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	// Only reset if we're controlling the entire device, and not just a single interface.
	if (mUsbInterface) {
		ERROR_MSG((TEXT("USBKWrapperDrv!UsbDevice::Reset(...) - can't reset non-exclusive single interface devices\r\n")));
		SetLastError(ERROR_NOT_SUPPORTED);
		return FALSE;
	}
	return mUsbFuncs->lpDisableDevice(mDevice, TRUE, 0);
}	

BOOL UsbDevice::CheckKernelDriverActiveForInterface(UCHAR ifnum)
{
	// Kernel driver is active for interface if:
	// (a) We're not controlling the whole device and we're not controlling that interface, or
	// (b) If the interface cannot be claimed (indicates we've already attached a driver).
	if (mUsbInterface && mUsbInterface->Descriptor.bInterfaceNumber != ifnum) {
		return TRUE;
	}
	for (DWORD i = 0; i < mInterfaceClaimersCount; i++) {
		if (mInterfaceClaimers[i].InterfaceValue() == ifnum &&
			!mInterfaceClaimers[i].IsClaimable()) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL UsbDevice::CheckKernelDriverActiveForDevice()
{
	// Kernel driver is active for device if:
	// (a) We're not controlling the whole device
	// (b) Any interface cannot be claimed (indicates we've already attached a driver).
	if (mUsbInterface) {
		return TRUE;
	}
	for (DWORD i = 0; i < mInterfaceClaimersCount; i++) {
		if (!mInterfaceClaimers[i].IsClaimable()) {
			return TRUE;
		}
	}
	return FALSE;
}

USB_PIPE UsbDevice::GetPipeForEndpoint(DWORD dwInterface, UCHAR Endpoint)
{
	for (DWORD i = 0; i < mInterfaceClaimersCount; i++) {
		if (mInterfaceClaimers[i].InterfaceValue() == dwInterface) {
			return mInterfaceClaimers[i].GetPipeForEndpoint(Endpoint);
		}
	}
	return NULL;
}

BOOL UsbDevice::OpenPipes(InterfaceClaimers& Iface)
{
	if (Closed()) {
		return FALSE;
	}

	LPCUSB_DEVICE info = mUsbFuncs->lpGetDeviceInfo(mDevice);
	// Find the interface
	LPCUSB_CONFIGURATION config = info->lpActiveConfig;
	for (DWORD i = 0; i < config->dwNumInterfaces; ++i) {
		LPCUSB_INTERFACE ifaceDesc = &(config->lpInterfaces[i]);
		if (ifaceDesc->Descriptor.bInterfaceNumber != Iface.InterfaceValue())
			continue;
		// Found the interface, iterate through endpoints
		for (DWORD epIdx = 0; epIdx < ifaceDesc->Descriptor.bNumEndpoints; ++epIdx) {
			LPCUSB_ENDPOINT epDesc = &(ifaceDesc->lpEndpoints[epIdx]);
			USB_PIPE p = Iface.GetPipeForEndpoint(epDesc->Descriptor.bEndpointAddress);
			if (p != NULL) {
				// Already been opened, so skip this endpoint
				continue;	
			}
			p = mUsbFuncs->lpOpenPipe(mDevice, &epDesc->Descriptor);
			if (p == NULL) {
				return FALSE;
			}
			Iface.SetPipeForEndpoint(epDesc->Descriptor.bEndpointAddress, p);
		}
		// Setup all endpoints correctly
		return TRUE;
	}
	// Interface not found
	return FALSE;
}

void UsbDevice::ClosePipes(InterfaceClaimers& Iface)
{
	if (Closed()) {
		return;
	}
	for (DWORD i = 0; i < Iface.GetPipeCount(); ++i) {
		USB_PIPE p = Iface.GetPipeForIndex(i);
		if (p != NULL) {
			mUsbFuncs->lpClosePipe(p);
			Iface.SetPipeForIndex(i, NULL);
		}
	}
}

BOOL UsbDevice::IsKernelDriverActiveForInterface(DWORD dwInterface, PBOOL active)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	// Convert DWORD to UCHAR and catch any out-of-range values
	if (dwInterface > UCHAR_MAX) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	UCHAR ifnum = static_cast<UCHAR>(dwInterface);

	LPCUSB_DEVICE devInfo = mUsbFuncs->lpGetDeviceInfo(mDevice);

	if (!mUsbFuncs->lpFindInterface(devInfo, ifnum, 0)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	(*active) = CheckKernelDriverActiveForInterface(ifnum);
	return TRUE;
}

void UsbDevice::SetInterfaceClaimable(UCHAR ifnum, BOOL claimable) 
{
	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		if (mInterfaceClaimers[i].InterfaceValue() == ifnum) {
			mInterfaceClaimers[i].SetClaimable(claimable);
		}
	}
}

void UsbDevice::SetAllInterfacesClaimable(BOOL claimable) 
{
	for (DWORD i = 0; i < mInterfaceClaimersCount; ++i) {
		mInterfaceClaimers[i].SetClaimable(claimable);
	}
}

BOOL UsbDevice::DoAttachKernelDriver(WriteLocker& lock, LPCUSB_INTERFACE devIf)
{
	// Set interface(s) as unclaimable first, so nobody tries to use them
	// whilst we're attaching the driver. We set them back to claimable
	// if the operation fails.
	if (devIf) {
		SetInterfaceClaimable(devIf->Descriptor.bInterfaceNumber, FALSE);
	} else {
		SetAllInterfacesClaimable(FALSE);
	}

	// Release mutex whilst attaching kernel driver (operation may
	// be long blocking). Close() will wait until attach operations
	// have completed.
	++mAttachKernelDriverCount;
	lock.unlock();

	BOOL result = mUsbFuncs->lpLoadGenericInterfaceDriver(mDevice, devIf);

	lock.relock();
	--mAttachKernelDriverCount;
	SetEvent(mAttachKernelDriverEvent);
	
	if (!result) {
		// Attach failed, set interface(s) as claimable again.
		if (devIf) {
			SetInterfaceClaimable(devIf->Descriptor.bInterfaceNumber, TRUE);
		} else {
			SetAllInterfacesClaimable(TRUE);
		}
	}

	return result;
}

BOOL UsbDevice::AttachKernelDriverForInterface(DWORD dwInterface)
{
	WriteLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	// Convert DWORD to UCHAR and catch any out-of-range values
	if (dwInterface > UCHAR_MAX) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	UCHAR ifnum = static_cast<UCHAR>(dwInterface);

	LPCUSB_DEVICE devInfo = mUsbFuncs->lpGetDeviceInfo(mDevice);
	LPCUSB_INTERFACE devIf= mUsbFuncs->lpFindInterface(devInfo, ifnum, 0);
	if (!devIf) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	BOOL active = CheckKernelDriverActiveForInterface(ifnum);
	if (active) {
		SetLastError(ERROR_BUSY);
		return FALSE;
	}

	return DoAttachKernelDriver(lock, devIf);
}

BOOL UsbDevice::DetachKernelDriverForInterface(DWORD dwInterface)
{
	ReadLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	// Until we can find a way to do this in Windows CE, this operation is not supported.
	SetLastError(ERROR_NOT_SUPPORTED);
	return FALSE;
}

BOOL UsbDevice::AttachKernelDriverForDevice()
{
	WriteLocker lock(mCloseMutex);
	if (Closed()) {
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	BOOL active = CheckKernelDriverActiveForDevice();
	if (active) {
		SetLastError(ERROR_BUSY);
		return FALSE;
	} 

	return DoAttachKernelDriver(lock, NULL);
}
