
// UsbFwWriterDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include <vector>
#include "IBScanUltimateApi_defs.h"
#include "afxcmn.h"
#include "EditProperty.h"
#include "btnst.h"
#include "ShadeButtonST.h"
#include "Progressive.h"

#define		STRINGLENGTH  32


typedef struct tagPRODUCTPROPERTY
{
	char product_ID[STRINGLENGTH];
	char firmware_version[STRINGLENGTH];
	char vender_ID[STRINGLENGTH];
	char revison[STRINGLENGTH];
	char reserve[STRINGLENGTH];
} PRODUCTPROPERTY;

typedef struct tagFIRMWARE_FILE_INFO
{
	char pathName[MAX_PATH];
	char fileName[MAX_PATH];
	int	 fileLength;
	UCHAR fwData[8192];
	char product_ID[STRINGLENGTH];
	char firmware_version[STRINGLENGTH];
	char vender_ID[STRINGLENGTH];
	char revison[STRINGLENGTH];
	char reserve[STRINGLENGTH];
	UINT crc;
} FIRMWARE_FILE_INFO;

// CUsbFwWriterDlg dialog
class CUsbFwWriterDlg : public CDialog
{
// Construction
public:
	CUsbFwWriterDlg(CWnd* pParent = NULL);	// standard constructor

	virtual ~CUsbFwWriterDlg();

// Dialog Data
	enum { IDD = IDD_USBFWWRITER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	/// Callback functions.
	/// Please refer to IBScanUltimateApi_defs.h in include folder.
	static void CALLBACK OnEvent_DeviceCommunicationBreak(
						 const int deviceHandle,
						 void*     pContext
					   );

	static void CALLBACK OnEvent_PreviewImage(
						 const int deviceHandle,
						 void*     pContext,
						 const	   IBSU_ImageData image
					   );

	static void CALLBACK OnEvent_FingerCount(
						   const int                   deviceHandle,
						   void*                       pContext,
						   const IBSU_FingerCountState fingerCountState
						 );

	static void CALLBACK OnEvent_FingerQuality(
                          const int                     deviceHandle,   
                          void                          *pContext,       
                          const IBSU_FingerQualityState *pQualityArray, 
                          const int                     qualityArrayCount    
						 );

	static void CALLBACK OnEvent_DeviceCount(
						 const int detectedDevices,
						 void      *pContext
					   );

	static void CALLBACK OnEvent_InitProgress(
						 const int deviceIndex,
						 void      *pContext,
						 const int progressValue
					   );

	static void CALLBACK OnEvent_TakingAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   );

	static void CALLBACK OnEvent_CompleteAcquisition(
						 const int				deviceHandle,
						 void					*pContext,
						 const IBSU_ImageType	imageType
					   );

/****
 ** This IBSU_CallbackResultImage callback was deprecated since 1.7.0
 ** Please use IBSU_CallbackResultImageEx instead
    static void CALLBACK OnEvent_ResultImage(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_ImageData        image,
						  const IBSU_ImageType        imageType,
                          const IBSU_ImageData        *pSplitImageArray,
                          const int                   splitImageArrayCount
					   );
*/
    static void CALLBACK OnEvent_ResultImageEx(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const int                   imageStatus,
                          const IBSU_ImageData        image,
						  const IBSU_ImageType        imageType,
						  const int                   detectedFingerCount,
                          const int                   segmentImageArrayCount,
                          const IBSU_ImageData        *pSegmentImageArray,
						  const IBSU_SegmentPosition  *pSegmentPositionArray
                        );

	static void CALLBACK OnEvent_ClearPlatenAtCapture(
                          const int                   deviceHandle,
                          void                        *pContext,
                          const IBSU_PlatenState      platenState
                        );

	static void CALLBACK OnEvent_AsyncOpenDevice(
                          const int                   deviceIndex,
                          void                        *pContext,
                          const int                   deviceHandle,
                          const int                   errorCode
                        );

public:
	static UINT		FwProgressThread(LPVOID pParam);
	int				Control_WriteUsbFirmware(int handle, UCHAR *fwBuffer, int bufSize);//unsigned char* fwImage, int imageLen);
	int				_GetCheckedFwFileCount();
	int				_GetCheckedDevCount();
	void			_SetStatusBarMessage( LPCTSTR Format, ... );
	int				_ReleaseDevice(int deviceHandle);
	void			_BeepFail();
	void			_BeepSuccess();
	void			_BeepOk();
	void			_BeepDeviceCommunicationBreak();
	void			_UpdateScreenElements();

	BOOL			LoadFwFileToList(CString path, CString ext);
	void			WriteIniFile(char *saveDir);
	void			ReadIniFile();


// Member variables
public:
	char				m_cFwFilePath[MAX_PATH];
	char				m_cIniFileName[MAX_PATH];
	int					m_chknumberofprogress;
    CString				m_IBKProductionMode;				// Production mode for IBKorea
    CString				m_keyIBKProductionMode;				// Saved key-code variable for Production mode
	BOOL				m_bIBKProductionMode;
	CString				m_IBPartnerMode;					// To write Reserved string by partner
	CString				m_IBPartnerMode2;					// To write Reserved string by partner
    CString				m_keyIBPartnerMode;					// Saved key-code variable for partner mode
	BOOL				m_bIBPartnerMode;
//	Warning				WarningpDlg;
	CProgressive		*mpDlg;
	IBSU_SdkVersion		m_verInfo;
	FIRMWARE_FILE_INFO	m_fwFileInfo;
//	PROPERTY_INFO		m_propertyInfo;

	CWinThread*			m_FwThread;

	
private:
	
	

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	LRESULT OnMsg_CaptureSeqStart(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_CaptureSeqNext(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DeviceCommunicationBreak(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DrawClientWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_UpdateDeviceList(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_InitWarning(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_UpdateDisplayResources(WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT OnMsg_UpdateStatusMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_Beep(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_DrawFingerQuality(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_IBKProductionMode(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsg_IBPartnerMode(WPARAM wParam, LPARAM lParam);


	DECLARE_MESSAGE_MAP()

public:
	CFont* Property_old_font;
	CFont Property_new_font;
	LOGFONT Property_logfont;
	
	CListCtrl m_devList;
	CListCtrl m_FwFileList;
	CButtonST m_btnFirmwareUp;
	CButtonST m_editProperty;
	CButtonST m_btnChgFirmwareDir;


	virtual BOOL DestroyWindow();
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListFile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnChangeFwDir();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnFwUpdate();
	afx_msg void OnBnClickedBtnEditProperty();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnClose();
};




