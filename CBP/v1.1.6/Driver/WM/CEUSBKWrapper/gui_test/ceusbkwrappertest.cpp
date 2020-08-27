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

// ceusbkwrappertest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ceusbkwrapper.h"
#include "DlgPrint.h"

#include <msgqueue.h>
#include <pnp.h>

#define MAX_LINE_LENGTH 80
#define MAX_DEVICE_COUNT 10
#define ASYNC_TIMEOUT 1000
// Maximum config descriptor buffer size
#define MAX_CONFIG_BUFFER 2048
// Number of bytes per row when printing hex buffers
#define BYTES_PER_ROW 8

static HANDLE gDeviceHandle = INVALID_HANDLE_VALUE;
static UKW_DEVICE gDeviceList[MAX_DEVICE_COUNT];
static DWORD gDeviceListSize = 0;

static void printHexDump(unsigned char buf[], DWORD length) {
	DWORD offset = 0;
	do {
		// Print the row header
		DlgPrintf("   %04x: ", offset);
		for (DWORD i = 0; i < BYTES_PER_ROW && i < length; ++i)
			DlgPrintf("%02x ", buf[i]);
		DlgPrintf("\n");
		buf += BYTES_PER_ROW;
		offset += BYTES_PER_ROW;
		if (length >= BYTES_PER_ROW)
			length -= BYTES_PER_ROW;
		else
			length = 0;
	} while (length > 0);
}

void printMenu()
{
	DlgPrintf("\n");
	if (gDeviceHandle == INVALID_HANDLE_VALUE) {
		DlgPrintf("o ) open device\n");
	} else {
		if (gDeviceListSize) {
			DlgPrintf("p ) print USB device list\n");
			DlgPrintf("r ) release USB device list\n");
			DlgPrintf("m ) send a control request\n");
			DlgPrintf("a ) request AAP mode from device\n");
			DlgPrintf("br) read bulk transfer from AAP device\n");
			DlgPrintf("bw) write bulk transfer to AAP device\n");
			DlgPrintf("c ) read a configuration descriptor\n");
			DlgPrintf("o ) get active configuration value\n");
			DlgPrintf("s ) set active configuration value\n");
			DlgPrintf("ic ) claim an interface\n");
			DlgPrintf("ir ) release an interface\n");
			DlgPrintf("ia ) set an alternate setting on an interface\n");
			DlgPrintf("r ) reset device\n");
			DlgPrintf("kt ) test if kernel driver is active on interface\n");
			DlgPrintf("ka ) attach a kernel driver to an interface\n");
			DlgPrintf("kd ) detach a kernel driver from an interface\n");
			DlgPrintf("hq ) test if an endpoint is halted");
			DlgPrintf("hc ) clear stall/halt (host) on an endpoint\n");
			DlgPrintf("hs ) clear stall/halt (device) on an endpoint\n");
		} else {
			DlgPrintf("g ) get USB device list\n");
		}
		DlgPrintf("c ) close device\n");
	}
	DlgPrintf("q ) quit\n");
	DlgPrintf("\n");
}

static void openDriver()
{
	gDeviceHandle = UkwOpenDriver();
	if (gDeviceHandle != INVALID_HANDLE_VALUE)
		DlgPrintf("UkwOpenDriver() returned 0x%08x\n", gDeviceHandle);
	else
		DlgPrintf("UkwOpenDriver() returned INVALID_HANDLE_VALUE, GetLastError() = %d\n",
			GetLastError());
}

static void closeDriver()
{
	UkwCloseDriver(gDeviceHandle);
	gDeviceHandle = INVALID_HANDLE_VALUE;
	DlgPrintf("UkwCloseDriver() called to close handle");
}

static void getDeviceList()
{
	gDeviceListSize = 0;
	if (UkwGetDeviceList(gDeviceHandle,
			gDeviceList, sizeof(gDeviceList)/sizeof(gDeviceList[0]),
			&gDeviceListSize))
		DlgPrintf("UkwGetDeviceList() succeeded with a list size of %d\n",
			gDeviceListSize);
	else
		DlgPrintf("UkwGetDeviceList() failed with error %d\n", GetLastError());
}

