VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   1  '단일 고정
   Caption         =   "IBScanUltimate_SampleForVB"
   ClientHeight    =   7890
   ClientLeft      =   45
   ClientTop       =   435
   ClientWidth     =   11595
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   8.25
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   7890
   ScaleWidth      =   11595
   StartUpPosition =   3  'Windows 기본값
   Begin VB.ComboBox m_cboSmearLevel 
      Height          =   315
      Left            =   3600
      TabIndex        =   22
      Top             =   6720
      Width           =   615
   End
   Begin VB.CheckBox m_chkDetectSmear 
      Caption         =   "Detect smear"
      Height          =   255
      Left            =   2280
      TabIndex        =   21
      Top             =   6720
      Width           =   1335
   End
   Begin VB.CheckBox m_chkInvalidArea 
      Caption         =   "Invalid area"
      Height          =   255
      Left            =   2280
      TabIndex        =   20
      Top             =   6360
      Width           =   1815
   End
   Begin VB.CheckBox m_chkDrawSegmentImage 
      Caption         =   "Draw quadrangle for segment image"
      Height          =   375
      Left            =   2280
      TabIndex        =   19
      Top             =   5520
      Width           =   2055
   End
   Begin VB.TextBox m_txtNFIQScore 
      Enabled         =   0   'False
      Height          =   285
      Left            =   3240
      TabIndex        =   18
      Top             =   6000
      Width           =   975
   End
   Begin VB.CheckBox m_chkNFIQScore 
      Caption         =   "NFIQ"
      Height          =   255
      Left            =   2280
      TabIndex        =   17
      Top             =   6000
      Width           =   735
   End
   Begin VB.Timer Timer_Get_BImage 
      Interval        =   50
      Left            =   3240
      Top             =   7320
   End
   Begin VB.TextBox m_ImageFrame 
      BackColor       =   &H8000000F&
      Height          =   6015
      Left            =   4320
      TabIndex        =   16
      Top             =   1320
      Width           =   7095
   End
   Begin VB.Timer Timer_PoolingDevice 
      Interval        =   300
      Left            =   2520
      Top             =   7440
   End
   Begin VB.CheckBox m_chkUseClearPlaten 
      Caption         =   "Detect clear platen"
      Height          =   255
      Left            =   2280
      TabIndex        =   14
      Top             =   5160
      Width           =   2055
   End
   Begin VB.CommandButton m_btnCaptureStart 
      Caption         =   "Start"
      Height          =   375
      Left            =   2520
      TabIndex        =   12
      Top             =   3840
      Width           =   1575
   End
   Begin VB.CheckBox m_chkSaveImages 
      Caption         =   "Save images"
      Height          =   255
      Left            =   240
      TabIndex        =   9
      Top             =   3480
      Width           =   1215
   End
   Begin VB.CheckBox m_chkIgnoreFingerCount 
      Caption         =   "Trigger invalid finger count on auto-capture mode"
      Height          =   255
      Left            =   240
      TabIndex        =   8
      Top             =   3120
      Width           =   3855
   End
   Begin VB.CheckBox m_chkAutoCapture 
      Caption         =   "Automatic capture for fingerprints"
      Height          =   255
      Left            =   240
      TabIndex        =   7
      Top             =   2880
      Width           =   3855
   End
   Begin VB.ComboBox m_cboCaptureSeq 
      Height          =   315
      Left            =   240
      TabIndex        =   5
      Top             =   2280
      Width           =   3855
   End
   Begin VB.Frame Frame2 
      Caption         =   "Fingerprint Capture"
      Height          =   2895
      Left            =   120
      TabIndex        =   4
      Top             =   2040
      Width           =   4095
      Begin VB.CommandButton m_btnCaptureStop 
         Caption         =   "Stop"
         Height          =   375
         Left            =   120
         TabIndex        =   11
         Top             =   1800
         Width           =   1575
      End
      Begin VB.CommandButton m_btnImageFolder 
         Caption         =   "..."
         Height          =   255
         Left            =   1440
         TabIndex        =   10
         Top             =   1440
         Width           =   495
      End
      Begin VB.CheckBox m_chkAutoContrast 
         Caption         =   "Automatic contrast optimization"
         Enabled         =   0   'False
         Height          =   255
         Left            =   120
         TabIndex        =   6
         Top             =   600
         Width           =   3855
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Devices"
      Height          =   735
      Left            =   120
      TabIndex        =   1
      Top             =   1200
      Width           =   4095
      Begin VB.ComboBox m_cboUsbDevices 
         Height          =   315
         ItemData        =   "SDKMainForm.frx":0000
         Left            =   120
         List            =   "SDKMainForm.frx":0002
         TabIndex        =   3
         Top             =   240
         Width           =   3855
      End
   End
   Begin VB.Frame Frame3 
      Caption         =   "Device Quality"
      Height          =   2295
      Left            =   120
      TabIndex        =   13
      Top             =   5040
      Width           =   2055
      Begin VB.Shape Shape_Quality_4 
         BackStyle       =   1  '투명하지 않음
         BorderStyle     =   0  '투명
         Height          =   975
         Left            =   1340
         Top             =   720
         Width           =   290
      End
      Begin VB.Shape Shape_Quality_3 
         BackColor       =   &H00FFFFFF&
         BackStyle       =   1  '투명하지 않음
         BorderStyle     =   0  '투명
         Height          =   975
         Left            =   1010
         Top             =   720
         Width           =   290
      End
      Begin VB.Shape Shape_Quality_2 
         BackColor       =   &H000080FF&
         BackStyle       =   1  '투명하지 않음
         BorderStyle     =   0  '투명
         Height          =   975
         Left            =   680
         Top             =   720
         Width           =   290
      End
      Begin VB.Shape Shape_Quality_1 
         BackColor       =   &H000000FF&
         BackStyle       =   1  '투명하지 않음
         BorderStyle     =   0  '투명
         Height          =   975
         Left            =   350
         Top             =   720
         Width           =   290
      End
      Begin VB.Image m_picScanner 
         Height          =   1860
         Left            =   120
         Picture         =   "SDKMainForm.frx":0004
         Top             =   240
         Width           =   1725
      End
   End
   Begin VB.Label m_txtStatusMessage 
      BorderStyle     =   1  '단일 고정
      Caption         =   "Ready"
      Height          =   255
      Left            =   120
      TabIndex        =   15
      Top             =   7440
      Width           =   11295
   End
   Begin VB.Label Label1 
      AutoSize        =   -1  'True
      BackStyle       =   0  '투명
      Caption         =   "Copyright (c) Integrated Biometrics"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   9
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   225
      Left            =   6720
      TabIndex        =   2
      Top             =   840
      Width           =   2580
   End
   Begin VB.Label m_txtNameOnIBLogo 
      AutoSize        =   -1  'True
      BackStyle       =   0  '투명
      Caption         =   "VB sample with DLL ver. "
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   14.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   315
      Left            =   6720
      TabIndex        =   0
      Top             =   480
      Width           =   2940
   End
   Begin VB.Image Image1 
      Height          =   1155
      Left            =   0
      Picture         =   "SDKMainForm.frx":A8D6
      Top             =   0
      Width           =   11700
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private m_nDeviceCount As Long
Private m_bEnable_BImage_Timer As Boolean
Private FileSystem As Object
Private Const s_brushRed = &HFF&
Private Const s_brushOrange = &H7FFF&
Private Const s_brushGreen = &H7F00&
Private Const s_brushPlaten = &H424242

Public Function fnByteToStr(bytArray() As Byte) As String
    Dim sAns As String
    Dim iPos As String
    
    sAns = StrConv(bytArray, vbUnicode)
    iPos = InStr(sAns, Chr(0))
    If iPos > 0 Then sAns = Left(sAns, iPos - 1)
    
    fnByteToStr = sAns
End Function

Public Sub fnSetStatusBarMessage(ByVal strMessage As String)
    m_txtStatusMessage.Caption = strMessage
