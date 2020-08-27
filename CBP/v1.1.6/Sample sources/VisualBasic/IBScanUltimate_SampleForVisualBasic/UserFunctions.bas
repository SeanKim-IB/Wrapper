Attribute VB_Name = "Module6"
Option Explicit


'///////////////////////////////////////////////////////////////////////////////////////
'// Make window message handler

Public Const GWL_WNDPROC = -4

'/// Defined window form message
Public Const WM_NCDESTROY = &H82&
Public Const WM_NCPAINT = &H85&


'/// Use user window messages
Public Const WM_USER = &H400&

Public Const WM_USER_CAPTURE_SEQ_START = (WM_USER + 1)
Public Const WM_USER_CAPTURE_SEQ_NEXT = (WM_USER + 2)
Public Const WM_USER_DEVICE_COMMUNICATION_BREAK = (WM_USER + 3)
Public Const WM_USER_DRAW_CLIENT_WINDOW = (WM_USER + 4)
Public Const WM_USER_UPDATE_DEVICE_LIST = (WM_USER + 5)
Public Const WM_USER_INIT_WARNING = (WM_USER + 6)
Public Const WM_USER_UPDATE_DISPLAY_RESOURCES = (WM_USER + 7)
Public Const WM_USER_UPDATE_STATUS_MESSAGE = (WM_USER + 8)
Public Const WM_USER_BEEP = (WM_USER + 9)
Public Const WM_USER_DRAW_FINGER_QUALITY = (WM_USER + 10)

Public Type CaptureInfo
    PreCaptureMessage As String         '// to display on fingerprint window
    PostCaptuerMessage As String        '// to display on fingerprint window
    ImageType As IBSU_ImageType         '// capture mode
    NumberOfFinger As Long              '// number of finger count
    fingerName As String                '// finger name (e.g left thumbs, left index ... )
End Type

'// Beep definitions
Public Const BEEP_FAIL As Long = 0
Public Const BEEP_SUCCESS As Long = 1
Public Const BEEP_OK As Long = 2
Public Const BEEP_DEVICE_COMMUNICATION_BREAK As Long = 3

'// LED color definitions
Public Const LED_COLOR_NONE As Long = 0
Public Const LED_COLOR_GREEN As Long = 1
Public Const LED_COLOR_RED As Long = 2
Public Const LED_COLOR_YELLOW As Long = 3

'// Key button definitions
Public Const LEFT_KEY_BUTTON As Long = 1
Public Const RIGHT_KEY_BUTTON As Long = 2

'// Capture sequence definitions
Public Const CAPTURE_SEQ_FLAT_SINGLE_FINGER As String = "Single flat finger"
Public Const CAPTURE_SEQ_ROLL_SINGLE_FINGER As String = "Single rolled finger"
Public Const CAPTURE_SEQ_2_FLAT_FINGERS As String = "2 flat fingers"
Public Const CAPTURE_SEQ_10_SINGLE_FLAT_FINGERS As String = "10 single flat fingers"
Public Const CAPTURE_SEQ_10_SINGLE_ROLLED_FINGERS As String = "10 single rolled fingers"
Public Const CAPTURE_SEQ_4_FLAT_FINGERS As String = "4 flat fingers"
Public Const CAPTURE_SEQ_10_FLAT_WITH_4_FINGER_SCANNER As String = "10 flat fingers with 4-finger scanner"

Public m_nSelectedDevIndex As Integer                  '///< Index of selected device
Public m_nDevHandle As Long                            '///< Device handle
Public m_bInitializing As Boolean                         '///< Device initialization is in progress
Public m_ImgSaveFolder As String                       '///< Base folder for image saving
Public m_ImgSubFolder As String                        '///< Sub Folder for image sequence
Public m_strImageMessage As String
Public m_bNeedClearPlaten As Boolean
Public m_bBlank As Boolean

Public m_nOvImageTextHandle As Long
Public m_nOvClearPlatenHandle As Long
Public m_nOvSegmentHandle(0 To IBSU_MAX_SEGMENT_COUNT - 1) As Long

Public m_vecCaptureSeq() As CaptureInfo
Public m_nCurrentCaptureStep As Long

Public m_verInfo As IBSU_SdkVersion
Public m_FingerQuality(0 To IBSU_MAX_SEGMENT_COUNT - 1) As IBSU_FingerQualityState

Public m_ImageData As IBSU_ImageData

Public m_saveFingerCountState As IBSU_FingerCountState
Public m_LedType As IBSU_LedType


