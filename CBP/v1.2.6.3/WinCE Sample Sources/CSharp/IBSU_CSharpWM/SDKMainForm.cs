using System;
using System.Linq;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using IBscanUltimate;
using System.Runtime.InteropServices;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.IO;

namespace IBSU_CSharpWM
{
    public partial class SDKMainForm : Form
    {
        private struct CaptureInfo
        {
            public string PreCaptureMessage;		// to display on fingerprint window
            public string PostCaptuerMessage;		// to display on fingerprint window
            public DLL.IBSU_ImageType ImageType;				// capture mode
            public int NumberOfFinger;			// number of finger count
            public string fingerName;				// finger name (e.g left thumbs, left index ... )
            public DLL.IBSM_FingerPosition fingerPosition;		// Finger position. e.g Right Thumb, Right Index finger
        }

        private struct ThreadParam
        {
            public IntPtr pParentHandle;
            public IntPtr pFrameImageHandle;
            public int devIndex;
        }


        // Capture sequence definitions
        private const string CAPTURE_SEQ_FLAT_SINGLE_FINGER = "Single flat finger";
        private const string CAPTURE_SEQ_ROLL_SINGLE_FINGER = "Single rolled finger";
        private const string CAPTURE_SEQ_2_FLAT_FINGERS = "2 flat fingers";
        private const string CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS = "10 single flat fingers";
        private const string CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS = "10 single rolled fingers";
        private const string CAPTURE_SEQ_4_FLAT_FINGERS = "4 flat fingers";
        private const string CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER = "10 flat fingers with 4-finger scanner";

        // Beep definitions
        private const int __BEEP_FAIL__ = 0;
        private const int __BEEP_SUCCESS__ = 1;
        private const int __BEEP_OK__ = 2;
        private const int __BEEP_DEVICE_COMMUNICATION_BREAK__ = 3;

        //        static Thread m_initThread;
        static ThreadStart m_initThread;
        static Object m_sync = new Object();
        static Thread m_thread;

        private DLL.IBSU_Callback m_callbackDeviceCommunicationBreak = null;
        private DLL.IBSU_CallbackPreviewImage m_callbackPreviewImage = null;
        private DLL.IBSU_CallbackFingerCount m_callbackFingerCount = null;
        private DLL.IBSU_CallbackFingerQuality m_callbackFingerQuality = null;
        private DLL.IBSU_CallbackDeviceCount m_callbackDeviceCount = null;
        private DLL.IBSU_CallbackInitProgress m_callbackInitProgress = null;
        private DLL.IBSU_CallbackTakingAcquisition m_callbackTakingAcquisition = null;
        private DLL.IBSU_CallbackCompleteAcquisition m_callbackCompleteAcquisition = null;
        private DLL.IBSU_CallbackClearPlatenAtCapture m_callbackClearPlaten = null;
        private DLL.IBSU_CallbackResultImageEx m_callbackResultImageEx = null;


        private DLL.IBSU_SdkVersion m_verInfo;
        private int m_nSelectedDevIndex;						///< Index of selected device
        private int m_nDevHandle;								///< Device handle
        public bool m_bInitializing;							///< Device initialization is in progress
        private int m_nCurrentCaptureStep;
        private string m_ImgSaveFolder;				            ///< Base folder for image saving
        private string m_ImgSubFolder;								///< Sub Folder for image sequence
        private string m_strImageMessage;
        private bool m_bNeedClearPlaten;
        private bool m_bBlank;
        private int m_nOvImageTextHandle;
        private int m_nOvClearPlatenHandle;
        private int[] m_nOvSegmentHandle = new int[DLL.IBSU_MAX_SEGMENT_COUNT];


        private List<CaptureInfo> m_vecCaptureSeq = new List<CaptureInfo>();
        private DLL.IBSU_FingerQualityState[] m_FingerQuality = new DLL.IBSU_FingerQualityState[DLL.IBSU_MAX_SEGMENT_COUNT];


        private bool m_bUseClearPlaten;
        private bool m_bDrawSegmentImage;
        private bool m_bNFIQScore;
        private bool m_bSaveImages;
        private ThreadParam m_param = new ThreadParam();

        public SDKMainForm()
        {
            InitializeComponent();
        }

        ////////////////////////////////////////////////////////////////////////////////////////////
        // User defined functions
        private bool _ChkFolder(string sPath)
        {
            DirectoryInfo di = new DirectoryInfo(sPath);

            return di.Exists;
        }