static void releaseDeviceList()
{
	UkwReleaseDeviceList(gDeviceHandle, gDeviceList, gDeviceListSize);
	DlgPrintf("UkwReleaseDeviceList() called successfully\n");
	gDeviceListSize = 0;
}

static void printDeviceList()
{
	DlgPrintf("\n%d available devices:\n", gDeviceListSize);
	for(DWORD i = 0; i < gDeviceListSize; ++i) {
		DlgPrintf("Device % 3d: ", i);
		UKW_DEVICE dev = gDeviceList[i];
		unsigned char bus, devAddr;
		unsigned long sessionId;
		if (!UkwGetDeviceAddress(dev, &bus, &devAddr, &sessionId)) {
			DlgPrintf("Failed to retrieve address\n");
			continue;
		}
		DlgPrintf("bus %d, device address %d, session id %d\n", bus, devAddr, sessionId);
		DlgPrintf("\t");
		UKW_DEVICE_DESCRIPTOR desc;
		if (!UkwGetDeviceDescriptor(dev, &desc)) {
			DlgPrintf("Failed to retrieve device descriptor\n");
			continue;
		}
		DlgPrintf("vid: 0x%04x pid: 0x%04x\n", desc.idVendor, desc.idProduct);
	}
	DlgPrintf("\n");
}

static char* parseNumber(char* line, DWORD& device)
{
	// Skip whitespace
	while((*line) != '\0' &&
		((*line) == ' ' || (*line) == '\t' ||
		 (*line) == '\r' || (*line) == '\n'))
		++line;
	if ((*line) < '0' || (*line) > '9') {
		return NULL;
	}
	device = (*line) - '0';
	++line;
	while((*line) >= '0' && (*line) <= '9') {
		device = device * 10 + ((*line) - '0');
		++line;
	}
	return line;
}

static BOOL waitForOverlapped(OVERLAPPED& overlapped)
{
	DWORD waitState = WaitForSingleObject(overlapped.hEvent, ASYNC_TIMEOUT);
	switch(waitState) {
		case WAIT_ABANDONED:
			DlgPrintf("Wait abandonded\n");
			return FALSE;
		case WAIT_OBJECT_0:
			return TRUE;
		case WAIT_TIMEOUT:
			DlgPrintf("Wait timed out\n");
			return FALSE;
		case WAIT_FAILED:
			DlgPrintf("Wait failed with %d\n", GetLastError());
			return FALSE;
		default:
			DlgPrintf("Unknown wait state returned: 0x%08x\n", waitState);
			return FALSE;
	}
	return FALSE;
}

static void sendControlRequest(char line[])
{
	// Parse the device index
	DWORD devIdx = 0;
	line = parseNumber(line, devIdx);
	if (!line) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}
	
	// Parse the option async parameter
	BOOL async;
	DWORD asyncval;
	line = parseNumber(line, asyncval);
	if (line && asyncval != 0) {
		DlgPrintf("Performing control request asynchronously.\n");
		async = TRUE;
	} else {
		DlgPrintf("Performing control request synchronously.\n");
		async = FALSE;
	}
	
	// Input validated so form the request
	UKW_DEVICE device = gDeviceList[devIdx];
	UKW_CONTROL_HEADER header;
	OVERLAPPED overlapped;
	if (async) {
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (overlapped.hEvent == NULL) {
			DlgPrintf("Failed to create event for asynchronous request.\n");
			return;
		}
	}
	DWORD flags = UKW_TF_OUT_TRANSFER;
	header.bmRequestType = 0x40;
	header.bRequest = 0xF0;
	header.wValue = 0x0001;
	header.wIndex = 0x0000;
	header.wLength = 0x0000;
	// Now perform the request
	DWORD transferred = -1;
	BOOL status = UkwIssueControlTransfer(device, flags, &header,
		NULL, 0,
		&transferred,
		async ? &overlapped : NULL);
	if (!status) {
		DlgPrintf("Control transfer failed with %d.\n", GetLastError());
		return;
	}
	if (async) {
		if (!waitForOverlapped(overlapped)) {
			// Attempt to cancel it
			if (!UkwCancelTransfer(device, &overlapped, 0)) {
				DlgPrintf("Attempt to cancel timed out transfer failed with %d\n", GetLastError());
				goto out;
			} 
			DlgPrintf("Cancelled transfer due to timeout\n");
			if (!waitForOverlapped(overlapped)) {
				DlgPrintf("Timeout out waiting for cancel to complete\n");
				goto out;
			}
			DlgPrintf("Transfer cancel completed with: Internal: %d InternalHigh: %d Offset: %d OffsetHigh %d\n",
				overlapped.Internal, overlapped.InternalHigh,
				overlapped.Offset, overlapped.OffsetHigh);
			goto out;
		}
		// Check for the overlapped members being as expected
		if (overlapped.Internal != 0 || overlapped.InternalHigh != 0 ||
			overlapped.Offset != 0 || overlapped.OffsetHigh != 0) {
			DlgPrintf("Overlapped not as expected. Internal: %d InternalHigh: %d Offset: %d OffsetHigh %d\n",
				overlapped.Internal, overlapped.InternalHigh,
				overlapped.Offset, overlapped.OffsetHigh);
			goto out;
		}
	}
	if (transferred != 0) {
		DlgPrintf("Transferred data length not updated, was %d\n", transferred);
	}
