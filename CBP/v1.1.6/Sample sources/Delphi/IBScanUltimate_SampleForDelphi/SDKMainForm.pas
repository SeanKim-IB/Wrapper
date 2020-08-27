unit SDKMainForm;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, ComCtrls, ShlObj, SyncObjs, InitialzeDevice,
  IBScanUltimate, IBScanUltimateApi, IBScanUltimateApi_defs, IBScanUltimateApi_err;

  // Capture sequence definitions
  const CAPTURE_SEQ_FLAT_SINGLE_FINGER        = 'Single flat finger';
  const CAPTURE_SEQ_ROLL_SINGLE_FINGER        = 'Single rolled finger';
  const CAPTURE_SEQ_2_FLAT_FINGERS            = '2 flat fingers';
  const CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS    = '10 single flat fingers';
  const CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS  = '10 single rolled fingers';
  const CAPTURE_SEQ_4_FLAT_FINGERS			      = '4 flat fingers';
  const CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER	= '10 flat fingers with 4-finger scanner';

  // Use user window messages
  const WM_USER_CAPTURE_SEQ_START =				    WM_USER + 1;
  const WM_USER_CAPTURE_SEQ_NEXT =				    WM_USER + 2;
  const WM_USER_DEVICE_COMMUNICATION_BREAK =	WM_USER + 3;
  const WM_USER_DRAW_CLIENT_WINDOW =				  WM_USER + 4;
  const WM_USER_UPDATE_DEVICE_LIST =				  WM_USER + 5;
  const WM_USER_INIT_WARNING =				      	WM_USER + 6;
  const WM_USER_UPDATE_DISPLAY_RESOURCES =		WM_USER + 7;
  const WM_USER_UPDATE_STATUS_MESSAGE	=		    WM_USER + 8;
  const WM_USER_BEEP =							          WM_USER + 9;
  const WM_USER_DRAW_FINGER_QUALITY	=			    WM_USER + 10;

  // Beep definitions
  const __BEEP_FAIL__							            = 0;
  const __BEEP_SUCCESS__						          = 1;
  const __BEEP_OK__							              = 2;
  const __BEEP_DEVICE_COMMUNICATION_BREAK__	  = 3;

  // LED color definitions
  const __LED_COLOR_NONE__	= 0;
  const __LED_COLOR_GREEN__	= 1;
  const __LED_COLOR_RED__		= 2;
  const __LED_COLOR_YELLOW__	= 3;

  // Key button definitions
  const __LEFT_KEY_BUTTON__	= 1;
  const __RIGHT_KEY_BUTTON__	= 2;

type
  CaptureInfo = record
    PreCaptureMessage: string;
    PostCaptureMessage: string;
    ImageType: IBSU_ImageType;
    NumberOfFinger: integer;
    fingerName: string;
  end;
  pCaptureInfo = ^CaptureInfo;

type
  TInitDeviceThread = class(TThread)
  private
    { Private declarations }
  protected
    procedure Execute; override;
  public
  end;


  TForm1 = class(TForm)
    m_picIBLogo: TImage;
    GroupBox1: TGroupBox;
    m_cboUsbDevices: TComboBox;
    GroupBox2: TGroupBox;
    m_cboCaptureSeq: TComboBox;
    m_chkAutoContrast: TCheckBox;
    m_chkIgnoreFingerCount: TCheckBox;
    m_chkAutoCapture: TCheckBox;
    m_chkSaveImages: TCheckBox;
    m_btnImageFolder: TButton;
    m_btnCaptureStop: TButton;
    m_btnCaptureStart: TButton;
    m_staticContrast: TLabel;
    m_sliderContrast: TTrackBar;
    m_txtContrast: TEdit;
    GroupBox3: TGroupBox;
    m_picScanner: TImage;
    m_chkUseClearPlaten: TCheckBox;
    m_FrameImage: TEdit;
    m_txtStatusMessage: TEdit;
    Timer_StatusFingerQuality: TTimer;
    m_chkNFIQScore: TCheckBox;
    m_txtNFIQScore: TEdit;
    m_chkDrawSegmentImage: TCheckBox;
    m_chkInvalidArea: TCheckBox;
    m_chkDetectSmear: TCheckBox;
    m_cboSmearLevel: TComboBox;
    procedure m_btnCaptureStartClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure m_btnCaptureStopClick(Sender: TObject);
    procedure m_chkAutoContrastClick(Sender: TObject);
    procedure m_sliderContrastChange(Sender: TObject);
    procedure m_cboUsbDevicesChange(Sender: TObject);
    procedure m_cboCaptureSeqChange(Sender: TObject);
    procedure Timer_StatusFingerQualityTimer(Sender: TObject);
    procedure m_btnImageFolderClick(Sender: TObject);
  private
    { Private declarations }
	  m_nSelectedDevIndex       : integer;		///< Index of selected device
	  m_nDevHandle              : integer;	  ///< Device handle
	  m_bInitializing           : BOOL;		    ///< Device initialization is in progress
	  m_ImgSaveFolder           : string;			///< Base folder for image saving
	  m_ImgSubFolder            : string;			///< Sub Folder for image sequence
	  m_strImageMessage         : string;
	  m_bNeedClearPlaten        : BOOL;
	  m_bBlank                  : BOOL;

    m_nOvImageTextHandle      : integer;
    m_nOvClearPlatenHandle    : integer;
	  m_nOvSegmentHandle        : array[0..IBSU_MAX_SEGMENT_COUNT-1] of integer;


	  m_vecCaptureSeq           : array[0..10] of CaptureInfo;				///< Sequence of capture steps
	  m_nCurrentCaptureStep     : integer;		///< Current capture step
    m_nSetCaptureSeqCount     : integer;

	  m_verInfo                 : IBSU_SdkVersion;
	  m_FingerQuality           : array[0..IBSU_MAX_SEGMENT_COUNT-1] of IBSU_FingerQualityState;
	  m_LedType                 : IBSU_LedType;

  public
    { Public declarations }
    function _BrowseForFolder(titleName: string): string;
    procedure _SetStatusBarMessage(strMessage: string);
    procedure _SetImageMessage(strMessage: string);
    procedure _UpdateCaptureSequences();
    function _ReleaseDevice(): integer;
    procedure _BeepFail();
    procedure _BeepSuccess();
    procedure _BeepOk();
    procedure _BeepDeviceCommunicationBreak();
    procedure _SaveBitmapImage(image: IBSU_ImageData; fingerName: string);
    procedure _SaveWsqImage(image: IBSU_ImageData; fingerName: string);
    procedure _SavePngImage(image: IBSU_ImageData; fingerName: string);
    procedure _SaveJP2Image(image: IBSU_ImageData; fingerName: string);
    procedure _SetLEDs(deviceHandle: integer; info: CaptureInfo; ledColor: integer; bBlink: BOOL);

    procedure OnMsg_CaptureSeqStart(var Msg: TMessage); message WM_USER_CAPTURE_SEQ_START;
    procedure OnMsg_CaptureSeqNext(var Msg: TMessage); message WM_USER_CAPTURE_SEQ_NEXT;
    procedure OnMsg_DeviceCommunicationBreak(var Msg: TMessage); message WM_USER_DEVICE_COMMUNICATION_BREAK;
    procedure OnMsg_DrawClientWindow(var Msg: TMessage); message WM_USER_DRAW_CLIENT_WINDOW;
    procedure OnMsg_UpdateDeviceList(var Msg: TMessage); message WM_USER_UPDATE_DEVICE_LIST;
    procedure OnMsg_InitWarning(var Msg: TMessage); message WM_USER_INIT_WARNING;
    procedure OnMsg_UpdateDisplayResources(var Msg: TMessage); message WM_USER_UPDATE_DISPLAY_RESOURCES;
    procedure OnMsg_UpdateStatusMessage(var Msg: TMessage); message WM_USER_UPDATE_STATUS_MESSAGE;
    procedure OnMsg_Beep(var Msg: TMessage); message WM_USER_BEEP;
    procedure OnMsg_DrawFingerQuality(var Msg: TMessage); message WM_USER_DRAW_FINGER_QUALITY;

  end;

  procedure OnEvent_DeviceCommunicationBreak(deviceHandle: integer; pContext: pointer); stdcall;
  procedure OnEvent_PreviewImage(deviceHandle: integer; pContext: pointer; image: IBSU_ImageData); stdcall;
  procedure OnEvent_FingerCount(deviceHandle: integer; pContext: pointer; fingerCountState: IBSU_FingerCountState); stdcall;
  procedure OnEvent_FingerQuality(deviceHandle: integer; pContext: pointer; pQualityArray: pIBSU_FingerQualityState; qualityArrayCount: integer); stdcall;
  procedure OnEvent_DeviceCount(detectedDevices: integer; pContext: pointer); stdcall;
  procedure OnEvent_InitProgress(deviceIndex: integer; pContext: pointer; progressValue: integer); stdcall;
  procedure OnEvent_TakingAcquisition(deviceHandle: integer; pContext: pointer; imageType: IBSU_ImageType); stdcall;
  procedure OnEvent_CompleteAcquisition(deviceHandle: integer; pContext: pointer; imageType: IBSU_ImageType); stdcall;
  procedure OnEvent_ResultImageEx(deviceHandle: integer; pContext: pointer; imageStatus: integer; image: IBSU_ImageData; imageType: IBSU_ImageType; detectedFingers: integer; segmentImageArrayCount: integer; pSegmentImageArray: pIBSU_ImageData; pSegmentPositionArray: pIBSU_SegmentPosition); stdcall;
  procedure OnEvent_ClearPlatenAtCapture(deviceHandle: integer; pContext: pointer; platenState: IBSU_PlatenState); stdcall;
  procedure OnEvent_PressedKeyButtons(deviceHandle: integer; pContext: pointer; pressedKeyButtons: integer); stdcall;