        private void _InitializeDeviceThreadCallback()
        {
            IntPtr formHandle = m_param.pParentHandle;
            IntPtr frameImageHandle = m_param.pFrameImageHandle;
            int devIndex = m_param.devIndex;
            int devHandle = -1;
            int nRc = DLL.IBSU_STATUS_OK;

            m_bInitializing = true;
            nRc = DLL._IBSU_OpenDeviceEx(devIndex, m_ImgSaveFolder, false, ref devHandle);
            m_bInitializing = false;

            if (nRc >= DLL.IBSU_STATUS_OK)
            {
                m_nDevHandle = devHandle;

                DLL.IBSU_RECT clientRect = new DLL.IBSU_RECT();
                Win32.GetClientRect(frameImageHandle, ref clientRect);

                // Enable power save mode
                StringBuilder propertyValue = new StringBuilder("TRUE");
                DLL._IBSU_SetProperty(devHandle, DLL.IBSU_PropertyId.ENUM_IBSU_PROPERTY_ENABLE_POWER_SAVE_MODE, propertyValue);

                // Create display window
                DLL._IBSU_CreateClientWindow(devHandle, frameImageHandle, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
                DLL._IBSU_AddOverlayQuadrangle(devHandle, ref m_nOvClearPlatenHandle, 0, 0, 0, 0, 0, 0, 0, 0, 0, (uint)0);
                DLL._IBSU_AddOverlayText(devHandle, ref m_nOvImageTextHandle, "Arial", 10, true, "", 10, 10, 0);

                for (int i = 0; i < DLL.IBSU_MAX_SEGMENT_COUNT; i++)
                {
                    DLL._IBSU_AddOverlayQuadrangle(devHandle, ref m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0,
                    0, (uint)0);
                }

                /** Default value is 0x00d8e9ec( COLOR_BTNFACE ) for enumeration ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR
                 ** You can change the background color as using method below
                                string cValue;
                                Color bkColor = SystemColors.ButtonFace;
                                cValue = String.Format("{0}", (uint)ColorTranslator.ToWin32(bkColor));
                                DLL._IBSU_SetClientDisplayProperty(devHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_BK_COLOR, cValue);
                */

                /** Default value is TRUE for enumeration ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE
                 ** You can remove the guide line of rolling as using method below
                                cValue = "FALSE";
                                DLL._IBSU_SetClientDisplayProperty( devHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_ROLL_GUIDE_LINE, cValue );
                */

                // register callback functions
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_COMMUNICATION_BREAK, m_callbackDeviceCommunicationBreak, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_PREVIEW_IMAGE, m_callbackPreviewImage, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_TAKING_ACQUISITION, m_callbackTakingAcquisition, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_COMPLETE_ACQUISITION, m_callbackCompleteAcquisition, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_RESULT_IMAGE_EX, m_callbackResultImageEx, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_FINGER_COUNT, m_callbackFingerCount, formHandle);
                DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_FINGER_QUALITY, m_callbackFingerQuality, formHandle);
                if (m_bUseClearPlaten)
                    DLL._IBSU_RegisterCallbacks(devHandle, DLL.IBSU_Events.ENUM_IBSU_OPTIONAL_EVENT_CLEAR_PLATEN_AT_CAPTURE, m_callbackClearPlaten, formHandle);
            }

            // status notification and sequence start
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

            OnMsg_UpdateDeviceList();
        }

        private void _SetStatusBarMessage(string message)
        {
            m_txtStatusMessage.Text = message;
        }

        private void _SetImageMessage(string message)
        {
            int font_size = 8;
            int x = 10;
            int y = 10;
            Color cr = Color.FromArgb(0, 0, 255);

            if (m_bNeedClearPlaten)
            {
                cr = Color.FromArgb(255, 0, 0);
            }

            uint textColor = (uint)(cr.R + (cr.G << 8) + (cr.B << 16));

            DLL._IBSU_ModifyOverlayText(m_nDevHandle, m_nOvImageTextHandle, "Arial", font_size, true, message, x, y, textColor);
        }

        /*        private void _UpdateCaptureSequences()
                {
                    // store currently selected sequence
                    string strSelectedText = "";
                    int selectedSeq = m_cboCaptureSeq.SelectedIndex;
                    if (selectedSeq > -1)
                        strSelectedText = m_cboCaptureSeq.Text;

                    // populate combo box
                    m_cboCaptureSeq.Items.Clear();
                    m_cboCaptureSeq.Items.Add("- Please select -");

                    int devIndex = m_cboUsbDevices.SelectedIndex - 1;
                    DLL.IBSU_DeviceDesc devDesc = new DLL.IBSU_DeviceDesc();
                    if (devIndex > -1)
                        DLL._IBSU_GetDeviceDescription(devIndex, ref devDesc);

                    if ((devDesc.productName == "WATSON") ||
                        (devDesc.productName == "WATSON MINI") ||
                        (devDesc.productName == "SHERLOCK_ROIC") ||
                        (devDesc.productName == "SHERLOCK"))
                    {
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_FLAT_SINGLE_FINGER);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_ROLL_SINGLE_FINGER);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_2_FLAT_FINGERS);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS);
                    }
                    else if ((devDesc.productName == "COLUMBO") ||
                              (devDesc.productName == "CURVE"))
                    {
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_FLAT_SINGLE_FINGER);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS);
                    }
                    else if ((devDesc.productName == "HOLMES"))
                    {
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_FLAT_SINGLE_FINGER);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_ROLL_SINGLE_FINGER);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_2_FLAT_FINGERS);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_4_FLAT_FINGERS);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS);
                        m_cboCaptureSeq.Items.Add(CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER);
                    }

                    // select previously selected sequence
                    if (selectedSeq > -1)
                        selectedSeq = m_cboCaptureSeq.FindString(strSelectedText, 0);
                    if (selectedSeq == -1)
                        m_cboCaptureSeq.SelectedIndex = 0;
                    else
                        m_cboCaptureSeq.SelectedIndex = selectedSeq;

                    OnMsg_UpdateDisplayResources();
                }
        */
        private int _ReleaseDevice()
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

        private void _BeepFail()
        {
            Win32.MessageBeep(-1);
            Thread.Sleep(150);
            Win32.MessageBeep(-1);
            Thread.Sleep(150);
            Win32.MessageBeep(-1);
            Thread.Sleep(150);
            Win32.MessageBeep(-1);
        }