out:
	if (async)
		CloseHandle(overlapped.hEvent);
}

static BOOL waitForTransferCompletion(
	BOOL status, UKW_DEVICE device,
	OVERLAPPED& overlapped, DWORD& transferred,
	const DWORD expectedTransferred)
{
	if (!status) {
		DlgPrintf("Transfer failed with %d.\n", GetLastError());
		return FALSE;
	}
	if (!waitForOverlapped(overlapped)) {
		// Attempt to cancel it
		if (!UkwCancelTransfer(device, &overlapped, 0)) {
			DlgPrintf("Attempt to cancel timed out transfer failed with %d\n", GetLastError());
			return FALSE;
		}
		DlgPrintf("Cancelled transfer due to timeout\n");
		if (!waitForOverlapped(overlapped)) {
			DlgPrintf("Timeout out waiting for cancel to complete\n");
			
		}
		DlgPrintf("Transfer cancel completed with: Internal: %d InternalHigh: %d Offset: %d OffsetHigh %d\n",
			overlapped.Internal, overlapped.InternalHigh,
			overlapped.Offset, overlapped.OffsetHigh);
		return FALSE;
	}
	// Check for the overlapped members being as expected
	if (overlapped.Internal != 0 || overlapped.InternalHigh != expectedTransferred ||
		overlapped.Offset != 0 || overlapped.OffsetHigh != 0) {
		DlgPrintf("Overlapped structure not as expected. Internal: %d InternalHigh: %d Offset: %d OffsetHigh %d",
			overlapped.Internal, overlapped.InternalHigh,
			overlapped.Offset, overlapped.OffsetHigh);
		return FALSE;
	}
	if (transferred != expectedTransferred) {
		DlgPrintf("Failed to transfer expected bytes, expecting %d, got %d\n",
			expectedTransferred, transferred);
		return FALSE;	
	}
	return TRUE;
}

