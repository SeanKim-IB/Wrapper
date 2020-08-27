Imports System.Collections.Generic
Imports System.ComponentModel
Imports System.Data
Imports System.Drawing
Imports System.Text
Imports System.Windows.Forms
Imports System.Threading
Imports System.Runtime.InteropServices
Imports System.Drawing.Drawing2D
Imports System.Drawing.Text
Imports System.IO
Imports System.DBNull



Public Class SDKMainForm

    Inherits Form
    Private Structure CaptureInfo
        Public PreCaptureMessage As String
        ' to display on fingerprint window
        Public PostCaptuerMessage As String
        ' to display on fingerprint window
        Public ImageType As DLL.IBSU_ImageType
        ' capture mode
        Public NumberOfFinger As Integer
        ' number of finger count
        Public fingerName As String
        ' finger name (e.g left thumbs, left index ... )
        Public fingerPosition As DLL.IBSM_FingerPosition
        '// Finger position. e.g Right Thumb, Right Index finger
    End Structure

    Private Structure ThreadParam
        Public pParentHandle As IntPtr
        Public pFrameImageHandle As IntPtr
        Public devIndex As Integer
    End Structure


    ' Capture sequence definitions
    Private Const CAPTURE_SEQ_FLAT_SINGLE_FINGER As String = "Single flat finger"
    Private Const CAPTURE_SEQ_ROLL_SINGLE_FINGER As String = "Single rolled finger"
    Private Const CAPTURE_SEQ_2_FLAT_FINGERS As String = "2 flat fingers"
    Private Const CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS As String = "10 single flat fingers"
    Private Const CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS As String = "10 single rolled fingers"
    Private Const CAPTURE_SEQ_4_FLAT_FINGERS As String = "4 flat fingers"
    Private Const CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER As String = "10 flat fingers with 4-finger scanner"

    ' Beep definitions
    Private Const __BEEP_FAIL__ As Integer = 0
    Private Const __BEEP_SUCCESS__ As Integer = 1
    Private Const __BEEP_OK__ As Integer = 2
    Private Const __BEEP_DEVICE_COMMUNICATION_BREAK__ As Integer = 3

    ' LED color definitions
    Private Const __LED_COLOR_NONE__ As Integer = 0
    Private Const __LED_COLOR_GREEN__ As Integer = 1
    Private Const __LED_COLOR_RED__ As Integer = 2
    Private Const __LED_COLOR_YELLOW__ As Integer = 3

    ' Key button definitions
    Private Const __LEFT_KEY_BUTTON__ As Integer = 1
    Private Const __RIGHT_KEY_BUTTON__ As Integer = 2
    Shared m_initThread As Thread
    Shared m_sync As New [Object]()

    Private m_callbackDeviceCommunicationBreak As DLL.IBSU_Callback = Nothing
    Private m_callbackPreviewImage As DLL.IBSU_CallbackPreviewImage = Nothing
    Private m_callbackFingerCount As DLL.IBSU_CallbackFingerCount = Nothing
    Private m_callbackFingerQuality As DLL.IBSU_CallbackFingerQuality = Nothing
    Private m_callbackDeviceCount As DLL.IBSU_CallbackDeviceCount = Nothing
    Private m_callbackInitProgress As DLL.IBSU_CallbackInitProgress = Nothing
    Private m_callbackTakingAcquisition As DLL.IBSU_CallbackTakingAcquisition = Nothing
    Private m_callbackCompleteAcquisition As DLL.IBSU_CallbackCompleteAcquisition = Nothing
    Private m_callbackResultImage As DLL.IBSU_CallbackResultImage = Nothing
    Private m_callbackClearPlaten As DLL.IBSU_CallbackClearPlatenAtCapture = Nothing
    Private m_callbackResultImageEx As DLL.IBSU_CallbackResultImageEx = Nothing
    Private m_callbackPressedKeyButtons As DLL.IBSU_CallbackKeyButtons = Nothing


    Private m_verInfo As DLL.IBSU_SdkVersion
    Private m_nSelectedDevIndex As Integer
    '///< Index of selected device
    Private m_nDevHandle As Integer
    '///< Device handle

    Public m_bSelectDev As Boolean

    Public m_bInitializing As Boolean
    '///< Device initialization is in progress
    Private m_nCurrentCaptureStep As Integer
    Private m_ImgSaveFolder As String
    '///< Base folder for image saving
    Private m_ImgSubFolder As String
    '///< Sub Folder for image sequence
    Private m_strImageMessage As String
    Private m_bNeedClearPlaten As Boolean
    Private m_bBlank As Boolean
    Private m_bSaveWarningOfClearPlaten As Boolean

    Private m_nOvImageTextHandle As Integer
    Private m_nOvClearPlatenHandle As Integer
    Private m_nOvSegmentHandle(DLL.IBSU_MAX_SEGMENT_COUNT) As Integer

    Private m_vecCaptureSeq As New List(Of CaptureInfo)()
    Private m_FingerQuality As DLL.IBSU_FingerQualityState() = New DLL.IBSU_FingerQualityState(DLL.IBSU_MAX_SEGMENT_COUNT) {}
    Private m_LedType As DLL.IBSU_LedType

    Public Sub New()
        InitializeComponent()
    End Sub


    '///////////////////////////////////////////////////////////////////////////////////////////
    ' User defined functions
    Private Function _ChkFolder(ByVal sPath As String) As Boolean
        Dim di As New DirectoryInfo(sPath)
        Return di.Exists
    End Function

    Private Sub _InitializeDeviceThreadCallback(ByVal pParam As Object)
        If pParam Is Nothing Then
            Return
        End If
        Dim param As ThreadParam = CType(pParam, ThreadParam)
        Dim formHandle As IntPtr = param.pParentHandle
        Dim frameImageHandle As IntPtr = param.pFrameImageHandle
        Dim devIndex As Integer = param.devIndex
        Dim devHandle As Integer = -1
        Dim nRc As Integer = DLL.IBSU_STATUS_OK
        Dim ledCount As Integer
        Dim operableLEDs As UInteger

        m_bInitializing = True
        nRc = DLL._IBSU_OpenDevice(devIndex, devHandle)
        m_bInitializing = False

        If nRc >= DLL.IBSU_STATUS_OK Then
            m_nDevHandle = devHandle

            DLL._IBSU_GetOperableLEDs(devHandle, m_LedType, ledCount, operableLEDs)

            Dim clientRect As New DLL.IBSU_RECT()
            Win32.GetClientRect(frameImageHandle, clientRect)

            ' Create display window
            DLL._IBSU_CreateClientWindow(devHandle, frameImageHandle, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom)
            DLL._IBSU_AddOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
            DLL._IBSU_AddOverlayText(m_nDevHandle, m_nOvImageTextHandle, "Arial", 10, True, "", 10, 10, 0)
            For i = 0 To DLL.IBSU_MAX_SEGMENT_COUNT - 1
                DLL._IBSU_AddOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle(i), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
            Next


            '* Default value is 0x00d8e9ec( COLOR_BTNFACE ) for enumeration ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR
            ' ** You can change the background color as using method below
            '                string cValue;
            '                Color bkColor = SystemColors.ButtonFace;
            '                cValue = String.Format("{0}", (uint)ColorTranslator.ToWin32(bkColor));
            '                DLL._IBSU_SetClientDisplayProperty(devHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR, cValue);
            '


            '* Default value is TRUE for enumeration ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE
            ' ** You can remove the guide line of rolling as using method below
            '                cValue = "FALSE";
            '                DLL._IBSU_SetClientDisplayProperty( devHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE, cValue );
            '


            ' register callback functions
            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, m_callbackDeviceCommunicationBreak, formHandle)
            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, m_callbackPreviewImage, formHandle)
            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, m_callbackTakingAcquisition, formHandle)
            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, m_callbackCompleteAcquisition, formHandle)
            ' DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE, m_callbackResultImage, formHandle)
            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, m_callbackResultImageEx, formHandle)
            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, m_callbackFingerCount, formHandle)
            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, m_callbackFingerQuality, formHandle)
            If m_chkUseClearPlaten.Checked Then
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, m_callbackClearPlaten, formHandle)
            End If

            DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_KEYBUTTON, m_callbackPressedKeyButtons, formHandle)
        End If

        ' status notification and sequence start
        If nRc = DLL.IBSU_STATUS_OK Then
            OnMsg_CaptureSeqStart()
            Return
        End If

        If nRc > DLL.IBSU_STATUS_OK Then
            OnMsg_InitWarning()
        Else
            Dim message As String
            Select Case nRc
                Case DLL.IBSU_ERR_DEVICE_ACTIVE
                    message = [String].Format("[Error code = {0}] Device initialization failed because in use by another thread/process.", nRc)
                    OnMsg_UpdateStatusMessage(message)
                    Exit Select
                Case DLL.IBSU_ERR_USB20_REQUIRED
                    message = [String].Format("[Error code = {0}] Device initialization failed because SDK only works with USB 2.0.", nRc)
                    OnMsg_UpdateStatusMessage(message)
                    Exit Select
                Case Else
                    message = [String].Format("[Error code = {0}] Device initialization failed", nRc)
                    OnMsg_UpdateStatusMessage(message)
                    Exit Select
            End Select
        End If

        OnMsg_UpdateDeviceList()
    End Sub

    Private Sub _SetStatusBarMessage(ByVal message As String)
        m_txtStatusMessage.Text = message
    End Sub

    Private Sub _SetImageMessage(ByVal message As String)
        Dim font_size As Integer = 10
        Dim x As Integer = 10
        Dim y As Integer = 10
        Dim cr As Color = Color.FromArgb(0, 0, 255)

        If (m_bNeedClearPlaten) Then
            cr = Color.FromArgb(255, 0, 0)
        End If

        DLL._IBSU_ModifyOverlayText(m_nDevHandle, m_nOvImageTextHandle, "Arial", font_size, True, message, x, y, CUInt(ColorTranslator.ToWin32(cr)))

    End Sub

    Private Sub _UpdateCaptureSequences()
        ' store currently selected sequence
        Dim strSelectedText As String = ""
        Dim selectedSeq As Integer = m_cboCaptureSeq.SelectedIndex
        If selectedSeq > -1 Then
            strSelectedText = m_cboCaptureSeq.Text
        End If

        ' populate combo box
        m_cboCaptureSeq.Items.Clear()
        m_cboCaptureSeq.Items.Add("- Please select -")

        Dim devIndex As Integer = m_cboUsbDevices.SelectedIndex - 1
        Dim devDesc As New DLL.IBSU_DeviceDesc()
        If devIndex > -1 Then
            DLL._IBSU_GetDeviceDescription(devIndex, devDesc)
        End If

        If (devDesc.productName = "WATSON") _
        OrElse (devDesc.productName = "WATSON MINI") _
        OrElse (devDesc.productName = "SHERLOCK_ROIC") _
        OrElse (devDesc.productName = "SHERLOCK") Then
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_FLAT_SINGLE_FINGER)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_ROLL_SINGLE_FINGER)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_2_FLAT_FINGERS)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS)
        ElseIf (devDesc.productName = "COLUMBO") _
        OrElse (devDesc.productName = "CURVE") Then
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_FLAT_SINGLE_FINGER)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS)
        ElseIf (devDesc.productName = "HOLMES") _
        OrElse (devDesc.productName = "KOJAK") _
        OrElse (devDesc.productName = "FIVE-0") Then
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_FLAT_SINGLE_FINGER)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_ROLL_SINGLE_FINGER)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_2_FLAT_FINGERS)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_4_FLAT_FINGERS)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS)
            m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER)
        End If

        ' select previously selected sequence
        If selectedSeq > -1 Then
            selectedSeq = m_cboCaptureSeq.FindString(strSelectedText, 0)
        End If
        If selectedSeq = -1 Then
            m_cboCaptureSeq.SelectedIndex = 0
        Else
            m_cboCaptureSeq.SelectedIndex = selectedSeq
        End If

        OnMsg_UpdateDisplayResources()
    End Sub

    Private Function _ReleaseDevice() As Integer
        Dim nRc As Integer = DLL.IBSU_STATUS_OK

        If m_nDevHandle <> -1 Then
            nRc = DLL._IBSU_CloseDevice(m_nDevHandle)
        End If

        If nRc >= DLL.IBSU_STATUS_OK Then
            m_nDevHandle = -1
            m_nCurrentCaptureStep = -1
            m_bInitializing = False
        End If

        Return nRc
    End Function

    Private Sub _BeepFail()
        Dim beeperType As DLL.IBSU_BeeperType

        If DLL._IBSU_GetOperableBeeper(m_nDevHandle, beeperType) <> DLL.IBSU_STATUS_OK Then
            Win32.Beep(3500, 300)
            Thread.Sleep(150)
            Win32.Beep(3500, 150)
            Thread.Sleep(150)
            Win32.Beep(3500, 150)
            Thread.Sleep(150)
            Win32.Beep(3500, 150)
        Else
            DLL._IBSU_SetBeeper(m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 12, 0, 0)
            Thread.Sleep(150)
            DLL._IBSU_SetBeeper(m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 6, 0, 0)
            Thread.Sleep(150)
            DLL._IBSU_SetBeeper(m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 6, 0, 0)
            Thread.Sleep(150)
            DLL._IBSU_SetBeeper(m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 6, 0, 0)
        End If
    End Sub

    Private Sub _BeepSuccess()
        Dim beeperType As DLL.IBSU_BeeperType

        If DLL._IBSU_GetOperableBeeper(m_nDevHandle, beeperType) <> DLL.IBSU_STATUS_OK Then
            Win32.Beep(3500, 100)
            Thread.Sleep(50)
            Win32.Beep(3500, 100)
        Else
            DLL._IBSU_SetBeeper(m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
            Thread.Sleep(150)
            DLL._IBSU_SetBeeper(m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
        End If
    End Sub

    Private Sub _BeepOk()
        Dim beeperType As DLL.IBSU_BeeperType

        If DLL._IBSU_GetOperableBeeper(m_nDevHandle, beeperType) <> DLL.IBSU_STATUS_OK Then
            Win32.Beep(3500, 100)
        Else
            DLL._IBSU_SetBeeper(m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
        End If
    End Sub

    Private Sub _BeepDeviceCommunicationBreak()
        For i As Integer = 0 To 7
            Win32.Beep(3500, 100)
            Thread.Sleep(100)
        Next
    End Sub

    Private Sub _SaveBitmapImage(ByRef image As DLL.IBSU_ImageData, ByVal fingerName As String)
        If (m_ImgSaveFolder Is Nothing) OrElse (m_ImgSubFolder Is Nothing) OrElse _
           (m_ImgSaveFolder.Length = 0) OrElse (m_ImgSubFolder.Length = 0) Then
            Return
        End If

        Dim strFolder As String
        strFolder = [String].Format("{0}\{1}", m_ImgSaveFolder, m_ImgSubFolder)
        System.IO.Directory.CreateDirectory(strFolder)

        Dim strFileName As String
        strFileName = [String].Format("{0}\Image_{1}_{2}.bmp", strFolder, m_nCurrentCaptureStep, fingerName)

        If DLL._IBSU_SaveBitmapImage(strFileName, image.Buffer, image.Width, image.Height, image.Pitch, image.ResolutionX, _
         image.ResolutionY) <> DLL.IBSU_STATUS_OK Then
            MessageBox.Show("Failed to save bitmap image!")
        End If
    End Sub

    Private Sub _SaveWsqImage(ByRef image As DLL.IBSU_ImageData, ByVal fingerName As String)
        If (m_ImgSaveFolder Is Nothing) OrElse (m_ImgSubFolder Is Nothing) OrElse _
           (m_ImgSaveFolder.Length = 0) OrElse (m_ImgSubFolder.Length = 0) Then
            Return
        End If

        Dim strFolder As String
        strFolder = [String].Format("{0}\{1}", m_ImgSaveFolder, m_ImgSubFolder)
        System.IO.Directory.CreateDirectory(strFolder)

        Dim strFileName As String
        strFileName = [String].Format("{0}\Image_{1}_{2}.wsq", strFolder, m_nCurrentCaptureStep, fingerName)

        '// Bug Fixed, WSQ image was flipped vertically.
        '// Pitch parameter is required to fix bug.
        If DLL._IBSU_WSQEncodeToFile(strFileName, image.Buffer, image.Width, image.Height, image.Pitch, image.BitsPerPixel, image.ResolutionX, _
         0.75, "") <> DLL.IBSU_STATUS_OK Then
            MessageBox.Show("Failed to save bitmap image!")
        End If

        '/***********************************************************
        ' * Example codes for WSQ encoding based on memory

        'Dim filename As String
        'Dim pCompressedData As IntPtr
        'Dim pDecompressedData As IntPtr
        'Dim pDecompressedData2 As IntPtr
        'Dim compressedLength As Integer
        'If DLL._IBSU_WSQEncodeMem(image.Buffer, _
        '                        image.Width, image.Height, image.Pitch, image.BitsPerPixel, _
        '                        image.ResolutionX, 0.75, "", _
        '                        pCompressedData, compressedLength) <> DLL.IBSU_STATUS_OK Then
        '    MessageBox.Show("Failed to save WSQ_1 image!")
        'End If

        'Dim compressedBuffer(compressedLength) As Byte
        'Marshal.Copy(pCompressedData, compressedBuffer, 0, compressedLength)

        'filename = [String].Format("{0}\Image_{1}_{2}_v1.wsq", strFolder, m_nCurrentCaptureStep, fingerName)
        'Dim fs As FileStream = New FileStream(filename, FileMode.Create)
        'Dim w As BinaryWriter = New BinaryWriter(fs)
        'w.Write(compressedBuffer, 0, compressedLength)
        'w.Close()
        'Dim width, height, pitch, bitsPerPixel, pixelPerInch As Integer
        'If (DLL._IBSU_WSQDecodeMem(pCompressedData, compressedLength, _
        '                           pDecompressedData, width, height, _
        '                           pitch, bitsPerPixel, pixelPerInch) <> DLL.IBSU_STATUS_OK) Then
        '    MessageBox.Show("Failed to Decode WSQ image!")
        'End If

        'filename = [String].Format("{0}\Image_{1}_{2}_v1.bmp", strFolder, m_nCurrentCaptureStep, fingerName)
        'If (DLL._IBSU_SaveBitmapImage(filename, pDecompressedData, _
        '                              CType(width, UInteger), CType(height, UInteger), pitch, _
        '                              pixelPerInch, pixelPerInch) <> DLL.IBSU_STATUS_OK) Then
        '    MessageBox.Show("Failed to save bitmap v1 image!")
        'End If

        'If (DLL._IBSU_WSQDecodeFromFile(strFileName, pDecompressedData2, _
        '                                width, height, pitch, _
        '                                pixelPerInch, pixelPerInch) <> DLL.IBSU_STATUS_OK) Then
        '    MessageBox.Show("Failed to Decode WSQ image!")
        'End If

        'filename = [String].Format("{0}\Image_{1}_{2}_v2.bmp", strFolder, m_nCurrentCaptureStep, fingerName)
        'If (DLL._IBSU_SaveBitmapImage(filename, pDecompressedData2, _
        '                              CType(width, UInteger), CType(height, UInteger), pitch, _
        '                              pixelPerInch, pixelPerInch) <> DLL.IBSU_STATUS_OK) Then
        '    MessageBox.Show("Failed to save bitmap v2 image!")
        'End If

        'DLL._IBSU_FreeMemory(pCompressedData)
        'DLL._IBSU_FreeMemory(pDecompressedData)
        'DLL._IBSU_FreeMemory(pDecompressedData2)
        '***********************************************************/

    End Sub

    Private Sub _SavePngImage(ByRef image As DLL.IBSU_ImageData, ByVal fingerName As String)
        If (m_ImgSaveFolder Is Nothing) OrElse (m_ImgSubFolder Is Nothing) OrElse _
           (m_ImgSaveFolder.Length = 0) OrElse (m_ImgSubFolder.Length = 0) Then
            Return
        End If

        Dim strFolder As String
        strFolder = [String].Format("{0}\{1}", m_ImgSaveFolder, m_ImgSubFolder)
        System.IO.Directory.CreateDirectory(strFolder)

        Dim strFileName As String
        strFileName = [String].Format("{0}\Image_{1}_{2}.png", strFolder, m_nCurrentCaptureStep, fingerName)

        If DLL._IBSU_SavePngImage(strFileName, image.Buffer, image.Width, image.Height, image.Pitch, image.ResolutionX, _
         image.ResolutionY) <> DLL.IBSU_STATUS_OK Then
            MessageBox.Show("Failed to save png image!")
        End If
    End Sub

    Private Sub _SaveJP2Image(ByRef image As DLL.IBSU_ImageData, ByVal fingerName As String)
        If (m_ImgSaveFolder Is Nothing) OrElse (m_ImgSubFolder Is Nothing) OrElse _
           (m_ImgSaveFolder.Length = 0) OrElse (m_ImgSubFolder.Length = 0) Then
            Return
        End If

        Dim strFolder As String
        strFolder = [String].Format("{0}\{1}", m_ImgSaveFolder, m_ImgSubFolder)
        System.IO.Directory.CreateDirectory(strFolder)

        Dim strFileName As String
        strFileName = [String].Format("{0}\Image_{1}_{2}.jp2", strFolder, m_nCurrentCaptureStep, fingerName)

        If DLL._IBSU_SaveJP2Image(strFileName, image.Buffer, image.Width, image.Height, image.Pitch, image.ResolutionX, _
         image.ResolutionY, 80) <> DLL.IBSU_STATUS_OK Then
            MessageBox.Show("Failed to save jpeg-2000 image!")
        End If
    End Sub

    Private Sub _DrawRoundRect(ByVal g As Graphics, ByVal brush As Brush, ByVal X As Single, ByVal Y As Single, ByVal width As Single, ByVal height As Single, _
     ByVal radius As Single)
        Dim gp As New GraphicsPath()

        gp.AddLine(X + radius, Y, X + width - (radius * 2), Y)
        gp.AddArc(X + width - (radius * 2), Y, radius * 2, radius * 2, 270, 90)
        gp.AddLine(X + width, Y + radius, X + width, Y + height - (radius * 2))
        gp.AddArc(X + width - (radius * 2), Y + height - (radius * 2), radius * 2, radius * 2, 0, 90)
        gp.AddLine(X + width - (radius * 2), Y + height, X + radius, Y + height)
        gp.AddArc(X, Y + height - (radius * 2), radius * 2, radius * 2, 90, 90)
        gp.AddLine(X, Y + height - (radius * 2), X, Y + radius)
        gp.AddArc(X, Y, radius * 2, radius * 2, 180, 90)
        gp.CloseFigure()

        '            g.DrawPath(p, gp);
        g.SmoothingMode = SmoothingMode.AntiAlias
        g.FillPath(brush, gp)
        gp.Dispose()
    End Sub

    Private Function _ModifyOverlayForWarningOfClearPlaten(ByVal bVisible As [Boolean]) As Integer
        If m_nDevHandle = -1 Then
            Return -1
        End If

        Dim nRc As Integer = DLL.IBSU_STATUS_OK
        Dim cr As Color = Color.FromArgb(255, 0, 0)
        Dim left As Integer, top As Integer, right As Integer, bottom As Integer

        Dim clientRect As New DLL.IBSU_RECT()
        Win32.GetClientRect(m_FrameImage.Handle, clientRect)

        left = 0
        top = 0
        right = clientRect.right - clientRect.left
        bottom = clientRect.bottom - clientRect.top
        If bVisible Then
            nRc = DLL._IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle, left, top, right, top, _
             right, bottom, left, bottom, 20, CUInt(ColorTranslator.ToWin32(cr)))
        Else
            nRc = DLL._IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle, 0, 0, 0, 0, _
             0, 0, 0, 0, 0, CUInt(0))
        End If

        Return nRc
    End Function

    Private Sub _SetLEDs(ByVal deviceHandle As Integer, ByVal info As CaptureInfo, ByVal ledColor As Integer, ByVal bBlink As Boolean)
        Dim setLEDs As UInteger = 0

        If m_LedType = DLL.IBSU_LedType.ENUM_IBSU_LED_TYPE_FSCAN Then
            If bBlink Then
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_BLINK_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_BLINK_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_BLINK_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_BLINK_RED
                End If
            End If

            If info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_ROLL
            End If

            If info.fingerName = "SFF_Right_Thumb" OrElse info.fingerName = "SRF_Right_Thumb" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_TWO_THUMB
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_RED
                End If
            ElseIf info.fingerName = "SFF_Left_Thumb" OrElse info.fingerName = "SRF_Left_Thumb" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_TWO_THUMB
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_RED
                End If
            ElseIf info.fingerName = "TFF_2_Thumbs" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_TWO_THUMB
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_THUMB_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_THUMB_RED
                End If
                ' LEFT HAND////////////////////
            ElseIf info.fingerName = "SFF_Left_Index" OrElse info.fingerName = "SRF_Left_Index" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_LEFT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_RED
                End If
            ElseIf info.fingerName = "SFF_Left_Middle" OrElse info.fingerName = "SRF_Left_Middle" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_LEFT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_RED
                End If
            ElseIf info.fingerName = "SFF_Left_Ring" OrElse info.fingerName = "SRF_Left_Ring" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_LEFT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_RED
                End If
            ElseIf info.fingerName = "SFF_Left_Little" OrElse info.fingerName = "SRF_Left_Little" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_LEFT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_RED
                End If
            ElseIf info.fingerName = "4FF_Left_4_Fingers" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_LEFT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_INDEX_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_MIDDLE_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_RING_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_LEFT_LITTLE_RED
                End If
                ' RIGHT HAND /////////////////////////
            ElseIf info.fingerName = "SFF_Right_Index" OrElse info.fingerName = "SRF_Right_Index" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_RIGHT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_RED
                End If
            ElseIf info.fingerName = "SFF_Right_Middle" OrElse info.fingerName = "SRF_Right_Middle" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_RIGHT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_RED
                End If
            ElseIf info.fingerName = "SFF_Right_Ring" OrElse info.fingerName = "SRF_Right_Ring" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_RIGHT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_RED
                End If
            ElseIf info.fingerName = "SFF_Right_Little" OrElse info.fingerName = "SRF_Right_Little" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_RIGHT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_RED
                End If
            ElseIf info.fingerName = "4FF_Right_4_Fingers" Then
                setLEDs = setLEDs Or DLL.IBSU_LED_F_PROGRESS_RIGHT_HAND
                If ledColor = __LED_COLOR_GREEN__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_GREEN
                ElseIf ledColor = __LED_COLOR_RED__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_RED
                ElseIf ledColor = __LED_COLOR_YELLOW__ Then
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_GREEN
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_INDEX_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_MIDDLE_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_RING_RED
                    setLEDs = setLEDs Or DLL.IBSU_LED_F_RIGHT_LITTLE_RED
                End If
            End If

            If ledColor = __LED_COLOR_NONE__ Then
                setLEDs = 0
            End If

            DLL._IBSU_SetLEDs(deviceHandle, setLEDs)
        End If
    End Sub










    Private Shared Sub OnEvent_DeviceCommunicationBreak(ByVal deviceIndex As Integer, ByVal pContext As IntPtr)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            pDlg.OnMsg_DeviceCommunicationBreak()
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_PreviewImage(ByVal deviceHandle As Integer, ByVal pContext As IntPtr, ByVal image As DLL.IBSU_ImageData)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock SDKMainForm.m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_FingerCount(ByVal deviceHandle As Integer, ByVal pContext As IntPtr, ByVal fingerCountState As DLL.IBSU_FingerCountState)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock SDKMainForm.m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            If deviceHandle <> pDlg.m_nDevHandle Then
                Return
                '
                '                string fingerState;
                '                if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_FINGER_COUNT_OK)
                '                    fingerState = "FINGER_COUNT_OK";
                '                else if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_TOO_MANY_FINGERS)
                '                    fingerState = "TOO_MANY_FINGERS";
                '                else if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_TOO_FEW_FINGERS)
                '                    fingerState = "TOO_FEW_FINGERS";
                '                else if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_NON_FINGER)
                '                    fingerState = "NON-FINGER";
                '                else
                '                    fingerState = "UNKNOWN";
                '

            End If
            Dim info As CaptureInfo = pDlg.m_vecCaptureSeq(pDlg.m_nCurrentCaptureStep)

            If fingerCountState = DLL.IBSU_FingerCountState.ENUM_IBSU_NON_FINGER Then
                pDlg._SetLEDs(deviceHandle, info, __LED_COLOR_RED__, True)
            Else
                pDlg._SetLEDs(deviceHandle, info, __LED_COLOR_YELLOW__, True)
            End If
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_FingerQuality(ByVal deviceHandle As Integer, ByVal pContext As IntPtr, ByVal pQualityArray As IntPtr, ByVal qualityArrayCount As Integer)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock SDKMainForm.m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            Dim qualityArray As Integer() = New Integer(3) {}
            Marshal.Copy(pQualityArray, qualityArray, 0, qualityArrayCount)
            pDlg.m_FingerQuality(0) = DirectCast(qualityArray(0), DLL.IBSU_FingerQualityState)
            pDlg.m_FingerQuality(1) = DirectCast(qualityArray(1), DLL.IBSU_FingerQualityState)
            pDlg.m_FingerQuality(2) = DirectCast(qualityArray(2), DLL.IBSU_FingerQualityState)
            pDlg.m_FingerQuality(3) = DirectCast(qualityArray(3), DLL.IBSU_FingerQualityState)

            pDlg.m_picScanner.Invalidate()
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_DeviceCount(ByVal detectedDevices As Integer, ByVal pContext As IntPtr)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock SDKMainForm.m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            pDlg.OnMsg_UpdateDeviceList()
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_InitProgress(ByVal deviceIndex As Integer, ByVal pContext As IntPtr, ByVal progressValue As Integer)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            Dim message As String
            message = [String].Format("Initializing device... {0}%", progressValue)
            pDlg.OnMsg_UpdateStatusMessage(message)
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_TakingAcquisition(ByVal deviceHandle As Integer, ByVal pContext As IntPtr, ByVal imageType As DLL.IBSU_ImageType)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            If imageType = DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER Then
                pDlg.OnMsg_Beep(__BEEP_OK__)
                pDlg.m_strImageMessage = "When done remove finger from sensor"
                pDlg.OnMsg_UpdateImageMessage(pDlg.m_strImageMessage)
                pDlg.OnMsg_UpdateStatusMessage(pDlg.m_strImageMessage)
            End If
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_CompleteAcquisition(ByVal deviceHandle As Integer, ByVal pContext As IntPtr, ByVal imageType As DLL.IBSU_ImageType)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            If imageType = DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER Then
                pDlg.OnMsg_Beep(__BEEP_OK__)
            Else
                pDlg.OnMsg_Beep(__BEEP_SUCCESS__)
                pDlg.m_strImageMessage = "Remove fingers from sensor"
                pDlg.OnMsg_UpdateImageMessage(pDlg.m_strImageMessage)
                pDlg.m_strImageMessage = "Acquisition completed, postprocessing.."
                pDlg.OnMsg_UpdateStatusMessage(pDlg.m_strImageMessage)
                DLL._IBSU_RedrawClientWindow(deviceHandle)
            End If
        End SyncLock
    End Sub

	'/****
	' ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
	' ** Please use IBSU_CallbackResultImageEx instead
	'*/
    Private Shared Sub OnEvent_ResultImageEx(ByVal deviceHandle As Integer, ByVal pContext As IntPtr, ByVal imageStatus As Integer, ByVal image As DLL.IBSU_ImageData, ByVal imageType As DLL.IBSU_ImageType, ByVal detectedFingerCount As Integer, ByVal segmentImageArrayCount As Integer, ByVal pSegmentImageArray As IntPtr, ByVal pSegmentPositionArray As IntPtr)
        SyncLock m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            If deviceHandle <> pDlg.m_nDevHandle Then
                Return
            End If

            If imageStatus >= DLL.IBSU_STATUS_OK Then
                If imageType = DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER Then
                    pDlg.OnMsg_Beep(__BEEP_SUCCESS__)
                End If
            End If
            ' Added 2012-11-30
            If pDlg.m_bNeedClearPlaten Then
                pDlg.m_bNeedClearPlaten = False
                pDlg.m_picScanner.Invalidate()
            End If

            ' Image acquisition successful
            Dim imgTypeName As String

            Select Case imageType
                Case DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER
                    imgTypeName = "-- Rolling single finger --"
                    Exit Select
                Case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER
                    imgTypeName = "-- Flat single finger --"
                    Exit Select
                Case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS
                    imgTypeName = "-- Flat two fingers --"
                    Exit Select
                Case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_FOUR_FINGERS
                    imgTypeName = "-- Flat 4 fingers --"
                    Exit Select
                Case Else
                    imgTypeName = "-- Unknown --"
                    Exit Select
            End Select

            Dim i As Integer = 0

            If imageStatus >= DLL.IBSU_STATUS_OK Then
                Dim info As CaptureInfo = pDlg.m_vecCaptureSeq(pDlg.m_nCurrentCaptureStep)
                pDlg._SetLEDs(deviceHandle, info, __LED_COLOR_GREEN__, False)

                If pDlg.m_chkSaveImages.Checked Then
                    pDlg.OnMsg_UpdateStatusMessage("Saving image...")
                    info = pDlg.m_vecCaptureSeq(pDlg.m_nCurrentCaptureStep)
                    pDlg._SaveBitmapImage(image, info.fingerName)
                    pDlg._SaveWsqImage(image, info.fingerName)
                    pDlg._SavePngImage(image, info.fingerName)
                    pDlg._SaveJP2Image(image, info.fingerName)
                    If segmentImageArrayCount > 0 Then
                        Dim imageArray As DLL.IBSU_ImageData() = New DLL.IBSU_ImageData(segmentImageArrayCount - 1) {}
                        Dim segmentName As String
                        Dim ptrRunner As IntPtr = pSegmentImageArray
                        For i = 0 To segmentImageArrayCount - 1
                            segmentName = [String].Format("{0}_Segement_{1}", info.fingerName, i)
                            imageArray(i) = DirectCast(Marshal.PtrToStructure(ptrRunner, GetType(DLL.IBSU_ImageData)), DLL.IBSU_ImageData)
                            pDlg._SaveBitmapImage(imageArray(i), segmentName)
                            pDlg._SaveWsqImage(imageArray(i), segmentName)
                            pDlg._SavePngImage(imageArray(i), segmentName)
                            pDlg._SaveJP2Image(imageArray(i), segmentName)
                            ptrRunner = IntPtr.op_Explicit(CLng(ptrRunner) + Marshal.SizeOf(imageArray(0)))
                        Next
                    End If
                End If

                If pDlg.m_chkDrawSegmentImage.Checked Then
                    Dim propertyValue As New StringBuilder()
                    Dim scaleFactor As Double
                    Dim leftMargin As Integer
                    Dim TopMargin As Integer

                    DLL._IBSU_GetClientWindowProperty(deviceHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR, propertyValue)
                    scaleFactor = Double.Parse(propertyValue.ToString())

                    DLL._IBSU_GetClientWindowProperty(deviceHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN, propertyValue)
                    leftMargin = Integer.Parse(propertyValue.ToString())

                    DLL._IBSU_GetClientWindowProperty(deviceHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN, propertyValue)
                    TopMargin = Integer.Parse(propertyValue.ToString())

                    'Dim segmentArray As DLL.IBSU_SegmentPosition() = New DLL.IBSU_SegmentPosition(segmentImageArrayCount - 1) {}
                    Dim segmentArray As DLL.IBSU_SegmentPosition() = New DLL.IBSU_SegmentPosition(DLL.IBSU_MAX_SEGMENT_COUNT) {}
                    Dim ptrRunner_segmentArray As IntPtr = pSegmentPositionArray

                    For i = 0 To segmentImageArrayCount - 1
                        Dim cr As Color = Color.FromArgb(0, 128, 0)
                        Dim x1 As Integer
                        Dim x2 As Integer
                        Dim x3 As Integer
                        Dim x4 As Integer

                        Dim y1 As Integer
                        Dim y2 As Integer
                        Dim y3 As Integer
                        Dim y4 As Integer

                        segmentArray(i) = DirectCast(Marshal.PtrToStructure(ptrRunner_segmentArray, GetType(DLL.IBSU_SegmentPosition)), DLL.IBSU_SegmentPosition)

                        x1 = leftMargin + segmentArray(i).x1 * scaleFactor
                        x2 = leftMargin + segmentArray(i).x2 * scaleFactor
                        x3 = leftMargin + segmentArray(i).x3 * scaleFactor
                        x4 = leftMargin + segmentArray(i).x4 * scaleFactor

                        y1 = TopMargin + segmentArray(i).y1 * scaleFactor
                        y2 = TopMargin + segmentArray(i).y2 * scaleFactor
                        y3 = TopMargin + segmentArray(i).y3 * scaleFactor
                        y4 = TopMargin + segmentArray(i).y4 * scaleFactor

                        DLL._IBSU_ModifyOverlayQuadrangle(deviceHandle, pDlg.m_nOvSegmentHandle(i), x1, y1, x2, y2, x3, y3, x4, y4, 1, CUInt(ColorTranslator.ToWin32(cr)))


                        ptrRunner_segmentArray = IntPtr.op_Explicit(CLng(ptrRunner_segmentArray) + Marshal.SizeOf(segmentArray(0)))
                    Next
                End If

                If pDlg.m_chkNFIQScore.Checked Then
                    Dim nfiq_score(DLL.IBSU_MAX_SEGMENT_COUNT) As Integer
                    Dim score As Integer = 0
                    Dim nRc As Integer
                    Dim segment_pos As Integer = 0
                    Dim imageArray As DLL.IBSU_ImageData() = New DLL.IBSU_ImageData(DLL.IBSU_MAX_SEGMENT_COUNT) {}
                    Dim ptrRunner As IntPtr = pSegmentImageArray

                    For i = 0 To DLL.IBSU_MAX_SEGMENT_COUNT - 1
                        nfiq_score(i) = 0
                    Next

                    Dim strValue As String = "Err"
                    pDlg.OnMsg_UpdateStatusMessage("Get NFIQ score...")

                    For i = 0 To DLL.IBSU_MAX_SEGMENT_COUNT - 1
                        If pDlg.m_FingerQuality(i) = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT Then
                            Continue For
                        End If
                        imageArray(i) = DirectCast(Marshal.PtrToStructure(ptrRunner, GetType(DLL.IBSU_ImageData)), DLL.IBSU_ImageData)

                        nRc = DLL._IBSU_GetNFIQScore(deviceHandle, imageArray(i).Buffer, imageArray(i).Width, imageArray(i).Height, imageArray(i).BitsPerPixel, score)

                        If nRc = DLL.IBSU_STATUS_OK Then
                            nfiq_score(i) = score
                        Else
                            nfiq_score(i) = -1
                        End If

                        segment_pos = segment_pos + 1
                        ptrRunner = IntPtr.op_Explicit(CLng(ptrRunner) + Marshal.SizeOf(imageArray(0)))
                    Next
                    strValue = [String].Format("{0}-{1}-{2}-{3}", nfiq_score(0), nfiq_score(1), nfiq_score(2), nfiq_score(3))
                    pDlg.OnMsg_UpdateNFIQScore(strValue)
                End If

                Dim strValue1 As String
                If imageStatus = DLL.IBSU_STATUS_OK Then
                    strValue1 = [String].Format("{0} acquisition completed successfully", imgTypeName)
                    pDlg._SetImageMessage(strValue1)
                    pDlg.OnMsg_UpdateStatusMessage(strValue1)
                Else
                    strValue1 = [String].Format("{0} acquisition Waring (Warning code = {1})", imgTypeName, imageStatus)
                    pDlg._SetImageMessage(strValue1)
                    pDlg.OnMsg_UpdateStatusMessage(strValue1)

                    pDlg.OnMsg_AskRecapture(imageStatus)
                    Return
                End If
            Else
                pDlg.m_strImageMessage = [String].Format("{0} acquisition fail (Error code = {1})", imgTypeName, imageStatus)
                pDlg._SetImageMessage(pDlg.m_strImageMessage)
                pDlg.OnMsg_UpdateStatusMessage(pDlg.m_strImageMessage)
                pDlg.m_nCurrentCaptureStep = pDlg.m_vecCaptureSeq.Count

            End If
            pDlg.OnMsg_CaptureSeqNext()
        End SyncLock

    End Sub

    Private Shared Sub OnEvent_ClearPlatenAtCapture(ByVal deviceIndex As Integer, ByVal pContext As IntPtr, ByVal platenState As DLL.IBSU_PlatenState)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            If platenState = DLL.IBSU_PlatenState.ENUM_IBSU_PLATEN_HAS_FINGERS Then
                pDlg.m_bNeedClearPlaten = True
            Else
                pDlg.m_bNeedClearPlaten = False
            End If

            If pDlg.m_bNeedClearPlaten Then
                pDlg.m_strImageMessage = "Please remove your fingers on the platen first!"
                pDlg._SetImageMessage(pDlg.m_strImageMessage)
                pDlg.OnMsg_UpdateStatusMessage(pDlg.m_strImageMessage)
            Else
                Dim info As CaptureInfo = pDlg.m_vecCaptureSeq(pDlg.m_nCurrentCaptureStep)

                ' Display message for image acuisition again
                Dim strMessage As String
                strMessage = [String].Format("{0}", info.PreCaptureMessage)

                pDlg.OnMsg_UpdateStatusMessage(strMessage)
                If Not pDlg.m_chkAutoCapture.Checked Then
                    strMessage += [String].Format(vbCr & vbLf & "Press button 'Take Result Image' when image is good!")
                End If

                pDlg._SetImageMessage(strMessage)
                pDlg.m_strImageMessage = strMessage
            End If

            pDlg.m_picScanner.Invalidate()
        End SyncLock
    End Sub

    Private Shared Sub OnEvent_PressedKeyButtons(ByVal deviceIndex As Integer, ByVal pContext As IntPtr, ByVal pressedKeyButtons As Integer)
        'If pContext Is Nothing Then
        'Return
        'End If

        SyncLock m_sync
            Dim pDlg As SDKMainForm = DirectCast(SDKMainForm.FromHandle(pContext), SDKMainForm)
            Dim message As String
            message = [String].Format("OnEvent_PressedKeyButtons = {0}", pressedKeyButtons)
            pDlg.OnMsg_UpdateStatusMessage(message)

            pDlg.OnMsg_GetSelectDevice()
            Dim idle As Boolean = Not pDlg.m_bInitializing And (pDlg.m_nCurrentCaptureStep = -1)
            Dim active As Boolean = Not pDlg.m_bInitializing And Not (pDlg.m_nCurrentCaptureStep = -1)

            If pressedKeyButtons = __LEFT_KEY_BUTTON__ Then
                If pDlg.m_bSelectDev AndAlso idle Then
                    DLL._IBSU_SetBeeper(pDlg.m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
                    pDlg.OnMsg_CaptureStartClick()
                End If
            ElseIf pressedKeyButtons = __RIGHT_KEY_BUTTON__ Then
                If active Then
                    DLL._IBSU_SetBeeper(pDlg.m_nDevHandle, DLL.IBSU_BeepPattern.ENUM_IBSU_BEEP_PATTERN_GENERIC, 2, 4, 0, 0)
                    pDlg.OnMsg_CaptureStopClick()
                End If
            End If
        End SyncLock
    End Sub



    Private Delegate Sub OnMsg_GetSelectDeviceDelegate()
    Private Sub OnMsg_GetSelectDevice()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.Invoke(New OnMsg_GetSelectDeviceDelegate(AddressOf OnMsg_GetSelectDevice))

            Return
        End If

        m_bSelectDev = m_cboUsbDevices.SelectedIndex > 0

        Return
    End Sub



    Private Delegate Sub OnMsg_CaptureStartClickDelegate()
    Private Sub OnMsg_CaptureStartClick()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_CaptureStartClickDelegate(AddressOf OnMsg_CaptureStartClick))

            Return
        End If

        If m_bInitializing Then
            Return
        End If

        Dim devIndex As Integer = m_cboUsbDevices.SelectedIndex - 1
        If devIndex < 0 Then
            Return
        End If

        If m_nCurrentCaptureStep <> -1 Then
            Dim IsActive As Boolean = False
            Dim nRc As Integer
            nRc = DLL._IBSU_IsCaptureActive(m_nDevHandle, IsActive)
            If nRc = DLL.IBSU_STATUS_OK AndAlso IsActive Then
                DLL._IBSU_TakeResultImageManually(m_nDevHandle)
            End If

            Return
        End If

        If m_nDevHandle = -1 Then
            m_bInitializing = True
            m_initThread = New Thread(New ParameterizedThreadStart(AddressOf _InitializeDeviceThreadCallback))
            Dim param As New ThreadParam()
            param.devIndex = devIndex
            param.pParentHandle = Me.Handle
            param.pFrameImageHandle = m_FrameImage.Handle
            m_initThread.Start(param)
        Else
            ' device already initialized
            OnMsg_CaptureSeqStart()
        End If

        OnMsg_UpdateDisplayResources()
    End Sub



    Private Delegate Sub OnMsg_CaptureStopClickDelegate()
    Private Sub OnMsg_CaptureStopClick()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_CaptureStopClickDelegate(AddressOf OnMsg_CaptureStopClick))

            Return
        End If

        If m_nDevHandle = -1 Then
            Return
        End If

        DLL._IBSU_CancelCaptureImage(m_nDevHandle)
        Dim tmpInfo As CaptureInfo = New CaptureInfo()
        _SetLEDs(m_nDevHandle, tmpInfo, __LED_COLOR_NONE__, False)
        m_nCurrentCaptureStep = -1
        m_bNeedClearPlaten = False

        Dim message As String
        message = [String].Format("Capture Sequence aborted")
        _SetStatusBarMessage(message)
        m_strImageMessage = ""
        _SetImageMessage("")
        OnMsg_UpdateDisplayResources()
    End Sub





    Private Delegate Sub OnMsg_CaptureSeqStartDelegate()
    Private Sub OnMsg_CaptureSeqStart()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_CaptureSeqStartDelegate(AddressOf OnMsg_CaptureSeqStart))

            Return
        End If

        If m_nDevHandle = -1 Then
            OnMsg_UpdateDisplayResources()
            Return
        End If

        Dim strCaptureSeq As String = ""
        Dim nSelectedSeq As Integer
        nSelectedSeq = m_cboCaptureSeq.SelectedIndex
        If nSelectedSeq > -1 Then
            strCaptureSeq = m_cboCaptureSeq.Text
        End If

        m_vecCaptureSeq.Clear()
        Dim info As New CaptureInfo()

        '* Please refer to definition below
        '            private const string CAPTURE_SEQ_FLAT_SINGLE_FINGER	= "Single flat finger";
        '            private const string CAPTURE_SEQ_ROLL_SINGLE_FINGER = "Single rolled finger";
        '            private const string CAPTURE_SEQ_2_FLAT_FINGERS = "2 flat fingers";
        '            private const string CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS = "10 single flat fingers";
        '            private const string CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS = "10 single rolled fingers";
        '            

        If strCaptureSeq = CAPTURE_SEQ_FLAT_SINGLE_FINGER Then
            info.PreCaptureMessage = "Please put a single finger on the sensor!"
            info.PostCaptuerMessage = "Keep finger on the sensor!"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER
            info.NumberOfFinger = 1
            info.fingerName = "SFF_Unknown"
            m_vecCaptureSeq.Add(info)
            Dim count As Integer = m_vecCaptureSeq.Count
        End If

        If strCaptureSeq = CAPTURE_SEQ_ROLL_SINGLE_FINGER Then
            info.PreCaptureMessage = "Please put a single finger on the sensor!"
            info.PostCaptuerMessage = "Roll finger!"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER
            info.NumberOfFinger = 1
            info.fingerName = "SRF_Unknown"
            m_vecCaptureSeq.Add(info)
        End If

        If strCaptureSeq = CAPTURE_SEQ_2_FLAT_FINGERS Then
            info.PreCaptureMessage = "Please put two fingers on the sensor!"
            info.PostCaptuerMessage = "Keep fingers on the sensor!"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS
            info.NumberOfFinger = 2
            info.fingerName = "TFF_Unknown"
            m_vecCaptureSeq.Add(info)
        End If

        If strCaptureSeq = CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS Then
            info.PreCaptureMessage = "Please put right thumb on the sensor!"
            info.fingerName = "SFF_Right_Thumb"
            info.PostCaptuerMessage = "Keep fingers on the sensor!"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER
            info.NumberOfFinger = 1
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right index on the sensor!"
            info.fingerName = "SFF_Right_Index"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right middle on the sensor!"
            info.fingerName = "SFF_Right_Middle"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right ring on the sensor!"
            info.fingerName = "SFF_Right_Ring"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right little on the sensor!"
            info.fingerName = "SFF_Right_Little"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left thumb on the sensor!"
            info.fingerName = "SFF_Left_Thumb"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left index on the sensor!"
            info.fingerName = "SFF_Left_Index"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left middle on the sensor!"
            info.fingerName = "SFF_Left_Middle"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left ring on the sensor!"
            info.fingerName = "SFF_Left_Ring"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left little on the sensor!"
            info.fingerName = "SFF_Left_Little"
            m_vecCaptureSeq.Add(info)
        End If

        If strCaptureSeq = CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS Then
            info.PreCaptureMessage = "Please put right thumb on the sensor!"
            info.PostCaptuerMessage = "Roll finger!"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER
            info.NumberOfFinger = 1
            info.fingerName = "SRF_Right_Thumb"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right index on the sensor!"
            info.fingerName = "SRF_Right_Index"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right middle on the sensor!"
            info.fingerName = "SRF_Right_Middle"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right ring on the sensor!"
            info.fingerName = "SRF_Right_Ring"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put right little on the sensor!"
            info.fingerName = "SRF_Right_Little"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left thumb on the sensor!"
            info.fingerName = "SRF_Left_Thumb"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left index on the sensor!"
            info.fingerName = "SRF_Left_Index"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left middle on the sensor!"
            info.fingerName = "SRF_Left_Middle"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left ring on the sensor!"
            info.fingerName = "SRF_Left_Ring"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left little on the sensor!"
            info.fingerName = "SRF_Left_Little"
            m_vecCaptureSeq.Add(info)
        End If

        If strCaptureSeq = CAPTURE_SEQ_4_FLAT_FINGERS Then
            info.PreCaptureMessage = "Please put 4 fingers on the sensor!"
            info.PostCaptuerMessage = "Keep fingers on the sensor!"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_FOUR_FINGERS
            info.NumberOfFinger = 4
            info.fingerName = "4FF_Unknown"
            m_vecCaptureSeq.Add(info)
        End If

        If strCaptureSeq = CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER Then
            info.PreCaptureMessage = "Please put right 4-fingers on the sensor!"
            info.fingerName = "4FF_Right_4_Fingers"
            info.PostCaptuerMessage = "Keep fingers on the sensor!"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_FOUR_FINGERS
            info.NumberOfFinger = 4
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put left 4-fingers on the sensor!"
            info.fingerName = "4FF_Left_4_Fingers"
            m_vecCaptureSeq.Add(info)

            info.PreCaptureMessage = "Please put 2-thumbs on the sensor!"
            info.fingerName = "TFF_2_Thumbs"
            info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS
            info.NumberOfFinger = 2
            m_vecCaptureSeq.Add(info)
        End If

        ' Make subfolder name
        m_ImgSubFolder = DateTime.Now.ToString("yyyy-MM-dd HHmmss")

        OnMsg_CaptureSeqNext()
    End Sub

    Private Delegate Sub OnMsg_CaptureSeqNextDelegate()
    Private Sub OnMsg_CaptureSeqNext()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_CaptureSeqNextDelegate(AddressOf OnMsg_CaptureSeqNext))

            Return
        End If

        Dim nRc As Integer

        If m_nDevHandle = -1 Then
            Return
        End If

        m_bBlank = False
        m_FingerQuality(0) = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT
        m_FingerQuality(1) = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT
        m_FingerQuality(2) = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT
        m_FingerQuality(3) = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT


        m_nCurrentCaptureStep += 1
        If m_nCurrentCaptureStep >= m_vecCaptureSeq.Count Then
            ' All of capture sequence completely
            Dim tmpInfo As CaptureInfo = New CaptureInfo()
            _SetLEDs(m_nDevHandle, tmpInfo, __LED_COLOR_NONE__, False)
            m_nCurrentCaptureStep = -1
            m_ImgSubFolder = ""

            OnMsg_UpdateDisplayResources()
            Win32.SetFocus(m_btnCaptureStart.Handle)
            Return
        End If

        If m_chkInvalidArea.Checked Then
            Dim propertyValue As New StringBuilder("TRUE")
            DLL._IBSU_SetClientDisplayProperty(m_nDevHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, propertyValue)
        Else
            Dim propertyValue As New StringBuilder("FALSE")
            DLL._IBSU_SetClientDisplayProperty(m_nDevHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_DISP_INVALID_AREA, propertyValue)
        End If

        If m_chkDetectSmear.Checked Then
            Dim propertyValue As New StringBuilder("1")
            DLL._IBSU_SetProperty(m_nDevHandle, DLL.IBSU_PropertyId.ENUM_IBSU_PROPERTY_ROLL_MODE, propertyValue)
            Dim strValue As New StringBuilder()
            strValue.AppendFormat("%d", m_cboSmearLevel.SelectedIndex)
            DLL._IBSU_SetProperty(m_nDevHandle, DLL.IBSU_PropertyId.ENUM_IBSU_PROPERTY_ROLL_LEVEL, strValue)
        Else
            Dim propertyValue As New StringBuilder("0")
            DLL._IBSU_SetProperty(m_nDevHandle, DLL.IBSU_PropertyId.ENUM_IBSU_PROPERTY_ROLL_MODE, propertyValue)
        End If

        Dim i As Integer
        For i = 0 To DLL.IBSU_MAX_SEGMENT_COUNT - 1
            DLL._IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle(i), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        Next

        ' Make capture delay for display result image on multi capture mode (500 ms)
        If m_nCurrentCaptureStep > 0 Then
            Thread.Sleep(500)
            m_strImageMessage = ""
        End If

        Dim info As CaptureInfo = m_vecCaptureSeq(m_nCurrentCaptureStep)

        Dim imgRes As DLL.IBSU_ImageResolution = DLL.IBSU_ImageResolution.ENUM_IBSU_IMAGE_RESOLUTION_500
        Dim bAvailable As Boolean = False
        nRc = DLL._IBSU_IsCaptureAvailable(m_nDevHandle, info.ImageType, imgRes, bAvailable)
        If nRc <> DLL.IBSU_STATUS_OK OrElse Not bAvailable Then
            Dim message As String
            message = [String].Format("The capture mode {0} is not available", info.ImageType)
            _SetStatusBarMessage(message)
            m_nCurrentCaptureStep = -1
            OnMsg_UpdateDisplayResources()
            Return
        End If

        ' Start capture
        Dim captureOptions As UInteger = 0
        If m_chkAutoContrast.Checked Then
            captureOptions = captureOptions Or DLL.IBSU_OPTION_AUTO_CONTRAST
        End If
        If m_chkAutoCapture.Checked Then
            captureOptions = captureOptions Or DLL.IBSU_OPTION_AUTO_CAPTURE
        End If
        If m_chkIgnoreFingerCount.Checked Then
            captureOptions = captureOptions Or DLL.IBSU_OPTION_IGNORE_FINGER_COUNT
        End If

        nRc = DLL._IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions)
        If nRc >= DLL.IBSU_STATUS_OK Then
            ' Display message for image acuisition
            Dim strMessage As String
            strMessage = [String].Format("{0}", info.PreCaptureMessage)
            _SetStatusBarMessage(strMessage)
            If Not m_chkAutoCapture.Checked Then
                strMessage += [String].Format(vbCr & vbLf & "Press button 'Take Result Image' when image is good!")
            End If

            _SetImageMessage(strMessage)
            m_strImageMessage = strMessage

            _SetLEDs(m_nDevHandle, info, __LED_COLOR_RED__, True)
        Else
            Dim strMessage As String
            strMessage = [String].Format("Failed to execute IBSU_BeginCaptureImage()")
            _SetStatusBarMessage(strMessage)
        End If

        OnMsg_UpdateDisplayResources()
    End Sub

    Private Delegate Sub OnMsg_DeviceCommunicationBreakDelegate()
    Private Sub OnMsg_DeviceCommunicationBreak()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_DeviceCommunicationBreakDelegate(AddressOf OnMsg_DeviceCommunicationBreak))

            Return
        End If

    End Sub

    Private Delegate Sub OnMsg_InitWarningDelegate()
    Private Sub OnMsg_InitWarning()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_InitWarningDelegate(AddressOf OnMsg_InitWarning))

            Return
        End If

    End Sub

    Private Delegate Sub OnMsg_DrawClientWindowDelegate()
    Private Sub OnMsg_DrawClientWindow()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_DrawClientWindowDelegate(AddressOf OnMsg_DrawClientWindow))

            Return
        End If

    End Sub

    Private Delegate Sub OnMsg_UpdateDeviceListDelegate()
    Private Sub OnMsg_UpdateDeviceList()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_UpdateDeviceListDelegate(AddressOf OnMsg_UpdateDeviceList))

            Return
        End If

        Dim idle As Boolean = Not m_bInitializing AndAlso (m_nCurrentCaptureStep = -1)
        If idle Then
            m_btnCaptureStop.Enabled = False
            m_btnCaptureStart.Enabled = False
        End If

        ' store currently selected device
        Dim strSelectedText As String = ""
        Dim selectedDev As Integer = m_cboUsbDevices.SelectedIndex
        If selectedDev > -1 Then
            strSelectedText = m_cboUsbDevices.Text
        End If



        m_cboUsbDevices.Items.Clear()
        m_cboUsbDevices.Items.Add("- Please select -")

        ' populate combo box
        Dim devices As Integer = 0
        DLL._IBSU_GetDeviceCount(devices)

        selectedDev = 0
        For i As Integer = 0 To devices - 1
            Dim devDesc As New DLL.IBSU_DeviceDesc()
            If DLL._IBSU_GetDeviceDescription(i, devDesc) < DLL.IBSU_STATUS_OK Then
                Continue For
            End If

            Dim strDevice As String
            If devDesc.productName.Length = 0 Then
                strDevice = "unknown device"
            Else
                strDevice = (devDesc.productName + "_v" + devDesc.fwVersion & "(") + devDesc.serialNumber & ")"
            End If

            m_cboUsbDevices.Items.Add(strDevice)
            If strDevice = strSelectedText Then
                selectedDev = i + 1
            End If
        Next

        If (selectedDev = 0) AndAlso (m_cboUsbDevices.Items.Count = 2) Then
            selectedDev = 1
        End If

        m_cboUsbDevices.SelectedIndex = selectedDev
        m_cboUsbDevices.Update()

        If idle Then
            m_cboUsbDevices_SelectedIndexChanged(Nothing, Nothing)
            _UpdateCaptureSequences()
        End If
    End Sub

    Private Delegate Sub OnMsg_UpdateDisplayResourcesDelegate()
    Private Sub OnMsg_UpdateDisplayResources()
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_UpdateDisplayResourcesDelegate(AddressOf OnMsg_UpdateDisplayResources))

            Return
        End If

        Dim selectedDev As Boolean = m_cboUsbDevices.SelectedIndex > 0
        Dim captureSeq As Boolean = m_cboCaptureSeq.SelectedIndex > 0
        Dim idle As Boolean = Not m_bInitializing AndAlso (m_nCurrentCaptureStep = -1)
        Dim active As Boolean = Not m_bInitializing AndAlso (m_nCurrentCaptureStep <> -1)
        Dim uninitializedDev As Boolean = selectedDev AndAlso (m_nDevHandle = -1)


        m_cboUsbDevices.Enabled = idle
        m_cboCaptureSeq.Enabled = selectedDev AndAlso idle

        m_btnCaptureStart.Enabled = captureSeq
        m_btnCaptureStop.Enabled = active

        m_chkAutoContrast.Enabled = selectedDev AndAlso idle
        m_chkAutoCapture.Enabled = selectedDev AndAlso idle
        m_chkIgnoreFingerCount.Enabled = selectedDev AndAlso idle
        m_chkSaveImages.Enabled = selectedDev AndAlso idle
        m_btnImageFolder.Enabled = selectedDev AndAlso idle

        m_chkUseClearPlaten.Enabled = uninitializedDev

        Dim strCaption As String = ""
        If active Then
            strCaption = "Take Result Image"
        ElseIf Not active AndAlso Not m_bInitializing Then
            strCaption = "Start"
        End If

        m_btnCaptureStart.Text = strCaption
    End Sub

    Private Delegate Sub OnMsg_UpdateStatusMessageDelegate(ByVal message As String)
    Private Sub OnMsg_UpdateStatusMessage(ByVal message As String)
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_UpdateStatusMessageDelegate(AddressOf OnMsg_UpdateStatusMessage), New Object() {message})

            Return
        End If

        _SetStatusBarMessage(message)
    End Sub

    Private Delegate Sub OnMsg_UpdateImageMessageDelegate(ByVal message As String)
    Private Sub OnMsg_UpdateImageMessage(ByVal message As String)
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_UpdateImageMessageDelegate(AddressOf OnMsg_UpdateImageMessage), New Object() {message})

            Return
        End If

        _SetImageMessage(message)
    End Sub

    Private Delegate Sub OnMsg_BeepDelegate(ByVal beepType As Integer)
    Private Sub OnMsg_Beep(ByVal beepType As Integer)
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_BeepDelegate(AddressOf OnMsg_Beep), New Object() {beepType})

            Return
        End If

        If beepType = __BEEP_FAIL__ Then
            _BeepFail()
        ElseIf beepType = __BEEP_SUCCESS__ Then
            _BeepSuccess()
        ElseIf beepType = __BEEP_OK__ Then
            _BeepOk()
        ElseIf beepType = __BEEP_DEVICE_COMMUNICATION_BREAK__ Then
            _BeepDeviceCommunicationBreak()
        End If
    End Sub

    Private Delegate Sub OnMsg_UpdateNFIQScoreDelegate(ByVal score As String)
    Private Sub OnMsg_UpdateNFIQScore(ByVal score As String)
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_UpdateNFIQScoreDelegate(AddressOf OnMsg_UpdateNFIQScore), New Object() {score})
            Return
        End If

        m_txtNFIQScore.Text = score
    End Sub

    Private Delegate Sub OnMsg_AskRecaptureDelegate(ByVal imageStatus As Integer)
    Private Sub OnMsg_AskRecapture(ByVal imageStatus As Integer)
        ' Check if we need to call beginInvoke.
        If Me.InvokeRequired Then
            ' Pass the same function to BeginInvoke,
            ' but the call would come on the correct
            ' thread and InvokeRequired will be false
            Me.BeginInvoke(New OnMsg_AskRecaptureDelegate(AddressOf OnMsg_AskRecapture), New Object() {imageStatus})
            Return
        End If

        Dim strValue As String
        strValue = [String].Format("[Warning = {0}] Do you want a recapture?", imageStatus)

        Dim response As MsgBoxResult = MsgBox(strValue, MsgBoxStyle.YesNo)

        If response = MsgBoxResult.Yes Then
            m_nCurrentCaptureStep = m_nCurrentCaptureStep - 1
        End If

        OnMsg_CaptureSeqNext()
    End Sub




    '//////////////////////////////////////////////////////////////////////////////////////////
    ' Form event
    '//////////////////////////////////////////////////////////////////////////////////////////
    Private Sub SDKMainForm_Load(ByVal sender As Object, ByVal e As EventArgs) Handles MyBase.Load

        Dim netVersion As String
        netVersion = System.Runtime.InteropServices.RuntimeEnvironment.GetSystemVersion()
        If netVersion(1) < "2" Then
            MessageBox.Show("SDK works with .Net Version 2.0 and higher")
            Application.Exit()
        End If

        m_nDevHandle = -1
        m_nCurrentCaptureStep = -1
        m_bInitializing = False
        m_strImageMessage = ""
        m_bNeedClearPlaten = False
        m_bSaveWarningOfClearPlaten = False

        m_chkAutoContrast.Checked = True
        m_chkAutoCapture.Checked = True
        m_chkUseClearPlaten.Checked = True
        m_chkNFIQScore.Checked = True
        m_chkDrawSegmentImage.Checked = True
        m_chkDetectSmear.Checked = True
        m_cboSmearLevel.Items.Clear()
        m_cboSmearLevel.Items.Add("LOW")
        m_cboSmearLevel.Items.Add("MEDIUM")
        m_cboSmearLevel.Items.Add("HIGH")
        m_cboSmearLevel.SelectedIndex = 1

        m_sliderContrast.SetRange(DLL.IBSU_MIN_CONTRAST_VALUE, DLL.IBSU_MAX_CONTRAST_VALUE)
        m_sliderContrast.Value = 0
        m_sliderContrast.TickFrequency = 5
        m_txtContrast.Text = "0"

        m_verInfo = New DLL.IBSU_SdkVersion()
        DLL._IBSU_GetSDKVersion(m_verInfo)
        Dim titleName As String = "IntegrationSample for VB.NET"
        Me.Text = titleName

        m_callbackDeviceCommunicationBreak = New DLL.IBSU_Callback(AddressOf OnEvent_DeviceCommunicationBreak)
        m_callbackPreviewImage = New DLL.IBSU_CallbackPreviewImage(AddressOf OnEvent_PreviewImage)
        m_callbackFingerCount = New DLL.IBSU_CallbackFingerCount(AddressOf OnEvent_FingerCount)
        m_callbackFingerQuality = New DLL.IBSU_CallbackFingerQuality(AddressOf OnEvent_FingerQuality)
        m_callbackDeviceCount = New DLL.IBSU_CallbackDeviceCount(AddressOf OnEvent_DeviceCount)
        m_callbackInitProgress = New DLL.IBSU_CallbackInitProgress(AddressOf OnEvent_InitProgress)
        m_callbackTakingAcquisition = New DLL.IBSU_CallbackTakingAcquisition(AddressOf OnEvent_TakingAcquisition)
        m_callbackCompleteAcquisition = New DLL.IBSU_CallbackCompleteAcquisition(AddressOf OnEvent_CompleteAcquisition)
        m_callbackClearPlaten = New DLL.IBSU_CallbackClearPlatenAtCapture(AddressOf OnEvent_ClearPlatenAtCapture)
        m_callbackResultImageEx = New DLL.IBSU_CallbackResultImageEx(AddressOf OnEvent_ResultImageEx)
        m_callbackPressedKeyButtons = New DLL.IBSU_CallbackKeyButtons(AddressOf OnEvent_PressedKeyButtons)

        DLL._IBSU_RegisterCallbacks(0, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, m_callbackDeviceCount, Me.Handle)
        DLL._IBSU_RegisterCallbacks(0, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, m_callbackInitProgress, Me.Handle)


        OnMsg_UpdateDeviceList()
    End Sub

    Private Sub m_btnCaptureStart_Click(ByVal sender As Object, ByVal e As EventArgs) Handles m_btnCaptureStart.Click
        OnMsg_CaptureStartClick()
    End Sub

    Private Sub m_btnCaptureStop_Click(ByVal sender As Object, ByVal e As EventArgs) Handles m_btnCaptureStop.Click
        OnMsg_CaptureStopClick()
    End Sub

    Private Sub m_btnImageFolder_Click(ByVal sender As Object, ByVal e As EventArgs) Handles m_btnImageFolder.Click
        If folderBrowserDialog1.ShowDialog() = DialogResult.OK Then
            m_ImgSaveFolder = folderBrowserDialog1.SelectedPath
        End If

        If DLL._IBSU_IsWritableDirectory(m_ImgSaveFolder, True) <> DLL.IBSU_STATUS_OK Then
            MsgBox("You don't have writing permission on this folder\r\nPlease select another folder (e.g. desktop folder)", _
                        MsgBoxStyle.Critical, "Error")
        End If
    End Sub

    Private Sub Timer_StatusFingerQuality_Tick(ByVal sender As Object, ByVal e As EventArgs) Handles Timer_StatusFingerQuality.Tick
        Dim idle As [Boolean] = Not m_bInitializing AndAlso (m_nCurrentCaptureStep = -1)

        If Not idle Then
            If m_bNeedClearPlaten AndAlso m_bBlank Then
                If m_bSaveWarningOfClearPlaten = False Then
                    _ModifyOverlayForWarningOfClearPlaten(True)
                    m_bSaveWarningOfClearPlaten = True
                End If
            Else
                If m_bSaveWarningOfClearPlaten = True Then
                    _ModifyOverlayForWarningOfClearPlaten(False)
                    m_bSaveWarningOfClearPlaten = False
                End If
            End If
        End If

        m_picScanner.Invalidate()
        If m_bNeedClearPlaten Then
            m_bBlank = Not m_bBlank
        End If
    End Sub

    Private Sub SDKMainForm_FormClosing(ByVal sender As Object, ByVal e As FormClosingEventArgs) Handles MyBase.FormClosing
        _ReleaseDevice()
    End Sub

    Private Sub m_cboUsbDevices_SelectedIndexChanged(ByVal sender As Object, ByVal e As EventArgs) Handles m_cboUsbDevices.SelectedIndexChanged
        If m_cboUsbDevices.SelectedIndex = m_nSelectedDevIndex Then
            Return
        End If

        Cursor = Cursors.WaitCursor

        m_nSelectedDevIndex = m_cboUsbDevices.SelectedIndex
        If m_nDevHandle <> -1 Then
            m_btnCaptureStop_Click(sender, e)
            _ReleaseDevice()
        End If

        _UpdateCaptureSequences()

        Cursor = Cursors.[Default]
    End Sub

    Private Sub m_sliderContrast_Scroll(ByVal sender As Object, ByVal e As EventArgs) Handles m_sliderContrast.Scroll
        Dim pos As Integer = m_sliderContrast.Value
        m_txtContrast.Text = m_sliderContrast.Value.ToString()
        DLL._IBSU_SetContrast(m_nDevHandle, pos)
    End Sub

    Private Sub m_chkAutoContrast_CheckedChanged(ByVal sender As Object, ByVal e As EventArgs) Handles m_chkAutoContrast.CheckedChanged
        If m_chkAutoContrast.Checked Then
            m_sliderContrast.Enabled = False
            m_txtContrast.Enabled = False
            m_staticContrast.Enabled = False
        Else
            m_sliderContrast.Enabled = True
            m_txtContrast.Enabled = True
            m_staticContrast.Enabled = True
        End If
    End Sub

    Private Sub m_cboCaptureSeq_SelectedIndexChanged(ByVal sender As Object, ByVal e As EventArgs) Handles m_cboCaptureSeq.SelectedIndexChanged
        OnMsg_UpdateDisplayResources()
    End Sub


    Private Sub picIBLogo_Paint(ByVal sender As System.Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles m_picIBLogo.Paint
        Dim message As String = [String].Format("VB.NET sample with DLL ver. {0}", m_verInfo.Product)
        e.Graphics.DrawString(message, New Font("Times New Roman", 14), Brushes.White, 450, 30)
        e.Graphics.DrawString("Copyright (c) Integrated Biometrics", New Font("Arial", 9), Brushes.White, 450, 55)
    End Sub

    Private Sub m_picScanner_Paint(ByVal sender As System.Object, ByVal e As System.Windows.Forms.PaintEventArgs) Handles m_picScanner.Paint
        Dim hbr_finger As SolidBrush
        '        Dim hbr_touch As SolidBrush

        For i As Integer = 0 To 3
            If m_bNeedClearPlaten Then
                If m_bBlank Then
                    hbr_finger = New SolidBrush(Color.Red)
                Else
                    hbr_finger = New SolidBrush(Color.FromArgb(78, 78, 78))
                End If
            Else
                Select Case m_FingerQuality(i)
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_GOOD
                        hbr_finger = New SolidBrush(Color.Green)
                        Exit Select
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_FAIR
                        hbr_finger = New SolidBrush(Color.Orange)
                        Exit Select
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_POOR
                        hbr_finger = New SolidBrush(Color.Red)
                        Exit Select
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT
                        hbr_finger = New SolidBrush(Color.FromArgb(78, 78, 78))
                        Exit Select
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_INVALID_AREA_TOP
                        hbr_finger = New SolidBrush(Color.Red)
                        Exit Select
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_INVALID_AREA_BOTTOM
                        hbr_finger = New SolidBrush(Color.Red)
                        Exit Select
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_INVALID_AREA_LEFT
                        hbr_finger = New SolidBrush(Color.Red)
                        Exit Select
                    Case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT
                        hbr_finger = New SolidBrush(Color.Red)
                        Exit Select
                    Case Else
                        Return
                End Select
            End If

            _DrawRoundRect(e.Graphics, hbr_finger, 15 + i * 22, 30, 19, 70, 9)
        Next

        ' Draw detected finger on the touch sensor
        '        Dim touchInValue As Integer = 0
        '       DLL._IBSU_IsTouchedFinger(m_nDevHandle, touchInValue)
        '      If touchInValue = 1 Then
        'hbr_touch = New SolidBrush(Color.Green)
        'Else
        'hbr_touch = New SolidBrush(Color.FromArgb(78, 78, 78))
        'End If
        '
        '            g.FillRectangle(hbr_touch, 15, 112, 85, 5);
        '_DrawRoundRect(e.Graphics, hbr_touch, 15, 112, 85, 5, 2)
    End Sub
End Class
