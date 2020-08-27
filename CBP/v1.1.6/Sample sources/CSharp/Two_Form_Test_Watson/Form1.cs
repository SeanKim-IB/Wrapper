using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
//using System.Linq;
using System.Text;
//using System.Threading.Tasks;
using System.Threading;
using System.Windows.Forms;
using IBscanUltimate;
using System.Drawing.Drawing2D;
using System.Runtime.InteropServices;

namespace Two_Form_Test_Watson
{
    public partial class frmMain : Form
    {
        public frmMain()
        {
            InitializeComponent();
        }
        #region Fields

        // Form initializations
        frmWindow LeftWindow;
        frmWindow RightWindow;

        // List to store all active devices
        public static List<string> ActiveDeviceList = new List<string>();

        #endregion
        #region User_Defined_Methods

        /// <summary>
        /// Populates list ActiveDeviceList with connected devices.
        /// </summary>
        public void LoadDevices()
        {
            // Populate list
            int devices = 0;
            DLL._IBSU_GetDeviceCount(ref devices);

            for (int i = 0; i < devices; i++)
            {
                DLL.IBSU_DeviceDesc devDesc = new DLL.IBSU_DeviceDesc();
                if (DLL._IBSU_GetDeviceDescription(i, ref devDesc) < DLL.IBSU_STATUS_OK)
                    continue;

                string strDevice;
                if (devDesc.productName[0] == 0)
                    strDevice = "unknown device";
                else
                    strDevice = devDesc.productName + "_v" + devDesc.fwVersion + "(" + devDesc.serialNumber + ")";

                if (!ActiveDeviceList.Contains(strDevice))
                    ActiveDeviceList.Add(strDevice);
            }
        }

        #endregion
        #region Form_Methods

        private void frmMain_Load(object sender, EventArgs e)
        {
            LoadDevices();

            if (ActiveDeviceList.Count != 2)
            {
                MessageBox.Show("To excute this program,\nYou need to connect two IBScanners on your Hub", "Important Note",
                    MessageBoxButtons.OK, MessageBoxIcon.Exclamation, MessageBoxDefaultButton.Button1);
                Close();
                return;
            }

            LeftWindow = new frmWindow(0, ActiveDeviceList[0].ToString());
            RightWindow = new frmWindow(1, ActiveDeviceList[1].ToString());

            LeftWindow.Name = "frm" + LeftWindow.Handle.ToString();
            LeftWindow.MdiParent = this;
            LeftWindow.Show();

            RightWindow.Name = "frm" + RightWindow.Handle.ToString();
            RightWindow.MdiParent = this;
            RightWindow.Show();
            RightWindow.Location = new Point(255, 0);
        }

        private void frmMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (ActiveDeviceList.Count != 2)
            {
                return;
            }

            LeftWindow.Close();
            RightWindow.Close();
        }

        #endregion
        #region Form_Buttons

        private void cmdStart_Click(object sender, EventArgs e)
        {
            Thread tLeft = new Thread(() =>
            {
                LeftWindow.RunStartSequence((PictureBox)LeftWindow.Controls[0], 0);
            });

            Thread tRight = new Thread(() =>
            {
                RightWindow.RunStartSequence((PictureBox)RightWindow.Controls[0], 1);
            });

            tLeft.Start();
            tRight.Start();
        }

