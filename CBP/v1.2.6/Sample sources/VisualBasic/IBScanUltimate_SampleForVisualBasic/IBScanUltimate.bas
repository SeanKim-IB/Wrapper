Attribute VB_Name = "Module1"
'/*
'****************************************************************************************************
'* IBScanUltimate.h
'*
'* DESCRIPTION:
'*     Definition of image data structures for IBScanUltimate.
'*     http://www.integratedbiometrics.com
'*
'* NOTES:
'*     Copyright (c) Integrated Biometrics, 2009-2015
'*
'* HISTORY:
'*     2012/04/12  Created.
'*     2014/07/23  Reformatted.
'*     2015/04/07  Added enumeration value to IBSU_ImageData.
'*                     (ProcessThres)
'*     2015/12/11  Added enumeration value to IBSU_ImageData to make a same byte padding with C++ structure
'*                     (reserved2, reserved3)
'****************************************************************************************************
'*/


Option Explicit

'/*
'****************************************************************************************************
'* GLOBAL TYPES
'****************************************************************************************************
'*/
'
'/*
'****************************************************************************************************
'* IBSU_ImageFormat
'*
'* DESCRIPTION:
'*     Enumeration of image formats.
'****************************************************************************************************
'*/
Public Enum IBSU_ImageFormat
    IBSU_IMG_FORMAT_GRAY                    '/* Gray-scale image. */
    IBSU_IMG_FORMAT_RGB24                   '/* 24-bit color image. */
    IBSU_IMG_FORMAT_RGB32                   '/* True-color RGB image. */
    IBSU_IMG_FORMAT_UNKNOWN                 '/* Unknown format. */
End Enum

'/*
'****************************************************************************************************
'* IBSU_ImageData
'*
'* DESCRIPTION:
'*     Container for image data and metadata.
'****************************************************************************************************
'*/
Public Type IBSU_ImageData
'    /* Pointer to image buffer.  If this structure is supplied by a callback function, this pointer
'     * must not be retained; the data should be copied to an application buffer for any processing
'     * after the callback returns. */
    Buffer As Long

'    /* Image horizontal size (in pixels). */
    width As Long

'    /* Image vertical size (in pixels). */
    height As Long

'    /* To make a same byte padding with C++ structure */
    reserved As Long

'    /* Horizontal image resolution (in pixels/inch). */
    ResolutionX As Double

'    /* Vertical image resolution (in pixels/inch). */
    ResolutionY As Double

'    /* Image acquisition time, excluding processing time (in seconds). */
    FrameTime As Double

'    /* Image line pitch (in bytes).  A positive value indicates top-down line order; a negative
'     * value indicates bottom-up line order. */
    pitch As Long

'    /* Number of bits per pixel. */
    bitsPerPixel As Byte

'    /* To make a same byte padding with C++ structure */
    reserved2(0 To 2) As Byte

'    /* Image color format. */
    Format As IBSU_ImageFormat

'    /* Marks image as the final processed result from the capture.  If this is FALSE, the image is
'     * a preview image or a preliminary result. */
    IsFinal As Long
    
'    /* Threshold of image processing. */
    ProcessThres As Long

'    /* To make a same byte padding with C++ structure */
    reserved3 As Long
End Type