// Attempts to request a device switches into Android Accessory Protocol mode.
// See: http://developer.android.com/guide/topics/usb/adk.html#accessory-protocol
static void sendAapRequest(char line[])
{
	// Parse the device index
	DWORD devIdx = 0;
	line = parseNumber(line, devIdx);
	if (!line) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}
	
	// Input validated so form the request
	UKW_DEVICE device = gDeviceList[devIdx];
	UKW_CONTROL_HEADER header;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (overlapped.hEvent == NULL) {
		DlgPrintf("Failed to create event for asynchronous request.\n");
		return;
	}
	
	// send control request "Get Protocol" (51) -- gets version of AAP supported by device (if any)
	unsigned char ioBuffer[2];
	DWORD flags = UKW_TF_IN_TRANSFER;
	header.bmRequestType = 0xC0;
	header.bRequest = 0x33;
	header.wValue = 0x0000;
	header.wIndex = 0x0000;
	header.wLength = 0x0002;
	// Now perform the request
	DWORD transferred = -1;
	BOOL status = UkwIssueControlTransfer(device, flags, &header,
		ioBuffer, 2,
		&transferred,
		&overlapped);
	if (!waitForTransferCompletion(status, device, overlapped, transferred, 2))
		goto out;
	DWORD protocolVersion = (ioBuffer[1] << 8) | ioBuffer[0];
	if (protocolVersion != 1) {
		DlgPrintf("Device does not support the required Android Accessory Protocol version (supports %d)\n",
			protocolVersion);
		goto out;
	}
	DlgPrintf("Device supports protocol version: %d\n", protocolVersion);
	// Send the identity strings
	char* identityStrings[] = {
		"RealVNC", // Manufacturer
		"AAPTest", // Model name
		"RealVNC AAP Bearer", // Description
		"0.1", // Version
		"http://www.realvnc.com", // URI
		"0123456789", // Device serial 'number'
	};
	flags = UKW_TF_OUT_TRANSFER;
	header.bmRequestType = 0x40;
	header.bRequest = 0x34;
	header.wValue = 0x0000;
	for(DWORD i = 0; i < (sizeof(identityStrings) / sizeof(char*)) ; ++i) {
		DWORD length = strlen(identityStrings[i]);
		header.wIndex = static_cast<UINT16>(i);
		header.wLength =  static_cast<UINT16>(length);
		status = UkwIssueControlTransfer(device, flags, &header,
			identityStrings[i], length,
			&transferred,
			&overlapped);
		if (!waitForTransferCompletion(status, device, overlapped, transferred, length))
			goto out;
	}
	
	// Finally request that the device goes into AAP mode
	DlgPrintf("Requesting device switches to AAP mode\n");
	flags = UKW_TF_OUT_TRANSFER;
	header.bmRequestType = 0x40;
	header.bRequest = 0x35;
	header.wValue = 0x0000;
	header.wIndex = 0x0000;
	header.wLength = 0x0000;
	status = UkwIssueControlTransfer(device, flags, &header,
		NULL, 0,
		&transferred,
		&overlapped);
	if (!waitForTransferCompletion(status, device, overlapped, transferred, 0))
		goto out;
out:
	CloseHandle(overlapped.hEvent);
	return;
}

static void requestConfigurationDescriptor(char line[])
{
	// Parse the device index
	DWORD devIdx = 0;
	line = parseNumber(line, devIdx);
	if (!line) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}
	// Parse the optional configuration number
	DWORD configuration = UKW_ACTIVE_CONFIGURATION;
	line = parseNumber(line, configuration);

	// Parse the optional buffer size number
	DWORD bufferLength = MAX_CONFIG_BUFFER;
	if (line) {
		// See if there is another parameter for buffer length
		line = parseNumber(line, bufferLength);
		if (bufferLength > MAX_CONFIG_BUFFER) {
			DlgPrintf("Provided buffer length is too large, the maximum is %d\n", MAX_CONFIG_BUFFER);
			return;
		}
	}

	// Print the command which is about to be issued
	if (configuration == UKW_ACTIVE_CONFIGURATION) {
		DlgPrintf("No configuration specified, reading active configuration from device %d, length %d\n",
			devIdx, bufferLength);
	} else {
		DlgPrintf("Requesting descriptor for configuration %d from device %d, length %d\n",
			configuration, devIdx, bufferLength);
	}

	// All parameters decoded, issue command
	UKW_DEVICE device = gDeviceList[devIdx];
	unsigned char descBuf[MAX_CONFIG_BUFFER];
	DWORD descLength = 0;
	BOOL status = UkwGetConfigDescriptor(device, configuration, descBuf, bufferLength, &descLength);
	if (status) {
		DlgPrintf("Retrieved descriptor of length %d\n", descLength);
		printHexDump(descBuf, descLength);
	} else {
		DlgPrintf("Failed to retrieve configuration descriptor with error %d\n", GetLastError());
	}
}

static void getActiveConfigValue(char line[])
{
	// Parse the device index
	DWORD devIdx = 0;
	line = parseNumber(line, devIdx);
	if (!line) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}
	
	// Input validated so fetch the configuration value
	UKW_DEVICE device = gDeviceList[devIdx];

	UCHAR config = 0;
	BOOL result = UkwGetConfig(device, &config);
  
	if (!result) {
		DlgPrintf("Failed to get USB configuration value: %d\n", GetLastError());
	} else {
		DlgPrintf("bConfigurationValue = %.02x\n", config);
	}
}