End Sub

Public Sub fnSetImageMessage(ByVal strMessage As String)
    Dim font_size, x, y, cr As Long
    
    font_size = 10
    x = 10
    y = 10
    cr = RGB(0, 0, 255)
    
    If m_bNeedClearPlaten Then
        cr = RGB(255, 0, 0)
    End If
    
'    // IBSU_SetClientWindowOverlayText was deprecated since 1.7.0
'    // Please use the function IBSU_AddOverlayText and IBSU_ModifyOverlayText instead
    Call IBSU_ModifyOverlayText(m_nDevHandle, m_nOvImageTextHandle, "Arial", font_size, 1, strMessage, x, y, cr)
End Sub

Public Sub fnUpdateCaptureSequences()
    '// store currently selected sequence
    Dim strSelectedText As String
    Dim devIndex, selectedSeq As Integer
    Dim devDesc As IBSU_DeviceDesc
    Dim i As Integer
    
    
    selectedSeq = m_cboCaptureSeq.ListIndex
    If selectedSeq > -1 Then
        strSelectedText = m_cboCaptureSeq.text
    End If
    
    '// populate combo box
    m_cboCaptureSeq.Clear
    m_cboCaptureSeq.AddItem ("- Please select -")

    devIndex = m_cboUsbDevices.ListIndex - 1
    If devIndex > -1 Then
        Call IBSU_GetDeviceDescription(devIndex, devDesc)
    End If
    
    If (fnByteToStr(devDesc.productName) = "WATSON") Or _
       (fnByteToStr(devDesc.productName) = "WATSON MINI") Or _
       (fnByteToStr(devDesc.productName) = "SHERLOCK_ROIC") Or _
       (fnByteToStr(devDesc.productName) = "SHERLOCK") Then
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_FLAT_SINGLE_FINGER)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_ROLL_SINGLE_FINGER)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_2_FLAT_FINGERS)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS)
    ElseIf (fnByteToStr(devDesc.productName) = "COLUMBO") Or _
       (fnByteToStr(devDesc.productName) = "CURVE") Then
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_FLAT_SINGLE_FINGER)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS)
    ElseIf (fnByteToStr(devDesc.productName) = "HOLMES") Or _
       (fnByteToStr(devDesc.productName) = "KOJAK") Or _
       (fnByteToStr(devDesc.productName) = "FIVE-0") Then
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_FLAT_SINGLE_FINGER)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_ROLL_SINGLE_FINGER)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_2_FLAT_FINGERS)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_4_FLAT_FINGERS)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS)
        m_cboCaptureSeq.AddItem (CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER)
    End If

    '// select previously selected sequence
    If selectedSeq > -1 Then
        selectedSeq = SendMessage(m_cboCaptureSeq.hwnd, CB_FINDSTRING, -1, ByVal strSelectedText)
    End If
    
    If selectedSeq = -1 Then
        m_cboCaptureSeq.ListIndex = 0
    Else
        m_cboCaptureSeq.ListIndex = selectedSeq
    End If

    Call OnMsg_UpdateDisplayResources(0, 0)
End Sub

Public Function fnReleaseDevice() As Long
    Dim nRc As Long
    
    nRc = IBSU_STATUS_OK
    
    If m_nDevHandle <> -1 Then
        nRc = IBSU_CloseDevice(m_nDevHandle)
    End If
    
    If nRc >= IBSU_STATUS_OK Then
        m_nDevHandle = -1
        m_nCurrentCaptureStep = -1
        m_bInitializing = False
    End If
End Function

Public Sub fnBeepFail()
    Dim beeperType As IBSU_BeeperType
    
    If IBSU_GetOperableBeeper(m_nDevHandle, beeperType) <> IBSU_STATUS_OK Then
        Call Beep(3500, 300)
        Sleep (150)
        Call Beep(3500, 150)
        Sleep (150)
        Call Beep(3500, 150)
        Sleep (150)
        Call Beep(3500, 150)
    Else
        Call IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 12, 0, 0)
        Sleep (150)
        Call IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 6, 0, 0)
        Sleep (150)
        Call IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 6, 0, 0)
        Sleep (150)
        Call IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 6, 0, 0)
    End If
End Sub

Public Sub fnBeepSuccess()
    Dim beeperType As IBSU_BeeperType
    
    If IBSU_GetOperableBeeper(m_nDevHandle, beeperType) <> IBSU_STATUS_OK Then
        Call Beep(3500, 100)
        Sleep (50)
        Call Beep(3500, 100)
    Else
        Call IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
        Sleep (150)
        Call IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
    End If
End Sub