var
  Form1: TForm1;

  InitThread: TInitDeviceThread;
  criticalSection: TRTLCriticalSection;

implementation
 {$R *.dfm}





////////////////////////////////////////////////////////////////////////////////////////////
// User defined functions
procedure TInitDeviceThread.Execute;
var
  devIndex, devHandle, nRc: integer;
  clientRect: TRect;
  i: integer;
  ledCount: integer;
  operableLEDs: DWORD;
begin
  devIndex := Form1.m_cboUsbDevices.ItemIndex - 1;

	Form1.m_bInitializing := true;
	nRc := IBSU_OpenDevice( devIndex, @devHandle );

	Form1.m_bInitializing := false;

	if nRc >= IBSU_STATUS_OK then
	  begin
		Form1.m_nDevHandle := devHandle;

    IBSU_GetOperableLEDs(devHandle, @Form1.m_LedType, @ledCount, @operableLEDs);

		clientRect := Form1.m_FrameImage.ClientRect;
		// set display window
		IBSU_CreateClientWindow( devHandle, Form1.m_FrameImage.Handle, clientRect.Left, clientRect.Top, clientRect.Right, clientRect.Bottom);

		IBSU_AddOverlayQuadrangle(devHandle, @Form1.m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		IBSU_AddOverlayText(devHandle, @Form1.m_nOvImageTextHandle, 'Arial', 10, TRUE(*bold*), '', 10, 10, 0);
		for i := 0 to IBSU_MAX_SEGMENT_COUNT-1 do
      begin
			IBSU_AddOverlayQuadrangle(devHandle, @Form1.m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    end;

		// register callback functions
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK), @OnEvent_DeviceCommunicationBreak, @Form1 );
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE), @OnEvent_PreviewImage, @Form1 );
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION), @OnEvent_TakingAcquisition, @Form1 );
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION), @OnEvent_CompleteAcquisition, @Form1 );
//		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE), @OnEvent_ResultImage, @Form1 );
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX), @OnEvent_ResultImageEx, @Form1 );
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT), @OnEvent_FingerCount, @Form1 );
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY), @OnEvent_FingerQuality, @Form1 );
		if Form1.m_chkUseClearPlaten.Checked then
			IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE), @OnEvent_ClearPlatenAtCapture, @Form1 );
		IBSU_RegisterCallbacks( devHandle, integer(ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON), @OnEvent_PressedKeyButtons, @Form1 );
	end;

	// status notification and sequence start
	if nRc = IBSU_STATUS_OK then
	  begin
		PostMessage(Form1.Handle, WM_USER_CAPTURE_SEQ_START, 0, 0);
		exit;
	end;

	if nRc > IBSU_STATUS_OK then
		PostMessage(Form1.Handle, WM_USER_INIT_WARNING, nRc, 0)
	else
	  begin
		case nRc of
	  IBSU_ERR_DEVICE_ACTIVE:
		  begin
        Form1._SetStatusBarMessage(Format('[Error code = %d] Device initialization failed because in use by another thread/process.', [nRc]));
      end;
	  IBSU_ERR_USB20_REQUIRED:
		  begin
        Form1._SetStatusBarMessage(Format('[Error code = %d] Device initialization failed because SDK only works with USB 2.0.', [nRc]));
      end;
	  else
		  begin
          Form1._SetStatusBarMessage(Format('[Error code = %d] Device initialization failed', [nRc]));
      end;
	  end;
  end;

	PostMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);

  InitThread := nil;
end;

function TForm1._BrowseForFolder(titleName: string): string;
var
  lpItemID  : PItemIDList;
  BrowseInfo : TBrowseInfo;

  DisplayName: array[0..MAX_PATH] of char;
  TempPath: array[0..MAX_PATH] of char;
begin
  FillChar(BrowseInfo, SizeOf(TBrowseInfo), #0);
  with BrowseInfo do
    begin
    hwndOwner := Application.Handle;
    pszDisplayName := @DisplayName;
    lpszTitle := PChar(titleName);
    ulFlags := BIF_RETURNONLYFSDIRS;
  end;
  lpItemID := SHBrowseForFolder(BrowseInfo);
  if lpItemId <> nil then
    begin
    SHGetPathFromIDList(lpItemID, TempPath);
    GlobalFreePtr(lpItemID);
    Result := TempPath;
  end else
    Result := '';
end;

procedure TForm1._SetStatusBarMessage(strMessage: string);
begin
	m_txtStatusMessage.Text := strMessage;
end;

procedure TForm1._SetImageMessage(strMessage: string);
var
  font_size, x, y: integer;
  cr: DWORD;
begin
	font_size:= 10;
	x := 10;
	y := 10;
	cr := RGB(0, 0, 255);

	IBSU_SetClientWindowOverlayText( m_nDevHandle, 'Arial', font_size, TRUE(*bold*), strMessage, x, y, cr );
  if m_bNeedClearPlaten then
    cr := RGB(255, 0, 0);

  // IBSU_SetClientWindowOverlayText was deprecated since 1.7.0
  // Please use the function IBSU_AddOverlayText and IBSU_ModifyOverlayText instead
	IBSU_ModifyOverlayText( m_nDevHandle, m_nOvImageTextHandle, 'Arial', font_size, TRUE(*bold*), strMessage, x, y, cr );
end;

procedure TForm1._UpdateCaptureSequences();
var
	// store currently selected sequence
	strSelectedText: string;
	selectedSeq, devIndex : integer;
  devDesc: IBSU_DeviceDesc;
begin
	selectedSeq := m_cboCaptureSeq.ItemIndex;
  if selectedSeq > -1 then
		strSelectedText := m_cboCaptureSeq.Text;

	// populate combo box
	m_cboCaptureSeq.Clear;
	m_cboCaptureSeq.Items.Add('- Please select -');

	devIndex := m_cboUsbDevices.ItemIndex - 1;
	if devIndex > -1 then
		IBSU_GetDeviceDescription( devIndex, @devDesc );

	if (devDesc.productName = 'WATSON') or
  (devDesc.productName = 'WATSON MINI' ) or
  (devDesc.productName = 'SHERLOCK_ROIC' ) or
  (devDesc.productName = 'SHERLOCK' ) then
	  begin
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_FLAT_SINGLE_FINGER );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_ROLL_SINGLE_FINGER );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_2_FLAT_FINGERS );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS );
    end
  else if (devDesc.productName = 'COLUMBO') or
  (devDesc.productName = 'CURVE' ) then
    begin
 		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_FLAT_SINGLE_FINGER );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
 	  end
  else if (devDesc.productName = 'HOLMES') or
  (devDesc.productName = 'KOJAK' ) or
  (devDesc.productName = 'FIVE-0' ) then
    begin
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_FLAT_SINGLE_FINGER );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_ROLL_SINGLE_FINGER );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_2_FLAT_FINGERS );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_4_FLAT_FINGERS );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS );
		m_cboCaptureSeq.Items.Add( CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER );
 	  end;

	// select previously selected sequence
	if selectedSeq > -1 then
    selectedSeq := SendMessage(m_cboCaptureSeq.Handle, CB_FINDSTRING, -1, Integer(PChar(strSelectedText)));
  if selectedSeq = -1 then
    m_cboCaptureSeq.ItemIndex := 0
  else
	  m_cboCaptureSeq.ItemIndex := selectedSeq;

	SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
end;

function TForm1._ReleaseDevice(): integer;
var
  nRc: integer;