static void setActiveConfigValue(char line[])
{
	// Parse the device index
	DWORD devIdx = 0;
	line = parseNumber(line, devIdx);
	if (!line) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}

	// Parse the configuration value
	DWORD cv = -1;
	line = parseNumber(line, cv);
	if (!line) {
		DlgPrintf("Please provide a decimal value for the configuration value\n");
		return;
	}
	if (cv < 0 || cv > 255) {
		DlgPrintf("Configuration value '%d' out of range\n", cv);
		return;
	}

	UKW_DEVICE device = gDeviceList[devIdx];
	BOOL result = UkwSetConfig(device, static_cast<UCHAR>(cv));
	if (!result) {
		DlgPrintf("Failed to set configuration value '%d' on device '%d': %d\n", cv, devIdx, GetLastError());
	} else {
		DlgPrintf("bConfigurationValue = %.02x\n", cv);
	}
}

static void performInterfaceOperation(char line[])
{
	switch(line[0])
	{
	case 'c':
	case 'r':
	{
		// Claiming and releasing interfaces is almost the same, so
		// handle it with common code
		BOOL claiming = line[0] == 'c';
		const char* opStr = claiming ? "claiming" : "releasing";
		++line;
		DWORD devIdx = 0;
		line = parseNumber(line, devIdx);
		if (!line) {
			DlgPrintf("Please provide a decimal device number following the command\n");
			return;
		}
		if (devIdx >= gDeviceListSize || devIdx < 0) {
			DlgPrintf("Invalid device index '%d' provided\n", devIdx);
			return;
		}
		// Parse the interface number
		DWORD interfaceNumber = 0;
		line = parseNumber(line, interfaceNumber);
		if (!line) {
			DlgPrintf("Please provide a decimal interface number following the device number");
			return;
		}
		
		// All parameters decoded, perform the operation
		UKW_DEVICE device = gDeviceList[devIdx];
		BOOL status = FALSE;
		if (claiming)
			status = UkwClaimInterface(device, interfaceNumber);
		else
			status = UkwReleaseInterface(device, interfaceNumber);

		if (status) {
			DlgPrintf("Success when %s interface %d\n", opStr, interfaceNumber);
		} else {
			DlgPrintf("Failure when attempting %s of interface %d: %d\n", opStr, interfaceNumber, GetLastError());
		}
		break;
	}
	case 'a':
	{
		++line;
		DWORD devIdx = 0;
		line = parseNumber(line, devIdx);
		if (!line) {
			DlgPrintf("Please provide a decimal device number following the command\n");
			return;
		}
		if (devIdx >= gDeviceListSize || devIdx < 0) {
			DlgPrintf("Invalid device index '%d' provided\n", devIdx);
			return;
		}
		// Parse the interface number
		DWORD interfaceNumber = 0;
		line = parseNumber(line, interfaceNumber);
		if (!line) {
			DlgPrintf("Please provide a decimal interface number following the device number");
			return;
		}
		// Parse the alternate setting number
		DWORD altSetting = 0;
		line = parseNumber(line, altSetting);
		if (!line) {
			DlgPrintf("Please provide a decimal alternate setting number following the interface number");
			return;
		}

		// All parameters decoded, perform the operation
		UKW_DEVICE device = gDeviceList[devIdx];
		if (UkwSetInterfaceAlternateSetting(device, interfaceNumber, altSetting)) {
			DlgPrintf("Successfully set interface %d to alternate setting %d\n",
				interfaceNumber, altSetting);
		} else {
			DlgPrintf("Failed to set interface %d to alternate setting %d: %d\n",
				interfaceNumber, altSetting, GetLastError());
		}
		break;
	}
	default:
		DlgPrintf("Unknown interface operation requested, not doing anything\n");
		break;
	}
}

static void resetDevice(char line[])
{
	// Parse the device index
	DWORD devIdx = 0;
	line = parseNumber(line, devIdx);
	if (!line) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}

	UKW_DEVICE device = gDeviceList[devIdx];
	BOOL result = UkwResetDevice(device);
	if (!result) {
		DlgPrintf("Failed to reset device '%d': %d\n", devIdx, GetLastError());
	} else {
		DlgPrintf("Device '%d' reset\n", devIdx);
	}
}	