Public Sub fnBeepOk()
    Dim beeperType As IBSU_BeeperType
    
    If IBSU_GetOperableBeeper(m_nDevHandle, beeperType) <> IBSU_STATUS_OK Then
        Call Beep(3500, 100)
    Else
        Call IBSU_SetBeeper(m_nDevHandle, ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
    End If
End Sub

Public Sub fnBeepDeviceCommunicationBreak()
    Dim i As Integer
    
    For i = 0 To 7
        Call Beep(3500, 100)
        Sleep (100)
    Next
End Sub

Public Sub fnSaveBitmapImage(ByVal fingerName As String)
    
    If (IsNull(m_ImgSaveFolder)) Or (IsNull(m_ImgSaveFolder)) Or _
       (m_ImgSaveFolder = "") Or (m_ImgSubFolder = "") Then
        Exit Sub
    End If
  
    Dim strFolder As String
    strFolder = m_ImgSaveFolder + "\" + m_ImgSubFolder + "\"
    
    If Dir(strFolder, vbDirectory) = "" Then
        Set FileSystem = CreateObject("Scripting.FileSystemObject")
        If FileSystem.FolderExists(strFolder) = False Then
            FileSystem.CreateFolder (strFolder)
            Set FileSystem = Nothing
        End If
    End If
    
    Dim strFileName As String
    Dim info As CaptureInfo
    info = m_vecCaptureSeq(m_nCurrentCaptureStep)
    strFileName = strFolder + "Image_" & m_nCurrentCaptureStep & "_" + fingerName + ".bmp"

    If IBSU_SaveBitmapImage(strFileName, m_ImageData.Buffer, m_ImageData.width, m_ImageData.height, m_ImageData.pitch, m_ImageData.ResolutionX, m_ImageData.ResolutionY) <> IBSU_STATUS_OK Then
        MsgBox "Failed to save bitmap image!"
    End If
End Sub

Public Sub fnSaveWsqImage(ByVal fingerName As String)
    
    If (IsNull(m_ImgSaveFolder)) Or (IsNull(m_ImgSaveFolder)) Or _
       (m_ImgSaveFolder = "") Or (m_ImgSubFolder = "") Then
        Exit Sub
    End If
  
    Dim strFolder As String
    strFolder = m_ImgSaveFolder + "\" + m_ImgSubFolder + "\"
    
    If Dir(strFolder, vbDirectory) = "" Then
        Set FileSystem = CreateObject("Scripting.FileSystemObject")
        If FileSystem.FolderExists(strFolder) = False Then
            FileSystem.CreateFolder (strFolder)
            Set FileSystem = Nothing
        End If
    End If
    
    Dim strFileName As String
    Dim info As CaptureInfo
    info = m_vecCaptureSeq(m_nCurrentCaptureStep)
    strFileName = strFolder + "Image_" & m_nCurrentCaptureStep & "_" + fingerName + ".wsq"

    If IBSU_WSQEncodeToFile(strFileName, m_ImageData.Buffer, m_ImageData.width, m_ImageData.height, m_ImageData.pitch, m_ImageData.bitsPerPixel, m_ImageData.ResolutionX, 0.75, "") <> IBSU_STATUS_OK Then
        MsgBox "Failed to save bitmap image!"
    End If

    '/***********************************************************
    ' * Example codes for WSQ encoding based on memory

    Dim filename As String
    Dim pCompressedData As Long
    Dim pDecompressedData As Long
    Dim pDecompressedData2 As Long
    Dim compressedLength As Long

    If IBSU_WSQEncodeMem(m_ImageData.Buffer, _
                            m_ImageData.width, m_ImageData.height, m_ImageData.pitch, m_ImageData.bitsPerPixel, _
                            m_ImageData.ResolutionX, 0.75, "", _
                            pCompressedData, compressedLength) <> IBSU_STATUS_OK Then
        MsgBox "Failed to save WSQ_1 image!"
    End If

    Dim compressedBuffer() As Byte
    ReDim compressedBuffer(compressedLength - 1)
    CopyMemory compressedBuffer(0), ByVal pCompressedData, compressedLength
    
    Dim fp As Long
    
    fp = FreeFile
    filename = strFolder + "Image_" & m_nCurrentCaptureStep & "_" + fingerName + "_v1.wsq"
    Open filename For Binary As #fp
    Put #fp, , compressedBuffer()
    Close #fp
    
    Dim width, height, pitch, bitsPerPixel, pixelPerInch As Long
    If IBSU_WSQDecodeMem(pCompressedData, compressedLength, _
                               pDecompressedData, width, height, _
                               pitch, bitsPerPixel, pixelPerInch) <> IBSU_STATUS_OK Then
        MsgBox "Failed to Decode WSQ image!"
    End If

    filename = strFolder + "Image_" & m_nCurrentCaptureStep & "_" + fingerName + "_v1.bmp"
    If IBSU_SaveBitmapImage(filename, pDecompressedData, _
                                  width, height, pitch, _
                                  pixelPerInch, pixelPerInch) <> IBSU_STATUS_OK Then
        MsgBox "Failed to save bitmap v1 image!"
    End If

    If IBSU_WSQDecodeFromFile(strFileName, pDecompressedData2, _
                                    width, height, pitch, _
                                    pixelPerInch, pixelPerInch) <> IBSU_STATUS_OK Then
        MsgBox "Failed to Decode WSQ image!"
    End If

    filename = strFolder + "Image_" & m_nCurrentCaptureStep & "_" + fingerName + "_v2.bmp"
    If IBSU_SaveBitmapImage(filename, pDecompressedData2, _
                                  width, height, pitch, _
                                  pixelPerInch, pixelPerInch) <> IBSU_STATUS_OK Then
        MsgBox "Failed to save bitmap v2 image!"
    End If

    IBSU_FreeMemory (pCompressedData)
    IBSU_FreeMemory (pDecompressedData)
    IBSU_FreeMemory (pDecompressedData2)
    '***********************************************************/
End Sub

Public Sub fnSavePngImage(ByVal fingerName As String)
    
    If (IsNull(m_ImgSaveFolder)) Or (IsNull(m_ImgSaveFolder)) Or _
       (m_ImgSaveFolder = "") Or (m_ImgSubFolder = "") Then
        Exit Sub
    End If
  
    Dim strFolder As String
    strFolder = m_ImgSaveFolder + "\" + m_ImgSubFolder + "\"
    
    If Dir(strFolder, vbDirectory) = "" Then
        Set FileSystem = CreateObject("Scripting.FileSystemObject")
        If FileSystem.FolderExists(strFolder) = False Then
            FileSystem.CreateFolder (strFolder)
            Set FileSystem = Nothing
        End If
    End If
    
    Dim strFileName As String
    Dim info As CaptureInfo
    info = m_vecCaptureSeq(m_nCurrentCaptureStep)
    strFileName = strFolder + "Image_" & m_nCurrentCaptureStep & "_" + fingerName + ".png"

    If IBSU_SavePngImage(strFileName, m_ImageData.Buffer, m_ImageData.width, m_ImageData.height, m_ImageData.pitch, m_ImageData.ResolutionX, m_ImageData.ResolutionY) <> IBSU_STATUS_OK Then
        MsgBox "Failed to png bitmap image!"
    End If
End Sub

Public Sub fnSaveJP2Image(ByVal fingerName As String)
    
    If (IsNull(m_ImgSaveFolder)) Or (IsNull(m_ImgSaveFolder)) Or _
       (m_ImgSaveFolder = "") Or (m_ImgSubFolder = "") Then
        Exit Sub
    End If
  
    Dim strFolder As String
    strFolder = m_ImgSaveFolder + "\" + m_ImgSubFolder + "\"
    
    If Dir(strFolder, vbDirectory) = "" Then
        Set FileSystem = CreateObject("Scripting.FileSystemObject")
        If FileSystem.FolderExists(strFolder) = False Then
            FileSystem.CreateFolder (strFolder)
            Set FileSystem = Nothing
        End If
    End If
    
    Dim strFileName As String
    Dim info As CaptureInfo
    info = m_vecCaptureSeq(m_nCurrentCaptureStep)
    strFileName = strFolder + "Image_" & m_nCurrentCaptureStep & "_" + fingerName + ".jp2"

    If IBSU_SaveJP2Image(strFileName, m_ImageData.Buffer, m_ImageData.width, m_ImageData.height, m_ImageData.pitch, m_ImageData.ResolutionX, m_ImageData.ResolutionY, 80) <> IBSU_STATUS_OK Then
        MsgBox "Failed to save jpeg-2000 image!"
    End If
End Sub

Public Sub fnSetLEDs(ByVal deviceHandle As Integer, ByVal fingerName As String, ByVal ImageType As IBSU_ImageType, ByVal ledColor As Integer, ByVal bBlink As Boolean)
    Dim setLEDs As Long

    If m_LedType = ENUM_IBSU_LED_TYPE_FSCAN Then
        If bBlink Then
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_BLINK_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_BLINK_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_BLINK_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_BLINK_RED
            End If
        End If
        
        If ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_ROLL
        End If
    
        If fingerName = "SFF_Right_Thumb" Or fingerName = "SRF_Right_Thumb" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_TWO_THUMB
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_RED
            End If
        ElseIf fingerName = "SFF_Left_Thumb" Or fingerName = "SRF_Left_Thumb" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_TWO_THUMB
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_RED
            End If
        ElseIf fingerName = "TFF_2_Thumbs" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_TWO_THUMB
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_THUMB_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_THUMB_RED
            End If
            '''////////////////LEFT HAND////////////////////
        ElseIf fingerName = "SFF_Left_Index" Or fingerName = "SRF_Left_Index" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_LEFT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_RED
            End If
        ElseIf fingerName = "SFF_Left_Middle" Or fingerName = "SRF_Left_Middle" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_LEFT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_RED
            End If
        ElseIf fingerName = "SFF_Left_Ring" Or fingerName = "SRF_Left_Ring" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_LEFT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_RED
            End If
        ElseIf fingerName = "SFF_Left_Little" Or fingerName = "SRF_Left_Little" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_LEFT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_RED
            End If
        ElseIf fingerName = "4FF_Left_4_Fingers" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_LEFT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_RED
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_RED
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_RED
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_INDEX_RED
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_MIDDLE_RED
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_RING_RED
                setLEDs = setLEDs Or IBSU_LED_F_LEFT_LITTLE_RED
            End If
            '''////////RIGHT HAND /////////////////////////
        ElseIf fingerName = "SFF_Right_Index" Or fingerName = "SRF_Right_Index" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_RIGHT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_RED
            End If
        ElseIf fingerName = "SFF_Right_Middle" Or fingerName = "SRF_Right_Middle" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_RIGHT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_RED
            End If
        ElseIf fingerName = "SFF_Right_Ring" Or fingerName = "SRF_Right_Ring" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_RIGHT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_RED
            End If
        ElseIf fingerName = "SFF_Right_Little" Or fingerName = "SRF_Right_Little" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_RIGHT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_RED
            End If
        ElseIf fingerName = "4FF_Right_4_Fingers" Then
            setLEDs = setLEDs Or IBSU_LED_F_PROGRESS_RIGHT_HAND
            If ledColor = LED_COLOR_GREEN Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_GREEN
            ElseIf ledColor = LED_COLOR_RED Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_RED
            ElseIf ledColor = LED_COLOR_YELLOW Then
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_GREEN
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_INDEX_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_MIDDLE_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_RING_RED
                setLEDs = setLEDs Or IBSU_LED_F_RIGHT_LITTLE_RED
            End If
        End If
        
        If ledColor = LED_COLOR_NONE Then
            setLEDs = 0
        End If
        
        Call IBSU_SetLEDs(deviceHandle, setLEDs)
    End If
End Sub





'////////////////////////////////////////////////////////////////////////////////////////////
Public Sub OnMsg_CaptureSeqStart(ByVal wParam As Long, ByVal lParam As Long)
    If m_nDevHandle = -1 Then
        Call OnMsg_UpdateDisplayResources(0, 0)
        Exit Sub
    End If
    
    Dim strCaptureSeq As String
    Dim nSelectedSeq As Integer
    Dim pos As Integer
    
    nSelectedSeq = m_cboCaptureSeq.ListIndex
    If nSelectedSeq > -1 Then
        strCaptureSeq = m_cboCaptureSeq.text
    End If
    
    Dim info As CaptureInfo

    pos = 0
    If strCaptureSeq = CAPTURE_SEQ_FLAT_SINGLE_FINGER Then
        ReDim m_vecCaptureSeq(0) As CaptureInfo
        pos = 0
        info.PreCaptureMessage = "Please put a single finger on the sensor!"
        info.PostCaptuerMessage = "Keep finger on the sensor!"
        info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER
        info.NumberOfFinger = 1
        info.fingerName = "SFF_Unknown"
        m_vecCaptureSeq(pos) = info
    End If
    
    If strCaptureSeq = CAPTURE_SEQ_ROLL_SINGLE_FINGER Then
        ReDim m_vecCaptureSeq(0) As CaptureInfo
        pos = 0
        info.PreCaptureMessage = "Please put a single finger on the sensor!"
        info.PostCaptuerMessage = "Roll finger!"
        info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER
        info.NumberOfFinger = 1
        info.fingerName = "SRF_Unknown"
        m_vecCaptureSeq(pos) = info
    End If

    If strCaptureSeq = CAPTURE_SEQ_2_FLAT_FINGERS Then
        ReDim m_vecCaptureSeq(0) As CaptureInfo
        pos = 0
        info.PreCaptureMessage = "Please put two fingers on the sensor!"
        info.PostCaptuerMessage = "Keep fingers on the sensor!"
        info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS
        info.NumberOfFinger = 2
        info.fingerName = "TFF_Unknown"
        m_vecCaptureSeq(pos) = info
    End If

    If strCaptureSeq = CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS Then
        ReDim m_vecCaptureSeq(0 To 9) As CaptureInfo
        pos = 0
        info.PreCaptureMessage = "Please put right thumb on the sensor!"
        info.fingerName = "SFF_Right_Thumb"
        info.PostCaptuerMessage = "Keep fingers on the sensor!"
        info.ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER
        info.NumberOfFinger = 1
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right index on the sensor!"
        info.fingerName = "SFF_Right_Index"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right middle on the sensor!"
        info.fingerName = "SFF_Right_Middle"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right ring on the sensor!"
        info.fingerName = "SFF_Right_Ring"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right little on the sensor!"
        info.fingerName = "SFF_Right_Little"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left thumb on the sensor!"
        info.fingerName = "SFF_Left_Thumb"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left index on the sensor!"
        info.fingerName = "SFF_Left_Index"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left middle on the sensor!"
        info.fingerName = "SFF_Left_Middle"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left ring on the sensor!"
        info.fingerName = "SFF_Left_Ring"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left little on the sensor!"
        info.fingerName = "SFF_Left_Little"
        m_vecCaptureSeq(pos) = info
    End If

    If strCaptureSeq = CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS Then
        ReDim m_vecCaptureSeq(0 To 9) As CaptureInfo
        pos = 0
        info.PreCaptureMessage = "Please put right thumb on the sensor!"
        info.PostCaptuerMessage = "Roll finger!"
        info.ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER
        info.NumberOfFinger = 1
        info.fingerName = "SRF_Right_Thumb"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right index on the sensor!"
        info.fingerName = "SRF_Right_Index"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right middle on the sensor!"
        info.fingerName = "SRF_Right_Middle"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right ring on the sensor!"
        info.fingerName = "SRF_Right_Ring"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put right little on the sensor!"
        info.fingerName = "SRF_Right_Little"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left thumb on the sensor!"
        info.fingerName = "SRF_Left_Thumb"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left index on the sensor!"
        info.fingerName = "SRF_Left_Index"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left middle on the sensor!"
        info.fingerName = "SRF_Left_Middle"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left ring on the sensor!"
        info.fingerName = "SRF_Left_Ring"
        m_vecCaptureSeq(pos) = info

        info.PreCaptureMessage = "Please put left little on the sensor!"
        info.fingerName = "SRF_Left_Little"
        m_vecCaptureSeq(pos) = info
    End If

    If strCaptureSeq = CAPTURE_SEQ_4_FLAT_FINGERS Then
        ReDim m_vecCaptureSeq(0) As CaptureInfo
        pos = 0
        info.PreCaptureMessage = "Please put 4 fingers on the sensor!"
        info.PostCaptuerMessage = "Keep fingers on the sensor!"
        info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS
        info.NumberOfFinger = 4
        info.fingerName = "4FF_Unknown"
        m_vecCaptureSeq(pos) = info
    End If

    If strCaptureSeq = CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER Then
        ReDim m_vecCaptureSeq(0 To 2) As CaptureInfo
        pos = 0
        info.PreCaptureMessage = "Please put right 4-fingers on the sensor!"
        info.fingerName = "4FF_Right_4_Fingers"
        info.PostCaptuerMessage = "Keep fingers on the sensor!"
        info.ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS
        info.NumberOfFinger = 4
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put left 4-fingers on the sensor!"
        info.fingerName = "4FF_Left_4_Fingers"
        m_vecCaptureSeq(pos) = info

        pos = pos + 1
        info.PreCaptureMessage = "Please put 2-thumbs on the sensor!"
        info.fingerName = "TFF_2_Thumbs"
        info.ImageType = ENUM_IBSU_FLAT_TWO_FINGERS
        info.NumberOfFinger = 2
        m_vecCaptureSeq(pos) = info
    End If

    '// Make subfolder name
    m_ImgSubFolder = Format(Now, "yyyy-MM-dd HHmmss")
  
   Call OnMsg_CaptureSeqNext(0, 0)
End Sub

Public Sub OnMsg_CaptureSeqNext(ByVal wParam As Long, ByVal lParam As Long)
    Dim i, nRc As Long
    Dim info As CaptureInfo
    Dim imgRes As IBSU_ImageResolution
    Dim strMessage As String
    

    m_bEnable_BImage_Timer = False
    
    If m_nDevHandle = -1 Then
        Exit Sub
    End If
    
    m_bBlank = False
    For i = 0 To IBSU_MAX_SEGMENT_COUNT - 1
        m_FingerQuality(i) = ENUM_IBSU_FINGER_NOT_PRESENT
    Next

    m_nCurrentCaptureStep = m_nCurrentCaptureStep + 1
    If m_nCurrentCaptureStep >= UBound(m_vecCaptureSeq) - LBound(m_vecCaptureSeq) + 1 Then
        '// All of capture sequence completely
        Call fnSetLEDs(m_nDevHandle, "", ENUM_IBSU_TYPE_NONE, LED_COLOR_NONE, False)
        m_nCurrentCaptureStep = -1
        m_ImgSubFolder = ""

        Call OnMsg_UpdateDisplayResources(0, 0)
        m_btnCaptureStart.SetFocus
        Exit Sub
    End If

    If m_chkInvalidArea.Value Then
        Call IBSU_SetClientDisplayProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "TRUE")
    Else
        Call IBSU_SetClientDisplayProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, "FALSE")
    End If

    If m_chkDetectSmear.Value Then
        Call IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_ROLL_MODE, "1")
        Dim strProperty As String
        strProperty = m_cboSmearLevel.ListIndex
        Call IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_ROLL_LEVEL, strProperty)
    Else
        Call IBSU_SetProperty(m_nDevHandle, ENUM_IBSU_PROPERTY_ROLL_MODE, "0")
    End If
    
    For i = 0 To IBSU_MAX_SEGMENT_COUNT - 1
        Call IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle(i), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    Next

    '// Make capture delay for display result image on multi capture mode (500 ms)
    If m_nCurrentCaptureStep > 0 Then
        Sleep 500
        m_strImageMessage = ""
    End If

    info = m_vecCaptureSeq(m_nCurrentCaptureStep)

    imgRes = ENUM_IBSU_IMAGE_RESOLUTION_500
    Dim bAvailable As Long
    bAvailable = 0
    nRc = IBSU_IsCaptureAvailable(m_nDevHandle, info.ImageType, imgRes, bAvailable)
    If (nRc <> IBSU_STATUS_OK) Or (bAvailable <> 1) Then
        strMessage = "The capture mode (" & info.ImageType & ") is not available"
        Call fnSetStatusBarMessage(strMessage)
        m_nCurrentCaptureStep = -1
        Call OnMsg_UpdateDisplayResources(0, 0)
        Exit Sub
    End If

    '// Start capture
    Dim captureOptions As Long
    captureOptions = 0
    If m_chkAutoContrast.Value Then
        captureOptions = captureOptions Or IBSU_OPTION_AUTO_CONTRAST
    End If
    
    If m_chkAutoCapture.Value Then
        captureOptions = captureOptions Or IBSU_OPTION_AUTO_CAPTURE
    End If
    
    If m_chkIgnoreFingerCount.Value Then
        captureOptions = captureOptions Or IBSU_OPTION_IGNORE_FINGER_COUNT
    End If
    
    nRc = IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions)
    If nRc >= IBSU_STATUS_OK Then
        '// Display message for image acuisition
        strMessage = info.PreCaptureMessage

        Call fnSetStatusBarMessage(strMessage)
        If m_chkAutoCapture.Value <> 1 Then
            strMessage = strMessage + "\r\nPress button 'Take Result Image' when image is good!"
        End If

        Call fnSetImageMessage(strMessage)
        m_strImageMessage = strMessage

        Dim activeLEDs As Long
        Call fnSetLEDs(m_nDevHandle, info.fingerName, info.ImageType, LED_COLOR_RED, True)
    Else
        Call fnSetStatusBarMessage("Failed to execut IBSU_BeginCaptureImage()")
    End If

    Call OnMsg_UpdateDisplayResources(0, 0)
    
    m_bEnable_BImage_Timer = True