begin
	nRc := IBSU_STATUS_OK;
	
	if m_nDevHandle <> -1 then
		nRc := IBSU_CloseDevice( m_nDevHandle );

	if nRc >= IBSU_STATUS_OK then
	begin
		m_nDevHandle := -1;
		m_nCurrentCaptureStep := -1;
		m_bInitializing := false;
	end;

	Result := nRc;
end;

procedure TForm1._BeepFail();
var
    beeperType: IBSU_BeeperType;

begin
    if IBSU_GetOperableBeeper(m_nDevHandle, @beeperType) <> IBSU_STATUS_OK then
    begin
        Windows.Beep( 3500, 300 );
        Sleep(150);
        Windows.Beep( 3500, 150 );
        Sleep(150);
        Windows.Beep( 3500, 150 );
        Sleep(150);
        Windows.Beep( 3500, 150 );
    end else begin
        IBSU_SetBeeper(m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2(*Sol*), 12(*300ms = 12*25ms*), 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2(*Sol*), 6(*150ms = 6*25ms*), 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2, 6, 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2, 6, 0, 0);
    end;
end;

procedure TForm1._BeepSuccess();
var
    beeperType: IBSU_BeeperType;

begin
    if IBSU_GetOperableBeeper(m_nDevHandle, @beeperType) <> IBSU_STATUS_OK then
    begin
        Windows.Beep( 3500, 100 );
        Sleep(50);
        Windows.Beep( 3500, 100 );
    end else begin
        IBSU_SetBeeper(m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2, 4, 0, 0);
	    Sleep(150);
   	    IBSU_SetBeeper(m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2, 4, 0, 0);
    end;
end;

procedure TForm1._BeepOk();
var
    beeperType: IBSU_BeeperType;

begin
    if IBSU_GetOperableBeeper(m_nDevHandle, @beeperType) <> IBSU_STATUS_OK then
    begin
    	Windows.Beep( 3500, 100 );
    end else begin
        IBSU_SetBeeper(m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2, 4, 0, 0);
    end;
end;

procedure TForm1._BeepDeviceCommunicationBreak();
var
  i: integer;
begin
	for i:=0 to 7 do
	begin
		Windows.Beep( 3500, 100 );
		Sleep( 100 );
	end;
end;

procedure TForm1._SaveBitmapImage(image: IBSU_ImageData; fingerName: string);
var
  strFolder, strFileName: string;
begin
	if ( Length(m_ImgSaveFolder) = 0 ) or ( Length(m_ImgSubFolder) = 0 ) then
		exit;

	strFolder := Format('%s\%s', [m_ImgSaveFolder, m_ImgSubFolder]);
    CreateDir(strFolder);

    strFileName := Format('%s\Image_%02d_%s.bmp', [strFolder, m_nCurrentCaptureStep, fingerName]);

	if IBSU_SaveBitmapImage( strFileName, image.Buffer,
							  image.Width, image.Height, image.Pitch,
							  image.ResolutionX, image.ResolutionY ) <> IBSU_STATUS_OK then
	begin
		ShowMessage ('Failed to save bitmap image!');
	end;
end;

procedure TForm1._SaveWsqImage(image: IBSU_ImageData; fingerName: string);
var
  strFolder, strFileName, filename: string;
  pCompressedData, pDecompressedData, pDecompressedData2: pbyte;
  compressedLength, width, height, pitch, bitsPerPixel, pixelPerInch: integer;
  fp: TFileStream;
  compressedArray: Array of byte;
begin
	if ( Length(m_ImgSaveFolder) = 0 ) or ( Length(m_ImgSubFolder) = 0 ) then
		exit;

	strFolder := Format('%s\%s', [m_ImgSaveFolder, m_ImgSubFolder]);
    CreateDir(strFolder);

    strFileName := Format('%s\Image_%02d_%s.wsq', [strFolder, m_nCurrentCaptureStep, fingerName]);

	if IBSU_WSQEncodeToFile( strFileName, image.Buffer,
							  image.Width, image.Height, image.Pitch, image.BitsPerPixel,
							  Trunc(image.ResolutionX), 0.75, '' ) <> IBSU_STATUS_OK then
	begin
		ShowMessage ('Failed to save bitmap image!');
	end;

    (***********************************************************
    * TEST codes for WSQ
    *)
	if IBSU_WSQEncodeMem(image.Buffer, image.Width, image.Height, image.Pitch, image.BitsPerPixel,
							  Trunc(image.ResolutionX), 0.75, '',
							  @pCompressedData, @compressedLength) <> IBSU_STATUS_OK then
	begin
		ShowMessage('Failed to save WSQ_1 image!');
 	end;

    filename := Format('%s\Image_%02d_%s_v1.wsq', [strFolder, m_nCurrentCaptureStep, fingerName]);
	fp := TFileStream.Create(filename, fmCreate);
    SetLength(compressedArray, compressedLength);
    CopyMemory(@compressedArray[0], pCompressedData, compressedLength);
    fp.Write(compressedArray[0], compressedLength);
	fp.free;

	if IBSU_WSQDecodeMem(pCompressedData, compressedLength, @pDecompressedData, @width, @height,
							  @pitch, @bitsPerPixel, @pixelPerInch) <> IBSU_STATUS_OK then
	begin
		ShowMessage('Failed to Decode WSQ image!');
 	end;

    filename := Format('%s\Image_%02d_%s_v1.bmp', [strFolder, m_nCurrentCaptureStep, fingerName]);
	if IBSU_SaveBitmapImage( filename, pDecompressedData,
							  width, height, pitch,
							  pixelPerInch, pixelPerInch ) <> IBSU_STATUS_OK then
	begin
		ShowMessage('Failed to save bitmap image!');
 	end;

	if IBSU_WSQDecodeFromFile(strFileName, @pDecompressedData2, @width, @height, @pitch,
							  @bitsPerPixel, @pixelPerInch) <> IBSU_STATUS_OK then
	begin
		ShowMessage('Failed to Decode WSQ image!');
 	end;

    filename := Format('%s\Image_%02d_%s_v2.bmp', [strFolder, m_nCurrentCaptureStep, fingerName]);
	if IBSU_SaveBitmapImage( filename, pDecompressedData2,
							  width, height, pitch,
							  pixelPerInch, pixelPerInch ) <> IBSU_STATUS_OK then
	begin
		ShowMessage('Failed to save bitmap image!');
 	end;

	IBSU_FreeMemory(pCompressedData);
	IBSU_FreeMemory(pDecompressedData);
	IBSU_FreeMemory(pDecompressedData2);
(***********************************************************)
end;

procedure TForm1._SavePngImage(image: IBSU_ImageData; fingerName: string);
var
  strFolder, strFileName: string;
begin
	if ( Length(m_ImgSaveFolder) = 0 ) or ( Length(m_ImgSubFolder) = 0 ) then
		exit;

	strFolder := Format('%s\%s', [m_ImgSaveFolder, m_ImgSubFolder]);
    CreateDir(strFolder);

    strFileName := Format('%s\Image_%02d_%s.png', [strFolder, m_nCurrentCaptureStep, fingerName]);

	if IBSU_SavePngImage( strFileName, image.Buffer,
							  image.Width, image.Height, image.Pitch,
							  image.ResolutionX, image.ResolutionY ) <> IBSU_STATUS_OK then
	begin
		ShowMessage ('Failed to save png image!');
	end;
end;

procedure TForm1._SaveJP2Image(image: IBSU_ImageData; fingerName: string);
var
  strFolder, strFileName: string;
begin
	if ( Length(m_ImgSaveFolder) = 0 ) or ( Length(m_ImgSubFolder) = 0 ) then
		exit;

	strFolder := Format('%s\%s', [m_ImgSaveFolder, m_ImgSubFolder]);
    CreateDir(strFolder);

    strFileName := Format('%s\Image_%02d_%s.jp2', [strFolder, m_nCurrentCaptureStep, fingerName]);

	if IBSU_SaveJP2Image( strFileName, image.Buffer,
							  image.Width, image.Height, image.Pitch,
							  image.ResolutionX, image.ResolutionY, 80 ) <> IBSU_STATUS_OK then
	begin
		ShowMessage ('Failed to save jpeg-2000 image!');
	end;
end;

procedure TForm1._SetLEDs(deviceHandle: integer; info: CaptureInfo; ledColor: integer; bBlink: BOOL);
var
  setLEDs: DWORD;