static void performKernelAttachOperation(char line[])
{
	char* linePtr = line + 1;

	// Parse the device index
	DWORD devIdx = 0;
	linePtr = parseNumber(linePtr, devIdx);
	if (!linePtr) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}	

	// Parse the interface number
	DWORD ifnum = 0;
	linePtr = parseNumber(linePtr, ifnum);
	if (!linePtr) {
		DlgPrintf("Please provide a decimal interface number\n");
		return;
	}

	UKW_DEVICE device = gDeviceList[devIdx];
	BOOL result = FALSE;
	switch (line[0]) {
		case 't':
			{
				BOOL active = FALSE;
				result = UkwKernelDriverActive(device, ifnum, &active);
				if (result) {
					DlgPrintf("Kernel driver active on interface %d = %d\n", ifnum, active);
				}
				break;
			}
		case 'a':
			{
				result = UkwAttachKernelDriver(device, ifnum);
				break;
			}
		case 'd':
			{
				result = UkwDetachKernelDriver(device, ifnum);
				break;
			}
		default:
			{
				DlgPrintf("Unknown operation '%c'\n", line[0]);
				return;
			}
	}

	if (result) {
		DlgPrintf("Kernel attach operation on interface %d successful.\n", ifnum);
	} else {
		DlgPrintf("Kernel attach operation on interface %d failed: %d\n", ifnum, GetLastError());
	}
}

static BOOL findAAPEndpoints(UKW_DEVICE& device, UCHAR& epin, UCHAR& epout)
{
	// Device must be in AAP mode:
	// vendorID = 0x18D1, productID = 0x2D00 or 0x2D01
	UKW_DEVICE_DESCRIPTOR desc;
	if (!UkwGetDeviceDescriptor(device, &desc)) {
		DlgPrintf("Failed to retrieve device descriptor: error %d\n", GetLastError());
		return FALSE;
	}

	if (desc.idVendor != 0x18D1 || (desc.idProduct != 0x2D00 && desc.idProduct != 0x2D01)) {
		DlgPrintf("Device is not in AAP mode (idVendor = 0x%.2x, idProduct = 0x%.2x)\n", desc.idVendor, desc.idProduct);
		return FALSE;
	}

	// Really, we should scan the interface descriptor for this info.
	// Fake up (based on LG Optimus 2X P990 (CyanogenMod7 OS)) for now.
	epin = 8;
	epout = 6;
	return TRUE;
}

