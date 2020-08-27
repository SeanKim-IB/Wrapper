namespace IBSU_CSharpWM
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
            this.m_cboUsbDevices = new System.Windows.Forms.ComboBox();
            this.m_picScanner = new System.Windows.Forms.PictureBox();
            this.m_FrameImage = new System.Windows.Forms.Label();
            this.m_txtStatusMessage = new System.Windows.Forms.TextBox();
            this.m_btnCaptureStop = new System.Windows.Forms.Button();
            this.m_btnCaptureStart = new System.Windows.Forms.Button();
            this.m_btnClose = new System.Windows.Forms.Button();
            this.Timer_StatusFingerQuality = new System.Windows.Forms.Timer();
            this.SuspendLayout();
            // 
            // m_cboUsbDevices
            // 
            this.m_cboUsbDevices.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Regular);
            this.m_cboUsbDevices.Location = new System.Drawing.Point(3, 3);
            this.m_cboUsbDevices.Name = "m_cboUsbDevices";
            this.m_cboUsbDevices.Size = new System.Drawing.Size(209, 20);
            this.m_cboUsbDevices.TabIndex = 0;
            this.m_cboUsbDevices.SelectedIndexChanged += new System.EventHandler(this.m_cboUsbDevices_SelectedIndexChanged);
            // 
            // m_picScanner
            // 
            this.m_picScanner.BackColor = System.Drawing.Color.Transparent;
            this.m_picScanner.Location = new System.Drawing.Point(4, 51);
            this.m_picScanner.Name = "m_picScanner";
            this.m_picScanner.Size = new System.Drawing.Size(233, 16);
            this.m_picScanner.Paint += new System.Windows.Forms.PaintEventHandler(this.m_picScanner_Paint);
            // 
            // m_FrameImage
            // 
            this.m_FrameImage.Location = new System.Drawing.Point(4, 72);
            this.m_FrameImage.Name = "m_FrameImage";
            this.m_FrameImage.Size = new System.Drawing.Size(233, 168);
            // 
            // m_txtStatusMessage
            // 
            this.m_txtStatusMessage.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Regular);
            this.m_txtStatusMessage.Location = new System.Drawing.Point(4, 249);
            this.m_txtStatusMessage.Name = "m_txtStatusMessage";
            this.m_txtStatusMessage.Size = new System.Drawing.Size(234, 19);
            this.m_txtStatusMessage.TabIndex = 8;
            this.m_txtStatusMessage.Text = "Ready";
            // 
            // m_btnCaptureStop
            // 
            this.m_btnCaptureStop.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Regular);
            this.m_btnCaptureStop.Location = new System.Drawing.Point(125, 26);
            this.m_btnCaptureStop.Name = "m_btnCaptureStop";
            this.m_btnCaptureStop.Size = new System.Drawing.Size(112, 21);
            this.m_btnCaptureStop.TabIndex = 6;
            this.m_btnCaptureStop.Text = "Stop";
            this.m_btnCaptureStop.Click += new System.EventHandler(this.m_btnCaptureStop_Click);
            // 
            // m_btnCaptureStart
            // 
            this.m_btnCaptureStart.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Regular);
            this.m_btnCaptureStart.Location = new System.Drawing.Point(4, 26);
            this.m_btnCaptureStart.Name = "m_btnCaptureStart";
            this.m_btnCaptureStart.Size = new System.Drawing.Size(112, 21);
            this.m_btnCaptureStart.TabIndex = 4;
            this.m_btnCaptureStart.Text = "Start";
            this.m_btnCaptureStart.Click += new System.EventHandler(this.m_btnCaptureStart_Click);
            // 
            // m_btnClose
            // 
            this.m_btnClose.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Bold);
            this.m_btnClose.Location = new System.Drawing.Point(219, 3);
            this.m_btnClose.Name = "m_btnClose";
            this.m_btnClose.Size = new System.Drawing.Size(18, 20);
            this.m_btnClose.TabIndex = 9;
            this.m_btnClose.Text = "X";
            this.m_btnClose.Click += new System.EventHandler(this.m_btnClose_Click);
            // 
            // Timer_StatusFingerQuality
            // 
            this.Timer_StatusFingerQuality.Enabled = true;
            this.Timer_StatusFingerQuality.Interval = 500;
            this.Timer_StatusFingerQuality.Tick += new System.EventHandler(this.Timer_StatusFingerQuality_Tick);
            // 
            // SDKMainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.ClientSize = new System.Drawing.Size(240, 273);
            this.Controls.Add(this.m_btnClose);
            this.Controls.Add(this.m_picScanner);
            this.Controls.Add(this.m_FrameImage);
            this.Controls.Add(this.m_txtStatusMessage);
            this.Controls.Add(this.m_btnCaptureStop);
            this.Controls.Add(this.m_btnCaptureStart);
            this.Controls.Add(this.m_cboUsbDevices);
            this.Font = new System.Drawing.Font("Tahoma", 8F, System.Drawing.FontStyle.Regular);
            this.Name = "SDKMainForm";
            this.Text = "IBSU_CSharpWM";
            this.Load += new System.EventHandler(this.SDKMainForm_Load);
            this.Closing += new System.ComponentModel.CancelEventHandler(this.SDKMainForm_FormClosing);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox m_cboUsbDevices;
        private System.Windows.Forms.PictureBox m_picScanner;
        private System.Windows.Forms.Label m_FrameImage;
        private System.Windows.Forms.TextBox m_txtStatusMessage;
        private System.Windows.Forms.Button m_btnCaptureStop;
        private System.Windows.Forms.Button m_btnCaptureStart;
        private System.Windows.Forms.Button m_btnClose;
        private System.Windows.Forms.Timer Timer_StatusFingerQuality;
    }
}