        #endregion
    }

    public class frmWindow : Form
    {
        #region Structs

        public struct CaptureInfo
        {
            public string PreCaptureMessage;		// to display on fingerprint window
            public string PostCaptuerMessage;		// to display on fingerprint window
            public DLL.IBSU_ImageType ImageType;				// capture mode
            public int NumberOfFinger;			// number of finger count
            public string fingerName;				// finger name (e.g left thumbs, left index ... )
        }

        private struct ThreadParam
        {
            public IntPtr pParentHandle;
            public IntPtr pFrameImageHandle;
            public int devIndex;
        }

        #endregion
        #region Fields

        // Device ID
        private int DeviceID;

        // List to store all active devices
        public static List<string> ActiveDeviceList = new List<string>();

        // Capture sequence definitions
        private const string CAPTURE_SEQ_FLAT_SINGLE_FINGER = "Single flat finger";
        private const string CAPTURE_SEQ_2_FLAT_FINGERS = "2 flat fingers";
        private const string CAPTURE_SEQ_ALL = "2 flat fingers + 2 flat fingers + thumb";

        // Call thread definitions and sync lock
        public static Thread m_initThread;
        public static Object m_sync = new Object();

        // Callback definitions
        public DLL.IBSU_Callback m_callbackDeviceCommunicationBreak = null;
        public DLL.IBSU_CallbackPreviewImage m_callbackPreviewImage = null;
        public DLL.IBSU_CallbackFingerCount m_callbackFingerCount = null;
        public DLL.IBSU_CallbackFingerQuality m_callbackFingerQuality = null;
        public DLL.IBSU_CallbackDeviceCount m_callbackDeviceCount = null;
        public DLL.IBSU_CallbackInitProgress m_callbackInitProgress = null;
        public DLL.IBSU_CallbackTakingAcquisition m_callbackTakingAcquisition = null;
        public DLL.IBSU_CallbackCompleteAcquisition m_callbackCompleteAcquisition = null;
        public DLL.IBSU_CallbackResultImage m_callbackResultImage = null;
        public DLL.IBSU_CallbackClearPlatenAtCapture m_callbackClearPlaten = null;

        public DLL.IBSU_SdkVersion m_verInfo;
        public int m_nSelectedDevIndex;						    ///< Index of selected device
        public int m_nDevHandle;								///< Device handle
        public bool m_bInitializing;							///< Device initialization is in progress
        public int m_nCurrentCaptureStep;
        public string m_ImgSaveFolder = @"C:\Scans";		///< Base folder for image saving
        public string m_ImgSubFolder = "";							///< Sub Folder for image sequence
        public string m_strImageMessage;
        public bool m_bNeedClearPlaten;
        public bool m_bBlank;
        public int devIndex;

        public List<CaptureInfo> m_vecCaptureSeq = new List<CaptureInfo>();
        public DLL.IBSU_FingerQualityState[] m_FingerQuality = new DLL.IBSU_FingerQualityState[4];

        #endregion
        #region Constructor

        public frmWindow(int deviceID, string DeviceObj)
        {
            ActiveDeviceList.Add(DeviceObj);
            this.DeviceID = deviceID;
            InitializeComponent();
        }

        private void InitializeComponent()
        {
            // 
            // frmWindow
            // 
            PictureBox pbx = new PictureBox()
            {
                Dock = DockStyle.Fill,
                BackColor = Color.LightGray
            };
            this.Controls.Add(pbx);
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(250, 250);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.ControlBox = false;
            this.IsMdiContainer = false;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Load += new System.EventHandler(this.frmWindow_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.frmWindow_FormClosing);
            this.ResumeLayout(false);
        }

        private void frmWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
            ReleaseDevice();
        }

        private void frmWindow_Load(object sender, EventArgs e)
        {
            m_nDevHandle = -1;
            m_nCurrentCaptureStep = -1;
            m_bInitializing = false;
            m_strImageMessage = "";
            m_bNeedClearPlaten = false;

            m_verInfo = new DLL.IBSU_SdkVersion();
            DLL._IBSU_GetSDKVersion(ref m_verInfo);
            string titleName = "IntegrationSample for C#";
            this.Text = titleName;

            m_callbackDeviceCommunicationBreak = new DLL.IBSU_Callback(OnEvent_DeviceCommunicationBreak);
            m_callbackPreviewImage = new DLL.IBSU_CallbackPreviewImage(OnEvent_PreviewImage);
            m_callbackFingerCount = new DLL.IBSU_CallbackFingerCount(OnEvent_FingerCount);
            m_callbackFingerQuality = new DLL.IBSU_CallbackFingerQuality(OnEvent_FingerQuality);
            //m_callbackDeviceCount = new DLL.IBSU_CallbackDeviceCount(OnEvent_DeviceCount);
            m_callbackInitProgress = new DLL.IBSU_CallbackInitProgress(OnEvent_InitProgress);
            m_callbackTakingAcquisition = new DLL.IBSU_CallbackTakingAcquisition(OnEvent_TakingAcquisition);
            m_callbackCompleteAcquisition = new DLL.IBSU_CallbackCompleteAcquisition(OnEvent_CompleteAcquisition);
            m_callbackResultImage = new DLL.IBSU_CallbackResultImage(OnEvent_ResultImage);
            m_callbackClearPlaten = new DLL.IBSU_CallbackClearPlatenAtCapture(OnEvent_ClearPlatenAtCapture);

            DLL._IBSU_RegisterCallbacks(0, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, m_callbackDeviceCount, this.Handle);
            DLL._IBSU_RegisterCallbacks(0, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, m_callbackInitProgress, this.Handle);
        }

        public frmWindow()
        {
            InitializeComponent();
        }

        #endregion
        #region User_Defined_Methods

        public delegate void RunStartSequenceDelegate(PictureBox pbxThis, int deviceIndex);
        public void RunStartSequence(PictureBox pbxThis, int deviceIndex)
        {
            if (this.InvokeRequired)
            {
                this.BeginInvoke(new RunStartSequenceDelegate(RunStartSequence),
                    new object[] { pbxThis, deviceIndex });

                return;
            }

            if (m_bInitializing)
                return;

            if (m_nCurrentCaptureStep != -1)
            {
                bool IsActive = false;
                int nRc = 0;
                nRc = DLL._IBSU_IsCaptureActive(m_nDevHandle, ref IsActive);
                if (nRc == DLL.IBSU_STATUS_OK && IsActive)
                    DLL._IBSU_TakeResultImageManually(m_nDevHandle);

                return;
            }

            if (m_nDevHandle == -1)
            {
                m_bInitializing = true;
                m_initThread = new Thread(new ParameterizedThreadStart(InitializeDeviceThreadCallback));
                ThreadParam param = new ThreadParam();
                param.devIndex = deviceIndex;
                param.pParentHandle = this.Handle;
                //pbxLeft = (PictureBox)LeftWindow.Controls[0];
                //param.pFrameImageHandle = pbxLeft.Handle;
                param.pFrameImageHandle = pbxThis.Handle;

                m_initThread.Start(param);
            }
            else
            {
                // Device already initialized
                OnMsg_CaptureSeqStart();
            }

            OnMsg_UpdateDisplayResources();
        }

        /// <summary>
        /// Initializes the device passed to it through the pParam
        /// object parameter and starts the scanning sequence for
        /// initialized device.
        /// </summary>
        /// <param name="pParam"></param>
        private void InitializeDeviceThreadCallback(object pParam)
        {
            if (pParam == null)
                return;

            ThreadParam param = (ThreadParam)pParam;
            IntPtr formHandle = param.pParentHandle;
            IntPtr frameImageHandle = param.pFrameImageHandle;
            devIndex = param.devIndex;
            int devHandle = -1;
            int nRc = DLL.IBSU_STATUS_OK;

            m_bInitializing = true;
            nRc = DLL._IBSU_OpenDevice(devIndex, ref devHandle);
            m_bInitializing = false;

            if (nRc >= DLL.IBSU_STATUS_OK)
            {
                m_nDevHandle = devHandle;

                DLL.IBSU_RECT clientRect = new DLL.IBSU_RECT();
                Win32.GetClientRect(frameImageHandle, ref clientRect);

                // Create display window
                DLL._IBSU_CreateClientWindow(devHandle, frameImageHandle,
                    clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

                // Register callback functions
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, m_callbackDeviceCommunicationBreak, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, m_callbackPreviewImage, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, m_callbackTakingAcquisition, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, m_callbackCompleteAcquisition, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE, m_callbackResultImage, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, m_callbackFingerCount, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, m_callbackFingerQuality, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, m_callbackClearPlaten, formHandle);
            }

            //LoadDevices();

            // Status notification and sequence start
            if (nRc == DLL.IBSU_STATUS_OK)
            {
                OnMsg_CaptureSeqStart();
                return;
            }

            if (nRc > DLL.IBSU_STATUS_OK)
                OnMsg_InitWarning();
            else
            {
                string message;
                switch (nRc)
                {
                    case DLL.IBSU_ERR_DEVICE_ACTIVE:
                        message = String.Format("[Error code = {0}] Device initialization failed because in use by another thread/process.", nRc);
                        OnMsg_UpdateStatusMessage(message);
                        break;
                    case DLL.IBSU_ERR_USB20_REQUIRED:
                        message = String.Format("[Error code = {0}] Device initialization failed because SDK only works with USB 2.0.", nRc);
                        OnMsg_UpdateStatusMessage(message);
                        break;
                    default:
                        message = String.Format("[Error code = {0}] Device initialization failed", nRc);
                        OnMsg_UpdateStatusMessage(message);
                        break;
                }
            }
        }

        /// <summary>
        /// Updates the text within the title bar of the main form.
        /// </summary>
        /// <param name="message"></param>
        public void SetStatusBarMessage(string message)
        {
            this.Text = message;
        }

        /// <summary>
        /// Updates the text displayed on the image.
        /// </summary>
        /// <param name="message"></param>
        public void SetImageMessage(string message)
        {
            int font_size = 6;
            int x = 10;
            int y = 10;
            Color cr = Color.FromArgb(0, 0, 255);

            DLL._IBSU_SetClientWindowOverlayText(m_nDevHandle, "Arial", font_size, true, message, x, y, (uint)ColorTranslator.ToWin32(cr));
        }

        /// <summary>
        /// Releases the device.
        /// </summary>
        /// <returns></returns>
        public int ReleaseDevice()
        {
            int nRc = DLL.IBSU_STATUS_OK;

            if (m_nDevHandle != -1)
                nRc = DLL._IBSU_CloseDevice(m_nDevHandle);

            if (nRc >= DLL.IBSU_STATUS_OK)
            {
                m_nDevHandle = -1;
                m_nCurrentCaptureStep = -1;
                m_bInitializing = false;
            }

            return nRc;
        }

        /// <summary>
        /// Saves the captured images passed through as parameters.
        /// </summary>
        /// <param name="image"></param>
        /// <param name="fingerName"></param>
        public void SaveBitmapImage(ref DLL.IBSU_ImageData image, string fingerName, int deviceIndex)
        {
            deviceIndex = this.devIndex;

            if ((m_ImgSaveFolder.Length == 0) || (m_ImgSubFolder.Length == 0))
            {
                return;
            }

            string strFolder = string.Empty;
            string strFileName = string.Empty;

            strFolder = String.Format("{0}\\{1}\\{2}", m_ImgSaveFolder, m_ImgSubFolder, ActiveDeviceList[DeviceID].ToString());
            System.IO.Directory.CreateDirectory(strFolder);

            strFileName = String.Format("{0}\\Image_{1}_{2}.bmp", strFolder, m_nCurrentCaptureStep, fingerName);

            if (DLL._IBSU_SaveBitmapImage(strFileName, image.Buffer,
                                      image.Width, image.Height, image.Pitch,
                                      image.ResolutionX, image.ResolutionY) != DLL.IBSU_STATUS_OK)
            {
                MessageBox.Show("Failed to save bitmap image!");
            }
        }

        public void DrawRoundRect(Graphics g, Brush brush, float X, float Y, float width, float height, float radius)
        {
            GraphicsPath gp = new GraphicsPath();

            gp.AddLine(X + radius, Y, X + width - (radius * 2), Y);
            gp.AddArc(X + width - (radius * 2), Y, radius * 2, radius * 2, 270, 90);
            gp.AddLine(X + width, Y + radius, X + width, Y + height - (radius * 2));
            gp.AddArc(X + width - (radius * 2), Y + height - (radius * 2), radius * 2, radius * 2, 0, 90);
            gp.AddLine(X + width - (radius * 2), Y + height, X + radius, Y + height);
            gp.AddArc(X, Y + height - (radius * 2), radius * 2, radius * 2, 90, 90);
            gp.AddLine(X, Y + height - (radius * 2), X, Y + radius);
            gp.AddArc(X, Y, radius * 2, radius * 2, 180, 90);
            gp.CloseFigure();

            g.SmoothingMode = SmoothingMode.AntiAlias;
            g.FillPath(brush, gp);
            gp.Dispose();
        }

        #endregion
        #region Delegate_Methods

        public delegate void OnMsg_CaptureSeqStartDelegate();
        public void OnMsg_CaptureSeqStart()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_CaptureSeqStartDelegate(OnMsg_CaptureSeqStart));

                return;
            }

            if (m_nDevHandle == -1)
            {
                OnMsg_UpdateDisplayResources();
                return;
            }

            //string strCaptureSeq = "Single flat finger";
            string strCaptureSeq = "2 flat fingers + 2 flat fingers + thumb";
            m_vecCaptureSeq.Clear();
            CaptureInfo info = new CaptureInfo();

            if (strCaptureSeq == CAPTURE_SEQ_FLAT_SINGLE_FINGER)
            {
                info.PreCaptureMessage = "Please put a single finger on the sensor!";
                info.PostCaptuerMessage = "Keep finger on the sensor!";
                info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER;
                info.NumberOfFinger = 1;
                info.fingerName = "SFF_Unknown";
                m_vecCaptureSeq.Add(info);
                int count = m_vecCaptureSeq.Count;
            }

            if (strCaptureSeq == CAPTURE_SEQ_2_FLAT_FINGERS)
            {
                info.PreCaptureMessage = "Please put two fingers on the sensor!";
                info.PostCaptuerMessage = "Keep fingers on the sensor!";
                info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS;
                info.NumberOfFinger = 2;
                info.fingerName = "TFF_Unknown";
                m_vecCaptureSeq.Add(info);
            }

            if (strCaptureSeq == CAPTURE_SEQ_ALL)
            {
                info.PreCaptureMessage = "Please put first two fingers on the sensor!";
                info.PostCaptuerMessage = "Keep fingers on the sensor!";
                info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS;
                info.NumberOfFinger = 2;
                info.fingerName = "TFF_Unknown";
                m_vecCaptureSeq.Add(info);

                info.PreCaptureMessage = "Please put second two fingers on the sensor!";
                info.PostCaptuerMessage = "Keep fingers on the sensor!";
                info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS;
                info.NumberOfFinger = 2;
                info.fingerName = "TFF_Unknown";
                m_vecCaptureSeq.Add(info);

                info.PreCaptureMessage = "Please put a single finger on the sensor!";
                info.PostCaptuerMessage = "Keep finger on the sensor!";
                info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER;
                info.NumberOfFinger = 1;
                info.fingerName = "SFF_Thumb";
                m_vecCaptureSeq.Add(info);
                int count = m_vecCaptureSeq.Count;
            }

            // Make subfolder name
            m_ImgSubFolder = DateTime.Now.ToString("yyyy-MM-dd HHmmss");

            OnMsg_CaptureSeqNext();
        }

        public delegate void OnMsg_CaptureSeqNextDelegate();
        public void OnMsg_CaptureSeqNext()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_CaptureSeqNextDelegate(OnMsg_CaptureSeqNext));

                return;
            }

            int nRc;

            if (m_nDevHandle == -1)
                return;

            m_bBlank = false;
            m_FingerQuality[0] = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT;
            m_FingerQuality[1] = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT;
            m_FingerQuality[2] = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT;
            m_FingerQuality[3] = DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT;


            m_nCurrentCaptureStep++;
            if (m_nCurrentCaptureStep >= m_vecCaptureSeq.Count)
            {
                // All of capture sequence completely
                m_nCurrentCaptureStep = -1;
                m_ImgSubFolder = "";

                OnMsg_UpdateDisplayResources();
                //Win32.SetFocus(cmdStart.Handle);
                return;
            }

            // Make capture delay for display result image on multi capture mode (500 ms)
            if (m_nCurrentCaptureStep > 0)
            {
                Thread.Sleep(500);
                m_strImageMessage = "";
            }

            CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];

            DLL.IBSU_ImageResolution imgRes = DLL.IBSU_ImageResolution.ENUM_IBSU_IMAGE_RESOLUTION_500;
            bool bAvailable = false;
            nRc = DLL._IBSU_IsCaptureAvailable(m_nDevHandle, info.ImageType, imgRes, ref bAvailable);
            if (nRc != DLL.IBSU_STATUS_OK || !bAvailable)
            {
                string message;
                message = String.Format("The capture mode {0} is not available", info.ImageType);
                SetStatusBarMessage(message);
                m_nCurrentCaptureStep = -1;
                OnMsg_UpdateDisplayResources();
                return;
            }

            // Start capture
            uint captureOptions = 0;
            captureOptions |= DLL.IBSU_OPTION_AUTO_CONTRAST;
            captureOptions |= DLL.IBSU_OPTION_AUTO_CAPTURE;
            captureOptions |= DLL.IBSU_OPTION_IGNORE_FINGER_COUNT;

            nRc = DLL._IBSU_BeginCaptureImage(m_nDevHandle, info.ImageType, imgRes, captureOptions);
            if (nRc >= DLL.IBSU_STATUS_OK)
            {
                // Display message for image acuisition
                string strMessage;
                strMessage = String.Format("{0}", info.PreCaptureMessage);
                SetStatusBarMessage(strMessage);

                SetImageMessage(strMessage);
                m_strImageMessage = strMessage;
            }
            else
            {
                string strMessage;
                strMessage = String.Format("Failed to execute IBSU_BeginCaptureImage()");
                SetStatusBarMessage(strMessage);
            }

            OnMsg_UpdateDisplayResources();
        }

        public delegate void OnMsg_DeviceCommunicationBreakDelegate();
        public void OnMsg_DeviceCommunicationBreak()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_DeviceCommunicationBreakDelegate(OnMsg_DeviceCommunicationBreak));

                return;
            }

        }

        public delegate void OnMsg_InitWarningDelegate();
        public void OnMsg_InitWarning()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_InitWarningDelegate(OnMsg_InitWarning));

                return;
            }

        }

        public delegate void OnMsg_DrawClientWindowDelegate();
        public void OnMsg_DrawClientWindow()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_DrawClientWindowDelegate(OnMsg_DrawClientWindow));

                return;
            }

        }

        public delegate void OnMsg_UpdateDisplayResourcesDelegate();
        public void OnMsg_UpdateDisplayResources()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_UpdateDisplayResourcesDelegate(OnMsg_UpdateDisplayResources));

                return;
            }

            bool idle = !m_bInitializing && (m_nCurrentCaptureStep == -1);
            bool active = !m_bInitializing && (m_nCurrentCaptureStep != -1);
            bool uninitializedDev = (m_nDevHandle == -1);

            //string strCaption = "";
            //if (active)
            //    strCaption = "Busy";
            //else if (!active && !m_bInitializing)
            //    strCaption = "Start";

            //cmdStart.Text = strCaption;
        }

        public delegate void OnMsg_UpdateStatusMessageDelegate(string message);
        public void OnMsg_UpdateStatusMessage(string message)
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_UpdateStatusMessageDelegate(OnMsg_UpdateStatusMessage),
                    new object[] { message });

                return;
            }

            SetStatusBarMessage(message);
        }

        public delegate void OnMsg_UpdateImageMessageDelegate(string message);
        public void OnMsg_UpdateImageMessage(string message)
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_UpdateImageMessageDelegate(OnMsg_UpdateImageMessage),
                    new object[] { message });

                return;
            }

            SetImageMessage(message);
        }

        #endregion
        #region Event_Methods

        private static void OnEvent_DeviceCommunicationBreak(int deviceIndex, IntPtr pContext)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
                pDlg.OnMsg_DeviceCommunicationBreak();
            }
        }

        private static void OnEvent_PreviewImage(int deviceHandle, IntPtr pContext, DLL.IBSU_ImageData image)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
            }
        }

        private static void OnEvent_FingerCount(int deviceHandle, IntPtr pContext, DLL.IBSU_FingerCountState fingerCountState)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
                if (deviceHandle != pDlg.m_nDevHandle)
                    return;
            }
        }

        private static void OnEvent_FingerQuality(int deviceHandle, IntPtr pContext, IntPtr pQualityArray, int qualityArrayCount)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
                int[] qualityArray = new int[4];
                Marshal.Copy(pQualityArray, qualityArray, 0, qualityArrayCount);
                pDlg.m_FingerQuality[0] = (DLL.IBSU_FingerQualityState)qualityArray[0];
                pDlg.m_FingerQuality[1] = (DLL.IBSU_FingerQualityState)qualityArray[1];
                pDlg.m_FingerQuality[2] = (DLL.IBSU_FingerQualityState)qualityArray[2];
                pDlg.m_FingerQuality[3] = (DLL.IBSU_FingerQualityState)qualityArray[3];
            }
        }

        //private static void OnEvent_DeviceCount(int detectedDevices, IntPtr pContext)
        //{
        //    if (pContext == null)
        //        return;

        //    lock (m_sync)
        //    {
        //        frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
        //        pDlg.LoadDevices();                
        //    }
        //}

        private static void OnEvent_InitProgress(int deviceIndex, IntPtr pContext, int progressValue)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
                string message;
                message = String.Format("Initializing device... {0}%", progressValue);
                pDlg.OnMsg_UpdateStatusMessage(message);
            }
        }

        private static void OnEvent_TakingAcquisition(int deviceHandle, IntPtr pContext, DLL.IBSU_ImageType imageType)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
                if (imageType == DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER)
                {
                    pDlg.m_strImageMessage = "When done remove finger from sensor";
                    pDlg.OnMsg_UpdateImageMessage(pDlg.m_strImageMessage);
                    pDlg.OnMsg_UpdateStatusMessage(pDlg.m_strImageMessage);
                }
            }
        }

        private static void OnEvent_CompleteAcquisition(int deviceHandle, IntPtr pContext, DLL.IBSU_ImageType imageType)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
            }
        }

        private static void OnEvent_ResultImage(int deviceHandle, IntPtr pContext, DLL.IBSU_ImageData image, DLL.IBSU_ImageType imageType, IntPtr pSplitImageArray, int splitImageArrayCount)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
                if (deviceHandle != pDlg.m_nDevHandle)
                    return;

                if (pDlg.m_bNeedClearPlaten)
                {
                    pDlg.m_bNeedClearPlaten = false;
                }

                // Image acquisition successful
                string imgTypeName;
                switch (imageType)
                {
                    case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER:
                        imgTypeName = "-- Flat single finger --"; break;
                    case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS:
                        imgTypeName = "-- Flat two fingers --"; break;
                    default:
                        imgTypeName = "-- Unknown --"; break;
                }

                // Save the images
                pDlg.OnMsg_UpdateStatusMessage("Saving image...");
                CaptureInfo info = pDlg.m_vecCaptureSeq[pDlg.m_nCurrentCaptureStep];
                pDlg.SaveBitmapImage(ref image, info.fingerName, pDlg.devIndex);
                if (splitImageArrayCount > 1)
                {
                    DLL.IBSU_ImageData[] imageArray = new DLL.IBSU_ImageData[splitImageArrayCount];
                    string splitName;
                    IntPtr ptrRunner = pSplitImageArray;
                    for (int i = 0; i < splitImageArrayCount; i++)
                    {
                        splitName = String.Format("{0}_Split_{1}", info.fingerName, i);
                        imageArray[i] = (DLL.IBSU_ImageData)Marshal.PtrToStructure(ptrRunner, typeof(DLL.IBSU_ImageData));
                        pDlg.SaveBitmapImage(ref imageArray[i], splitName, pDlg.devIndex);
                        ptrRunner = (IntPtr)((int)ptrRunner + Marshal.SizeOf(imageArray[0]));
                    }
                }

                pDlg.m_strImageMessage = String.Format("{0} acquisition completed successfully", imgTypeName);
                pDlg.SetImageMessage(pDlg.m_strImageMessage);
                pDlg.OnMsg_UpdateStatusMessage(pDlg.m_strImageMessage);

                pDlg.OnMsg_CaptureSeqNext();
            }
        }

        private static void OnEvent_ClearPlatenAtCapture(int deviceIndex, IntPtr pContext, DLL.IBSU_PlatenState platenState)
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                frmWindow pDlg = (frmWindow)frmWindow.FromHandle(pContext);
                if (platenState == DLL.IBSU_PlatenState.ENUM_IBSU_PLATEN_HAS_FINGERS)
                    pDlg.m_bNeedClearPlaten = true;
                else
                    pDlg.m_bNeedClearPlaten = false;

                if (pDlg.m_bNeedClearPlaten)
                {
                    pDlg.m_strImageMessage = "Please remove your fingers on the platen first!";
                    pDlg.SetImageMessage(pDlg.m_strImageMessage);
                    pDlg.OnMsg_UpdateStatusMessage(pDlg.m_strImageMessage);
                }
                else
                {
                    CaptureInfo info = pDlg.m_vecCaptureSeq[pDlg.m_nCurrentCaptureStep];

                    // Display message for image acuisition again
                    string strMessage;
                    strMessage = String.Format("{0}", info.PreCaptureMessage);

                    pDlg.OnMsg_UpdateStatusMessage(strMessage);

                    pDlg.SetImageMessage(strMessage);
                    pDlg.m_strImageMessage = strMessage;
                }
            }
        }

        #endregion
    }
}
