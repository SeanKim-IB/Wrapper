/*! @file		nex_sdk.h
    @brief		NexID Fingerprint PAD SDK Interface
	@details	This documents the NexID Biometrics PAD SDK API. Please see example.cpp for an example of how the API is used.
    @author		NexID Biometrics, LLC
    @copyright	NexID Biometrics, LLC
    $Rev: 57035 $
    $Date: 2018-03-29 09:47:47 -0400 (Thu, 29 Mar 2018) $
*/

#ifndef NEX_SDK_H
#define NEX_SDK_H

#define DLL_PUBLIC

//! @brief Enumeration representing various classifier configurations.
typedef enum {
    NEX_SDK_CLASSIFIER_ALL,	//!< specify the use of all classifiers
    NEX_SDK_CLASSIFIER_MLP, //!< MLP (Neural Network) classifier
} nex_sdk_classifier_t;

//! @brief Enumeration representing valid return values for the NexID PAD SDK.
typedef enum {
    NEX_SDK_OK							= 0,        //!< function completed successfully
    NEX_SDK_ERR_LICENSE,							//!< license expired or feature unlicensed
    NEX_SDK_ERR_VERSION,							//!< software version and model version are not incompatible
    NEX_SDK_ERR_NO_MEMORY,                          //!< memory allocation failed
    NEX_SDK_ERR_FILE_OFFSET,                        //!< model file is corrupt
    NEX_SDK_ERR_FILE_CORRUPT,						//!< model file is corrupt
    NEX_SDK_ERR_LIB_PNG,							//!< internal error in png library
    NEX_SDK_ERR_LIB_LTC,							//!< internal error in ltc library
    NEX_SDK_ERR_LIB,								//!< general library error

    NEX_SDK_ERR_FILE_OPEN,                          //!< could not open file
    NEX_SDK_ERR_FILE_UNLINK,                        //!< could not remove file
    NEX_SDK_ERR_FILE_READ,                          //!< could not read file
    NEX_SDK_ERR_FILE_WRITE,                         //!< could not write file
    NEX_SDK_ERR_FILE_POSITION,                      //!< file positioning error (file is corrupt)
    NEX_SDK_ERR_FILE_EXTENSION,                     //!< image file extensions should be .bmp or .png only
    NEX_SDK_ERR_FILE_UNRECOGNIZED,                  //!< could not determine type of image

    NEX_SDK_ERR_IMAGE_TYPE,                         //!< invalid image type (must be bmp or png)
    NEX_SDK_ERR_IMAGE_BITDEPTH,                     //!< invalid bit depth. images must be 8-bit grayscale
    NEX_SDK_ERR_IMAGE_COLORS,                       //!< image bust be grayscale
    NEX_SDK_ERR_IMAGE_MAX_AREA,						//!< image area exceeds maximum size
    NEX_SDK_ERR_IMAGE_COMPRESSED,					//!< image is compressed (it should not be)
    NEX_SDK_ERR_IMAGE_PLANES,						//!< number of planes should be one
    NEX_SDK_ERR_IMAGE_SIZE,							//!< invalid image size. height or width too big/small.

    NEX_SDK_ERR_NO_IMAGES,                          //!< directory or file list did not contain any valid images
    NEX_SDK_ERR_TOO_FEW_IMAGES,						//!< not enough live or spoof images exist to perform training or testing
    NEX_SDK_ERR_CLASSIFIER,                         //!< internal error training classifier or classifier does not exist

    NEX_SDK_ERR_NO_MODEL_LOADED,				    //!< no classifier was loaded when trying to score image
    NEX_SDK_ERR_BAD_VAL,							//!< miscellaneous error, probably pointer related

    NEX_SDK_UNDEFINED					= 999       //!< last value in enumeration

} nex_sdk_error_t;