static void startAAPBulkTransfer(char line[])
{
	char* linePtr = line + 1;

	// Parse the device index
	DWORD devIdx = 0;
	linePtr = parseNumber(linePtr, devIdx);
	if (!linePtr) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}		
	
	UKW_DEVICE device = gDeviceList[devIdx];

	// Find endpoints
	UCHAR epin, epout;
	if (!findAAPEndpoints(device, epin, epout))
		return;

	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (overlapped.hEvent == NULL) {
		DlgPrintf("Failed to create event for asynchronous request.\n");
		return;
	}

	UCHAR buf[16383];
	DWORD bytesTransferred;

	switch (line[0]) {
		case 'r':
			{
				// Read a transfer
				if (!UkwIssueBulkTransfer(device, UKW_TF_IN_TRANSFER, epin, buf, 16383, &bytesTransferred, &overlapped)) {
					DlgPrintf("Failed to read bulk transfer from endpoint %d on device %d: error %d", epin, devIdx, GetLastError());
				} else if (waitForOverlapped(overlapped)) {
					DlgPrintf("Read %d bytes: [ ", bytesTransferred);
					for (DWORD i = 0; i < bytesTransferred; i++) {
						DlgPrintf("%.02x ", buf[i]);
					}
					DlgPrintf("]\n");
				} else {
					// Attempt to cancel it
					if (!UkwCancelTransfer(device, &overlapped, 0)) {
						DlgPrintf("Attempt to cancel timed out transfer failed with %d\n", GetLastError());
						return;
					}
					DlgPrintf("Cancelled transfer due to timeout\n");
					if (!waitForOverlapped(overlapped)) {
						DlgPrintf("Timeout out waiting for cancel to complete\n");
					}
					DlgPrintf("Transfer cancel completed with: Internal: %d InternalHigh: %d Offset: %d OffsetHigh %d\n",
					overlapped.Internal, overlapped.InternalHigh,
					overlapped.Offset, overlapped.OffsetHigh);
				}
				break;
			}
		case 'w':
			{
				// Write a AAP handshake bulk transfer
				UCHAR Handshake[10];
				Handshake[0] = 0xff;
				Handshake[1] = 0xff;
				Handshake[2] = 0x00;
				Handshake[3] = 0x00;
				Handshake[4] = 0x0a;
				Handshake[5] = 0x0a;
				Handshake[6] = 'H';
				Handshake[7] = 'O';
				Handshake[8] = 'S';
				Handshake[9] = 'T';

				memcpy(buf, Handshake, 10);
				if (!UkwIssueBulkTransfer(device, UKW_TF_IN_TRANSFER, epout, buf, 10, &bytesTransferred, &overlapped)) {
					DlgPrintf("Failed to write bulk transfer from endpoint %d on device %d: error %d", epin, devIdx, GetLastError());
				} else if (waitForOverlapped(overlapped)) {
					DlgPrintf("Wrote %d bytes\n", bytesTransferred);
				} else {
					// Attempt to cancel it
					if (!UkwCancelTransfer(device, &overlapped, 0)) {
						DlgPrintf("Attempt to cancel timed out transfer failed with %d\n", GetLastError());
						return;
					}
					DlgPrintf("Cancelled transfer due to timeout\n");
					if (!waitForOverlapped(overlapped)) {
						DlgPrintf("Timeout out waiting for cancel to complete\n");
					}
					DlgPrintf("Transfer cancel completed with: Internal: %d InternalHigh: %d Offset: %d OffsetHigh %d\n",
					overlapped.Internal, overlapped.InternalHigh,
					overlapped.Offset, overlapped.OffsetHigh);
				}
				break;
			}
		default: 
			{
				DlgPrintf("Don't know bulk transfer operation '%c', doing nothing\n", line[0]);
			}
	}
	CloseHandle(overlapped.hEvent);
}

static void performHaltOperation(char line[])
{
	char* linePtr = line + 1;

	// Parse the device index
	DWORD devIdx = 0;
	linePtr = parseNumber(linePtr, devIdx);
	if (!linePtr) {
		DlgPrintf("Please provide a decimal device number following the command\n");
		return;
	}
	DWORD endpoint = 0;
	linePtr = parseNumber(linePtr, endpoint);
	if (!linePtr) {
		DlgPrintf("Please provide a decimal endpoint number following the command\n");
		return;
	}
	if (devIdx >= gDeviceListSize || devIdx < 0) {
		DlgPrintf("Invalid device index '%d' provided\n", devIdx);
		return;
	}
	if (endpoint >= UCHAR_MAX || endpoint < 0) {
		DlgPrintf("Invalid endpoint '%d' provided\n", endpoint);
		return;
	}
	UKW_DEVICE device = gDeviceList[devIdx];
	UCHAR ep = static_cast<UCHAR>(endpoint);
	switch (line[0]) {
	case 'q':
		{
		BOOL halted = FALSE;
		BOOL result = UkwIsPipeHalted(device, ep, &halted);
		if (!result) {
			DlgPrintf("Failed to retrieve pipe halted status on endpoint %d on device %d: %d\n",
				ep, devIdx, GetLastError());
		} else {
			DlgPrintf("Halted status on endpoint %d on device %d is %d\n", ep, devIdx, halted);
		}
		break;
		}
	case 'c':
		{
		BOOL result = UkwClearHaltHost(device, ep);
		if (!result) {
			DlgPrintf("Failed to clear halt/stall (host) on endpoint %d on device %d: %d\n", ep, devIdx, GetLastError());
		} else {
			DlgPrintf("Cleared halt/stall (host) on endpoint %d successfully\n", ep);
		}
		break;
		}
	case 's':
		{
		BOOL result = UkwClearHaltDevice(device, ep);
		if (!result) {
			DlgPrintf("Failed to clear halt/stall (device) on endpoint %d on device %d: %d\n", ep, devIdx, GetLastError());
		} else {
			DlgPrintf("Cleared halt/stall (device) on endpoint %d successfully\n", ep);
		}
		break;
		}
	default:
		DlgPrintf("Unknown halt operation provided\n");
		break;
	}
}