        private void _BeepSuccess()
        {
            Win32.MessageBeep(-1);
            Thread.Sleep(50);
            Win32.MessageBeep(-1);
        }

        private void _BeepOk()
        {
            Win32.MessageBeep(-1);
        }

        private void _BeepDeviceCommunicationBreak()
        {
            for (int i = 0; i < 5; i++)
            {
                Win32.MessageBeep(-1);
                Thread.Sleep(500);
            }
        }

        private void _SaveBitmapImage(ref DLL.IBSU_ImageData image, string fingerName)
        {
            if ((m_ImgSaveFolder.Length == 0) || (m_ImgSubFolder.Length == 0))
            {
                return;
            }

            string strFolder;
            strFolder = String.Format("{0}\\{1}", m_ImgSaveFolder, m_ImgSubFolder);
            System.IO.Directory.CreateDirectory(strFolder);

            string strFileName;
            strFileName = String.Format("{0}\\Image_{1}_{2}.bmp", strFolder, m_nCurrentCaptureStep, fingerName);

            if (DLL._IBSU_SaveBitmapImage(strFileName, image.Buffer,
                                      image.Width, image.Height, image.Pitch,
                                      image.ResolutionX, image.ResolutionY) != DLL.IBSU_STATUS_OK)
            {
                MessageBox.Show("Failed to save bitmap image!");
            }
        }

        private void _DrawRectangle(Graphics g, Brush brush, int X, int Y, int width, int height)
        {
            g.FillRectangle(brush, X, Y, width, height);
        }

