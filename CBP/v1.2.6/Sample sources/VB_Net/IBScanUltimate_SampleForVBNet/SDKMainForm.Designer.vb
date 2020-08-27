<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class SDKMainForm
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If m_initThread IsNot Nothing Then
                m_initThread.Abort()
            End If

            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(SDKMainForm))
        Me.m_txtStatusMessage = New System.Windows.Forms.Label
        Me.m_FrameImage = New System.Windows.Forms.Label
        Me.panel1 = New System.Windows.Forms.Panel
        Me.m_chkDrawSegmentImage = New System.Windows.Forms.CheckBox
        Me.m_txtNFIQScore = New System.Windows.Forms.Label
        Me.m_chkNFIQScore = New System.Windows.Forms.CheckBox
        Me.m_chkUseClearPlaten = New System.Windows.Forms.CheckBox
        Me.groupBox3 = New System.Windows.Forms.GroupBox
        Me.m_picScanner = New System.Windows.Forms.PictureBox
        Me.groupBox2 = New System.Windows.Forms.GroupBox
        Me.m_txtContrast = New System.Windows.Forms.Label
        Me.m_sliderContrast = New System.Windows.Forms.TrackBar
        Me.m_staticContrast = New System.Windows.Forms.Label
        Me.m_btnCaptureStart = New System.Windows.Forms.Button
        Me.m_btnCaptureStop = New System.Windows.Forms.Button
        Me.m_btnImageFolder = New System.Windows.Forms.Button
        Me.m_chkSaveImages = New System.Windows.Forms.CheckBox
        Me.m_chkIgnoreFingerCount = New System.Windows.Forms.CheckBox
        Me.m_chkAutoCapture = New System.Windows.Forms.CheckBox
        Me.m_chkAutoContrast = New System.Windows.Forms.CheckBox
        Me.m_cboCaptureSeq = New System.Windows.Forms.ComboBox
        Me.groupBox1 = New System.Windows.Forms.GroupBox
        Me.m_cboUsbDevices = New System.Windows.Forms.ComboBox
        Me.m_picIBLogo = New System.Windows.Forms.PictureBox
        Me.Timer_StatusFingerQuality = New System.Windows.Forms.Timer(Me.components)
        Me.folderBrowserDialog1 = New System.Windows.Forms.FolderBrowserDialog
        Me.m_cboSmearLevel = New System.Windows.Forms.ComboBox
        Me.m_chkDetectSmear = New System.Windows.Forms.CheckBox
        Me.m_chkInvalidArea = New System.Windows.Forms.CheckBox
        Me.panel1.SuspendLayout()
        Me.groupBox3.SuspendLayout()
        CType(Me.m_picScanner, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.groupBox2.SuspendLayout()
        CType(Me.m_sliderContrast, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.groupBox1.SuspendLayout()
        CType(Me.m_picIBLogo, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.SuspendLayout()
        '
        'm_txtStatusMessage
        '
        Me.m_txtStatusMessage.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.m_txtStatusMessage.Location = New System.Drawing.Point(8, 542)
        Me.m_txtStatusMessage.Name = "m_txtStatusMessage"
        Me.m_txtStatusMessage.Size = New System.Drawing.Size(759, 20)
        Me.m_txtStatusMessage.TabIndex = 7
        Me.m_txtStatusMessage.Text = "Ready"
        '
        'm_FrameImage
        '
        Me.m_FrameImage.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.m_FrameImage.FlatStyle = System.Windows.Forms.FlatStyle.Flat
        Me.m_FrameImage.Location = New System.Drawing.Point(304, 91)
        Me.m_FrameImage.Name = "m_FrameImage"
        Me.m_FrameImage.Size = New System.Drawing.Size(463, 439)
        Me.m_FrameImage.TabIndex = 6
        '
        'panel1
        '
        Me.panel1.Controls.Add(Me.m_cboSmearLevel)
        Me.panel1.Controls.Add(Me.m_chkDetectSmear)
        Me.panel1.Controls.Add(Me.m_chkInvalidArea)
        Me.panel1.Controls.Add(Me.m_chkDrawSegmentImage)
        Me.panel1.Controls.Add(Me.m_txtNFIQScore)
        Me.panel1.Controls.Add(Me.m_chkNFIQScore)
        Me.panel1.Controls.Add(Me.m_chkUseClearPlaten)
        Me.panel1.Controls.Add(Me.groupBox3)
        Me.panel1.Controls.Add(Me.groupBox2)
        Me.panel1.Controls.Add(Me.groupBox1)
        Me.panel1.Location = New System.Drawing.Point(9, 80)
        Me.panel1.Name = "panel1"
        Me.panel1.Size = New System.Drawing.Size(281, 450)
        Me.panel1.TabIndex = 5
        '
        'm_chkDrawSegmentImage
        '
        Me.m_chkDrawSegmentImage.Location = New System.Drawing.Point(144, 319)
        Me.m_chkDrawSegmentImage.Margin = New System.Windows.Forms.Padding(3, 3, 0, 3)
        Me.m_chkDrawSegmentImage.Name = "m_chkDrawSegmentImage"
        Me.m_chkDrawSegmentImage.Size = New System.Drawing.Size(123, 30)
        Me.m_chkDrawSegmentImage.TabIndex = 13
        Me.m_chkDrawSegmentImage.Text = "Draw quadrangle for segment image"
        Me.m_chkDrawSegmentImage.UseVisualStyleBackColor = True
        '
        'm_txtNFIQScore
        '
        Me.m_txtNFIQScore.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.m_txtNFIQScore.Location = New System.Drawing.Point(207, 354)
        Me.m_txtNFIQScore.Name = "m_txtNFIQScore"
        Me.m_txtNFIQScore.Size = New System.Drawing.Size(60, 20)
        Me.m_txtNFIQScore.TabIndex = 12
        Me.m_txtNFIQScore.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'm_chkNFIQScore
        '
        Me.m_chkNFIQScore.Location = New System.Drawing.Point(144, 354)
        Me.m_chkNFIQScore.Margin = New System.Windows.Forms.Padding(3, 3, 0, 3)
        Me.m_chkNFIQScore.Name = "m_chkNFIQScore"
        Me.m_chkNFIQScore.Size = New System.Drawing.Size(60, 20)
        Me.m_chkNFIQScore.TabIndex = 5
        Me.m_chkNFIQScore.Text = "NFIQ score"
        Me.m_chkNFIQScore.UseVisualStyleBackColor = True
        '
        'm_chkUseClearPlaten
        '
        Me.m_chkUseClearPlaten.Location = New System.Drawing.Point(144, 297)
        Me.m_chkUseClearPlaten.Margin = New System.Windows.Forms.Padding(3, 3, 0, 3)
        Me.m_chkUseClearPlaten.Name = "m_chkUseClearPlaten"
        Me.m_chkUseClearPlaten.Size = New System.Drawing.Size(123, 19)
        Me.m_chkUseClearPlaten.TabIndex = 4
        Me.m_chkUseClearPlaten.Text = "Detect clear platen"
        Me.m_chkUseClearPlaten.UseVisualStyleBackColor = True
        '
        'groupBox3
        '
        Me.groupBox3.Controls.Add(Me.m_picScanner)
        Me.groupBox3.Location = New System.Drawing.Point(0, 285)
        Me.groupBox3.Name = "groupBox3"
        Me.groupBox3.Size = New System.Drawing.Size(138, 162)
        Me.groupBox3.TabIndex = 2
        Me.groupBox3.TabStop = False
        Me.groupBox3.Text = "Device Quality"
        '
        'm_picScanner
        '
        Me.m_picScanner.ErrorImage = Nothing
        Me.m_picScanner.Image = CType(resources.GetObject("m_picScanner.Image"), System.Drawing.Image)
        Me.m_picScanner.Location = New System.Drawing.Point(9, 20)
        Me.m_picScanner.Name = "m_picScanner"
        Me.m_picScanner.Size = New System.Drawing.Size(120, 127)
        Me.m_picScanner.TabIndex = 0
        Me.m_picScanner.TabStop = False
        '
        'groupBox2
        '
        Me.groupBox2.Controls.Add(Me.m_txtContrast)
        Me.groupBox2.Controls.Add(Me.m_sliderContrast)
        Me.groupBox2.Controls.Add(Me.m_staticContrast)
        Me.groupBox2.Controls.Add(Me.m_btnCaptureStart)
        Me.groupBox2.Controls.Add(Me.m_btnCaptureStop)
        Me.groupBox2.Controls.Add(Me.m_btnImageFolder)
        Me.groupBox2.Controls.Add(Me.m_chkSaveImages)
        Me.groupBox2.Controls.Add(Me.m_chkIgnoreFingerCount)
        Me.groupBox2.Controls.Add(Me.m_chkAutoCapture)
        Me.groupBox2.Controls.Add(Me.m_chkAutoContrast)
        Me.groupBox2.Controls.Add(Me.m_cboCaptureSeq)
        Me.groupBox2.Location = New System.Drawing.Point(0, 66)
        Me.groupBox2.Name = "groupBox2"
        Me.groupBox2.Size = New System.Drawing.Size(278, 212)
        Me.groupBox2.TabIndex = 1
        Me.groupBox2.TabStop = False
        Me.groupBox2.Text = "Fingerprint Capture"
        '
        'm_txtContrast
        '
        Me.m_txtContrast.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D
        Me.m_txtContrast.Location = New System.Drawing.Point(235, 172)
        Me.m_txtContrast.Name = "m_txtContrast"
        Me.m_txtContrast.Size = New System.Drawing.Size(32, 20)
        Me.m_txtContrast.TabIndex = 11
        Me.m_txtContrast.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'm_sliderContrast
        '
        Me.m_sliderContrast.AutoSize = False
        Me.m_sliderContrast.Location = New System.Drawing.Point(53, 176)
        Me.m_sliderContrast.Maximum = 34
        Me.m_sliderContrast.Name = "m_sliderContrast"
        Me.m_sliderContrast.Size = New System.Drawing.Size(174, 16)
        Me.m_sliderContrast.TabIndex = 10
        Me.m_sliderContrast.TickFrequency = 5
        '
        'm_staticContrast
        '
        Me.m_staticContrast.AutoSize = True
        Me.m_staticContrast.Location = New System.Drawing.Point(6, 176)
        Me.m_staticContrast.Name = "m_staticContrast"
        Me.m_staticContrast.Size = New System.Drawing.Size(46, 13)
        Me.m_staticContrast.TabIndex = 9
        Me.m_staticContrast.Text = "Contrast"
        '
        'm_btnCaptureStart
        '
        Me.m_btnCaptureStart.Location = New System.Drawing.Point(160, 140)
        Me.m_btnCaptureStart.Name = "m_btnCaptureStart"
        Me.m_btnCaptureStart.Size = New System.Drawing.Size(107, 21)
        Me.m_btnCaptureStart.TabIndex = 8
        Me.m_btnCaptureStart.Text = "Start"
        Me.m_btnCaptureStart.UseVisualStyleBackColor = True
        '
        'm_btnCaptureStop
        '
        Me.m_btnCaptureStop.Location = New System.Drawing.Point(9, 140)
        Me.m_btnCaptureStop.Name = "m_btnCaptureStop"
        Me.m_btnCaptureStop.Size = New System.Drawing.Size(107, 21)
        Me.m_btnCaptureStop.TabIndex = 7
        Me.m_btnCaptureStop.Text = "Stop"
        Me.m_btnCaptureStop.UseVisualStyleBackColor = True
        '
        'm_btnImageFolder
        '
        Me.m_btnImageFolder.Location = New System.Drawing.Point(101, 108)
        Me.m_btnImageFolder.Name = "m_btnImageFolder"
        Me.m_btnImageFolder.Size = New System.Drawing.Size(28, 21)
        Me.m_btnImageFolder.TabIndex = 6
        Me.m_btnImageFolder.Text = "..."
        Me.m_btnImageFolder.UseVisualStyleBackColor = True
        '
        'm_chkSaveImages
        '
        Me.m_chkSaveImages.AutoSize = True
        Me.m_chkSaveImages.Location = New System.Drawing.Point(9, 111)
        Me.m_chkSaveImages.Name = "m_chkSaveImages"
        Me.m_chkSaveImages.Size = New System.Drawing.Size(87, 17)
        Me.m_chkSaveImages.TabIndex = 5
        Me.m_chkSaveImages.Text = "Save images"
        Me.m_chkSaveImages.UseVisualStyleBackColor = True
        '
        'm_chkIgnoreFingerCount
        '
        Me.m_chkIgnoreFingerCount.AutoSize = True
        Me.m_chkIgnoreFingerCount.Location = New System.Drawing.Point(9, 87)
        Me.m_chkIgnoreFingerCount.Name = "m_chkIgnoreFingerCount"
        Me.m_chkIgnoreFingerCount.Size = New System.Drawing.Size(258, 17)
        Me.m_chkIgnoreFingerCount.TabIndex = 4
        Me.m_chkIgnoreFingerCount.Text = "Trigger invalid finger count on auto-capture mode"
        Me.m_chkIgnoreFingerCount.UseVisualStyleBackColor = True
        '
        'm_chkAutoCapture
        '
        Me.m_chkAutoCapture.AutoSize = True
        Me.m_chkAutoCapture.Location = New System.Drawing.Point(9, 66)
        Me.m_chkAutoCapture.Name = "m_chkAutoCapture"
        Me.m_chkAutoCapture.Size = New System.Drawing.Size(181, 17)
        Me.m_chkAutoCapture.TabIndex = 3
        Me.m_chkAutoCapture.Text = "Automatic capture for fingerprints"
        Me.m_chkAutoCapture.UseVisualStyleBackColor = True
        '
        'm_chkAutoContrast
        '
        Me.m_chkAutoContrast.AutoSize = True
        Me.m_chkAutoContrast.Location = New System.Drawing.Point(9, 48)
        Me.m_chkAutoContrast.Name = "m_chkAutoContrast"
        Me.m_chkAutoContrast.Size = New System.Drawing.Size(172, 17)
        Me.m_chkAutoContrast.TabIndex = 2
        Me.m_chkAutoContrast.Text = "Automatic contrast optimization"
        Me.m_chkAutoContrast.UseVisualStyleBackColor = True
        '
        'm_cboCaptureSeq
        '
        Me.m_cboCaptureSeq.FormattingEnabled = True
        Me.m_cboCaptureSeq.Location = New System.Drawing.Point(9, 20)
        Me.m_cboCaptureSeq.Name = "m_cboCaptureSeq"
        Me.m_cboCaptureSeq.Size = New System.Drawing.Size(258, 21)
        Me.m_cboCaptureSeq.TabIndex = 1
        '
        'groupBox1
        '
        Me.groupBox1.Controls.Add(Me.m_cboUsbDevices)
        Me.groupBox1.Location = New System.Drawing.Point(0, 3)
        Me.groupBox1.Name = "groupBox1"
        Me.groupBox1.Size = New System.Drawing.Size(278, 57)
        Me.groupBox1.TabIndex = 0
        Me.groupBox1.TabStop = False
        Me.groupBox1.Text = "Devices"
        '
        'm_cboUsbDevices
        '
        Me.m_cboUsbDevices.FormattingEnabled = True
        Me.m_cboUsbDevices.Location = New System.Drawing.Point(9, 20)
        Me.m_cboUsbDevices.Name = "m_cboUsbDevices"
        Me.m_cboUsbDevices.Size = New System.Drawing.Size(258, 21)
        Me.m_cboUsbDevices.TabIndex = 0
        '
        'm_picIBLogo
        '
        Me.m_picIBLogo.ErrorImage = Nothing
        Me.m_picIBLogo.Image = CType(resources.GetObject("m_picIBLogo.Image"), System.Drawing.Image)
        Me.m_picIBLogo.Location = New System.Drawing.Point(-1, 0)
        Me.m_picIBLogo.Name = "m_picIBLogo"
        Me.m_picIBLogo.Size = New System.Drawing.Size(781, 80)
        Me.m_picIBLogo.TabIndex = 4
        Me.m_picIBLogo.TabStop = False
        '
        'Timer_StatusFingerQuality
        '
        Me.Timer_StatusFingerQuality.Enabled = True
        Me.Timer_StatusFingerQuality.Interval = 300
        '
        'm_cboSmearLevel
        '
        Me.m_cboSmearLevel.FormattingEnabled = True
        Me.m_cboSmearLevel.Location = New System.Drawing.Point(229, 407)
        Me.m_cboSmearLevel.Name = "m_cboSmearLevel"
        Me.m_cboSmearLevel.Size = New System.Drawing.Size(50, 21)
        Me.m_cboSmearLevel.TabIndex = 20
        '
        'm_chkDetectSmear
        '
        Me.m_chkDetectSmear.Location = New System.Drawing.Point(144, 407)
        Me.m_chkDetectSmear.Margin = New System.Windows.Forms.Padding(3, 3, 0, 3)
        Me.m_chkDetectSmear.Name = "m_chkDetectSmear"
        Me.m_chkDetectSmear.Size = New System.Drawing.Size(89, 18)
        Me.m_chkDetectSmear.TabIndex = 19
        Me.m_chkDetectSmear.Text = "Detect smear"
        Me.m_chkDetectSmear.UseVisualStyleBackColor = True
        '
        'm_chkInvalidArea
        '
        Me.m_chkInvalidArea.Location = New System.Drawing.Point(144, 383)
        Me.m_chkInvalidArea.Margin = New System.Windows.Forms.Padding(3, 3, 0, 3)
        Me.m_chkInvalidArea.Name = "m_chkInvalidArea"
        Me.m_chkInvalidArea.Size = New System.Drawing.Size(122, 18)
        Me.m_chkInvalidArea.TabIndex = 18
        Me.m_chkInvalidArea.Text = "Invalid area"
        Me.m_chkInvalidArea.UseVisualStyleBackColor = True
        '
        'SDKMainForm
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(779, 575)
        Me.Controls.Add(Me.m_txtStatusMessage)
        Me.Controls.Add(Me.m_FrameImage)
        Me.Controls.Add(Me.panel1)
        Me.Controls.Add(Me.m_picIBLogo)
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.Name = "SDKMainForm"
        Me.Text = "IntegrationSample for VB.Net"
        Me.panel1.ResumeLayout(False)
        Me.groupBox3.ResumeLayout(False)
        CType(Me.m_picScanner, System.ComponentModel.ISupportInitialize).EndInit()
        Me.groupBox2.ResumeLayout(False)
        Me.groupBox2.PerformLayout()
        CType(Me.m_sliderContrast, System.ComponentModel.ISupportInitialize).EndInit()
        Me.groupBox1.ResumeLayout(False)
        CType(Me.m_picIBLogo, System.ComponentModel.ISupportInitialize).EndInit()
        Me.ResumeLayout(False)

    End Sub
    Private WithEvents m_txtStatusMessage As System.Windows.Forms.Label
    Private WithEvents m_FrameImage As System.Windows.Forms.Label
    Private WithEvents panel1 As System.Windows.Forms.Panel
    Private WithEvents m_chkUseClearPlaten As System.Windows.Forms.CheckBox
    Private WithEvents groupBox3 As System.Windows.Forms.GroupBox
    Private WithEvents m_picScanner As System.Windows.Forms.PictureBox
    Private WithEvents groupBox2 As System.Windows.Forms.GroupBox
    Private WithEvents m_txtContrast As System.Windows.Forms.Label
    Private WithEvents m_sliderContrast As System.Windows.Forms.TrackBar
    Private WithEvents m_staticContrast As System.Windows.Forms.Label
    Private WithEvents m_btnCaptureStart As System.Windows.Forms.Button
    Private WithEvents m_btnCaptureStop As System.Windows.Forms.Button
    Private WithEvents m_btnImageFolder As System.Windows.Forms.Button
    Private WithEvents m_chkSaveImages As System.Windows.Forms.CheckBox
    Private WithEvents m_chkIgnoreFingerCount As System.Windows.Forms.CheckBox
    Private WithEvents m_chkAutoCapture As System.Windows.Forms.CheckBox
    Private WithEvents m_chkAutoContrast As System.Windows.Forms.CheckBox
    Private WithEvents m_cboCaptureSeq As System.Windows.Forms.ComboBox
    Private WithEvents groupBox1 As System.Windows.Forms.GroupBox
    Private WithEvents m_cboUsbDevices As System.Windows.Forms.ComboBox
    Private WithEvents m_picIBLogo As System.Windows.Forms.PictureBox
    Private WithEvents Timer_StatusFingerQuality As System.Windows.Forms.Timer
    Friend WithEvents folderBrowserDialog1 As System.Windows.Forms.FolderBrowserDialog
    Private WithEvents m_txtNFIQScore As System.Windows.Forms.Label
    Private WithEvents m_chkNFIQScore As System.Windows.Forms.CheckBox
    Private WithEvents m_chkDrawSegmentImage As System.Windows.Forms.CheckBox
    Private WithEvents m_cboSmearLevel As System.Windows.Forms.ComboBox
    Private WithEvents m_chkDetectSmear As System.Windows.Forms.CheckBox
    Private WithEvents m_chkInvalidArea As System.Windows.Forms.CheckBox

End Class
