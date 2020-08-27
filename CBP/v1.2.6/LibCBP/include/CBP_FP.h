#pragma once
#include "stdafx.h"
#include <Windows.h>

/// \file CBP_FP.h
/// \brief CBP Finger Print (FP) Application Programming Interface (API) Header File
/// \author BCEE Ten Print Break Out Group
/// \author Truc Dao
/// \author Alex Halili
/// \author Art Solano
/// \author Lawrence Travis
/// \bug No known bugs.
/// \version Beta 1
///
/// \paragraph DESCRIPTION Description
///
/// This file describes the standard API for CBP FP implimentation.  The purpose of this API is to manage the basic
/// enumeration, I/O, and imaging analysis associated with finger print scanners. </summary>
///
/// This API will form a level of abstraction
/// between vendors, so that device independance can be achieved for applications.  The API will enable applications to swap different
/// ten print scanners without requiring changes to the application.  It will be a form of "plug and play" operation.
///
/// Thus you will have:
/// \ul
/// \li Application A --> Ten Print Controller --> CBP FP API --> Vendor A specific libraries
/// \li Application A --> Ten Print Controller --> CBP FP API --> Vendor B specific libraries
/// \li Application A --> Ten Print Controller --> CBP FP API --> Vendor C specific libraries
///
/// \li Application B --> Ten Print Controller --> CBP FP API --> Vendor A specific libraries
/// \li Application B --> Ten Print Controller --> CBP FP API --> Vendor B specific libraries
/// \li Application B --> Ten Print Controller --> CBP FP API --> Vendor C specific libraries
///  \ul
///
/// All APIs are asynchronous and impliment a corresponding call back API.  Returns should be immediate.  Implimentations should use multithreading to create the
/// asynchronous behavior.
///
/// Most functions have a corresponding call back.  The call back names start with \c On followed by repeating the function that the call back belongs to.
/// Functions with call backs will initiate the corresponding call back or \c struct \c cbp_fp_callBacks.cbp_fp_onError.
///
/// Functions that do not have a corresponding callback will not return with a void, but instead an int.
///
/// All updates to share memory structures should be thread safe ensuring that only one thread may update the memory structure and that all other threads should wait
/// the memory structure is updated.
///