End Sub

Public Sub OnMsg_DeviceCommunicationBreak(ByVal wParam As Long, ByVal lParam As Long)
    Dim nRc As Long
    

    If m_nDevHandle = -1 Then
        Exit Sub
    End If

    Call fnSetStatusBarMessage("Device communication was broken")

    nRc = fnReleaseDevice()
    If nRc = IBSU_ERR_RESOURCE_LOCKED Then
        '// retry to release device
        Call OnMsg_DeviceCommunicationBreak(0, 0)
    Else
        Call OnMsg_Beep(BEEP_DEVICE_COMMUNICATION_BREAK, 0)
        Call OnMsg_UpdateDeviceList(0, 0)
    End If
End Sub

Public Sub OnMsg_InitWarning(ByVal wParam As Long, ByVal lParam As Long)

End Sub

Public Sub OnMsg_UpdateDeviceList(ByVal wParam As Long, ByVal lParam As Long)
    Dim idle As Boolean
    Dim strSelectedText As String
    Dim i, selectedDev As Integer
    Dim devices As Long
'    Dim devDesc As IBSU_DeviceDesc
    
    
    idle = Not m_bInitializing And (m_nCurrentCaptureStep = -1)
    If idle Then
        m_btnCaptureStop.Enabled = False
        m_btnCaptureStart.Enabled = False
    End If

    '// store currently selected device
    selectedDev = m_cboUsbDevices.ListIndex
    
    If selectedDev > -1 Then
        strSelectedText = m_cboUsbDevices.text
    End If

    m_cboUsbDevices.Clear
    m_cboUsbDevices.AddItem ("- Please select -")

    '// populate combo box
    devices = 0
    Call IBSU_GetDeviceCount(devices)       'You have to send pointer of varient

    selectedDev = 0
    For i = 0 To devices - 1
        Dim devDesc As IBSU_DeviceDesc
        If IBSU_GetDeviceDescription(i, devDesc) < IBSU_STATUS_OK Then
            GoTo CONT
        End If
        
        Dim strDevice As String
        If fnByteToStr(devDesc.productName) = "" Then
            strDevice = "unknown device"
        Else
            strDevice = fnByteToStr(devDesc.productName) & "_v" & fnByteToStr(devDesc.fwVersion) & _
            " (" & fnByteToStr(devDesc.serialNumber) & ")"
        End If
        
        m_cboUsbDevices.AddItem (strDevice)
        If strDevice = strSelectedText Then
            selectedDev = i + 1
        End If
