namespace IBScanUltimate_SampleForCSharp
{
    partial class SDKMainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if( m_initThread != null )
                m_initThread.Abort();

            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SDKMainForm));
            this.panel1 = new System.Windows.Forms.Panel();
            this.m_cboSmearLevel = new System.Windows.Forms.ComboBox();
            this.m_chkDetectSmear = new System.Windows.Forms.CheckBox();
            this.m_chkInvalidArea = new System.Windows.Forms.CheckBox();
            this.m_chkDrawSegmentImage = new System.Windows.Forms.CheckBox();
            this.m_txtNFIQScore = new System.Windows.Forms.Label();
            this.m_chkNFIQScore = new System.Windows.Forms.CheckBox();
            this.m_chkUseClearPlaten = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.m_picScanner = new System.Windows.Forms.PictureBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.m_txtContrast = new System.Windows.Forms.Label();
            this.m_sliderContrast = new System.Windows.Forms.TrackBar();
            this.m_staticContrast = new System.Windows.Forms.Label();
            this.m_btnCaptureStart = new System.Windows.Forms.Button();
            this.m_btnCaptureStop = new System.Windows.Forms.Button();
            this.m_btnImageFolder = new System.Windows.Forms.Button();
            this.m_chkSaveImages = new System.Windows.Forms.CheckBox();
            this.m_chkIgnoreFingerCount = new System.Windows.Forms.CheckBox();
            this.m_chkAutoCapture = new System.Windows.Forms.CheckBox();
            this.m_chkAutoContrast = new System.Windows.Forms.CheckBox();
            this.m_cboCaptureSeq = new System.Windows.Forms.ComboBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.m_cboUsbDevices = new System.Windows.Forms.ComboBox();
            this.m_FrameImage = new System.Windows.Forms.Label();
            this.m_txtStatusMessage = new System.Windows.Forms.Label();
            this.Timer_StatusFingerQuality = new System.Windows.Forms.Timer(this.components);
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.m_picIBLogo = new System.Windows.Forms.PictureBox();
            this.panel1.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_picScanner)).BeginInit();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_sliderContrast)).BeginInit();
            this.groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_picIBLogo)).BeginInit();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.m_cboSmearLevel);
            this.panel1.Controls.Add(this.m_chkDetectSmear);
            this.panel1.Controls.Add(this.m_chkInvalidArea);
            this.panel1.Controls.Add(this.m_chkDrawSegmentImage);
            this.panel1.Controls.Add(this.m_txtNFIQScore);
            this.panel1.Controls.Add(this.m_chkNFIQScore);
            this.panel1.Controls.Add(this.m_chkUseClearPlaten);
            this.panel1.Controls.Add(this.groupBox3);
            this.panel1.Controls.Add(this.groupBox2);
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Location = new System.Drawing.Point(11, 74);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(287, 415);
            this.panel1.TabIndex = 1;
            // 
            // m_cboSmearLevel
            // 
            this.m_cboSmearLevel.FormattingEnabled = true;
            this.m_cboSmearLevel.Location = new System.Drawing.Point(226, 376);
            this.m_cboSmearLevel.Name = "m_cboSmearLevel";
            this.m_cboSmearLevel.Size = new System.Drawing.Size(50, 21);
            this.m_cboSmearLevel.TabIndex = 17;
            // 
            // m_chkDetectSmear
            // 
            this.m_chkDetectSmear.Location = new System.Drawing.Point(141, 376);
            this.m_chkDetectSmear.Margin = new System.Windows.Forms.Padding(3, 3, 0, 3);
            this.m_chkDetectSmear.Name = "m_chkDetectSmear";
            this.m_chkDetectSmear.Size = new System.Drawing.Size(89, 18);
            this.m_chkDetectSmear.TabIndex = 16;
            this.m_chkDetectSmear.Text = "Detect smear";
            this.m_chkDetectSmear.UseVisualStyleBackColor = true;
            // 
            // m_chkInvalidArea
            // 
            this.m_chkInvalidArea.Location = new System.Drawing.Point(141, 352);
            this.m_chkInvalidArea.Margin = new System.Windows.Forms.Padding(3, 3, 0, 3);
            this.m_chkInvalidArea.Name = "m_chkInvalidArea";
            this.m_chkInvalidArea.Size = new System.Drawing.Size(122, 18);
            this.m_chkInvalidArea.TabIndex = 15;
            this.m_chkInvalidArea.Text = "Invalid area";
            this.m_chkInvalidArea.UseVisualStyleBackColor = true;
            // 
            // m_chkDrawSegmentImage
            // 
            this.m_chkDrawSegmentImage.Location = new System.Drawing.Point(141, 293);
            this.m_chkDrawSegmentImage.Margin = new System.Windows.Forms.Padding(3, 3, 0, 3);
            this.m_chkDrawSegmentImage.Name = "m_chkDrawSegmentImage";
            this.m_chkDrawSegmentImage.Size = new System.Drawing.Size(123, 30);
            this.m_chkDrawSegmentImage.TabIndex = 14;
            this.m_chkDrawSegmentImage.Text = "Draw quadrangle for segment image";
            this.m_chkDrawSegmentImage.UseVisualStyleBackColor = true;
            // 
            // m_txtNFIQScore
            // 
            this.m_txtNFIQScore.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.m_txtNFIQScore.Location = new System.Drawing.Point(200, 328);
            this.m_txtNFIQScore.Name = "m_txtNFIQScore";
            this.m_txtNFIQScore.Size = new System.Drawing.Size(63, 18);
            this.m_txtNFIQScore.TabIndex = 12;
            this.m_txtNFIQScore.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // m_chkNFIQScore
            // 
            this.m_chkNFIQScore.Location = new System.Drawing.Point(141, 328);
            this.m_chkNFIQScore.Margin = new System.Windows.Forms.Padding(3, 3, 0, 3);
            this.m_chkNFIQScore.Name = "m_chkNFIQScore";
            this.m_chkNFIQScore.Size = new System.Drawing.Size(56, 18);
            this.m_chkNFIQScore.TabIndex = 5;
            this.m_chkNFIQScore.Text = "NFIQ";
            this.m_chkNFIQScore.UseVisualStyleBackColor = true;
            // 
            // m_chkUseClearPlaten
            // 
            this.m_chkUseClearPlaten.Location = new System.Drawing.Point(141, 272);
            this.m_chkUseClearPlaten.Margin = new System.Windows.Forms.Padding(3, 3, 0, 3);
            this.m_chkUseClearPlaten.Name = "m_chkUseClearPlaten";
            this.m_chkUseClearPlaten.Size = new System.Drawing.Size(143, 18);
            this.m_chkUseClearPlaten.TabIndex = 4;
            this.m_chkUseClearPlaten.Text = "Detect clear platen";
            this.m_chkUseClearPlaten.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.m_picScanner);
            this.groupBox3.Location = new System.Drawing.Point(0, 263);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(135, 151);
            this.groupBox3.TabIndex = 2;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Device Quality";
            // 
            // m_picScanner
            // 
            this.m_picScanner.ErrorImage = null;
            this.m_picScanner.Image = ((System.Drawing.Image)(resources.GetObject("m_picScanner.Image")));
            this.m_picScanner.Location = new System.Drawing.Point(9, 18);
            this.m_picScanner.Name = "m_picScanner";
            this.m_picScanner.Size = new System.Drawing.Size(120, 127);
            this.m_picScanner.TabIndex = 5;
            this.m_picScanner.TabStop = false;
            this.m_picScanner.Paint += new System.Windows.Forms.PaintEventHandler(this.m_picScanner_Paint);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.m_txtContrast);
            this.groupBox2.Controls.Add(this.m_sliderContrast);
            this.groupBox2.Controls.Add(this.m_staticContrast);
            this.groupBox2.Controls.Add(this.m_btnCaptureStart);
            this.groupBox2.Controls.Add(this.m_btnCaptureStop);
            this.groupBox2.Controls.Add(this.m_btnImageFolder);
            this.groupBox2.Controls.Add(this.m_chkSaveImages);
            this.groupBox2.Controls.Add(this.m_chkIgnoreFingerCount);
            this.groupBox2.Controls.Add(this.m_chkAutoCapture);
            this.groupBox2.Controls.Add(this.m_chkAutoContrast);
            this.groupBox2.Controls.Add(this.m_cboCaptureSeq);
            this.groupBox2.Location = new System.Drawing.Point(0, 61);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(276, 196);
            this.groupBox2.TabIndex = 1;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Fingerprint Capture";
            // 
            // m_txtContrast
            // 
            this.m_txtContrast.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.m_txtContrast.Location = new System.Drawing.Point(226, 162);
            this.m_txtContrast.Name = "m_txtContrast";
            this.m_txtContrast.Size = new System.Drawing.Size(37, 18);
            this.m_txtContrast.TabIndex = 11;
            this.m_txtContrast.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // m_sliderContrast
            // 
            this.m_sliderContrast.AutoSize = false;
            this.m_sliderContrast.Location = new System.Drawing.Point(52, 162);
            this.m_sliderContrast.Maximum = 34;
            this.m_sliderContrast.Name = "m_sliderContrast";
            this.m_sliderContrast.Size = new System.Drawing.Size(168, 19);
            this.m_sliderContrast.TabIndex = 10;
            this.m_sliderContrast.TickFrequency = 5;
            this.m_sliderContrast.Scroll += new System.EventHandler(this.m_sliderContrast_Scroll);
            // 
            // m_staticContrast
            // 
            this.m_staticContrast.AutoSize = true;
            this.m_staticContrast.Location = new System.Drawing.Point(7, 162);
            this.m_staticContrast.Name = "m_staticContrast";
            this.m_staticContrast.Size = new System.Drawing.Size(46, 13);
            this.m_staticContrast.TabIndex = 9;
            this.m_staticContrast.Text = "Contrast";
            // 
            // m_btnCaptureStart
            // 
            this.m_btnCaptureStart.Location = new System.Drawing.Point(156, 129);
            this.m_btnCaptureStart.Name = "m_btnCaptureStart";
            this.m_btnCaptureStart.Size = new System.Drawing.Size(108, 19);
            this.m_btnCaptureStart.TabIndex = 8;
            this.m_btnCaptureStart.Text = "Start";
            this.m_btnCaptureStart.UseVisualStyleBackColor = true;
            this.m_btnCaptureStart.Click += new System.EventHandler(this.m_btnCaptureStart_Click);
            // 
            // m_btnCaptureStop
            // 
            this.m_btnCaptureStop.Location = new System.Drawing.Point(10, 129);
            this.m_btnCaptureStop.Name = "m_btnCaptureStop";
            this.m_btnCaptureStop.Size = new System.Drawing.Size(108, 19);
            this.m_btnCaptureStop.TabIndex = 7;
            this.m_btnCaptureStop.Text = "Stop";
            this.m_btnCaptureStop.UseVisualStyleBackColor = true;
            this.m_btnCaptureStop.Click += new System.EventHandler(this.m_btnCaptureStop_Click);
            // 
            // m_btnImageFolder
            // 
            this.m_btnImageFolder.Location = new System.Drawing.Point(102, 100);
            this.m_btnImageFolder.Name = "m_btnImageFolder";
            this.m_btnImageFolder.Size = new System.Drawing.Size(33, 19);
            this.m_btnImageFolder.TabIndex = 6;
            this.m_btnImageFolder.Text = "...";
            this.m_btnImageFolder.UseVisualStyleBackColor = true;
            this.m_btnImageFolder.Click += new System.EventHandler(this.m_btnImageFolder_Click);
            // 
            // m_chkSaveImages
            // 
            this.m_chkSaveImages.AutoSize = true;
            this.m_chkSaveImages.Location = new System.Drawing.Point(10, 102);
            this.m_chkSaveImages.Name = "m_chkSaveImages";
            this.m_chkSaveImages.Size = new System.Drawing.Size(87, 17);
            this.m_chkSaveImages.TabIndex = 5;
            this.m_chkSaveImages.Text = "Save images";
            this.m_chkSaveImages.UseVisualStyleBackColor = true;
            // 
            // m_chkIgnoreFingerCount
            // 
            this.m_chkIgnoreFingerCount.AutoSize = true;
            this.m_chkIgnoreFingerCount.Location = new System.Drawing.Point(10, 80);
            this.m_chkIgnoreFingerCount.Name = "m_chkIgnoreFingerCount";
            this.m_chkIgnoreFingerCount.Size = new System.Drawing.Size(258, 17);
            this.m_chkIgnoreFingerCount.TabIndex = 4;
            this.m_chkIgnoreFingerCount.Text = "Trigger invalid finger count on auto-capture mode";
            this.m_chkIgnoreFingerCount.UseVisualStyleBackColor = true;
            // 
            // m_chkAutoCapture
            // 
            this.m_chkAutoCapture.AutoSize = true;
            this.m_chkAutoCapture.Location = new System.Drawing.Point(10, 61);
            this.m_chkAutoCapture.Name = "m_chkAutoCapture";
            this.m_chkAutoCapture.Size = new System.Drawing.Size(181, 17);
            this.m_chkAutoCapture.TabIndex = 3;
            this.m_chkAutoCapture.Text = "Automatic capture for fingerprints";
            this.m_chkAutoCapture.UseVisualStyleBackColor = true;
            // 
            // m_chkAutoContrast
            // 
            this.m_chkAutoContrast.AutoSize = true;
            this.m_chkAutoContrast.Location = new System.Drawing.Point(10, 44);
            this.m_chkAutoContrast.Name = "m_chkAutoContrast";
            this.m_chkAutoContrast.Size = new System.Drawing.Size(172, 17);
            this.m_chkAutoContrast.TabIndex = 2;
            this.m_chkAutoContrast.Text = "Automatic contrast optimization";
            this.m_chkAutoContrast.UseVisualStyleBackColor = true;
            this.m_chkAutoContrast.CheckedChanged += new System.EventHandler(this.m_chkAutoContrast_CheckedChanged);
            // 
            // m_cboCaptureSeq
            // 
            this.m_cboCaptureSeq.FormattingEnabled = true;
            this.m_cboCaptureSeq.Location = new System.Drawing.Point(10, 18);
            this.m_cboCaptureSeq.Name = "m_cboCaptureSeq";
            this.m_cboCaptureSeq.Size = new System.Drawing.Size(254, 21);
            this.m_cboCaptureSeq.TabIndex = 1;
            this.m_cboCaptureSeq.SelectedIndexChanged += new System.EventHandler(this.m_cboCaptureSeq_SelectedIndexChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.m_cboUsbDevices);
            this.groupBox1.Location = new System.Drawing.Point(0, 3);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(276, 53);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Devices";
            // 
            // m_cboUsbDevices
            // 
            this.m_cboUsbDevices.FormattingEnabled = true;
            this.m_cboUsbDevices.Location = new System.Drawing.Point(10, 18);
            this.m_cboUsbDevices.Name = "m_cboUsbDevices";
            this.m_cboUsbDevices.Size = new System.Drawing.Size(254, 21);
            this.m_cboUsbDevices.TabIndex = 0;
            this.m_cboUsbDevices.SelectedIndexChanged += new System.EventHandler(this.m_cboUsbDevices_SelectedIndexChanged);
            // 
            // m_FrameImage
            // 
            this.m_FrameImage.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.m_FrameImage.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.m_FrameImage.Location = new System.Drawing.Point(298, 84);
            this.m_FrameImage.Name = "m_FrameImage";
            this.m_FrameImage.Size = new System.Drawing.Size(468, 405);
            this.m_FrameImage.TabIndex = 2;
            // 
            // m_txtStatusMessage
            // 
            this.m_txtStatusMessage.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.m_txtStatusMessage.Location = new System.Drawing.Point(10, 500);
            this.m_txtStatusMessage.Name = "m_txtStatusMessage";
            this.m_txtStatusMessage.Size = new System.Drawing.Size(755, 18);
            this.m_txtStatusMessage.TabIndex = 3;
            this.m_txtStatusMessage.Text = "Ready";
            // 
            // Timer_StatusFingerQuality
            // 
            this.Timer_StatusFingerQuality.Enabled = true;
            this.Timer_StatusFingerQuality.Interval = 500;
            this.Timer_StatusFingerQuality.Tick += new System.EventHandler(this.Timer_StatusFingerQuality_Tick);
            // 
            // m_picIBLogo
            // 
            this.m_picIBLogo.ErrorImage = null;
            this.m_picIBLogo.Image = ((System.Drawing.Image)(resources.GetObject("m_picIBLogo.Image")));
            this.m_picIBLogo.Location = new System.Drawing.Point(-2, -3);
            this.m_picIBLogo.Name = "m_picIBLogo";
            this.m_picIBLogo.Size = new System.Drawing.Size(780, 74);
            this.m_picIBLogo.TabIndex = 6;
            this.m_picIBLogo.TabStop = false;
            this.m_picIBLogo.Paint += new System.Windows.Forms.PaintEventHandler(this.m_picIBLogo_Paint);
            // 
            // SDKMainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(777, 527);
            this.Controls.Add(this.m_txtStatusMessage);
            this.Controls.Add(this.m_picIBLogo);
            this.Controls.Add(this.m_FrameImage);
            this.Controls.Add(this.panel1);
            this.DoubleBuffered = true;
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SDKMainForm";
            this.Text = "IntegrationSample for C#";
            this.Load += new System.EventHandler(this.SDKMainForm_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.SDKMainForm_FormClosing);
            this.panel1.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.m_picScanner)).EndInit();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.m_sliderContrast)).EndInit();
            this.groupBox1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.m_picIBLogo)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.CheckBox m_chkAutoContrast;
        private System.Windows.Forms.ComboBox m_cboCaptureSeq;
        private System.Windows.Forms.ComboBox m_cboUsbDevices;
        private System.Windows.Forms.CheckBox m_chkIgnoreFingerCount;
        private System.Windows.Forms.CheckBox m_chkAutoCapture;
        private System.Windows.Forms.Label m_FrameImage;
        private System.Windows.Forms.Button m_btnImageFolder;
        private System.Windows.Forms.CheckBox m_chkSaveImages;
        private System.Windows.Forms.Label m_staticContrast;
        private System.Windows.Forms.Button m_btnCaptureStart;
        private System.Windows.Forms.Button m_btnCaptureStop;
        private System.Windows.Forms.TrackBar m_sliderContrast;
        private System.Windows.Forms.Label m_txtContrast;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label m_txtStatusMessage;
        private System.Windows.Forms.CheckBox m_chkUseClearPlaten;
        private System.Windows.Forms.Timer Timer_StatusFingerQuality;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        private System.Windows.Forms.PictureBox m_picScanner;
        private System.Windows.Forms.PictureBox m_picIBLogo;
        private System.Windows.Forms.Label m_txtNFIQScore;
        private System.Windows.Forms.CheckBox m_chkNFIQScore;
        private System.Windows.Forms.CheckBox m_chkDrawSegmentImage;
        private System.Windows.Forms.CheckBox m_chkInvalidArea;
        private System.Windows.Forms.CheckBox m_chkDetectSmear;
        private System.Windows.Forms.ComboBox m_cboSmearLevel;
    }
}