BOOL handleCommand(char line[])
{
	BOOL ret = TRUE;
	if (strcmp(line, "q") == 0)
		ret = FALSE;
	else if (strcmp(line, "o") == 0 &&
		gDeviceHandle == INVALID_HANDLE_VALUE)
		openDriver();
	else if (strcmp(line, "c") == 0 &&
		gDeviceHandle != INVALID_HANDLE_VALUE)
		closeDriver();
	else if (strcmp(line, "g") == 0 &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize == 0)
		getDeviceList();
	else if (strcmp(line, "p") == 0 &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		printDeviceList();
	else if (strcmp(line, "r") == 0 &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		releaseDeviceList();
	else if (line[0] == 'm' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		sendControlRequest(line + 1);
	else if (line[0] == 'a' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		sendAapRequest(line + 1);
	else if (line[0] == 'c' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		requestConfigurationDescriptor(line + 1);
	else if (line[0] == 'o' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		getActiveConfigValue(line + 1);
	else if (line[0] == 's' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		setActiveConfigValue(line + 1);
	else if (line[0] == 'i' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		performInterfaceOperation(line + 1);
	else if (line[0] == 'r' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		resetDevice(line + 1);
	else if (line[0] == 'k' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		performKernelAttachOperation(line + 1);
	else if (line[0] == 'b' && 
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		startAAPBulkTransfer(line + 1);
	else if (line[0] == 'h' &&
		gDeviceHandle != INVALID_HANDLE_VALUE &&
		gDeviceListSize > 0)
		performHaltOperation(line + 1);
	else
		DlgPrintf("Unknown command '%s'\n", line);
	return ret;
}

void createDeviceNotificationQueue(HANDLE& queue, HANDLE& notif)
{
	MSGQUEUEOPTIONS msgopts;
	msgopts.dwSize = sizeof(msgopts);
	msgopts.dwFlags = 0;
	msgopts.dwMaxMessages = 0;
	msgopts.cbMaxMessage = sizeof(DEVDETAIL) + MAX_DEVCLASS_NAMELEN;
	msgopts.bReadAccess = TRUE;
	queue = CreateMsgQueue(NULL, &msgopts);
	notif = RequestDeviceNotifications(UkwDriverGUID(), queue, TRUE);
}

void destroyDeviceNotificationQueue(HANDLE& queue, HANDLE& notif)
{
	StopDeviceNotifications(notif);
	CloseMsgQueue(queue);
}

void checkDeviceNotificationQueue(HANDLE& queue)
{
	const DWORD maxBufLen = sizeof(DEVDETAIL) + MAX_DEVCLASS_NAMELEN;
	char buf[maxBufLen];
	DWORD bytesRead;
	DWORD flags;

	while (ReadMsgQueue(queue, &buf, maxBufLen, &bytesRead, 0, &flags)) {
		DEVDETAIL* detail = reinterpret_cast<DEVDETAIL*>(buf);
		if (detail->fAttached) {
			WDlgPrintf(L"\nDevice attached: %s\n", detail->szName);
		} else {
			WDlgPrintf(L"\nDevice detached: %s\n", detail->szName);
		}
	}
}


int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	char line[MAX_LINE_LENGTH + 1];
	int chr, i;
	DlgPrintf("Welcome to the USB Kernel Wrapper test console\n");
	BOOL running = TRUE;

	// Create message queue for device notifications
	HANDLE queue, notif;
	createDeviceNotificationQueue(queue, notif);

	while (running) {
		checkDeviceNotificationQueue(queue);
		printMenu();
		for (i = 0;	i < MAX_LINE_LENGTH; ++i) {
			chr = getchar();
			if (chr == EOF || chr == '\n')
				break;
			line[i] = (char)chr;
		}
		line[i] = '\0';
		running = handleCommand(line);
	}
	destroyDeviceNotificationQueue(queue, notif);
	return 0;
}