CONT:
    Next

    If (selectedDev = 0) And (m_cboUsbDevices.ListCount = 2) Then
        selectedDev = 1
    End If

    m_cboUsbDevices.ListIndex = selectedDev

    If idle Then
        Call m_cboUsbDevices_Click
        Call fnUpdateCaptureSequences
    End If
End Sub

Public Sub OnMsg_UpdateDisplayResources(ByVal wParam As Long, ByVal lParam As Long)
    Dim selectedDev As Boolean
    Dim captureSeq As Boolean
    Dim idle As Boolean
    Dim active As Boolean
    Dim uninitializedDev As Boolean
    
    
    selectedDev = (m_cboUsbDevices.ListIndex > 0)
    captureSeq = (m_cboCaptureSeq.ListIndex > 0)
    idle = Not (m_bInitializing) And (m_nCurrentCaptureStep = -1)
    active = Not (m_bInitializing) And (m_nCurrentCaptureStep <> -1)
    uninitializedDev = (selectedDev) And (m_nDevHandle = -1)


    m_cboUsbDevices.Enabled = idle
    m_cboCaptureSeq.Enabled = (selectedDev And idle)

    m_btnCaptureStart.Enabled = captureSeq
    m_btnCaptureStop.Enabled = active
    
'    m_chkAutoContrast.Enabled = (selectedDev And idle)
    m_chkAutoCapture.Enabled = (selectedDev And idle)
    m_chkIgnoreFingerCount.Enabled = (selectedDev And idle)
'    m_chkAutoContrast.Enabled = (selectedDev And idle)
    m_chkSaveImages.Enabled = (selectedDev And idle)
    m_btnImageFolder.Enabled = (selectedDev And idle)
    
    m_chkUseClearPlaten.Enabled = uninitializedDev

    Dim strCaption As String
    If active Then
        strCaption = "Take Result Image"
    ElseIf Not (active) And Not (m_bInitializing) Then
        strCaption = "Start"
    End If
    
    m_btnCaptureStart.Caption = strCaption
End Sub

Public Sub OnMsg_Beep(ByVal wParam As Long, ByVal lParam As Long)
    Dim Beep As Long
    
    Beep = wParam

    If Beep = BEEP_FAIL Then
        Call fnBeepFail
    ElseIf Beep = BEEP_SUCCESS Then
        Call fnBeepSuccess
    ElseIf Beep = BEEP_OK Then
        Call fnBeepOk
    ElseIf Beep = BEEP_DEVICE_COMMUNICATION_BREAK Then
        Call fnBeepDeviceCommunicationBreak
    End If
End Sub

Public Sub OnMsg_DrawFingerQuality(ByVal wParam As Long, ByVal lParam As Long)
    If m_nDevHandle = -1 Then
        Exit Sub
    End If