#ifdef __cplusplus
extern "C" {
#endif

//! @brief	Initialize the SDK. This is the first function that should be called.
//! @pre	none
//! @post	memory is allocated and initialized for the SDK. error status is set and can be
//!			determined from a call to ::nex_sdk_error_query().
DLL_PUBLIC void nex_sdk_initialize();

//! @brief	Terminate the SDK and clean up resources.
//!			This is the last function that should be called before exit.
//! @pre	none
//! @post	resources required for the SDK are cleaned up
DLL_PUBLIC void nex_sdk_destroy();

//! @brief	Query SDK for current error state. (See list of constants above)
//! @pre	::nex_sdk_initialize() called successfully
//! @return integer error code from ::nex_sdk_error_t
DLL_PUBLIC int nex_sdk_error_query();

//! @brief	Print message associated with current error state and
//!			reset the error state to ::NEX_SDK_OK and return the old error code.
//! @pre	::nex_sdk_initialize() called successfully
//! @post	error message is printed to console
//! @post	error status is reset to ::NEX_SDK_OK
//! @return error code from ::nex_sdk_error_t before being reset to ::NEX_SDK_OK.
DLL_PUBLIC int nex_sdk_error_print();

//! @brief	Load a NexID PAD classifier model file.
//!			Model should be one that was created using the application nex_performance.
//!			If there was a model already loaded from a previous call to ::nex_sdk_load_model()
//!			that was not destroyed, then the existing model in memory is first destroyed.
//! @pre	::nex_sdk_initialize() called successfully
//! @pre	\p model_path != NULL
//! @post	if successful, model file is loaded into memory.
//! @post	error status is set and can be determined from a call to ::nex_sdk_error_query().
//! @param	[in]	cl			if cl is ::NEX_SDK_CLASSIFIER_MLP load the MLP classifier.
//!								if cl is ::NEX_SDK_CLASSIFIER_SVM load the SVM classifier.
//!								if cl is ::NEX_SDK_CLASSIFIER_ALL load all available classifiers.
DLL_PUBLIC void nex_sdk_load_model(
    nex_sdk_classifier_t cl
);

//! @brief	Free up memory resources related to holding classifier model.
//!			If no model is loaded, then return silently.
//! @pre	::nex_sdk_initialize() called successfully
//! @post	memory where model file was stored has been freed.
//! @post	error status is set and can be determined from a call to ::nex_sdk_error_query().
DLL_PUBLIC void nex_sdk_destroy_model();

//! @brief	Load fingerprint image from memory.
//! @pre	::nex_sdk_initialize() called successfully
//! @pre	\p image must contain exactly \p height*\p width bytes.
//! @pre	\p height > 0 and \p width > 0
//! @pre	model file was loaded with a previous call to ::nex_sdk_load_model()
//! @post	error status is set and can be determined from a call to ::nex_sdk_error_query().
//! @param [in]		image		array of bytes
//! @param [in]		height		height of image in pixels
//! @param [in]		width		width of image in pixels
DLL_PUBLIC void nex_sdk_load_image_bytes(
    const unsigned char *image,
    unsigned int height,
    unsigned int width
);

//! @brief	Get PAD score for the previously loaded image.
//! @pre	::nex_sdk_initialize() called successfully
//! @pre	fingerprint image has been loaded successfully using either
//!			::nex_sdk_load_image_bytes() or ::nex_sdk_load_image_path().
//! @pre	model file was loaded with a previous call to ::nex_sdk_load_model()
//! @post	error status is set and can be determined from a call to ::nex_sdk_error_query().
//! @param [in]		cl			if cl is ::NEX_SDK_CLASSIFIER_MLP compute the MLP score.
//!								if cl is ::NEX_SDK_CLASSIFIER_SVM compute the SVM score.
//! @return normalized PAD score between 0 (spoof) and 1000 (live). Return -1 on error.
DLL_PUBLIC int nex_sdk_get_score(
    nex_sdk_classifier_t cl
);

#ifdef __cplusplus
}
#endif

#endif // NEX_SDK_H
