unit IBScanUltimate;

(*
****************************************************************************************************
* IBScanUltimate.h
*
* DESCRIPTION:
*     Definition of image data structures for IBScanUltimate.
*     http://www.integratedbiometrics.com
*
* NOTES:
*     Copyright (c) Integrated Biometrics, 2009-2015
*
* HISTORY:
*     2012/04/12  Created.
*     2014/07/23  Reformatted.
*     2015/04/07  Added enumeration value to IBSU_ImageData.
*                     (ProcessThres)
****************************************************************************************************
*)


interface

(* The "C2PTypes.pas" unit declares external windows data types for the conversion purposes. It's created
automatically by the CtoPas converter and saved under "\[Application Path]\CtoPas Projects\P_Files" folder.
Consult the help file for more information about "C2PTypes.pas" unit and it's data type declarations *)

uses
	Windows, Messages, SysUtils, Classes;


//pragma once



/// Container to hold image data together with meta information.
type
(*
****************************************************************************************************
* GLOBAL TYPES
****************************************************************************************************
*)

(*
****************************************************************************************************
* IBSU_ImageFormat
*
* DESCRIPTION:
*     Enumeration of image formats.
****************************************************************************************************
*)
enumIBSU_ImageFormat = (
    IBSU_IMG_FORMAT_GRAY,    (* Gray-scale image. *)
    IBSU_IMG_FORMAT_RGB24,   (* 24-bit color image. *)
    IBSU_IMG_FORMAT_RGB32,   (* True-color RGB image. *)
    IBSU_IMG_FORMAT_UNKNOWN  (* Unknown format. *)
);
IBSU_ImageFormat = integer;
pIBSU_ImageFormat = ^IBSU_ImageFormat;

(*
****************************************************************************************************
* IBSU_ImageData
*
* DESCRIPTION:
*     Container for image data and metadata.
****************************************************************************************************
*)
IBSU_ImageData	= record
    (* Pointer to image buffer.  If this structure is supplied by a callback function, this pointer
     * must not be retained; the data should be copied to an application buffer for any processing
     * after the callback returns. *)
    Buffer: pointer;
    
    (* Image horizontal size (in pixels). *)
	Width: DWORD;
    
    (* Image vertical size (in pixels). *)
	Height: DWORD;
    
    (* Horizontal image resolution (in pixels/inch). *)
	ResolutionX: Double;
    
    (* Vertical image resolution (in pixels/inch). *)
	ResolutionY: Double;

    (* Image acquisition time, excluding processing time (in seconds). *)
	FrameTime: Double;
    
    (* Image line pitch (in bytes).  A positive value indicates top-down line order; a negative 
     * value indicates bottom-up line order. *)
	Pitch: Integer;
    
    (* Number of bits per pixel. *)
	BitsPerPixel: BYTE;

    (* Image color format. *)
	Format: IBSU_ImageFormat;
    
    (* Marks image as the final processed result from the capture.  If this is FALSE, the image is
     * a preview image or a preliminary result. *)
	IsFinal: BOOL;
    
    (* Threshold of image processing. *)
    ProcessThres: DWORD;
end;
pIBSU_ImageData = ^IBSU_ImageData;

implementation

end.