End Sub

Public Sub fnWaitingForFinishInitDevice(ByVal devIndex As Long)
    Dim nRc As Long
    Dim IsComplete As Long
    Dim devHandle As Long
    Dim progressValue As Long
    Dim clientRect As IBSU_RECT
    Dim strMessage As String
    Dim count As Integer
    Dim ledCount As Long
    Dim operableLEDs As Long

    nRc = IBSU_STATUS_OK
    count = 0
    Do While nRc = IBSU_STATUS_OK
        nRc = IBSU_BGetInitProgress(devIndex, IsComplete, devHandle, progressValue)
        If nRc <> IBSU_STATUS_OK Or IsComplete Then
            Exit Do
        End If
        
        strMessage = "Initializing device..." & progressValue & "%"
        Call fnSetStatusBarMessage(strMessage)
        DoEvents
        Sleep (50)
    Loop

    If nRc = IBSU_STATUS_OK Then
        m_nDevHandle = devHandle
        
        Call IBSU_GetOperableLEDs(devHandle, m_LedType, ledCount, operableLEDs)

        Call GetClientRect(m_ImageFrame.hwnd, clientRect)

        '// set display window
        Call IBSU_CreateClientWindow(devHandle, Form1.m_ImageFrame.hwnd, clientRect.Left, clientRect.Top, clientRect.Right, clientRect.Bottom)
        Call IBSU_AddOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        Call IBSU_AddOverlayText(m_nDevHandle, m_nOvImageTextHandle, "Arial", 10, 1, "", 10, 10, 0)
        For i = 0 To IBSU_MAX_SEGMENT_COUNT - 1
            Call IBSU_AddOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle(i), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        Next
    End If

    '// status notification and sequence start
    If nRc = IBSU_STATUS_OK Then
        Call OnMsg_CaptureSeqStart(0, 0)
        Exit Sub
    End If

    If nRc <> IBSU_STATUS_OK Then
        strMessage = "[Error code = " & nRc & "] Device initialization fail"
        Call fnSetStatusBarMessage(strMessage)
    End If

    Call OnMsg_UpdateDisplayResources(0, 0)
End Sub

Public Sub fnDrawQuality()
    If m_bNeedClearPlaten Then
        If m_bBlank Then
            Shape_Quality_1.BackColor = s_brushRed
            Shape_Quality_2.BackColor = s_brushRed
            Shape_Quality_3.BackColor = s_brushRed
            Shape_Quality_4.BackColor = s_brushRed
        Else
            Shape_Quality_1.BackColor = s_brushPlaten
            Shape_Quality_2.BackColor = s_brushPlaten
            Shape_Quality_3.BackColor = s_brushPlaten
            Shape_Quality_4.BackColor = s_brushPlaten
        End If
    Else
        Shape_Quality_1.BackColor = s_brushPlaten
        If m_FingerQuality(0) = ENUM_IBSU_QUALITY_POOR Then
            Shape_Quality_1.BackColor = s_brushRed
        ElseIf m_FingerQuality(0) = ENUM_IBSU_QUALITY_FAIR Then
            Shape_Quality_1.BackColor = s_brushOrange
        ElseIf m_FingerQuality(0) = ENUM_IBSU_QUALITY_GOOD Then
            Shape_Quality_1.BackColor = s_brushGreen
        ElseIf m_FingerQuality(0) = ENUM_IBSU_QUALITY_INVALID_AREA_TOP Then
            Shape_Quality_1.BackColor = s_brushRed
        ElseIf m_FingerQuality(0) = ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM Then
            Shape_Quality_1.BackColor = s_brushRed
        ElseIf m_FingerQuality(0) = ENUM_IBSU_QUALITY_INVALID_AREA_LEFT Then
            Shape_Quality_1.BackColor = s_brushRed
        ElseIf m_FingerQuality(0) = ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT Then
            Shape_Quality_1.BackColor = s_brushRed
        End If
        
        Shape_Quality_2.BackColor = s_brushPlaten
        If m_FingerQuality(1) = ENUM_IBSU_QUALITY_POOR Then
            Shape_Quality_2.BackColor = s_brushRed
        ElseIf m_FingerQuality(1) = ENUM_IBSU_QUALITY_FAIR Then
            Shape_Quality_2.BackColor = s_brushOrange
        ElseIf m_FingerQuality(1) = ENUM_IBSU_QUALITY_GOOD Then
            Shape_Quality_2.BackColor = s_brushGreen
        ElseIf m_FingerQuality(1) = ENUM_IBSU_QUALITY_INVALID_AREA_TOP Then
            Shape_Quality_2.BackColor = s_brushRed
        ElseIf m_FingerQuality(1) = ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM Then
            Shape_Quality_2.BackColor = s_brushRed
        ElseIf m_FingerQuality(1) = ENUM_IBSU_QUALITY_INVALID_AREA_LEFT Then
            Shape_Quality_2.BackColor = s_brushRed
        ElseIf m_FingerQuality(1) = ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT Then
            Shape_Quality_2.BackColor = s_brushRed
        End If
        
        Shape_Quality_3.BackColor = s_brushPlaten
        If m_FingerQuality(2) = ENUM_IBSU_QUALITY_POOR Then
            Shape_Quality_3.BackColor = s_brushRed
        ElseIf m_FingerQuality(2) = ENUM_IBSU_QUALITY_FAIR Then
            Shape_Quality_3.BackColor = s_brushOrange
        ElseIf m_FingerQuality(2) = ENUM_IBSU_QUALITY_GOOD Then
            Shape_Quality_3.BackColor = s_brushGreen
        ElseIf m_FingerQuality(2) = ENUM_IBSU_QUALITY_INVALID_AREA_TOP Then
            Shape_Quality_3.BackColor = s_brushRed
        ElseIf m_FingerQuality(2) = ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM Then
            Shape_Quality_2.BackColor = s_brushRed
        ElseIf m_FingerQuality(2) = ENUM_IBSU_QUALITY_INVALID_AREA_LEFT Then
            Shape_Quality_3.BackColor = s_brushRed
        ElseIf m_FingerQuality(2) = ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT Then
            Shape_Quality_3.BackColor = s_brushRed
        End If
        
        Shape_Quality_4.BackColor = s_brushPlaten
        If m_FingerQuality(3) = ENUM_IBSU_QUALITY_POOR Then
            Shape_Quality_4.BackColor = s_brushRed
        ElseIf m_FingerQuality(3) = ENUM_IBSU_QUALITY_FAIR Then
            Shape_Quality_4.BackColor = s_brushOrange
        ElseIf m_FingerQuality(3) = ENUM_IBSU_QUALITY_GOOD Then
            Shape_Quality_4.BackColor = s_brushGreen
        ElseIf m_FingerQuality(3) = ENUM_IBSU_QUALITY_INVALID_AREA_TOP Then
            Shape_Quality_4.BackColor = s_brushRed
        ElseIf m_FingerQuality(3) = ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM Then
            Shape_Quality_2.BackColor = s_brushRed
        ElseIf m_FingerQuality(3) = ENUM_IBSU_QUALITY_INVALID_AREA_LEFT Then
            Shape_Quality_4.BackColor = s_brushRed
        ElseIf m_FingerQuality(3) = ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT Then
            Shape_Quality_4.BackColor = s_brushRed
        End If
    End If

End Sub

Private Function fnExistFolder(FolderPath As String)
     If LenB(Dir$(FolderPath, vbDirectory)) Then
          fnExistFolder = 1&
     Else
          fnExistFolder = 0&
     End If
End Function
 







Private Sub Form_Load()
    Dim titleName, logoName As String
    
    m_nDevHandle = -1
    m_nCurrentCaptureStep = -1
    m_bInitializing = False
    m_strImageMessage = ""
    m_bNeedClearPlaten = False

    m_chkAutoContrast.Value = 1
    m_chkAutoCapture.Value = 1
    m_chkUseClearPlaten.Value = 1
    m_chkNFIQScore.Value = 1
    m_chkDrawSegmentImage.Value = 1
    m_chkInvalidArea.Value = 0
    m_chkDetectSmear = 1
    
    m_ImgSaveFolder = ""