        private int _ModifyOverlayForWarningOfClearPlaten(Boolean bVisible)
        {
            if (m_nDevHandle == -1)
                return -1;

            int nRc = DLL.IBSU_STATUS_OK;
            Color cr = Color.FromArgb(255, 0, 0);
            int left, top, right, bottom;

            DLL.IBSU_RECT clientRect = new DLL.IBSU_RECT();
            Win32.GetClientRect(m_FrameImage.Handle, ref clientRect);

            left = 0; top = 0; right = clientRect.right - clientRect.left; bottom = clientRect.bottom - clientRect.top;
            if (bVisible)
            {
                nRc = DLL._IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle,
                    left, top, right, top, right, bottom, left, bottom, 20, (uint)(cr.ToArgb()));
            }
            else
            {
                nRc = DLL._IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvClearPlatenHandle,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, (uint)0);
            }

            return nRc;
        }











        private void OnEvent_DeviceCommunicationBreak(
            int deviceIndex,
            IntPtr pContext
            )
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                OnMsg_DeviceCommunicationBreak();
            }
        }

        private void OnEvent_PreviewImage(
            int deviceHandle,
            IntPtr pContext,
            DLL.IBSU_ImageData image
            )
        {
            if (pContext == null)
                return;

            lock (SDKMainForm.m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
            }
        }

        private void OnEvent_FingerCount(
            int deviceHandle,
            IntPtr pContext,
            DLL.IBSU_FingerCountState fingerCountState
            )
        {
            if (pContext == null)
                return;

            lock (SDKMainForm.m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                if (deviceHandle != m_nDevHandle)
                    return;
                /*
                                string fingerState;
                                if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_FINGER_COUNT_OK)
                                    fingerState = "FINGER_COUNT_OK";
                                else if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_TOO_MANY_FINGERS)
                                    fingerState = "TOO_MANY_FINGERS";
                                else if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_TOO_FEW_FINGERS)
                                    fingerState = "TOO_FEW_FINGERS";
                                else if (fingerCountState == DLL.IBSU_FingerCountState.ENUM_IBSU_NON_FINGER)
                                    fingerState = "NON-FINGER";
                                else
                                    fingerState = "UNKNOWN";
                */
            }
        }

        private void OnEvent_FingerQuality(
            int deviceHandle,
            IntPtr pContext,
            IntPtr pQualityArray,
            int qualityArrayCount
            )
        {
            if (pContext == null)
                return;

            lock (SDKMainForm.m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                int[] qualityArray = new int[4];
                Marshal.Copy(pQualityArray, qualityArray, 0, qualityArrayCount);
                m_FingerQuality[0] = (DLL.IBSU_FingerQualityState)qualityArray[0];
                m_FingerQuality[1] = (DLL.IBSU_FingerQualityState)qualityArray[1];
                m_FingerQuality[2] = (DLL.IBSU_FingerQualityState)qualityArray[2];
                m_FingerQuality[3] = (DLL.IBSU_FingerQualityState)qualityArray[3];

                OnMsg_InvalidateQuality();
            }
        }

        private void OnEvent_DeviceCount(
            int detectedDevices,
            IntPtr pContext
            )
        {
            if (pContext == null)
                return;

            lock (SDKMainForm.m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                OnMsg_UpdateDeviceList();
            }
        }

        private void OnEvent_InitProgress(
            int deviceIndex,
            IntPtr pContext,
            int progressValue
            )
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                string message;
                message = String.Format("Initializing device... {0}%", progressValue);
                OnMsg_UpdateStatusMessage(message);
            }
        }

        private void OnEvent_TakingAcquisition(
            int deviceHandle,
            IntPtr pContext,
            DLL.IBSU_ImageType imageType
            )
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                if (imageType == DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER)
                {
                    OnMsg_Beep(__BEEP_OK__);
                    m_strImageMessage = "When done remove finger from sensor";
                    OnMsg_UpdateImageMessage(m_strImageMessage);
                    OnMsg_UpdateStatusMessage(m_strImageMessage);
                }
            }
        }

        private void OnEvent_CompleteAcquisition(
            int deviceHandle,
            IntPtr pContext,
            DLL.IBSU_ImageType imageType
            )
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                if (imageType == DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER)
                {
                    OnMsg_Beep(__BEEP_OK__);
                }
                else
                {
                    string strValue = String.Format("Remove fingers from sensor");
                    _SetImageMessage(strValue);
                    strValue = String.Format("Acquisition completed, postprocessing..");
                    OnMsg_UpdateStatusMessage(strValue);
                }
            }
        }

        /****
         ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
         ** Please use IBSU_CallbackResultImageEx instead
        */
        private void OnEvent_ResultImageEx(int deviceHandle, IntPtr pContext, int imageStatus, DLL.IBSU_ImageData image, DLL.IBSU_ImageType imageType, int detectedFingerCount, int segmentImageArrayCount, IntPtr pSegmentImageArray, IntPtr pSegmentPositionArray)
        {
            lock (m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                if (deviceHandle != m_nDevHandle)
                {
                    return;
                }

                if (imageStatus >= DLL.IBSU_STATUS_OK)
                {
                    OnMsg_Beep(__BEEP_SUCCESS__);
                }
                else
                {
                    OnMsg_Beep(__BEEP_FAIL__);
                }

                // Added 2012-11-30
                if (m_bNeedClearPlaten)
                {
                    m_bNeedClearPlaten = false;
                    OnMsg_InvalidateQuality();
                }

                // Image acquisition successful
/*                string imgTypeName = null;

                switch (imageType)
                {
                    case DLL.IBSU_ImageType.ENUM_IBSU_ROLL_SINGLE_FINGER:
                        imgTypeName = "-- Rolling single finger --";
                        break;
                    case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER:
                        imgTypeName = "-- Flat single finger --";
                        break;
                    case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_TWO_FINGERS:
                        imgTypeName = "-- Flat two fingers --";
                        break;
                    case DLL.IBSU_ImageType.ENUM_IBSU_FLAT_FOUR_FINGERS:
                        imgTypeName = "-- Flat 4 fingers --";
                        break;
                    default:
                        imgTypeName = "-- Unknown --";
                        break;
                }
*/
                int i = 0;

                if (imageStatus >= DLL.IBSU_STATUS_OK)
                {
                    if (m_bSaveImages)
                    {
                        OnMsg_UpdateStatusMessage("Saving image...");
                        CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];
                        _SaveBitmapImage(ref image, info.fingerName);
                        if (segmentImageArrayCount > 0)
                        {
                            DLL.IBSU_ImageData[] imageArray = new DLL.IBSU_ImageData[segmentImageArrayCount];
                            string segmentName = null;
                            IntPtr ptrRunner = pSegmentImageArray;
                            for (i = 0; i < segmentImageArrayCount; i++)
                            {
                                segmentName = String.Format("{0}_Segement_{1}", info.fingerName, i);
                                imageArray[i] = (DLL.IBSU_ImageData)Marshal.PtrToStructure(ptrRunner, typeof(DLL.IBSU_ImageData));
                                _SaveBitmapImage(ref imageArray[i], segmentName);
                                ptrRunner = (IntPtr)((int)ptrRunner + Marshal.SizeOf(imageArray[0]));
                            }
                        }
                    }

                    if (m_bDrawSegmentImage)
                    {
                        StringBuilder propertyValue = new StringBuilder();
                        double scaleFactor = 0;
                        int leftMargin = 0;
                        int TopMargin = 0;

                        DLL._IBSU_GetClientWindowProperty(deviceHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_SCALE_FACTOR, propertyValue);
                        scaleFactor = double.Parse(propertyValue.ToString());
                        DLL._IBSU_GetClientWindowProperty(deviceHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_LEFT_MARGIN, propertyValue);
                        leftMargin = int.Parse(propertyValue.ToString());
                        DLL._IBSU_GetClientWindowProperty(deviceHandle, DLL.IBSU_ClientWindowPropertyId.ENUM_IBSU_WINDOW_PROPERTY_TOP_MARGIN, propertyValue);
                        TopMargin = int.Parse(propertyValue.ToString());

                        DLL.IBSU_SegmentPosition[] segmentArray = new DLL.IBSU_SegmentPosition[DLL.IBSU_MAX_SEGMENT_COUNT];
                        IntPtr ptrRunner_segmentArray = pSegmentPositionArray;

                        for (i = 0; i < segmentImageArrayCount; i++)
                        {
                            Color cr = Color.FromArgb(0, 128, 0);
                            uint lineColor = (uint)(cr.R + (cr.G << 8) + (cr.B << 16));
                            int x1 = 0;
                            int x2 = 0;
                            int x3 = 0;
                            int x4 = 0;

                            int y1 = 0;
                            int y2 = 0;
                            int y3 = 0;
                            int y4 = 0;

                            segmentArray[i] = (DLL.IBSU_SegmentPosition)Marshal.PtrToStructure(ptrRunner_segmentArray, typeof(DLL.IBSU_SegmentPosition));

                            x1 = leftMargin + (int)(segmentArray[i].x1 * scaleFactor);
                            x2 = leftMargin + (int)(segmentArray[i].x2 * scaleFactor);
                            x3 = leftMargin + (int)(segmentArray[i].x3 * scaleFactor);
                            x4 = leftMargin + (int)(segmentArray[i].x4 * scaleFactor);

                            y1 = TopMargin + (int)(segmentArray[i].y1 * scaleFactor);
                            y2 = TopMargin + (int)(segmentArray[i].y2 * scaleFactor);
                            y3 = TopMargin + (int)(segmentArray[i].y3 * scaleFactor);
                            y4 = TopMargin + (int)(segmentArray[i].y4 * scaleFactor);

                            DLL._IBSU_ModifyOverlayQuadrangle(deviceHandle, m_nOvSegmentHandle[i], x1, y1, x2, y2, x3, y3, x4, y4, 1, lineColor);
                            ptrRunner_segmentArray = (IntPtr)((int)ptrRunner_segmentArray + Marshal.SizeOf(segmentArray[0]));


                        }
                    }
                    
                    if (m_bNFIQScore)
                    {
                        int[] nfiq_score = new int[DLL.IBSU_MAX_SEGMENT_COUNT + 1];
                        int score = 0;
                        int nRc = 0;
                        int segment_pos = 0;
                        DLL.IBSU_ImageData[] imageArray = new DLL.IBSU_ImageData[DLL.IBSU_MAX_SEGMENT_COUNT];
                        IntPtr ptrRunner = pSegmentImageArray;

                        for (i = 0; i <= DLL.IBSU_MAX_SEGMENT_COUNT; i++)
                        {
                            nfiq_score[i] = 0;
                        }

                        string strValue = "Err";
                        OnMsg_UpdateStatusMessage("Get NFIQ score...");

                        for (i = 0; i < DLL.IBSU_MAX_SEGMENT_COUNT; i++)
                        {
                            if (m_FingerQuality[i] == DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT)
                            {
                                continue;
                            }
                            imageArray[i] = (DLL.IBSU_ImageData)Marshal.PtrToStructure(ptrRunner, typeof(DLL.IBSU_ImageData));

                            nRc = DLL._IBSU_GetNFIQScore(deviceHandle, imageArray[i].Buffer, imageArray[i].Width, imageArray[i].Height, imageArray[i].BitsPerPixel, ref score);

                            if (nRc == DLL.IBSU_STATUS_OK)
                            {
                                nfiq_score[i] = score;
                            }
                            else
                            {
                                nfiq_score[i] = -1;
                            }

                            segment_pos = segment_pos + 1;

                        }
                        strValue = String.Format("{0}-{1}-{2}-{3}", nfiq_score[0], nfiq_score[1], nfiq_score[2], nfiq_score[3]);
                        OnMsg_UpdateNFIQScore(strValue);
                    }

                    string strValue1 = null;
                    if (imageStatus == DLL.IBSU_STATUS_OK)
                    {
                        strValue1 = String.Format("Acquisition completed successfully");
                        _SetImageMessage(strValue1);
                        OnMsg_UpdateStatusMessage(strValue1);
                    }
                    else
                    {
                        strValue1 = String.Format("Acquisition Waring (Warning code = {0})", imageStatus);
                        _SetImageMessage(strValue1);
                        OnMsg_UpdateStatusMessage(strValue1);

                        OnMsg_AskRecapture(imageStatus);
                        return;
                    }
                }
                else
                {
                    m_strImageMessage = String.Format("Acquisition fail (Error code = {0})", imageStatus);
                    _SetImageMessage(m_strImageMessage);
                    OnMsg_UpdateStatusMessage(m_strImageMessage);
                    m_nCurrentCaptureStep = m_vecCaptureSeq.Count;

                }
                OnMsg_CaptureSeqNext();
            }

        }



        private void OnEvent_ClearPlatenAtCapture(
            int deviceIndex,
            IntPtr pContext,
            DLL.IBSU_PlatenState platenState
            )
        {
            if (pContext == null)
                return;

            lock (m_sync)
            {
                //                SDKMainForm pDlg = (SDKMainForm)SDKMainForm.FromHandle(pContext);
                if (platenState == DLL.IBSU_PlatenState.ENUM_IBSU_PLATEN_HAS_FINGERS)
                    m_bNeedClearPlaten = true;
                else
                    m_bNeedClearPlaten = false;

                if (m_bNeedClearPlaten)
                {
                    m_strImageMessage = "Please remove your fingers on the platen first!";
                    _SetImageMessage(m_strImageMessage);
                    OnMsg_UpdateStatusMessage(m_strImageMessage);
                }
                else
                {
                    CaptureInfo info = m_vecCaptureSeq[m_nCurrentCaptureStep];

                    // Display message for image acuisition again
                    string strMessage;
                    strMessage = String.Format("{0}", info.PreCaptureMessage);

                    OnMsg_UpdateStatusMessage(strMessage);

                    _SetImageMessage(strMessage);
                    m_strImageMessage = strMessage;
                }

                OnMsg_InvalidateQuality();
            }
        }






        private delegate void OnMsg_CaptureSeqStartDelegate();
        private void OnMsg_CaptureSeqStart()
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

            m_vecCaptureSeq.Clear();
            CaptureInfo info = new CaptureInfo();

            /** Please refer to definition below
            private const string CAPTURE_SEQ_FLAT_SINGLE_FINGER	= "Single flat finger";
            private const string CAPTURE_SEQ_ROLL_SINGLE_FINGER = "Single rolled finger";
            private const string CAPTURE_SEQ_2_FLAT_FINGERS = "2 flat fingers";
            private const string CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS = "10 single flat fingers";
            private const string CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS = "10 single rolled fingers";
            */
            //            if (strCaptureSeq == CAPTURE_SEQ_FLAT_SINGLE_FINGER)
            {
                info.PreCaptureMessage = "Please put a single finger on the sensor!";
                info.PostCaptuerMessage = "Keep finger on the sensor!";
                info.ImageType = DLL.IBSU_ImageType.ENUM_IBSU_FLAT_SINGLE_FINGER;
                info.NumberOfFinger = 1;
                info.fingerName = "SFF_Unknown";
                info.fingerPosition = DLL.IBSM_FingerPosition.IBSM_FINGER_POSITION_UNKNOWN;
                m_vecCaptureSeq.Add(info);
                int count = m_vecCaptureSeq.Count;
            }

            // Make subfolder name
            m_ImgSubFolder = DateTime.Now.ToString("yyyy-MM-dd HHmmss");

            OnMsg_CaptureSeqNext();
        }

        private delegate void OnMsg_CaptureSeqNextDelegate();
        private void OnMsg_CaptureSeqNext()
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
                Win32.SetFocus(m_btnCaptureStart.Handle);
                return;
            }

            for (int i = 0; i < DLL.IBSU_MAX_SEGMENT_COUNT; i++)
            {
                DLL._IBSU_ModifyOverlayQuadrangle(m_nDevHandle, m_nOvSegmentHandle[i], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
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
                _SetStatusBarMessage(message);
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
                _SetStatusBarMessage(strMessage);

                _SetImageMessage(strMessage);
                m_strImageMessage = strMessage;
            }
            else
            {
                string strMessage;
                strMessage = String.Format("Failed to execute IBSU_BeginCaptureImage()");
                _SetStatusBarMessage(strMessage);
            }

            OnMsg_UpdateDisplayResources();
        }

        private delegate void OnMsg_DeviceCommunicationBreakDelegate();
        private void OnMsg_DeviceCommunicationBreak()
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

        private delegate void OnMsg_InitWarningDelegate();
        private void OnMsg_InitWarning()
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

        private delegate void OnMsg_DrawClientWindowDelegate();
        private void OnMsg_DrawClientWindow()
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

        private delegate void OnMsg_UpdateDeviceListDelegate();
        private void OnMsg_UpdateDeviceList()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_UpdateDeviceListDelegate(OnMsg_UpdateDeviceList));

                return;
            }

            bool idle = !m_bInitializing && (m_nCurrentCaptureStep == -1);
            if (idle)
            {
                m_btnCaptureStop.Enabled = false;
                m_btnCaptureStart.Enabled = false;
            }

            // store currently selected device
            string strSelectedText = "";
            int selectedDev = m_cboUsbDevices.SelectedIndex;
            if (selectedDev > -1)
                strSelectedText = m_cboUsbDevices.Text;

            m_cboUsbDevices.Items.Clear();
            m_cboUsbDevices.Items.Add("- Please select -");

            // populate combo box
            int devices = 0;
            DLL._IBSU_GetDeviceCount(ref devices);

            selectedDev = 0;
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

                m_cboUsbDevices.Items.Add(strDevice);
                if (strDevice == strSelectedText)
                    selectedDev = i + 1;
            }

            if ((selectedDev == 0) && (m_cboUsbDevices.Items.Count == 2))
                selectedDev = 1;

            m_cboUsbDevices.SelectedIndex = selectedDev;
            m_cboUsbDevices.Update();

            if (idle)
            {
                m_cboUsbDevices_SelectedIndexChanged(null, null);
                //                _UpdateCaptureSequences();
                OnMsg_UpdateDisplayResources();
            }
        }

        private delegate void OnMsg_UpdateDisplayResourcesDelegate();
        private void OnMsg_UpdateDisplayResources()
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

            bool selectedDev = m_cboUsbDevices.SelectedIndex > 0;
            bool idle = !m_bInitializing && (m_nCurrentCaptureStep == -1);
            bool active = !m_bInitializing && (m_nCurrentCaptureStep != -1);
            bool uninitializedDev = selectedDev && (m_nDevHandle == -1);


            m_cboUsbDevices.Enabled = idle;

            m_btnCaptureStart.Enabled = true;
            m_btnCaptureStop.Enabled = active;

            string strCaption = "";
            if (active)
                strCaption = "Take Result Image";
            else if (!active && !m_bInitializing)
                strCaption = "Start";

            m_btnCaptureStart.Text = strCaption;
        }

        private delegate void OnMsg_UpdateStatusMessageDelegate(string message);
        private void OnMsg_UpdateStatusMessage(string message)
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

            _SetStatusBarMessage(message);
        }

        private delegate void OnMsg_UpdateImageMessageDelegate(string message);
        private void OnMsg_UpdateImageMessage(string message)
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

            _SetImageMessage(message);
        }

        private delegate void OnMsg_BeepDelegate(int beepType);
        private void OnMsg_Beep(int beepType)
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_BeepDelegate(OnMsg_Beep),
                    new object[] { beepType });

                return;
            }

            if (beepType == __BEEP_FAIL__)
                _BeepFail();
            else if (beepType == __BEEP_SUCCESS__)
                _BeepSuccess();
            else if (beepType == __BEEP_OK__)
                _BeepOk();
            else if (beepType == __BEEP_DEVICE_COMMUNICATION_BREAK__)
                _BeepDeviceCommunicationBreak();
        }

        private delegate void OnMsg_UpdateNFIQScoreDelegate(string score);
        private void OnMsg_UpdateNFIQScore(string score)
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_UpdateNFIQScoreDelegate(OnMsg_UpdateNFIQScore),
                    new object[] { score });

                return;
            }

            //            m_txtNFIQScore.Text = score;
        }

        private delegate void OnMsg_AskRecaptureDelegate(int imageStatus);
        private void OnMsg_AskRecapture(int imageStatus)
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_AskRecaptureDelegate(OnMsg_AskRecapture),
                    new object[] { imageStatus });

                return;
            }

            String strValue;

            strValue = String.Format("[Warning = {0}] Do you want a recapture?", imageStatus);

            if (MessageBox.Show(strValue, "IBScanUltimateSDK", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1) == DialogResult.Yes)
            {
                m_nCurrentCaptureStep = m_nCurrentCaptureStep - 1;
            }

            OnMsg_CaptureSeqNext();
        }

        private delegate void OnMsg_InvalidateQualityDelegate();
        private void OnMsg_InvalidateQuality()
        {
            // Check if we need to call beginInvoke.
            if (this.InvokeRequired)
            {
                // Pass the same function to BeginInvoke,
                // but the call would come on the correct
                // thread and InvokeRequired will be false
                this.BeginInvoke(new OnMsg_InvalidateQualityDelegate(OnMsg_InvalidateQuality));

                return;
            }

            m_picScanner.Invalidate();
        }



        ///////////////////////////////////////////////////////////////////////////////////////////
        // Form event
        ///////////////////////////////////////////////////////////////////////////////////////////
        private void SDKMainForm_Load(object sender, EventArgs e)
        {
            m_nDevHandle = -1;
            m_nCurrentCaptureStep = -1;
            m_bInitializing = false;
            m_strImageMessage = "";
            m_bNeedClearPlaten = false;

            m_bUseClearPlaten = false;
            m_bNFIQScore = false;
            m_bDrawSegmentImage = true;
            m_bSaveImages = true;

            /*            m_cboSmearLevel.Items.Clear();
                        m_cboSmearLevel.Items.Add("LOW");
                        m_cboSmearLevel.Items.Add("MEDIUM");
                        m_cboSmearLevel.Items.Add("HIGH");
                        m_cboSmearLevel.SelectedIndex = 1;

                        m_sliderContrast.SetRange(DLL.IBSU_MIN_CONTRAST_VALUE, DLL.IBSU_MAX_CONTRAST_VALUE);
                        m_sliderContrast.Value = 0;
                        m_sliderContrast.TickFrequency = 5;
                        m_txtContrast.Text = "0";
            */
            m_verInfo = new DLL.IBSU_SdkVersion();
            DLL._IBSU_GetSDKVersion(ref m_verInfo);

            string tileName;
            tileName = String.Format("IBSU_CSharpWM - {0}", m_verInfo.Product);
            this.Text = tileName;

            //            StringBuilder path = new StringBuilder(260);
            //            Win32.SHGetSpecialFolderPath(IntPtr.Zero, path, (int)Win32.CSIDL.CSIDL_MYPICTURES, false);
            //            m_ImgSaveFolder = path.ToString();
            m_ImgSaveFolder = "My Documents\\My Pictures";

            m_callbackDeviceCommunicationBreak = new DLL.IBSU_Callback(OnEvent_DeviceCommunicationBreak);
            m_callbackPreviewImage = new DLL.IBSU_CallbackPreviewImage(OnEvent_PreviewImage);
            m_callbackFingerCount = new DLL.IBSU_CallbackFingerCount(OnEvent_FingerCount);
            m_callbackFingerQuality = new DLL.IBSU_CallbackFingerQuality(OnEvent_FingerQuality);
            m_callbackDeviceCount = new DLL.IBSU_CallbackDeviceCount(OnEvent_DeviceCount);
            m_callbackInitProgress = new DLL.IBSU_CallbackInitProgress(OnEvent_InitProgress);
            m_callbackTakingAcquisition = new DLL.IBSU_CallbackTakingAcquisition(OnEvent_TakingAcquisition);
            m_callbackCompleteAcquisition = new DLL.IBSU_CallbackCompleteAcquisition(OnEvent_CompleteAcquisition);
            m_callbackClearPlaten = new DLL.IBSU_CallbackClearPlatenAtCapture(OnEvent_ClearPlatenAtCapture);
            m_callbackResultImageEx = new DLL.IBSU_CallbackResultImageEx(OnEvent_ResultImageEx);

            DLL._IBSU_RegisterCallbacks(0, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_DEVICE_COUNT, m_callbackDeviceCount, this.Handle);
            DLL._IBSU_RegisterCallbacks(0, DLL.IBSU_Events.ENUM_IBSU_ESSENTIAL_EVENT_INIT_PROGRESS, m_callbackInitProgress, this.Handle);

            m_thread = null;

            OnMsg_UpdateDeviceList();
        }

        private void m_btnCaptureStart_Click(object sender, EventArgs e)
        {
            int nRc;

            if (m_bInitializing)
                return;

            int devIndex = m_cboUsbDevices.SelectedIndex - 1;
            if (devIndex < 0)
                return;

            if (m_nCurrentCaptureStep != -1)
            {
                bool IsActive = false;
                nRc = DLL._IBSU_IsCaptureActive(m_nDevHandle, ref IsActive);
                if (nRc == DLL.IBSU_STATUS_OK && IsActive)
                {
                    DLL._IBSU_TakeResultImageManually(m_nDevHandle);
                }

                return;
            }

            if (m_nDevHandle == -1)
            {
                m_bInitializing = true;
                m_param.devIndex = devIndex;
                m_param.pParentHandle = this.Handle;
                m_param.pFrameImageHandle = m_FrameImage.Handle;
                m_initThread = new ThreadStart(_InitializeDeviceThreadCallback);
                m_thread = new Thread(m_initThread);
                m_thread.Start();
            }
            else
            {
                // device already initialized
                OnMsg_CaptureSeqStart();
            }

            OnMsg_UpdateDisplayResources();
        }

        private void m_btnCaptureStop_Click(object sender, EventArgs e)
        {
            if (m_nDevHandle == -1)
                return;

            DLL._IBSU_CancelCaptureImage(m_nDevHandle);
            m_nCurrentCaptureStep = -1;
            m_bNeedClearPlaten = false;

            string message;
            message = String.Format("Capture Sequence aborted");
            _SetStatusBarMessage(message);
            m_strImageMessage = "";
            _SetImageMessage("");
            OnMsg_UpdateDisplayResources();
        }

        private void Timer_StatusFingerQuality_Tick(object sender, EventArgs e)
        {
            Boolean idle = !m_bInitializing && (m_nCurrentCaptureStep == -1);

            if (!idle)
            {
                if (m_bNeedClearPlaten && m_bBlank)
                {
                    _ModifyOverlayForWarningOfClearPlaten(true);
                }
                else
                    _ModifyOverlayForWarningOfClearPlaten(false);
            }

            m_picScanner.Invalidate();
            if (m_bNeedClearPlaten)
                m_bBlank = !m_bBlank;
        }

        private void SDKMainForm_FormClosing(object sender, CancelEventArgs e)
        {
            _ReleaseDevice();
        }

        private void m_cboUsbDevices_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (m_cboUsbDevices.SelectedIndex == m_nSelectedDevIndex)
                return;

            m_nSelectedDevIndex = m_cboUsbDevices.SelectedIndex;
            if (m_nDevHandle != -1)
            {
                m_btnCaptureStop_Click(sender, e);
                _ReleaseDevice();
            }

            //            _UpdateCaptureSequences();
            OnMsg_UpdateDisplayResources();
        }

        private void m_cboCaptureSeq_SelectedIndexChanged(object sender, EventArgs e)
        {
            OnMsg_UpdateDisplayResources();
        }

        private void m_picScanner_Paint(object sender, PaintEventArgs e)
        {
            SolidBrush hbr_finger;
            //            SolidBrush hbr_touch;

            for (int i = 0; i < 4; i++)
            {
                if (m_bNeedClearPlaten)
                {
                    if (m_bBlank)
                        hbr_finger = new SolidBrush(Color.Red);
                    else
                        hbr_finger = new SolidBrush(Color.FromArgb(255, 255, 255));
                }
                else
                {
                    switch (m_FingerQuality[i])
                    {
                        case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_GOOD:
                            hbr_finger = new SolidBrush(Color.Green);
                            break;
                        case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_FAIR:
                            hbr_finger = new SolidBrush(Color.Orange);
                            break;
                        case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_POOR:
                            hbr_finger = new SolidBrush(Color.Red);
                            break;
                        case DLL.IBSU_FingerQualityState.ENUM_IBSU_FINGER_NOT_PRESENT:
                            hbr_finger = new SolidBrush(Color.FromArgb(255, 255, 255));
                            break;
                        case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_INVALID_AREA_TOP:
                            hbr_finger = new SolidBrush(Color.Red);
                            break;
                        case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_INVALID_AREA_LEFT:
                            hbr_finger = new SolidBrush(Color.Red);
                            break;
                        case DLL.IBSU_FingerQualityState.ENUM_IBSU_QUALITY_INVALID_AREA_RIGHT:
                            hbr_finger = new SolidBrush(Color.Red);
                            break;
                        default:
                            return;
                    }
                }

                _DrawRectangle(e.Graphics, hbr_finger, 0 + i * 118, 0, 110, 20);
            }

            // Draw detected finger on the touch sensor
            /*            int touchInValue = 0;
                        DLL._IBSU_IsTouchedFinger(m_nDevHandle, ref touchInValue);
                        if (touchInValue == 1)
                            hbr_touch = new SolidBrush(Color.Green);
                        else
                            hbr_touch = new SolidBrush(Color.FromArgb(78, 78, 78));

                        _DrawRectangle(e.Graphics, hbr_touch, 15, 112, 85, 52);
            */
        }

        private void m_btnClose_Click(object sender, EventArgs e)
        {
            _ReleaseDevice();
            DLL._IBSU_UnloadLibrary();
            Application.Exit();
            Application.DoEvents();
        }
    }
}