begin

	setLEDs := 0;
	if (Form1.m_LedType = integer(ENUM_IBSU_LED_TYPE_FSCAN)) then
	begin
		if( bBlink ) then
		begin
	    	if( ledColor = __LED_COLOR_GREEN__ ) then
	        begin
				setLEDs := setLEDs or IBSU_LED_F_BLINK_GREEN;
	        end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_BLINK_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_BLINK_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_BLINK_RED;
			end;
		end;
	
	    if info.ImageType = integer(ENUM_IBSU_ROLL_SINGLE_FINGER ) then
	    begin
	        setLEDs := setLEDs or IBSU_LED_F_PROGRESS_ROLL;
	    end;
	
		if( (info.fingerName = 'SFF_Right_Thumb') or
		    (info.fingerName = 'SRF_Right_Thumb') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_TWO_THUMB;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_RED;
			end;
		end
		else if( (info.fingerName = 'SFF_Left_Thumb') or
			     (info.fingerName = 'SRF_Left_Thumb') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_TWO_THUMB;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_RED;
			end;
		end
		else if( (info.fingerName = 'TFF_2_Thumbs') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_TWO_THUMB;
			if ( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_GREEN;
			end
			else if ( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_RED;
			end
			else if ( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_THUMB_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_THUMB_RED;
			end;
		end
		///////////////////LEFT HAND////////////////////
		else if( (info.fingerName = 'SFF_Left_Index') or
			     (info.fingerName = 'SRF_Left_Index') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_LEFT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_RED;
			end;
		end
		else if( (info.fingerName = 'SFF_Left_Middle') or
			     (info.fingerName = 'SRF_Left_Middle') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_LEFT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_RED;
			end;
		end
		else if( (info.fingerName = 'SFF_Left_Ring') or
			     (info.fingerName = 'SRF_Left_Ring') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_LEFT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_RED;
			end;
		end
		else if( (info.fingerName = 'SFF_Left_Little') or
			     (info.fingerName = 'SRF_Left_Little') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_LEFT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_RED;
			end;
		end
		else if( (info.fingerName = '4FF_Left_4_Fingers') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_LEFT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_RED;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_RED;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_RED;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_INDEX_RED;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_MIDDLE_RED;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_RING_RED;
				setLEDs := setLEDs or IBSU_LED_F_LEFT_LITTLE_RED;
			end;
		end
		///////////RIGHT HAND /////////////////////////
		else if( (info.fingerName = 'SFF_Right_Index') or
			     (info.fingerName = 'SRF_Right_Index') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_RIGHT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_RED;
			end;
		end
		else if( (info.fingerName = 'SFF_Right_Middle')	or
			     (info.fingerName = 'SRF_Right_Middle') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_RIGHT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_RED;
			end;
		end
		else if( (info.fingerName = 'SFF_Right_Ring') or
			     (info.fingerName = 'SRF_Right_Ring') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_RIGHT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_RED;
			end;
		end
		else if( (info.fingerName = 'SFF_Right_Little')	or
			     (info.fingerName = 'SRF_Right_Little')	) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_RIGHT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_RED;
			end;
		end
		else if( (info.fingerName = '4FF_Right_4_Fingers') ) then
		begin
			setLEDs := setLEDs or IBSU_LED_F_PROGRESS_RIGHT_HAND;
			if( ledColor = __LED_COLOR_GREEN__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_GREEN;
			end
			else if( ledColor = __LED_COLOR_RED__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_RED;
			end
			else if( ledColor = __LED_COLOR_YELLOW__ ) then
			begin
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_GREEN;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_INDEX_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_MIDDLE_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_RING_RED;
				setLEDs := setLEDs or IBSU_LED_F_RIGHT_LITTLE_RED;
			end;
		end;
		
		IBSU_SetLEDs(deviceHandle, setLEDs);
	end;
end;





////////////////////////////////////////////////////////////////////////////////////////////
procedure OnEvent_DeviceCommunicationBreak(
  deviceHandle: integer;
  pContext: pointer
  ); stdcall;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
    PostMessage(Form1.Handle, WM_USER_DEVICE_COMMUNICATION_BREAK, 0, 0 );
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_PreviewImage(
  deviceHandle: integer;
  pContext: pointer;
  image: IBSU_ImageData
  ); stdcall;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_FingerCount(
  deviceHandle: integer;
  pContext: pointer;
  fingerCountState: IBSU_FingerCountState
  ); stdcall;
var
  info: CaptureInfo;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);

	info := Form1.m_vecCaptureSeq[Form1.m_nCurrentCaptureStep];
	if(fingerCountState = integer(ENUM_IBSU_NON_FINGER)) then
	begin
		Form1._SetLEDs(deviceHandle, info, __LED_COLOR_RED__, TRUE);
	end
	else
	begin
		Form1._SetLEDs(deviceHandle, info, __LED_COLOR_YELLOW__, TRUE);
	end;
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_FingerQuality(
  deviceHandle: integer;
  pContext: pointer;
  pQualityArray: pIBSU_FingerQualityState;
  qualityArrayCount: integer
  ); stdcall;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
	  CopyMemory(@Form1.m_FingerQuality, pQualityArray, sizeof(IBSU_FingerQualityState)*qualityArrayCount);
	  PostMessage(Form1.Handle, WM_USER_DRAW_FINGER_QUALITY, 0, 0);
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_DeviceCount(
  detectedDevices: integer;
  pContext: pointer
  ); stdcall;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
    PostMessage(Form1.Handle, WM_USER_UPDATE_DEVICE_LIST, 0, 0);
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_InitProgress(
  deviceIndex: integer;
  pContext: pointer;
  progressValue: integer
  ); stdcall;
var
  strMessage: string;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
    strMessage := Format('Initializing device... %d%%', [progressValue]);
    Form1._SetStatusBarMessage(strMessage);
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_TakingAcquisition(
  deviceHandle: integer;
  pContext: pointer;
  imageType: IBSU_ImageType
  ); stdcall;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
	  if imageType = integer(ENUM_IBSU_ROLL_SINGLE_FINGER) then
	  begin
		  PostMessage(Form1.Handle, WM_USER_BEEP, __BEEP_OK__, 0);
		  Form1.m_strImageMessage := 'When done remove finger from sensor';
		  Form1._SetImageMessage(Form1.m_strImageMessage);
		  Form1._SetStatusBarMessage(Form1.m_strImageMessage);
	  end;
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_CompleteAcquisition(
  deviceHandle: integer;
  pContext: pointer;
  imageType: IBSU_ImageType
  ); stdcall;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
	  if imageType = integer(ENUM_IBSU_ROLL_SINGLE_FINGER) then
	  begin
	    PostMessage(Form1.Handle, WM_USER_BEEP, __BEEP_OK__, 0);
    end else
    begin
		  PostMessage(Form1.Handle, WM_USER_BEEP, __BEEP_SUCCESS__, 0);
		  Form1.m_strImageMessage := 'Remove fingers from sensor';
		  Form1._SetImageMessage(Form1.m_strImageMessage);
 		  Form1.m_strImageMessage := 'Acquisition completed, postprocessing..';
		  Form1._SetStatusBarMessage(Form1.m_strImageMessage);
      IBSU_RedrawClientWindow(deviceHandle);
    end;

  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

(****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please use IBSU_CallbackResultImageEx instead
*)
procedure OnEvent_ResultImageEx(
  deviceHandle: integer;
  pContext: pointer;
  imageStatus: integer;
  image: IBSU_ImageData;
  imageType: IBSU_ImageType;
  detectedFingers: integer;
  segmentImageArrayCount: integer;
  pSegmentImageArray: pIBSU_ImageData;
  pSegmentPositionArray: pIBSU_SegmentPosition
  ); stdcall;
var
  imgTypeName: string;
  info: CaptureInfo;
  segmentName: string;
  i: integer;
  nRc: integer;
	strValue: string;
  propertyValue: array[0..IBSU_MAX_STR_LEN] of AnsiChar;
  scaleFactor: Extended;
  leftMargin, topMargin: integer;
  cr: DWORD;
  x1, x2, x3, x4, y1, y2, y3, y4: integer;
 	nfiq_score: array[0..IBSU_MAX_SEGMENT_COUNT] of integer;
	score: integer;
  askMsg: string;
  retValue: integer;
  cntSegmentImageArray: integer;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
	  if deviceHandle <> Form1.m_nDevHandle then
		  exit;

    if imageStatus >= IBSU_STATUS_OK then
        begin
        if imageType = integer(ENUM_IBSU_ROLL_SINGLE_FINGER) then
          begin
          PostMessage(Form1.Handle, WM_USER_BEEP, __BEEP_SUCCESS__, 0);
        end;
    end;

  	// added 2012-11-30
	  if Form1.m_bNeedClearPlaten then
    begin
		  Form1.m_bNeedClearPlaten := FALSE;
      PostMessage(Form1.Handle, WM_USER_DRAW_FINGER_QUALITY, 0, 0);
	  end;

	  // Image acquisition successful
  	case imageType of
	  integer(ENUM_IBSU_ROLL_SINGLE_FINGER):
		  begin
        imgTypeName := '-- Rolling single finger --';
      end;
    integer(ENUM_IBSU_FLAT_SINGLE_FINGER):
		  begin
        imgTypeName := '-- Flat single finger --';
      end;
	  integer(ENUM_IBSU_FLAT_TWO_FINGERS):
		  begin
        imgTypeName := '-- Flat two fingers --';
      end;
	  integer(ENUM_IBSU_FLAT_FOUR_FINGERS):
		  begin
        imgTypeName := '-- Flat 4 fingers --';
      end;
	  else
		  begin
        imgTypeName := '-- Unknown --';
      end;
	  end;

    if imageStatus >= IBSU_STATUS_OK then
    begin
      // Image acquisition successful
      info := Form1.m_vecCaptureSeq[Form1.m_nCurrentCaptureStep];
      Form1._SetLEDs(deviceHandle, info, __LED_COLOR_GREEN__, FALSE);

      cntSegmentImageArray := 0;
      if Form1.m_chkSaveImages.Checked then
	    begin
		    Form1._SetStatusBarMessage('Saving image...');
	      info := Form1.m_vecCaptureSeq[Form1.m_nCurrentCaptureStep];
			  Form1._SaveBitmapImage(image, info.fingerName);
			  Form1._SaveWsqImage(image, info.fingerName);
			  Form1._SavePngImage(image, info.fingerName);
			  Form1._SaveJP2Image(image, info.fingerName);
        if segmentImageArrayCount > 0 then
          begin
          for i:=0 to segmentImageArrayCount-1 do
            begin
            segmentName := Format('%s_Segment_%02d', [info.fingerName, i]);
            Form1._SaveBitmapImage(pSegmentImageArray^, segmentName);
            Form1._SaveWsqImage(pSegmentImageArray^, segmentName);
            Form1._SavePngImage(pSegmentImageArray^, segmentName);
            Form1._SaveJP2Image(pSegmentImageArray^, segmentName);
            inc(cntSegmentImageArray);
            pSegmentImageArray := pIBSU_ImageData(pChar(pSegmentImageArray)+sizeof(IBSU_ImageData));
          end;
        end;
      end;

      for i:=0 to cntSegmentImageArray -1 do
        begin
        pSegmentImageArray := pIBSU_ImageData(pChar(pSegmentImageArray)-sizeof(IBSU_ImageData));
      end;

      if Form1.m_chkDrawSegmentImage.Checked then
      begin
        // Draw quadrangle for the segment image
			  IBSU_GetClientWindowProperty(deviceHandle, integer(ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR), @propertyValue);
			  scaleFactor := StrToFloat(propertyValue);
			  IBSU_GetClientWindowProperty(deviceHandle, integer(ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN), @propertyValue);
			  leftMargin := StrToInt(propertyValue);
			  IBSU_GetClientWindowProperty(deviceHandle, integer(ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN), @propertyValue);
			  topMargin := StrToInt(propertyValue);
			  for i:=0 to segmentImageArrayCount -1 do
			    begin
          cr := RGB(0, 128, 0);
				  x1 := leftMargin + trunc(pSegmentPositionArray.x1*scaleFactor);
				  x2 := leftMargin + trunc(pSegmentPositionArray.x2*scaleFactor);
				  x3 := leftMargin + trunc(pSegmentPositionArray.x3*scaleFactor);
				  x4 := leftMargin + trunc(pSegmentPositionArray.x4*scaleFactor);
				  y1 := topMargin +  trunc(pSegmentPositionArray.y1*scaleFactor);
				  y2 := topMargin +  trunc(pSegmentPositionArray.y2*scaleFactor);
				  y3 := topMargin +  trunc(pSegmentPositionArray.y3*scaleFactor);
				  y4 := topMargin +  trunc(pSegmentPositionArray.y4*scaleFactor);
          pSegmentPositionArray := pIBSU_SegmentPosition(pChar(pSegmentPositionArray)+sizeof(IBSU_SegmentPosition));

				  IBSU_ModifyOverlayQuadrangle(deviceHandle, Form1.m_nOvSegmentHandle[i],
					  x1, y1, x2, y2, x3, y3, x4, y4, 1, cr);
			  end;
      end;

	    if Form1.m_chkNFIQScore.Checked then
      begin
        score := 0;
        ZeroMemory(@nfiq_score, sizeof(nfiq_score));
		    strValue := 'Err';
        Form1._SetStatusBarMessage('Get NFIQ score...');
        for i:=0 to IBSU_MAX_SEGMENT_COUNT-1 do
          begin
          if Form1.m_FingerQuality[i] = integer(ENUM_IBSU_FINGER_NOT_PRESENT) then
            begin
            continue;
          end;

          nRc := IBSU_GetNFIQScore(deviceHandle, pSegmentImageArray.Buffer,
            pSegmentImageArray.Width, pSegmentImageArray.Height, pSegmentImageArray.BitsPerPixel, @score);
          pSegmentImageArray := pIBSU_ImageData(pChar(pSegmentImageArray)+sizeof(IBSU_ImageData));

          if nRc = IBSU_STATUS_OK then
            nfiq_score[i] := score
          else
            nfiq_score[i] := -1;
        end;

        strValue := Format('%d-%d-%d-%d', [nfiq_score[0], nfiq_score[1], nfiq_score[2], nfiq_score[3]]);
        Form1.m_txtNFIQScore.Text := strValue;
	    end;

      if imageStatus = IBSU_STATUS_OK then
      begin
	      Form1.m_strImageMessage := Format('%s acquisition completed successfully', [imgTypeName]);
	      Form1._SetImageMessage(Form1.m_strImageMessage);
	      Form1._SetStatusBarMessage(Form1.m_strImageMessage);
      end else
      begin
	      Form1.m_strImageMessage := Format('%s acquisition Warning (Warning code = %d)', [imgTypeName, imageStatus]);
	      Form1._SetImageMessage(Form1.m_strImageMessage);
	      Form1._SetStatusBarMessage(Form1.m_strImageMessage);

        askMsg := Format('[Warning = %d] Do you want a recapture?', [imageStatus]);
        retValue := Application.MessageBox(pAnsiChar(askMsg), 'Warning', MB_YESNO+MB_IConInformation);
        if retValue = IDYES then
          Form1.m_nCurrentCaptureStep := Form1.m_nCurrentCaptureStep - 1;
      end;

	    PostMessage(Form1.Handle, WM_USER_CAPTURE_SEQ_NEXT, 0, 0);
    end;
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_ClearPlatenAtCapture(
  deviceHandle: integer;
  pContext: pointer;
  platenState: IBSU_PlatenState
  ); stdcall;
var
  info: CaptureInfo;
  strMessage: string;
begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);
	  if platenState = integer(ENUM_IBSU_PLATEN_HAS_FINGERS) then
		  Form1.m_bNeedClearPlaten := TRUE
	  else
		  Form1.m_bNeedClearPlaten := FALSE;

	  if Form1.m_bNeedClearPlaten then
	  begin
		  Form1.m_strImageMessage := 'Please remove your fingers on the platen first!';
		  Form1._SetImageMessage(Form1.m_strImageMessage);
		  Form1._SetStatusBarMessage(Form1.m_strImageMessage);
	  end
	  else
	  begin
		  info := Form1.m_vecCaptureSeq[Form1.m_nCurrentCaptureStep];

	  	// Display message for image acuisition again
	  	strMessage := info.PreCaptureMessage;

	  	Form1._SetStatusBarMessage(strMessage);
		  if not(Form1.m_chkAutoCapture.Checked) then
			  strMessage := strMessage + '\r\nPress button "Take Result Image" when image is good!';

		  Form1._SetImageMessage(strMessage);
		  Form1.m_strImageMessage := strMessage;
	  end;

	  PostMessage(Form1.Handle, WM_USER_DRAW_FINGER_QUALITY, 0, 0);
  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;

procedure OnEvent_PressedKeyButtons(
  deviceHandle: integer;
  pContext: pointer;
  pressedKeyButtons: integer
  ); stdcall;
var
  selectedDev : BOOL;
  idle : BOOL;
  active : BOOL;

begin
	if pContext = nil then
		exit;

  try
    EnterCriticalSection(criticalSection);

    selectedDev := Form1.m_cboUsbDevices.ItemIndex > 0;
    idle := not Form1.m_bInitializing and ( Form1.m_nCurrentCaptureStep = -1 );
	  active := not Form1.m_bInitializing and (Form1.m_nCurrentCaptureStep <> -1 );

    if( pressedKeyButtons = __LEFT_KEY_BUTTON__ ) then
    begin
      if (selectedDev and idle) then
      begin
        IBSU_SetBeeper(Form1.m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2(*Sol*), 4(*100ms = 4*25ms*), 0, 0);
        Form1.m_btnCaptureStartClick(nil);
      end;
    end
    else if( pressedKeyButtons = __RIGHT_KEY_BUTTON__ ) then
    begin
      if ( (active) ) then
      begin
        IBSU_SetBeeper(Form1.m_nDevHandle, integer(ENUM_IBSU_BEEP_PATTERN_GENERIC), 2(*Sol*), 4(*100ms = 4*25ms*), 0, 0);
        Form1.m_btnCaptureStopClick(nil);
      end;
    end;

  finally
    LeaveCriticalSection(criticalSection) ;
  end;
end;
////////////////////////////////////////////////////////////////////////////////////////////





procedure TForm1.OnMsg_CaptureSeqStart(var Msg: TMessage);
var
  strCaptureSeq: string;
  pos, nSelectedSeq: integer;
  info: CaptureInfo;
begin
	if m_nDevHandle = -1 then
	begin
		SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
		exit;
	end;

	nSelectedSeq := m_cboCaptureSeq.ItemIndex;
	if nSelectedSeq > -1 then
	  strCaptureSeq := m_cboCaptureSeq.Text;

  pos := 0;
	if strCaptureSeq = CAPTURE_SEQ_FLAT_SINGLE_FINGER then
	begin
		info.PreCaptureMessage := 'Please put a single finger on the sensor!';
		info.PostCaptureMessage := 'Keep finger on the sensor!';
		info.ImageType := integer(ENUM_IBSU_FLAT_SINGLE_FINGER);
		info.NumberOfFinger := 1;
		info.fingerName := 'SFF_Unknown';
		m_vecCaptureSeq[pos] := info;
	end;

	if strCaptureSeq = CAPTURE_SEQ_ROLL_SINGLE_FINGER then
	begin
		info.PreCaptureMessage := 'Please put a single finger on the sensor!';
		info.PostCaptureMessage := 'Roll finger!';
		info.ImageType := integer(ENUM_IBSU_ROLL_SINGLE_FINGER);
		info.NumberOfFinger := 1;
		info.fingerName := 'SRF_Unknown';
		m_vecCaptureSeq[pos] := info;
	end;

	if strCaptureSeq = CAPTURE_SEQ_2_FLAT_FINGERS then
	begin
		info.PreCaptureMessage :='Please put two fingers on the sensor!';
		info.PostCaptureMessage := 'Keep fingers on the sensor!';
		info.ImageType := integer(ENUM_IBSU_FLAT_TWO_FINGERS);
		info.NumberOfFinger := 2;
		info.fingerName := 'TFF_Unknown';
		m_vecCaptureSeq[pos] := info;
	end;

	if strCaptureSeq = CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS then
	begin
    pos := 0;
		info.PreCaptureMessage := 'Please put right thumb on the sensor!';
		info.fingerName := 'SFF_Right_Thumb';
		info.PostCaptureMessage := 'Keep fingers on the sensor!';
		info.ImageType := integer(ENUM_IBSU_FLAT_SINGLE_FINGER);
		info.NumberOfFinger := 1;
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right index on the sensor!';
		info.fingerName := 'SFF_Right_Index';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right middle on the sensor!';
		info.fingerName := 'SFF_Right_Middle';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right ring on the sensor!';
		info.fingerName := 'SFF_Right_Ring';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right little on the sensor!';
		info.fingerName := 'SFF_Right_Little';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left thumb on the sensor!';
		info.fingerName := 'SFF_Left_Thumb';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left index on the sensor!';
		info.fingerName := 'SFF_Left_Index';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left middle on the sensor!';
		info.fingerName := 'SFF_Left_Middle';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left ring on the sensor!';
		info.fingerName := 'SFF_Left_Ring';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left little on the sensor!';
		info.fingerName := 'SFF_Left_Little';
		m_vecCaptureSeq[pos] := info;
	end;

	if strCaptureSeq = CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS then
	begin
    pos := 0;
		info.PreCaptureMessage := 'Please put right thumb on the sensor!';
		info.PostCaptureMessage := 'Roll finger!';
		info.ImageType := integer(ENUM_IBSU_ROLL_SINGLE_FINGER);
		info.NumberOfFinger := 1;
		info.fingerName := 'SRF_Right_Thumb';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right index on the sensor!';
		info.fingerName := 'SRF_Right_Index';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right middle on the sensor!';
		info.fingerName := 'SRF_Right_Middle';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right ring on the sensor!';
		info.fingerName := 'SRF_Right_Ring';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put right little on the sensor!';
		info.fingerName := 'SRF_Right_Little';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left thumb on the sensor!';
		info.fingerName := 'SRF_Left_Thumb';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left index on the sensor!';
		info.fingerName := 'SRF_Left_Index';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left middle on the sensor!';
		info.fingerName := 'SRF_Left_Middle';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left ring on the sensor!';
		info.fingerName := 'SRF_Left_Ring';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left little on the sensor!';
		info.fingerName := 'SRF_Left_Little';
		m_vecCaptureSeq[pos] := info;
	end;

	if strCaptureSeq = CAPTURE_SEQ_4_FLAT_FINGERS then
	begin
		info.PreCaptureMessage :='Please put 4 fingers on the sensor!';
		info.PostCaptureMessage := 'Keep fingers on the sensor!';
		info.ImageType := integer(ENUM_IBSU_FLAT_FOUR_FINGERS);
		info.NumberOfFinger := 4;
		info.fingerName := '4FF_Unknown';
		m_vecCaptureSeq[pos] := info;
	end;

	if strCaptureSeq = CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER then
	begin
    pos := 0;
		info.PreCaptureMessage := 'Please put right 4-fingers on the sensor!';
		info.fingerName := '4FF_Right_4_Fingers';
		info.PostCaptureMessage := 'Keep fingers on the sensor!';
		info.ImageType := integer(ENUM_IBSU_FLAT_FOUR_FINGERS);
		info.NumberOfFinger := 4;
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put left 4-fingers on the sensor!';
		info.fingerName := '4FF_Left_4_Fingers';
		m_vecCaptureSeq[pos] := info;

    inc(pos);
		info.PreCaptureMessage := 'Please put 2-thumbs on the sensor!';
		info.fingerName := 'TFF_2_Thumbs';
		info.ImageType := integer(ENUM_IBSU_FLAT_TWO_FINGERS);
		info.NumberOfFinger := 2;
		m_vecCaptureSeq[pos] := info;
	end;

  m_nSetCaptureSeqCount := pos + 1;

	// Make subfolder name
	m_ImgSubFolder := formatDateTime('yyyy-MM-dd HHmmss', now);

	PostMessage(Form1.Handle, WM_USER_CAPTURE_SEQ_NEXT, 0, 0);
end;

procedure TForm1.OnMsg_CaptureSeqNext(var Msg: TMessage);
var
  i, nRc: integer;
  info: CaptureInfo;
  imgRes: IBSU_ImageResolution;
  bAvailable: BOOL;
  captureOptions: DWORD;
  strMessage: string;
  propertyValue: array[0..IBSU_MAX_STR_LEN] of AnsiChar;
begin
	if m_nDevHandle = -1 then
		exit;

	m_bBlank := FALSE;
  for i:=0 to 3 do
    begin
    m_FingerQuality[i] := integer(ENUM_IBSU_FINGER_NOT_PRESENT);
  end;

	inc(m_nCurrentCaptureStep);
	if m_nCurrentCaptureStep >= m_nSetCaptureSeqCount then
	  begin
		// All of capture sequence completely
	  info.fingerName := 'None';
	  info.ImageType := integer(ENUM_IBSU_TYPE_NONE);
	  _SetLEDs(m_nDevHandle, info, __LED_COLOR_NONE__, FALSE);
		m_nCurrentCaptureStep := -1;
		m_ImgSubFolder := '';

		SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
    m_btnCaptureStart.SetFocus;
		exit;
	end;
 
	if m_chkInvalidArea.Checked then
	  begin
		IBSU_SetClientDisplayProperty(m_nDevHandle, integer(ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA), 'TRUE');
  end
 	else
    begin
		IBSU_SetClientDisplayProperty(m_nDevHandle, integer(ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA), 'FALSE');
	end;

	if m_chkDetectSmear.Checked then
	  begin
		IBSU_SetProperty(m_nDevHandle, integer(ENUM_IBSU_PROPERTY_ROLL_MODE), '1');
    if m_cboSmearLevel.ItemIndex = 0 then
      propertyValue := '0'
    else if m_cboSmearLevel.ItemIndex = 1 then
      propertyValue := '1'
    else
      propertyValue := '2';

		IBSU_SetProperty(m_nDevHandle, integer(ENUM_IBSU_PROPERTY_ROLL_LEVEL), propertyValue);
	end
	else
	  begin
		IBSU_SetProperty(m_nDevHandle, integer(ENUM_IBSU_PROPERTY_ROLL_MODE), '0');
	end;

  for i:=0 to IBSU_MAX_SEGMENT_COUNT-1 do
    begin
    IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  end;

  // Make capture delay for display result image on multi capture mode (500 ms)
	if m_nCurrentCaptureStep > 0 then
	  begin
		Sleep(500);
		m_strImageMessage := '';
	end;

	info := m_vecCaptureSeq[m_nCurrentCaptureStep];

	imgRes := integer(ENUM_IBSU_IMAGE_RESOLUTION_500);
	bAvailable := FALSE;
	nRc := IBSU_IsCaptureAvailable(m_nDevHandle, info.ImageType, imgRes, @bAvailable);
	if (nRc <> IBSU_STATUS_OK) or (not(bAvailable)) then
	  begin
    strMessage := Format('The capture mode (%d) is not available', [integer(info.ImageType)]);
		_SetStatusBarMessage(strMessage);
		m_nCurrentCaptureStep := -1;
		SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
		exit;
	end;

	// Start capture
	captureOptions := 0;
	if m_chkAutoContrast.Checked then
		captureOptions := captureOptions or IBSU_OPTION_AUTO_CONTRAST;
	if m_chkAutoCapture.Checked then
		captureOptions := captureOptions or IBSU_OPTION_AUTO_CAPTURE;
	if m_chkIgnoreFingerCount.Checked then
		captureOptions := captureOptions or IBSU_OPTION_IGNORE_FINGER_COUNT;

	nRc := IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions);
	if nRc >= IBSU_STATUS_OK then
	  begin
		// Display message for image acuisition
		strMessage := info.PreCaptureMessage;

		_SetStatusBarMessage(strMessage);
		if not(m_chkAutoCapture.Checked) then
			strMessage := strMessage + '\r\nPress button "Take Result Image" when image is good!';

		_SetImageMessage(strMessage);
		m_strImageMessage := strMessage;

        _SetLEDs(m_nDevHandle, info, __LED_COLOR_RED__, TRUE);
	end
	else
	  begin
		_SetStatusBarMessage('Failed to execut IBSU_BeginCaptureImage()');
	end;

	SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
end;

procedure TForm1.OnMsg_DeviceCommunicationBreak(var Msg: TMessage);
var
  nRc: integer;
begin
	if m_nDevHandle = -1 then
		exit;

	_SetStatusBarMessage('Device communication was broken');

	nRc := _ReleaseDevice();
	if nRc = IBSU_ERR_RESOURCE_LOCKED then
	  begin
		// retry to release device
		PostMessage(Form1.Handle, WM_USER_DEVICE_COMMUNICATION_BREAK, 0, 0);
	end
	else
	  begin
		PostMessage(Form1.Handle, WM_USER_BEEP, __BEEP_DEVICE_COMMUNICATION_BREAK__, 0);
		PostMessage(Form1.Handle, WM_USER_UPDATE_DEVICE_LIST, 0, 0);
	end
end;

procedure TForm1.OnMsg_DrawClientWindow(var Msg: TMessage);
begin
end;

procedure TForm1.OnMsg_UpdateDeviceList(var Msg: TMessage);
var
  idle: boolean;
  strSelectedText: string;
  selectedDev: integer;
  devices: integer;
  i: integer;
  devDesc: IBSU_DeviceDesc;
  strDevice: string;
begin
	idle := not(m_bInitializing) and ( m_nCurrentCaptureStep = -1 );
	if idle then
	  begin
		m_btnCaptureStop.Enabled := false;
		m_btnCaptureStart.Enabled := false;
	end;

	// store currently selected device
	selectedDev := m_cboUsbDevices.ItemIndex;
	if selectedDev > -1 then
    begin
	  strSelectedText:= m_cboUsbDevices.Text;
  end;

	m_cboUsbDevices.Clear;
	m_cboUsbDevices.Items.Add( '- Please select -' );

	// populate combo box
	devices := 0;
	IBSU_GetDeviceCount( @devices );

	selectedDev := 0;
	for i := 0 to devices-1 do
	  begin
		if IBSU_GetDeviceDescription( i, @devDesc ) < IBSU_STATUS_OK then
      begin
			continue;
    end;

		if devDesc.productName[0] = '\0' then
      begin
			strDevice := 'unknown device';
      end
		else
      begin
			strDevice := devDesc.productName + '_v' + devDesc.fwVersion + ' (' + devDesc.serialNumber + ')';
    end;
		m_cboUsbDevices.Items.Add( strDevice );
		if strDevice = strSelectedText then
      begin
			selectedDev := i + 1;
    end;
	end;

	if ( selectedDev = 0 ) and ( m_cboUsbDevices.Items.Count = 2 ) then
    begin
		selectedDev := 1;
  end;

  m_cboUsbDevices.ItemIndex := selectedDev;

	if idle then
	  begin
//		OnCbnSelchangeComboDevices();
		_UpdateCaptureSequences();
	end;
end;

procedure TForm1.OnMsg_InitWarning(var Msg: TMessage);
begin
end;

procedure TForm1.OnMsg_UpdateDisplayResources(var Msg: TMessage);
var
	selectedDev: BOOL;
  captureSeq: BOOL;
  idle: BOOL;
  active: BOOL;
	uninitializedDev: BOOL;
  strCaption: string;
begin
	selectedDev := m_cboUsbDevices.ItemIndex > 0;
	captureSeq := m_cboCaptureSeq.ItemIndex > 0;
	idle := not(m_bInitializing) and ( m_nCurrentCaptureStep = -1 );
	active := not(m_bInitializing) and (m_nCurrentCaptureStep <> -1 );
	uninitializedDev := (selectedDev) and ( m_nDevHandle = -1 );


	m_cboUsbDevices.Enabled := idle;
	m_cboCaptureSeq.Enabled := selectedDev and idle;

  m_btnCaptureStart.Enabled := captureSeq;
  m_btnCaptureStop.Enabled := active;

  m_chkAutoContrast.Enabled := selectedDev and idle;
  m_chkAutoCapture.Enabled := selectedDev and idle;
  m_chkIgnoreFingerCount.Enabled := selectedDev and idle;
  m_chkSaveImages.Enabled := selectedDev and idle;
  m_btnImageFolder.Enabled := selectedDev and idle;
  m_chkUseClearPlaten.Enabled := uninitializedDev;

	strCaption := '';
	if active then
		strCaption := 'Take Result Image'
	else
   begin
   if not(active) and not(m_bInitializing) then
		strCaption := 'Start';
  end;

	m_btnCaptureStart.Caption := strCaption;
end;

procedure TForm1.OnMsg_UpdateStatusMessage(var Msg: TMessage);
begin
end;

procedure TForm1.OnMsg_Beep(var Msg: TMessage);
var
  beep: integer;
begin
	beep := Msg.WParam;

  case beep of
    __BEEP_FAIL__:
      begin
        _BeepFail();
      end;
    __BEEP_SUCCESS__:
      begin
        _BeepSuccess();
      end;
    __BEEP_OK__:
      begin
        _BeepOk();
      end;
    __BEEP_DEVICE_COMMUNICATION_BREAK__:
      begin
        _BeepDeviceCommunicationBreak();
      end;
    else
  end;
end;

procedure TForm1.OnMsg_DrawFingerQuality(var Msg: TMessage);
var
  i: integer;
//  touchInValue: integer;
begin
	if m_nDevHandle = -1 then
		exit;

	for i := 0 to 3 do
    begin
		if m_bNeedClearPlaten then
      begin
			if m_bBlank then
				m_picScanner.Canvas.Brush.Color := clRed
      else
        m_picScanner.Canvas.Brush.Color := RGB(78,78,78);
		end
		else
		  begin
			case m_FingerQuality[i] of
  	  integer(ENUM_IBSU_QUALITY_GOOD):
			  begin
          m_picScanner.Canvas.Brush.Color := clGreen;
			  end;
  	  integer(ENUM_IBSU_QUALITY_FAIR):
			  begin
          m_picScanner.Canvas.Brush.Color := RGB(255,165,0);
			  end;
  	  integer(ENUM_IBSU_QUALITY_POOR):
			  begin
          m_picScanner.Canvas.Brush.Color := clRed;
			  end;
  	  integer(ENUM_IBSU_FINGER_NOT_PRESENT):
			  begin
          m_picScanner.Canvas.Brush.Color := RGB(78,78,78);
			  end;
  	  integer(ENUM_IBSU_QUALITY_INVALID_AREA_TOP):
			  begin
          m_picScanner.Canvas.Brush.Color := clRed;
			  end;
			integer(ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM):
			  begin
          m_picScanner.Canvas.Brush.Color := clRed;
			  end;
  	  integer(ENUM_IBSU_QUALITY_INVALID_AREA_LEFT):
			  begin
          m_picScanner.Canvas.Brush.Color := clRed;
			  end;
  	  integer(ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT):
			  begin
          m_picScanner.Canvas.Brush.Color := clRed;
			  end;
      else
        exit;
      end;
    end;

    SetBkMode(m_picScanner.Canvas.Handle, TRANSPARENT);
    m_picScanner.Canvas.Pen.Style := psClear;
    m_picScanner.Canvas.RoundRect(15+i*22, 30, 35+i*22, 100, 18, 18);
	end;

	// Draw detected finger on the touch sensor
(*	touchInValue := 0;
	IBSU_IsTouchedFinger(m_nDevHandle, @touchInValue);
	if touchInValue = 1 then
		m_picScanner.Canvas.Brush.Color := clGreen
	else
		m_picScanner.Canvas.Brush.Color := RGB(78,78,78);

  m_picScanner.Canvas.RoundRect(15, 112, 100, 117, 5, 5);
*)
end;









procedure TForm1.FormCreate(Sender: TObject);
var
  i: integer;
  titleName, cDrawString: string;
begin
  // delphi way of initializing a critical section
  InitializeCriticalSection(criticalSection);

  m_chkAutoContrast.Checked := true;
  m_chkAutoCapture.Checked := true;
  m_chkUseClearPlaten.Checked := true;
  m_chkNFIQScore.Checked := true;
  m_chkDrawSegmentImage.Checked := true;

	m_nSelectedDevIndex := -1;

	for i:=0 to 3 do
    begin
		m_FingerQuality[i] := integer(ENUM_IBSU_FINGER_NOT_PRESENT);
  end;

	m_nDevHandle := -1;
	m_nCurrentCaptureStep := -1;
	m_bInitializing := FALSE;
	m_strImageMessage := '';
	m_bNeedClearPlaten := FALSE;

	m_sliderContrast.Min := IBSU_MIN_CONTRAST_VALUE;
 	m_sliderContrast.Max := IBSU_MAX_CONTRAST_VALUE;
	m_sliderContrast.Position := 0;
	m_sliderContrast.Frequency := 5;
	m_txtContrast.Text := '0';

  IBSU_EnableTraceLog(TRUE);

	IBSU_GetSDKVersion(@m_verInfo);
	titleName := 'IntegrationSample for Delphi';
	Form1.Caption := titleName;

	(**
    * Draw product information on IB logo image
	*)
  m_picIBLogo.Canvas.Font.Size := 14;
  m_picIBLogo.Canvas.Font.Name := 'Times New Roman';
  m_picIBLogo.Canvas.Font.Color := RGB(255,255,255);

  SetBkMode(m_picIBLogo.Canvas.Handle, TRANSPARENT);
  cDrawString := Format('Delphi sample with DLL ver. %s', [m_verInfo.Product]);
	m_picIBLogo.Canvas.TextOut(450, 30, cDrawString);

  m_picIBLogo.Canvas.Font.Size := 9;
  cDrawString := 'Copyright (c) Integrated Biometrics';
	m_picIBLogo.Canvas.TextOut(450, 55, cDrawString);

  m_chkInvalidArea.Checked := false;
  m_chkDetectSmear.Checked := true;

  m_cboSmearLevel.Clear;
  m_cboSmearLevel.Items.Add('LOW');
  m_cboSmearLevel.Items.Add('MEDIUM');
  m_cboSmearLevel.Items.Add('HIGH');
  m_cboSmearLevel.ItemIndex := 1;

	IBSU_RegisterCallbacks( 0, integer(ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT), @OnEvent_DeviceCount, @Form1 );
	IBSU_RegisterCallbacks( 0, integer(ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS), @OnEvent_InitProgress, @Form1 );

	PostMessage(Form1.Handle, WM_USER_UPDATE_DEVICE_LIST, 0, 0 );

end;

procedure TForm1.m_btnCaptureStartClick(Sender: TObject);
var
  devIndex: integer;
  IsActive: BOOL;
  nRc: integer;
begin
  if m_bInitializing then
    begin
    exit;
  end;

	devIndex := m_cboUsbDevices.ItemIndex -1;
	if devIndex < 0 then
    begin
		exit;
  end;

	if m_nCurrentCaptureStep <> -1 then
	  begin
		nRc := IBSU_IsCaptureActive(m_nDevHandle, @IsActive);
		if (nRc = IBSU_STATUS_OK) and (IsActive) then
		  begin
			IBSU_TakeResultImageManually(m_nDevHandle);
		end;

		exit;
  end;

	if m_nDevHandle = -1 then
	  begin
		m_bInitializing := true;

    InitThread :=  TInitDeviceThread.Create(True);
    InitThread.FreeOnTerminate := True;
    InitThread.Resume;
    end
	else
	  begin
		// device already initialized
		PostMessage(Form1.Handle, WM_USER_CAPTURE_SEQ_START, 0, 0);
	end;

	SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
end;

procedure TForm1.m_btnCaptureStopClick(Sender: TObject);
var
  tmpInfo: CaptureInfo;
begin
	if m_nDevHandle = -1 then
		exit;

	IBSU_CancelCaptureImage( m_nDevHandle );
  tmpInfo.fingerName := 'None';
  tmpInfo.ImageType := integer(ENUM_IBSU_TYPE_NONE);
  _SetLEDs(m_nDevHandle, tmpInfo, __LED_COLOR_NONE__, FALSE);
	m_nCurrentCaptureStep := -1;
	m_bNeedClearPlaten := FALSE;
	m_bBlank := FALSE;

	_SetStatusBarMessage('Capture Sequence aborted');
	m_strImageMessage := ' ';
	_SetImageMessage(m_strImageMessage);
	SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
end;

procedure TForm1.m_btnImageFolderClick(Sender: TObject);
var
  msg: string;
begin
  m_ImgSaveFolder := _BrowseForFolder('Please select a folder to store captured images!');

  if IBSU_IsWritableDirectory(m_ImgSaveFolder, TRUE) <> IBSU_STATUS_OK then
	  begin
    msg := 'You do not have writing permission on this folder' + #13 + #10 + 'Please select another folder (e.g. desktop folder)';
    Application.MessageBox(pAnsiChar(msg), 'Warning', MB_IConInformation);
  end;
end;

procedure TForm1.m_cboUsbDevicesChange(Sender: TObject);
begin
	if m_cboUsbDevices.ItemIndex = m_nSelectedDevIndex then
		exit;

	Screen.Cursor := crHourglass;

	m_nSelectedDevIndex := m_cboUsbDevices.ItemIndex;
	if m_nDevHandle <> -1 then
	  begin
		m_btnCaptureStopClick(Form1.m_btnCaptureStart);
		_ReleaseDevice();
	end;

	_UpdateCaptureSequences();

	Screen.Cursor := crDefault;
end;

procedure TForm1.m_cboCaptureSeqChange(Sender: TObject);
begin
	SendMessage(Form1.Handle, WM_USER_UPDATE_DISPLAY_RESOURCES, 0, 0);
end;

procedure TForm1.m_chkAutoContrastClick(Sender: TObject);
begin
	if m_chkAutoContrast.Checked then
	  begin
    m_sliderContrast.Enabled := false;
    m_staticContrast.Enabled := false;
    m_txtContrast.Enabled := false;
	end
	else
	  begin
    m_sliderContrast.Enabled := true;
    m_staticContrast.Enabled := true;
    m_txtContrast.Enabled := true;
	end;
end;

procedure TForm1.m_sliderContrastChange(Sender: TObject);
var
  pos: integer;
begin
	if m_nDevHandle = -1 then
		exit;

	pos := m_sliderContrast.Position;
  m_txtContrast.Text := Format('%d', [pos]);
	IBSU_SetContrast(m_nDevHandle, pos);

end;

procedure TForm1.Timer_StatusFingerQualityTimer(Sender: TObject);
begin
  SendMessage(Form1.Handle, WM_USER_DRAW_FINGER_QUALITY, 0, 0);
  if m_bNeedClearPlaten then
    m_bBlank := not(m_bBlank);
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  _ReleaseDevice();

  // delphi way of deleting a critical section properly
  DeleteCriticalSection(criticalSection);
end;

end.