'    m_sliderContrast.Min = IBSU_MIN_CONTRAST_VALUE
'    m_sliderContrast.Max = IBSU_MAX_CONTRAST_VALUE
'    m_sliderContrast.Value = IBSU_MIN_CONTRAST_VALUE
'    m_sliderContrast.TickFrequency = 5
'    m_txtContrast.Caption = 0
    
    m_bEnable_BImage_Timer = False
    
    Shape_Quality_1.BackColor = s_brushPlaten
    Shape_Quality_2.BackColor = s_brushPlaten
    Shape_Quality_3.BackColor = s_brushPlaten
    Shape_Quality_4.BackColor = s_brushPlaten
    
    Call IBSU_EnableTraceLog(True)
    Call IBSU_SetProperty(1, ENUM_IBSU_PROPERTY_LOG_SERVER_INFO, "1|127.0.0.1|22000")

    Call IBSU_GetSDKVersion(m_verInfo)
    titleName = Format("IntegrationSample for VB6.0")
    Form1.Caption = titleName
    
    logoName = "VB6.0 sample with DLL ver. " + fnByteToStr(m_verInfo.Product)
    m_txtNameOnIBLogo.Caption = logoName
    
    m_cboSmearLevel.Clear
    m_cboSmearLevel.AddItem ("LOW")
    m_cboSmearLevel.AddItem ("MEDIUM")
    m_cboSmearLevel.AddItem ("HIGH")
    m_cboSmearLevel.ListIndex = 1

    Call IBSU_GetDeviceCount(m_nDeviceCount)
    
    Call OnMsg_UpdateDeviceList(0, 0)
    

End Sub

Private Sub Form_Unload(Cancel As Integer)
    Call fnReleaseDevice
End Sub

Private Sub m_btnCaptureStart_Click()
    If m_bInitializing Then
        Exit Sub
    End If

    Dim devIndex As Integer
    devIndex = m_cboUsbDevices.ListIndex - 1
    If devIndex < 0 Then
        Exit Sub
    End If
    
    Dim IsActive As Long
    Dim nRc As Long

    If m_nCurrentCaptureStep <> -1 Then
        nRc = IBSU_IsCaptureActive(m_nDevHandle, IsActive)
        If (nRc = IBSU_STATUS_OK) And (IsActive = 1) Then
            Call IBSU_TakeResultImageManually(m_nDevHandle)
        End If

        Exit Sub
    End If

    If m_nDevHandle = -1 Then
        m_bInitializing = True
        '// Asynchronously Open Device
        '// Function IBSU_OpenDevice() will not be blocked while initializing the algorithm (4 - 7seconds).
        '// And you can start the capture after getting callback funcation IBSU_CallbackAsyncOpenDevice().
        nRc = IBSU_AsyncOpenDevice(devIndex)
        m_bInitializing = False
        
        If nRc >= IBSU_STATUS_OK Then
            Call fnWaitingForFinishInitDevice(devIndex)
        Else
            Dim strMessage As String
            strMessage = "[Error code = " & nRc & "] Device initialization fail"
            Call fnSetStatusBarMessage(strMessage)
        End If
    Else
        '// device already initialized
        Call OnMsg_CaptureSeqStart(0, 0)
    End If

    Call OnMsg_UpdateDisplayResources(0, 0)

End Sub

Private Sub m_btnCaptureStop_Click()
    If m_nDevHandle = -1 Then
        Exit Sub
    End If

    Call IBSU_CancelCaptureImage(m_nDevHandle)
    Call fnSetLEDs(m_nDevHandle, "", ENUM_IBSU_TYPE_NONE, LED_COLOR_NONE, False)
    m_nCurrentCaptureStep = -1
    m_bNeedClearPlaten = False
    m_bBlank = False

    Call fnSetStatusBarMessage("Capture Sequence aborted")
    m_strImageMessage = ""
    Call fnSetImageMessage("")
    Call OnMsg_UpdateDisplayResources(0, 0)

End Sub

Private Sub m_btnImageFolder_Click()
    '// Select folder
    Dim lpIDList As Long
    Dim sBuffer As String
    Dim szTitle As String
    Dim tBrowseInfo As BrowseInfo
    Dim IsAdminRequired As Long
    
    szTitle = "Please select a folder to store captured images!"
    With tBrowseInfo
        .hWndOwner = Me.hwnd
        .lpszTitle = lstrcat(szTitle, "")
        .ulFlags = BIF_RETURNONLYFSDIRS + BIF_DONTGOBELOWDOMAIN
    End With
    
    lpIDList = SHBrowseForFolder(tBrowseInfo)
    
    If (lpIDList) Then
        sBuffer = Space(MAX_PATH)
        SHGetPathFromIDList lpIDList, sBuffer
        m_ImgSaveFolder = Left(sBuffer, InStr(sBuffer, vbNullChar) - 1)
    End If
    
    If IBSU_IsWritableDirectory(m_ImgSaveFolder, True) <> IBSU_STATUS_OK Then
        MsgBox "You don't have writing permission on this folder\r\nPlease select another folder (e.g. desktop folder)"
    End If

End Sub

Private Sub m_cboUsbDevices_Click()
    If m_cboUsbDevices.ListIndex = m_nSelectedDevIndex Then
        Exit Sub
    End If

    m_nSelectedDevIndex = m_cboUsbDevices.ListIndex
    If m_nDevHandle <> -1 Then
        Call m_btnCaptureStop_Click
        Call fnReleaseDevice
    End If

    Call fnUpdateCaptureSequences

End Sub

Private Sub m_cboCaptureSeq_Click()
    Call OnMsg_UpdateDisplayResources(0, 0)

End Sub

Private Sub m_chkAutoContrast_Click()
'    If m_chkAutoContrast.Value = 1 Then
'        m_sliderContrast.Enabled = False
'        m_txtContrast.Enabled = False
'        m_staticContrast.Enabled = False
'    Else
'        m_sliderContrast.Enabled = True
'        m_txtContrast.Enabled = True
'        m_staticContrast.Enabled = True
'    End If

End Sub

'Private Sub m_sliderContrast_Change()
'    If m_nDevHandle = -1 Then
'        Exit Sub
'    End If
'
'    Dim pos As Integer
'    pos = m_sliderContrast.Value
'    m_txtContrast.Caption = pos
'    Call IBSU_SetContrast(m_nDevHandle, pos)
'
'End Sub