#ifdef C_CALL ///< Macro compiler directive denoting that standard C calls are referenced in the header file.
extern "C"
{
#endif

// Finger Print Codes
#define CBP_FP_OK									0	///< everything is OK.
#define CBP_FP_ERROR								1	///< a general error has occurred and no details are available.
#define CBP_FP_ERROR_NOT_IMPLEMENTED				2	///< the function has not been implimented.  Some functions are not mandatory and this code is used to indicate that they are optional.
#define CBP_FP_ERROR_NOT_OPENED						3	///< the handle referencing a connection to the ten print scanner is not open.
#define CBP_FP_ERROR_INVALID_HANDLE					4	///< the handle referencing a connection to the ten print scanner is no longer valid.
#define CBP_FP_ERROR_SCANNER_NOT_FOUND				5	///< the scanner indicated has not been found 
#define CBP_FP_ERROR_INVALID_SCANNER_SERIAL_NUMBER	6	///< the scanner serial number is invalid
#define CBP_FP_ERROR_INVALID_SCANNER_NAME			7	///< a scanner name is improperly formatted or not found.
#define CBP_FP_ERROR_IO_ERROR						8	///< the connected scanner experienced an IO error.
#define CBP_FP_ERROR_UNSUPPORTED_SLAP				9	///< an unsupported slap has been requested in \c struct \c cbp_fp_callBacks.cbp_fp_startImaging.
#define CBP_FP_ERROR_BUSY							10	///< the connected scanner is busy.
#define CBP_FP_ERROR_NOT_INITIALIZED				11	///< the library is not initialized.
#define CBP_FP_ERROR_NOT_CLOSED						12	///< the scanner is not closed.
#define CBP_FP_ERROR_CALLBACK_REGISTRATION			13	///< an error occurred during the call back registration.
#define CBP_FP_ERROR_CAPTURE						14	///< an error occurred during a capture of a slap.
#define CBP_FP_ERROR_IMAGING						15	///< an error occurred during a imaging of a slap.
#define CBP_FP_ERROR_LOCK							16	///< an error occurred during a \c cbp_fp_lock call.
#define CBP_FP_ERROR_UNLOCK							17	///< an error occurred during a \c cbp_fp_unLock call.
#define CBP_FP_ERROR_UNITIALIZE						18	///< an error occurred during unitialization.
#define CBP_FP_ERROR_NOT_CONNECTED					19	///< the scanner is disconnected

// Warning
#define CBP_FP_WARNING								50	///< a warning. Vendor warnings are normalized to CBP_FP_WARNING.
#define CBP_FP_WARNING_CONFIGURE					51	///< an error occurred during configuration
#define CBP_FP_WARNING_GET_DIRTINESS				52	///< an error occurred when checking for dirtiness
#define CBP_FP_WARNING_POWER_SAVE					53	///< an error occurred during power save
#define CBP_FP_WARNING_ALREADY_LOCKED				54	///< the connected scanner is already locked.
#define CBP_FP_WARNING_NOT_LOCKED					55	///< the connected scanner is not locked.
#define CBP_FP_WARNING_PROPERTY_NOT_SETABLE			56	///< the property is not setable in \c cbp_fp_setProperty call.
#define CBP_FP_WARNING_PROPERTY						57	///< an error occurred during a \c cbp_fp_getProperty call.
#define CBP_FP_WARNING_CALIBRATE					58	///< an error occurred during the calibrate.


// use defined ERROR
#define CBP_FP_ERROR_INVALID_PARAM						100	///< the param is not valid.
#define CBP_FP_ERROR_PREVIEW							101 ///< an error occurred during preview or previewanalysis.
#define CBP_FP_ERROR_UNSUPPORTED_COLLECTION_TYPE		102	///< an unsupported collection type has been requested in \c struct \c cbp_fp_callBacks.cbp_fp_startImaging.
// CBP requirement #4
#define CBP_FP_ERROR_SCANNER_DISCONNECTED				150	///< an unexpected disconnection of Kojak.

// use defined WARNING
#define CBP_FP_WARNING_ALREADY_INITIALIZED				200	///< the scanner is initialied already.
#define CBP_FP_WARNING_SET_PROPERTY						201	///< the scanner doesn't set property.

/// This structure contains fields for each callback method that the client or middle teir needs to pass to the implimented wrapping layer.
struct cbp_fp_callBacks
{
	/// Returns confirmation that a \c cbp_fp_calibration has successfully occurred.
	/// \note This API is optional but at least needs to be stubbed.  Implimentation of \c cbp_fp_calibration may return \c cbp_fp_onError with a \c CBP_FP_ERROR_NOT_IMPLEMENTED error.
	/// \param handle Index to internal device descriptor information
	/// \see cbp_fp_calibrate
	void(*cbp_fp_onCalibrate)(int handle);

	/// Returns grey scale image when the ten print scanner is put into imaging mode by the call to \c cbp_fp_preview.  The grey scale image returned typically
	/// has a higher resolution than the preview images.  Memory is allocated for \c struct \c cbp_fp_grayScaleCapture on each call.
	/// The scope of the \c struct \c cbp_fp_grayScaleCapture is only within \c cbp_fp_onCapture.  The memory is released after the \c cbp_fp_onCapture.
	/// is finished.  So if the calling program wishes to use any of the parameters that are passed beyond the scope of the call back, it should copy them.
	/// Typically, a higher level module like a controller, will initiate a capture upon the fingers reaching a quality threshold or a capture timeout.
	/// \param handle Index to internal device descriptor information
	/// \param grayScaleCapture Pointer to a structure containing a grey scale image or WSQ, width and height.
	/// \see cbp_fp_capture
	void(*cbp_fp_onCapture)(int handle, struct cbp_fp_grayScaleCapture *grayScaleCapture);

	/// Returns to indicate that the device has been closed
	/// \param handle Index to internal device descriptor information
	/// \see cbp_fp_close
	void(*cbp_fp_onClose)(int handle);

	/// \c cbp_fp_configuration pops up a modal dialog which can alter the configuration of the finger print scanner.  Typically these configuration are persisistant.
	/// Upon closing the modal dialog, this call back is made.  It returns whether a change in configuration occurred or not.
	/// \note This API is optional but at least needs to be stubbed.  Implimentation of \c cbp_fp_configure may return \c cbp_fp_onError with a \c CBP_FP_ERROR_NOT_IMPLEMENTED error.
	/// \param handle Index to internal device descriptor information
	/// \param configurationChange Indicates whether a configuration change occurred or not.  true - if it occurred, false - if it did not occur.  This flag will help the client decide whether it needs to restart.
	/// \see cbp_fp_configure
	void(*cbp_fp_onConfigure)(int handle, bool configurationChange);

	/// Returns the enumerated devices. The memory is released after the \c cbp_fp_OnEnumDevices.
	/// is finished.  So if the calling program wishes to use any of the parameters that are passed beyond the scope of the call back, it should copy them.
	/// \param cbp_fp_deviceList List of detected devices
	/// \param deviceListLen The number of devices in the list.  0 - if no devices recognized.
	/// \param requestID a string which identifies who requested the call
	/// \see cbp_fp_enumDevices
	void(*cbp_fp_onEnumDevices)(struct cbp_fp_device *deviceList[], int deviceListLen, char *requestID);

	/// \brief Returns error information to the client.  All function expecting call back, can also expect that a \c cbp_fp_error may be called.
	/// The memory is released after the \c cbp_fp_onError is finished.  So if the calling program wishes to use any of the parameters that are passed beyond the scope of the call back, it should copy them.
	/// \param errorNumber Negative values are standard CBP_FP API errors.  Vendor error code should be appended to the error message for troubleshooting purpose.
	/// \param errorDescription Short description of the error.  The error description is expected to be read by users if the client selects to drill down on the details.
	void(*cbp_fp_onError)(int errorNumber, char *errorDescription);

	/// Returns the dirtiness level of the ten print scanner.  The dirtiness level is on a scale of 0 to 100, where at level 0, the platten or lens is spotless and free of any imperfections.
	/// At the dirtiness level of 100, the platten or lens is full of imperfections and very obscured.  In a contactless ten print collection environment, there is no platten and only a lens.
	/// In a contact ten print collection environment, this mainly applies to the platten.
	/// \note This API is optional but at least be stubbed.  Implimentation of \c cbp_fp_getDirtiness may return \c cbp_fp_onError with a \c CBP_FP_ERROR_NOT_IMPLEMENTED error.
	/// \param handle Index to internal device descriptor information
	/// \param dirtinessLevel Indicates the level of dirtiness, 0 - for very clean ... to 100 - for very dirty
	/// \see cbp_fp_getDirtiness
	void(*cbp_fp_onGetDirtiness)(int handle , int dirtinessLevel);

	/// Provides information about the scanner properties and lock information
	/// The memory is released after the \c cbp_fp_onGetLockInfo is finished.
	/// So if the calling program wishes to use any of the parameters that are passed beyond the scope of the call back, it should copy them.
	/// \param handle Index to internal device descriptor information
	/// \param lockInfo Provides information about the scanner lock information
	/// \see cbp_fp_getLockInfo
	void(*cbp_fp_onGetLockInfo)(int handle, struct cbp_fp_lock_info *lockInfo);

	/// Indicates that the wrapping layer component has been initialized.
	/// \param requestID a string which identifies who requested the call
	/// \see cbp_fp_initialize
	void(*cbp_fp_onInitialize)(char *requestID);

	/// Returns the status of a \c cbp_fp_lock request
	/// \param handle Index to internal device descriptor information
	/// \param processID Process identifier associated with the lock
	/// \see cbp_fp_lock
	void(*cbp_fp_onLock)(int handle, int processID);

	/// Returns the open ten print scanner
		
	/// \param handle Index to internal device descriptor information
	/// The memory is released after the \c cbp_fp_onOpen is finished.
	/// So if the calling program wishes to use any of the parameters that are passed beyond the scope of the call back, it should copy them.
	/// \param scanner_connection Scanner connection information
	/// \see cbp_fp_open
	void(*cbp_fp_onOpen)(int handle, struct cbp_fp_scanner_connection *scanner_connection);

	/// Returns confirmation that a \c cbp_fp_configuration has successfully occurred.  It returns whether a change in configuration occurred or not.
	/// \note This API is optional but at least needs to be stubbed.  Implimentation of \c cbp_fp_powerSave may return \c cbp_fp_onError with a \c CBP_FP_ERROR_NOT_IMPLEMENTED error.
	/// \param handle Index to internal device descriptor information
	/// \param isOnPowerSave True if the power save feature is on, false - if the power save feature is off
	/// \see cbp_fp_powerSave
	void(*cbp_fp_onPowerSave)(int  handle, bool isOnPowerSave);

	/// Returns grey scale image when the ten print scanner is put into imaging mode by the call to \c cbp_fp_imaging.
	/// Memory is allocated for \c struct \c cbp_fp_grayScalePreview on each call.
	/// The scope of the \c struct \c cbp_fp_grayScalePreview is only within \c struct \c cbp_fp_callBacks.cbp_fp_onPreview.
	/// The memory is released after the \c struct \c cbp_fp_callBacks.cbp_fp_onPreview is finished.
	///
	/// \param handle Index to internal device descriptor information
	/// \param grayScalePreview Pointer to a structure containing a grey scale image, width and height.
	/// \see cbp_fp_preview
	void(*cbp_fp_onPreview)(int handle, struct cbp_fp_grayScalePreview *preview);

	/// Returns preview analysis information when the ten print scanner is put into imaging mode by the call to \c cbp_fp_preview.
	/// The memory is released after the \c cbp_fp_onPreviewAnalysis is finished.
	/// So if the calling program wishes to use any of the parameters that are passed beyond the scope of the call back, it should copy them.		
	/// \param handle Index to internal device descriptor information
	/// \param previewAnalysis Pointer to a structure containing preview analysis information.
	/// \see cbp_fp_preview
	void(*cbp_fp_onPreviewAnalysis)(int handle, struct cbp_fp_previewAnalysis *previewAnalysis);

	/// Returns confirmation that a \c cbp_fp_stopPreview has successfully occurred.
	/// \param handle Index to internal device descriptor information
	/// \see cbp_fp_stopPreview
	void(*cbp_fp_onStopPreview)(int handle);

	/// Indicates that the wrapping layer component has been uninitialized.
	/// \param requestID a string which identifies who requested the call
	/// \see cbp_fp_uninitialize
	void(*cbp_fp_onUninitialize)(char *requestID);

	/// Returns that status of the \c cbp_fp_unLock request
	/// \param handle Index to internal device descriptor information
	/// \see cbp_fp_unlock
	void(*cbp_fp_onUnLock)(int handle);

	/// \brief Returns warning information to the client.  All function expecting call back, can also expect that a cbp_fp_warning may be called.
	/// The memory is released after the \c cbp_fp_onWarning is finished.  So if the calling program wishes to use any of the parameters that are passed beyond the scope of the call back, it should copy them.
	/// \param warningNumber Negative values are standard CBP_FP API errors.  Positive values are defined by the vendor.
	/// \param warningDescription Short description of the error.
	void(*cbp_fp_onWarning)(int warningNumber, char *warningDescription);
};


#define CBP_FP_MAX_PROPERTY_MAX			20	///< Maximum number of properties that a finger print camera can have
#define CBP_FP_MAX_PROPERTY_NAME_MAX	64	///< Maximum length of a finger print scanner property name
#define CBP_FP_MAX_PROPERTY_VALUE_MAX	128 ///< Maximum length of a finger print scanner property value
#define CBP_FP_MAX_SCANNER_NAME_MAX		64	///< Maximum length of a finger print scanner name
#define CBP_FP_MAX_FINGER				10	///< Maximum number of fingers.  I guess no Earnest Hemmingway cats will be processed :-)
#define CBP_MAX_FINGER_ATTRIBUTES		32	///< Maximum number of attributes each finger can have

/// Contains information pertaining to the locking state of a device
struct cbp_fp_lock_info
{
	int handle;			///< an index value that refers to a connection of each ten print scanner.
	int lockPID;		///< the process id belonging to the lock
	long lockDuration;	///< the duration of the lock in milliseconds. This is the number of milliseconds since the lock had been created.
};

/// Contains the connection information of a ten print scanner
struct cbp_fp_scanner_connection
{
	struct cbp_fp_device *device;									///< pointer to the ten print scanner of the connection
	struct cbp_fp_property *propertyList[CBP_FP_MAX_PROPERTY_MAX];	///< array of pointers to the propeties associeted with the ten print scanner
	int propertyListLen;											///< length of the property list.  This is used to exit an iteration through the propertyList.
};


#define CBP_FP_PROPERTY_SUPPORTED_COLLECTION_TYPES	"supportedCollectionTypes"	///< Indicates what kind of collection types a ten print scanner is capable of (flat or rolled).  Values are separated by values by a comma without spaces. \see cbp_fp_collectionType
#define CBP_FP_PROPERTY_SUPPORTED_SLAPS				"supportedSlaps"			///< Indicates what kind of slap types a ten print scanner is capable of. Values are separated by values by a comma without spaces. \see cbp_fp_slapType

#define CBP_FP_PROPERTY_MAKE						"make"						///< Indicates the make of the ten print scanner
#define CBP_FP_PROPERTY_MODEL						"model"						///< Indicates the model of the ten print scanner
#define CBP_FP_PROPERTY_SERIAL						"serial"					///< Indicates the serial number of the ten print scanner

#define CBP_FP_PROPERTY_SOFTWARE_VERSION			"softwareVersion"			///< Indicates the software version of the CBP_FPWrapper DLL
#define CBP_FP_PROPERTY_FIRMWARE_VERSION			"firmwareVersion"			///< Indicates the firmware version of the CBP_FPWrapper DLL


#define CBP_FP_PROPERTY_PREVIEW_PIXEL_HEIGHT		"previewPixelHeight"		///< Indicates the pixel height of the preview.  This value is typically lower than the capture pixel height.
#define CBP_FP_PROPERTY_PREVIEW_PIXEL_WIDTH			"previewPixelWidth"			///< Indicates the pixel width of the preview.  This vlaue is typically lower than the capture pixel width.
#define CBP_FP_PROPERTY_CAPTURE_PIXEL_HEIGHT		"capturePixelHeight"		///< Indicates the pixel height of the capture.  This value is typically higher than the preview pixel height.
#define CBP_FP_PROPERTY_CAPTURE_PIXEL_WIDTH			"capturePixelWidth"			///< Indicates the pixel width of the capture.  This vlaue is typically higher than the preview pixel width.

#define CBP_FP_PROPERTY_PREVIEW_DPI					"previewDPI"				///< Indicates the DPI (dots per inch) supported by the scanner imaging.
#define CBP_FP_PROPERTY_CAPTURE_DPI					"captureDPI"				///< Indicates the DPI (dots per inch) supported by the scanner imaging.

#define CBP_FP_PROPERTY_DEBUG_LEVEL					"debugLevel"				///< Indicates the level of debug where 0 - is off, 1 - low, 2 - high
#define CBP_FP_PROPERTY_DEBUG_FILE_NAME				"debugFile"					///< Indicates the file name of the logging.  The file name can be current directory or full path.  Debug information should be flushed to the log file.


#define CBP_FP_TRUE									"T"							///< Indicates a true value for "has..." properties.
#define CBP_FP_FALSE								"F"							///< Indicates a false value for "has..." properties.
#define CBP_FP_ANALYSIS_HAS_SEGMENTATION			"hasSegmentation"			///< Indicates whether the ten print scanner implimentation supports segmentation in the analysis
#define CBP_FP_ANALYSIS_HAS_CENTROID				"hasCentroid"				///< Indicates whether the ten print scanner implimentation supports centroid in the analysis
#define CBP_FP_ANALYSIS_HAS_SPOOF					"hasSpoof"					///< Indicates whether the ten print scanner implimentation supports spoof detection in the analysis
#define CBP_FP_ANALYSIS_HAS_PREVIEW					"hasPreview"				///< Indicates whether the ten print scanner implimentation any finger print analysis.  If this value is \c F, then the other analysis "has..." properties should be \c F

/// Collection Type Enumeration
enum cbp_fp_collectionType
{
	collection_rolled,			///< rolled collection
	collection_flat,			///< flat collection
	collection_contactless,		///< contactless finger print collection
	collection_unknown	///< unknown collection, this is to indicate an error
};

/// Slap Type Enumeration
enum cbp_fp_slapType
{
	slap_rightHand,			///< right hand four finger slap type
	slap_leftHand,			///< left hand four finger slap type
	slap_twoFingers,		///< two finger slap type, typically this is \c slap_leftIndex index and \c slap_rightIndex fingers, although it can be sometimes an index and middle finger from the same hand
	slap_twoThumbs,			///< two thumbs: \c slap_leftThumb and \c slap_rightThumb
	slap_rightThumb,		///< right thumb
	slap_rightIndex,		///< right index finger
	slap_rightMiddle,		///< right middle finger
	slap_rightRing,			///< right ring finger
	slap_rightLittle,		///< right pinky finger
	slap_leftThumb,			///< left thumb
	slap_leftIndex,			///< left index finger
	slap_leftMiddle,		///< left middle finger
	slap_leftRing,			///< left ring finger
	slap_leftLittle,		///< left pinky finger
	slap_twotThumbs,		///< two thumbs taken together
	slap_stitchedLeftThumb, ///< left thumb that will be stitched
	slap_stitchedRightThumb,///< right thumb that will be stitched
	slap_unknown			///< uknown slap type, used to indicate an error
};

/// Contains the boundingBox information derived from the preview analysis.  /c CBP_FP_ANALYSIS_HAS_SEGMENTATION should be set to \c T.
/// \see CBP_FP_ANALYSIS_HAS_SEGMENTATION
/// \see cbp_fp_onPreviewAnalysis
struct cbp_fp_boundingBox
{
	int width;		///< pixel width of the bounding box
	int height;		///< pixel height of the boudning box
	int x;			///< x position where 0 is the top left corner
	int y;			///< y position where 0 is the top left corner
};

/// Contains the centroid information derived from the preview analysis.  /c CBP_FP_ANALYSIS_HAS_CENTROID should be set to \c T.
/// \see CBP_FP_ANALYSIS_HAS_CENTROID
/// \see cbp_fp_onPreviewAnalysis
struct cbp_fp_centroid
{
	int x;			///< x position where 0 is the top left corner
	int y;			///< y position where 0 is the top left corner
};

/// Analysis Codes
#define CBP_FP_ANALYSIS_CODE_GOOD_PRINT					0		///< Finger Print analysis code indicating that no errors have occurred
#define CBP_FP_ANALYSIS_CODE_NO_PRINT					1		///< Finger Print analysis code indicating that no print was detected
#define CBP_FP_ANALYSIS_CODE_TOO_LIGHT					2		///< Finger Print analysis code indicating that the print is too light
#define CBP_FP_ANALYSIS_CODE_TOO_DARK					3		///< Finger Print analysis code indicating that the print is too dark
#define CBP_FP_ANALYSIS_CODE_BAD_SHAPE					4		///< Finger Print analysis code indicating that the print has a bad or irregular shape for a finger print
#define CBP_FP_ANALYSIS_CODE_WRONG_SLAP					5		///< Finger Print analysis code indicating that the wrong slap was detected.  This is usually derived with four finger slap collection based on the hand geometry

/// 2d positional codes
#define CBP_FP_ANALYSIS_CODE_BAD_POSITION				20		///< Finger Print analysis code indicating that a finger is in a bad position
#define CBP_FP_ANALYSIS_CODE_ROTATE_CLOCKWISE			21		///< Finger Print analysis code indicating that the finger should be rotated clockwise (right)
#define CBP_FP_ANALYSIS_CODE_ROTATE_COUNTERCLOCKWISE	22		///< Finger Print analysis code indicating that the finger should be rotated counterclockwise (left)
#define CBP_FP_ANALYSIS_CODE_TOO_HIGH					23		///< Finger Print analysis code indicating that the finger is too high and should appear lower
#define CBP_FP_ANALYSIS_CODE_TOO_LOW					24		///< Finger Print analysis code indicating that the finger is too low and should appear higher
#define CBP_FP_ANALYSIS_CODE_TOO_LEFT					25		///< Finger Print analysis code indicating that the finger is too far left and should move to the right
#define CBP_FP_ANALYSIS_CODE_TOO_RIGHT					26		///< Finger Print analysis code indicating that the finger is too far right and should move to the left

/// 3d positional codes
/// These are reserved for future facial or contactless finger print activity
#define CBP_FP_ANALYSIS_CODE_TOO_CLOSE					40		///< Finger Print analysis code indicating that the finger is too close and should move further away (for contactless collection)
#define CBP_FP_ANALYSIS_CODE_TOO_FAR					41		///< Finger Print analysis code indicating that the finger is too far and should move closer (for contactless collection)
#define CBP_FP_ANALYSIS_CODE_NOT_FOCUSED				42		///< Finger Print analysis code indicating that the finger is out of focus (for contactless collection)
#define CBP_FP_ANALYSIS_CODE_NOT_ALIGNED				43		///< Finger Print analysis code indicating that the finger is not aligned (for contactless collection)

#define CBP_FP_ANALYSIS_CODE_SPOOF_DETECTED				90		///< Finger Print analysis code indicating that the finger is likely a spoof attempt at a real finger \see CBP_FP_ANALYSIS_HAS_SPOOF

#define CBP_FP_MAX_FP_ANAYSIS_CODES						32		///< The maximum number of finger print codes per finger.

/// Contains the finger print attributes for each finger of a slap
/// \see struct cbp_fp_callBacks.cbp_fp_onPreviewAnalysis
struct cbp_fp_fingerAttributes
{
	int score;													///< indicates the quality rating of a finger print.  The range is from 0 to 100 - where 100 is a perfect print and 0 is horrible quality.  The 100 scale is used to be consistent with NIST NFIQ2
	int analysisCodeList[CBP_FP_MAX_FP_ANAYSIS_CODES];			///< contains the list of analysis codes for a slap.
	int analysisCodeListLen;									///< indicates the length to iterate through the list of analysis codes
	struct cbp_fp_boundingBox boundingBox;						///< indicates the bounding box of the finger.  \see CBP_FP_ANALYSIS_HAS_SEGMENTATION
	struct cbp_fp_centroid centroid;							///< indicates the centroid of the finger. \see CBP_FP_ANALYSIS_HAS_CENTROID
};

/// Contains the imaging information for the gray scale preview
/// \see struct cbp_fp_callBacks.cbp_fp_onPreview
struct cbp_fp_grayScalePreview
{
	int width;													///< indicates the pixel width of the gray scale preview. \see CBP_FP_PROPERTY_PREVIEW_PIXEL_WIDTH
	int height;													///< indicates the pixel height of the gray scale preview.  \see CBP_FP_PROPERTY_PREVIEW_PIXEL_HEIGHT
	cbp_fp_slapType slapType;									///< indicates the slap type, whether its a leftHand, leftThumb etc...
	cbp_fp_collectionType collectionType;						///< indicates the type of collection (flat, rolled) \see cbp_fp_collectionType
	BYTE *image;												///< contains the binary data of the image.  Memory is allocated for the image for the duration of the \c struct \c cbp_fp_callBacks.cbp_fp_onPreview call back.  Once the call back fishes, the memory is released.
};

/// Contains the imaging information for the gray scal capture.  It is expected this image will be of higher resolution then the preview image.
struct cbp_fp_grayScaleCapture
{
	int width;													///< indicates the pixel width of the gray scale capture. \see CBP_FP_PROPERTY_CAPTURE_PIXEL_WIDTH
	int height;													///< indicates the pixel height of the gray scale capture.  \see CBP_FP_PROPERTY_CAPTURE_PIXEL_HEIGHT
	cbp_fp_slapType slapType;									///< indicates the slap type, whether its a leftHand, leftThumb etc...
	cbp_fp_collectionType collectionType;						///< indicates the type of collection (flat, rolled) \see cbp_fp_collectionType
	BYTE *image;												///< contains the binary data of the image.  Memory is allocated for the image for the duration of the \c cbp_fp_onCapture call back.  Once the call back fishes, the memory is released.
};

/// Contains the analysis results for the preview images
/// \see struct cbp_fp_callBacks.cbp_fp_onPreviewAnalysis
struct cbp_fp_previewAnalysis
{
	cbp_fp_collectionType collectionType;									///< indicates the collection type (flat, rolled) \see cbp_fp_collectionType
	cbp_fp_slapType slapType;												///< indicates the slap type \see cbp_fp_slapType
	struct cbp_fp_fingerAttributes fingerAttributeList[CBP_FP_MAX_FINGER];	///< contains the attributes for each finger of the slap.  Fingers are arranged left = 0 to right = CBP_FP_MAX_FINGER \see CBP_FP_MAX_FINGER \see cbp_fp_fingerAttributes
	int fingerAttributeListLen;												///< indicates the length of the fingerAttributeList.  These are the number of fingers that are detected in a slap.
};

/// Contains information to identify a finger print device.  The combination of all three fields should uniquely identify the finger print scanner.
/// \see cbp_fp_OnEnumDevices
struct cbp_fp_device
{
	char make[CBP_FP_MAX_PROPERTY_NAME_MAX];					///< contains the name of the make of the finger print scanner. \see CBP_FP_MAX_PROPERTY_NAME_MAX
	char model[CBP_FP_MAX_PROPERTY_NAME_MAX];					///< contains the name of the model of the finger print scanner \see CBP_FP_MAX_PROPERTY_NAME_MAX
	char serialNumber[CBP_FP_MAX_PROPERTY_NAME_MAX];			///< contains the serial number of the finger print scanner. \see CBP_FP_MAX_PROPERTY_NAME_MAX
};

/// Contains the property value pair for ten print scanners
/// \see cbp_fp_setProperty
/// \see cbp_fp_getProperty
struct cbp_fp_property
{
	char name[CBP_FP_MAX_PROPERTY_NAME_MAX];					///< contains the name of the property \see CBP_FP_MAX_PROPERTY_NAME_LEN
	char value[CBP_FP_MAX_PROPERTY_VALUE_MAX];					///< contains the value of the property \see CBP_FP_MAX_PROPERTY_VALUE_LEN
};

#define STDCALL __stdcall						///<  Default calling mangling for the DLL call signitures

#ifdef CBP_FP_DEVICE_EXPORTS					///< Macro defined when created a Wrapping Layer DLL.  
#define CBP_FP_DEVICE_API __declspec(dllexport) ///< Macro definition for creating exports for a Wrapping Layer DLL.
#else
#define CBP_FP_DEVICE_API __declspec(dllimport) ///< Macro definition for a client to access a Wrapping Layer DLL API.
#endif


// finger print uAPIs

/// \brief Performs calibration on the scanner.  A lock is required.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onCalibrate
CBP_FP_DEVICE_API void STDCALL cbp_fp_calibrate(int handle);
	
///	\brief Captures a ten print slap image.  This image is typically of higher resolution than the image.
/// \param[in] handle handle to the scanner
/// \param[in] slapType indicates the type of slap to be processed.  The scanner may display guidance information based on the slap requested.  In addition the analysis information will be affected by the indicated slap type.
/// \param[in] collectionType indicates whether a flat or rolled slap collection type method will be used
/// \see CBP_FP_PROPERTY_CAPTURE_PIXEL_HEIGHT
/// \see CBP_FP_PROPERTY_CAPTURE_PIXEL_WIDTH
/// \see struct cbp_fp_callBacks.cbp_fp_onCapture
CBP_FP_DEVICE_API void STDCALL cbp_fp_capture(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType);

/// \brief Closes a connection to a ten print scanner and releases any memory associated with that connection.
/// \param[in] handle handle to the scanner to be closed.
/// \see struct cbp_fp_callBacks.cbp_fp_onClose
CBP_FP_DEVICE_API void STDCALL cbp_fp_close(int handle);

/// \brief Pops up a modal dialog that manages proprietary settings of the scanner.  The settings are persistantly cached.
/// It is expected that a restart of the application will be required freeing up all existing connections to the scanner.
/// This API may be optionally implimented.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onConfigure
CBP_FP_DEVICE_API void STDCALL cbp_fp_configure(int handle); // lock required

/// \brief Enumerates the devices recognized for the implimented wrapping layer. The devices may reflect which devices have been installed
/// or perhaps only a subset of those devices which are connected. Results in a call back to \c cbp_fp_onEnumDevices
/// \param[in] requestID a string which is used to pass back to the caller, the identifier of the request
/// \see struct cbp_fp_callBacks.cbp_fp_OnEnumDevices
CBP_FP_DEVICE_API void STDCALL cbp_fp_enumDevices(char *requestID);

/// \brief Gets the dirtiness level of the scanner.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onGetDirtiness
CBP_FP_DEVICE_API void STDCALL cbp_fp_getDirtiness(int handle); // lock required
	
/// \brief gets the lock information of the scanner.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onGetLockInfo
/// \see cbp_fp_lock_info
CBP_FP_DEVICE_API void STDCALL cbp_fp_getLockInfo(int handle);

/// \brief looks up a property in a connection session and returns the value.
/// \param[in] handle handle to the scanner
/// \param[in] name name of the property to look up
/// \return the value of the property is returned. If the property is not found in the connection session, a \c NULL is returned.  Memory returned is allocated from the session and should not be modified.
CBP_FP_DEVICE_API char * STDCALL cbp_fp_getProperty(int handle, char *name);  // no callback

/// \brief Initialized global memory structures and established connections that apply to all finger print scanner(s) associated with the implimented wrapper.
/// The expectation is that this call will only happen once during application initialization.  For example, if your implimentation requires a connecting to a service,
/// the initialization of that service would occur within the implimentation of this call.  Call results in a chain to \c cbp_fp_onInitialize
/// \param[in] requestID a string which is used to pass back to the caller, the identifier of the request
/// \return \c CBP_FP_OK if everything OK, else another error code.
/// \see struct cbp_fp_callBacks.cbp_fp_onInitialize
CBP_FP_DEVICE_API void STDCALL cbp_fp_initialize(char *requestID);

/// \brief Locks a scanner giving exclusive access.  The scanner must be unlocked for anyone else to access it.  Typically applications will lock a scanner on focus, and unlock the scanner on unfocus.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onLock
/// \see cbp_fp_getLockInfo
CBP_FP_DEVICE_API void STDCALL cbp_fp_lock(int handle);

/// \brief Establishes communications with a ten print scanner.  Some implimentations will not support multiple concurrent connections using their SDK.  In this case
/// \c cbp_fp_onError should be chained with an error of \c CBP_FP_ERROR_NOT_IMPLEMENTED.
/// A ten print scanner can be open in multiple ways with this API:
/// \ul
/// \li \c NULL - open will discover any ten print device implimented by the wrapping layer.  The discovered ten print device opened, shall be the first discovered.
/// \li \c cbp_fp_device->model - if this is defined, then a specific model will be discovered.
/// \li \c cbp_fp_device->serial - model must be defined, then if this is defined, a specific model and serial number will be discovered.
/// \ul
/// \param[in] device the scanner to be opened
/// \see struct cbp_fp_device
/// \see struct cbp_fp_callBacks.cbp_fp_onOpen
CBP_FP_DEVICE_API void STDCALL cbp_fp_open(struct cbp_fp_device *device);

/// \brief Sets the powersave mode of the scanner.  During powersave mode, the scanner will power down energy consumming components such as the LED.
/// It is the responsiblity of the client to restore the powersave mode to \c false before requesting any ten print processing from the scanner.
/// \param[in] handle handle to the scanner
/// \param[in] powerSaveOn \c CBP_FP_TRUE if power save mode on, \c CBP_FP_FALSE if power save mode off
/// \see struct cbp_fp_callBacks.cbp_fp_onPowerSave
CBP_FP_DEVICE_API void STDCALL cbp_fp_powerSave(int handle, bool powerSaveOn); // lock required

/// \brief Registers the call backs implimented by the client.  The implimented wrapping layer will call these call backs to indicate events and pass data back to the
/// client.  Each call back in the structure should be initialized to point to a call back function.
/// Implimentations should validate that all call backs are implimented.  This function has no call back to it.
/// \param[in] callBacks structure containing the function pointers to the call backs.  All call backs must be implimented or stubbed. \see struct cbp_fp_callBacks
/// \return \c CBP_FP_OK - if all is well.  \c CBP_FP_ERROR_NOT_IMPLEMENTED - if one or more of the call backs is not implimented.
CBP_FP_DEVICE_API int STDCALL cbp_fp_registerCallBacks(struct cbp_fp_callBacks *callBacks); // no callback
	
/// \brief sets a property value.  Each connection contains a session which cache's the property values.  Some properties may have their settings changed.  While others are read only.
/// If an attempt is made to alter a property value on a read only property, \c cbp_tp_onError is called with a \c CBP_FP_ERROR_PROPERTY_NOT_SETABLE
/// \param[in] handle handle to the device
/// \param[in] property the property name and value to be set in the session property cache
CBP_FP_DEVICE_API void STDCALL cbp_fp_setProperty(int handle, struct cbp_fp_property *property); // no callback

/// \brief Engages the scanner into imaging mode. Imaging and analysis will be returned to the client until a \c cbp_fp_stopImaging is requested.
/// \param[in] handle handle to the scanner
/// \param[in] slapType indicates the type of slap to be processed.  The scanner may display guidance information based on the slap requested.  In addition the analysis information will be affected by the indicated slap type.
/// \param[in] collectionType indicates whether a flat or rolled slap collection type method will be used
/// \see cbp_fp_slapType
/// \see cbp_fp_collectionType
/// \see struct cbp_fp_callBacks.cbp_fp_onPreview
/// \see struct cbp_fp_callBacks.cbp_fp_onPreviewAnalysis
/// \see cbp_fp_capture
/// \see cbp_fp_stopImaging
CBP_FP_DEVICE_API void STDCALL cbp_fp_startImaging(int handle, cbp_fp_slapType slapType, cbp_fp_collectionType collectionType);

/// \brief Disengages the scanner from imaging mode.  The client typically calls this after quality analysis yields sufficient quality for a capture or a time out has occurred.
/// \param[in] handle handle to the scanner
/// \see cbp_fp_stopImaging
CBP_FP_DEVICE_API void STDCALL cbp_fp_stopImaging(int handle);

/// \brief Uninitialized global memory structures and established connections that apply to all ten print scanner(s) associated with the implimented wrapper.
/// The expectation is that this call will only happen once during application uninitialization.  For example, if your implimentation requires a connecting to a service,
/// the uninitialization of that service would occur within the implimentation of this call.  Call results in a chain to \c struct \c cbp_fp_callBacks.cbp_fp_onUninitialize
/// \param[in] requestID a string which is used to pass back to the caller, the identifier of the request
/// \return \c CBP_FP_OK if everything OK, else another error code.
/// \see struct cbp_fp_callBacks.cbp_fp_onUninitialize
CBP_FP_DEVICE_API void STDCALL cbp_fp_uninitialize(char *requestID);

/// \brief Unlocks a scanner allowing for exclusive access of the scanner by another client or service.
/// \param[in] handle handle to the scanner
/// \see struct cbp_fp_callBacks.cbp_fp_onUnLock
CBP_FP_DEVICE_API void STDCALL cbp_fp_unlock(int handle);

#ifdef C_CALL
}
#endif