Private Sub Timer_Get_BImage_Timer()
    Dim nRc As Long
    Dim image As IBSU_ImageData
    Dim ImageType As IBSU_ImageType
    Dim m_segmentImageArray(0 To IBSU_MAX_SEGMENT_COUNT - 1) As IBSU_ImageData
    Dim m_segmentPositionArray(0 To IBSU_MAX_SEGMENT_COUNT - 1) As IBSU_SegmentPosition
    Dim m_segmentImageArrayCount As Long
    Dim fingerCountState As IBSU_FingerCountState
    Dim qualityArrayCount As Long
    Dim info As CaptureInfo
    Dim i As Integer
    Dim imageStatus, detectedFingerCount As Long

    
    If m_bEnable_BImage_Timer Then
        nRc = IBSU_BGetImageEx(m_nDevHandle, imageStatus, image, ImageType, detectedFingerCount, _
                             m_segmentImageArray(0), m_segmentPositionArray(0), m_segmentImageArrayCount, _
                             fingerCountState, m_FingerQuality(0), qualityArrayCount)
    
        i = LenB(image)
        If nRc = IBSU_STATUS_OK Then
            If m_saveFingerCountState <> fingerCountState Then
                m_saveFingerCountState = fingerCountState

                info = m_vecCaptureSeq(m_nCurrentCaptureStep)
                If fingerCountState = ENUM_IBSU_NON_FINGER Then
                    Call fnSetLEDs(m_nDevHandle, info.fingerName, info.ImageType, LED_COLOR_RED, True)
                Else
                    Call fnSetLEDs(m_nDevHandle, info.fingerName, info.ImageType, LED_COLOR_YELLOW, True)
                End If
            End If
            
            If image.IsFinal Then
                info = m_vecCaptureSeq(m_nCurrentCaptureStep)
                Call fnSetLEDs(m_nDevHandle, info.fingerName, info.ImageType, LED_COLOR_GREEN, False)

                If imageStatus >= IBSU_STATUS_OK Then
                    Call fnBeepSuccess
                Else
                    Call fnBeepFail
                End If
                
                ' added 2012-11-30
                If m_bNeedClearPlaten Then
                    m_bNeedClearPlaten = False
                End If
                
                '// Image acquisition successful
                '// Drawing finger qualtiy
                Call fnDrawQuality
                
                
                Dim imgTypeName As String
                If ImageType = ENUM_IBSU_ROLL_SINGLE_FINGER Then
                    imgTypeName = "-- Rolling single finger --"
                ElseIf ImageType = ENUM_IBSU_FLAT_SINGLE_FINGER Then
                    imgTypeName = "-- Flat single finger --"
                ElseIf ImageType = ENUM_IBSU_FLAT_TWO_FINGERS Then
                    imgTypeName = "-- Flat two fingers --"
                ElseIf ImageType = ENUM_IBSU_FLAT_FOUR_FINGERS Then
                    imgTypeName = "-- Flat 4 fingers --"
                Else
                    imgTypeName = "-- Unknown --"
                End If
    
                If imageStatus >= IBSU_STATUS_OK Then
                    '// Image acquisition successful
                    If m_chkSaveImages.Value Then
                        Call fnSetStatusBarMessage("Saving image...")
                        info = m_vecCaptureSeq(m_nCurrentCaptureStep)
                        m_ImageData = image
                        Call fnSaveBitmapImage(info.fingerName)
                        Call fnSaveWsqImage(info.fingerName)
                        Call fnSavePngImage(info.fingerName)
                        Call fnSaveJP2Image(info.fingerName)
                        'If m_segmentImageArrayCount > 1 Then
                        If m_segmentImageArrayCount > 0 Then
                            Dim segmentName As String
                            For i = 0 To m_segmentImageArrayCount - 1
                                segmentName = info.fingerName + "_Segment_" & i
                                m_ImageData = m_segmentImageArray(i)
                                Call fnSaveBitmapImage(segmentName)
                                Call fnSaveWsqImage(segmentName)
                                Call fnSavePngImage(segmentName)
                                Call fnSaveJP2Image(segmentName)
                            Next
                        End If
                    End If
                    
                    If m_chkDrawSegmentImage.Value Then
                        '// Draw quadrangle for the segmented image
                        Dim propertyValue(0 To 31) As Byte
                        Dim leftMargin, topMargin As Integer
                        Dim scaleFactor As Double
                        Dim cr As Long
                        Dim x1, x2, x3, x4, y1, y2, y3, y4 As Long
                        
                        Call IBSU_GetClientWindowProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR, propertyValue(0))
                        scaleFactor = Val(fnByteToStr(propertyValue))
                        Call IBSU_GetClientWindowProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN, propertyValue(0))
                        leftMargin = Val(fnByteToStr(propertyValue))
                        Call IBSU_GetClientWindowProperty(m_nDevHandle, ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN, propertyValue(0))
                        topMargin = Val(fnByteToStr(propertyValue))
                        For i = 0 To m_segmentImageArrayCount - 1
                            cr = RGB(0, 128, 0)
                            x1 = leftMargin + CInt(m_segmentPositionArray(i).x1 * scaleFactor)
                            x2 = leftMargin + CInt(m_segmentPositionArray(i).x2 * scaleFactor)
                            x3 = leftMargin + CInt(m_segmentPositionArray(i).x3 * scaleFactor)
                            x4 = leftMargin + CInt(m_segmentPositionArray(i).x4 * scaleFactor)
                            y1 = topMargin + CInt(m_segmentPositionArray(i).y1 * scaleFactor)
                            y2 = topMargin + CInt(m_segmentPositionArray(i).y2 * scaleFactor)
                            y3 = topMargin + CInt(m_segmentPositionArray(i).y3 * scaleFactor)
                            y4 = topMargin + CInt(m_segmentPositionArray(i).y4 * scaleFactor)
    
                            Call IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle(i), _
                                x1, y1, x2, y2, x3, y3, x4, y4, 1, cr)
                        Next
                    End If
                    
                    If m_chkNFIQScore.Value Then
                        Dim nfiq_score(0 To IBSU_MAX_SEGMENT_COUNT) As Long
                        Dim score, segment_pos As Integer
                        Dim strValue As String
                        
                        For i = 0 To IBSU_MAX_SEGMENT_COUNT - 1
                            nfiq_score(i) = 0
                        Next
                        score = 0
                        segment_pos = 0
                        strValue = "Err"
                        Call fnSetStatusBarMessage("Get NFIQ score...")
                        For i = 0 To IBSU_MAX_SEGMENT_COUNT - 1
                            If m_FingerQuality(i) = ENUM_IBSU_FINGER_NOT_PRESENT Then
                                GoTo NextIteration
                            End If
                                
                            nRc = IBSU_GetNFIQScore(deviceHandle, m_segmentImageArray(segment_pos).Buffer, _
                                    m_segmentImageArray(segment_pos).width, m_segmentImageArray(segment_pos).height, _
                                    m_segmentImageArray(segment_pos).bitsPerPixel, score)
                            If nRc = IBSU_STATUS_OK Then
                                nfiq_score(i) = score
                            Else
                                nfiq_score(i) = -1
                            End If
                            
                            segment_pos = segment_pos + 1
                            
NextIteration:
                        Next
                        strValue = nfiq_score(0) & "-" & nfiq_score(1) & "-" & nfiq_score(2) & "-" & nfiq_score(3)
                        m_txtNFIQScore.text = strValue
                    End If
        
                    If imageStatus = IBSU_STATUS_OK Then
                        m_strImageMessage = imgTypeName + " acquisition completed successfully"
                        Call fnSetImageMessage(m_strImageMessage)
                        Call fnSetStatusBarMessage(m_strImageMessage)
                    Else
                        '// > IBSU_STATUS_OK
                        m_strImageMessage = imgTypeName + " acquisition Warning (Warning code = " & imageStatus & ")"
                        Call fnSetImageMessage(m_strImageMessage)
                        Call fnSetStatusBarMessage(m_strImageMessage)
                        
                        Dim askMsg As String
                        askMsg = "[Warning = " & imageStatus & "] Do you want a recapture?"
                        If MsgBox(askMsg, vbYesNo) = vbYes Then
                            '// To recapture current finger position
                            m_nCurrentCaptureStep = m_nCurrentCaptureStep - 1
                        End If
                    End If
                    Call OnMsg_CaptureSeqNext(0, 0)
                End If
            End If
        End If
    End If
    
End Sub

Private Sub Timer_PoolingDevice_Timer()
    '// Detected device count
    Dim devCount As Long
    Dim i As Integer
    
    Call IBSU_GetDeviceCount(devCount)
    If m_nDeviceCount <> devCount Then
        m_nDeviceCount = devCount
        Call OnMsg_UpdateDeviceList(0, 0)
    End If

    '// Is check that device's communication is breaked
    If IBSU_IsDeviceOpened(m_nDevHandle) = IBSU_ERR_DEVICE_INVALID_STATE Then
        Call OnMsg_DeviceCommunicationBreak(0, 0)
    End If

    '// Drawing finger qualtiy
    Call fnDrawQuality
    
    If m_bNeedClearPlaten Then
        m_bBlank = Not m_bBlank
    End If
End Sub
