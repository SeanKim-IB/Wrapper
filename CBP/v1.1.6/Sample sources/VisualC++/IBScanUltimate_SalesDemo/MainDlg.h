#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "ZoomDlg.h"
#include "ColorButton.h"
#include "IBScanUltimateApi_defs.h"

class CIBScanUltimate_SalesDemoDlg;

// CMainDlg 대화 상자입니다.

class CMainDlg : public CDialog
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMainDlg();

	void Init_Layout();


	void OnlySingleFlat_Layout();					//10
	void OnlyTwoFlat_Layout();						//1
	void TwoFlatAndSingleFlat_Layout();				//3
	void TwoFlatAndSingleFlatAndRoll_Layout();		//14
	void TwoFlatAndRoll_1_Layout();					//16
	void TwoFlatAndRoll_2_Layout();					//19
	void doubleFourFlat_Layout();				//442layout

	void DrawImage();
	void _UpdateCaptureSequences();
	static UINT _InitializeDeviceThreadCallback( LPVOID pParam );

	int SaveWSQ(CString filename,unsigned char *buffer, int width, int height, int pitch = -800);
	/*void BinFinger(IBSM_Template Template, int index,int type);
	void AddFinger(IBSM_Template Template, int index);
	void AddFourFinger(IBSM_Template Template, int index);
	void AddTwoFinger(IBSM_Template Template, int index);
	void AddThumbFinger(IBSM_Template Template, int index);

	int CheckDuplicate(unsigned char *ImgBuf, int width, int height, IBSM_Template *Template,UINT m_CaptureFinger);
	int CheckTwoFingerDuplicate(unsigned char *ImgBuf, int width, int height, IBSM_Template *Template,UINT m_CaptureFinger);
	int CheckFourFingerDuplicate(unsigned char *ImgBuf, int width, int height, IBSM_Template *Template,UINT m_CaptureFinger);
	int CheckThumbDuplicate(unsigned char *ImgBuf, int width, int height, IBSM_Template *Template,UINT m_CaptureFinger);

	int CheckMatch(int index, unsigned char *ImgBuf, int width, int height, IBSM_Template *Template);
	int CheckMatch2(int index, unsigned char *ImgBuf, int width, int height, IBSM_Template *Template);
	int CheckMatch4(int index, unsigned char *ImgBuf, int width, int height, IBSM_Template *Template);
	*/


	int SaveISOFIR(CString filename,unsigned char *buffer,int width,int height);
	//int SaveImgFormat(CString format, unsigned char *buffer, BYTE bitmap , BYTE wsq, BYTE iso);

	//Sequence CaptureImage
	void _FourFingerDevice(UINT m_CaptureMode, IBSU_ImageType imageType, int segmentImageArrayCount, int nfiq_score[IBSU_MAX_SEGMENT_COUNT], 
							int nfiq2_score[IBSU_MAX_SEGMENT_COUNT], int spoof_score[IBSU_MAX_SEGMENT_COUNT], IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray);
	void _TowOtherDevice(UINT m_CaptureMode, int imageStatus, int segmentImageArrayCount, int nfiq_score[IBSU_MAX_SEGMENT_COUNT], 
							int nfiq2_score[IBSU_MAX_SEGMENT_COUNT], int spoof_score[IBSU_MAX_SEGMENT_COUNT], IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray);
	void _TwoBasicDevice(UINT m_CaptureMode, int imageStatus, int segmentImageArrayCount, int nfiq_score[IBSU_MAX_SEGMENT_COUNT], int nfiq2_score[IBSU_MAX_SEGMENT_COUNT], int spoof_score[IBSU_MAX_SEGMENT_COUNT], 
							IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray);
	void _SingleDevice(UINT m_CaptureMode, int imageStatus, int nfiq_score[IBSU_MAX_SEGMENT_COUNT], int nfiq2_score[IBSU_MAX_SEGMENT_COUNT], int spoof_score[IBSU_MAX_SEGMENT_COUNT], 
							  IBSU_ImageData image, const IBSU_ImageData *pSegmentImageArray, const IBSU_SegmentPosition *pSegmentPositionArray);

	int	_WaitingForFinishInitDevice( const int devIndex );

    // 대화 상자 데이터입니다.
	enum { IDD = IDD_MAIN_DIALOG };

	CIBScanUltimate_SalesDemoDlg	*m_pParent;
	CZoomDlg			*m_ZoomDlg;



	UINT				m_CaptureMode;
	UINT				m_CaptureFinger;
	UINT				m_CaptureseqTabIdx;
	BOOL				m_CaptureCompleted;
	
	BYTE				*m_ImgBuf[IMAGE_BUFFER_COUNT];
	BYTE				*m_ImgBuf_S[IMAGE_BUFFER_COUNT];
	BITMAPINFO			*m_Info;
	BITMAPINFO			*m_Info_1;
	BITMAPINFO			*m_Info_2;
	BITMAPINFO			*m_Info_3;
	BITMAPINFO			*m_Info_4;
	

	int					m_CaptureBtnFinger;
	int					m_nCurrWidth;
	int					m_nCurrHeight;

	int					m_nCurrWidth_1;
	int					m_nCurrHeight_1;

	int					m_nCurrWidth_3;
	int					m_nCurrHeight_3;

	int					m_nCurrentWidthForSplit[IMAGE_BUFFER_COUNT];
	int					m_nCurrentHeightForSplit[IMAGE_BUFFER_COUNT];

	BOOL				m_PressedBtn;
	
	BYTE				m_NFIQBuf[IMAGE_BUFFER_COUNT]; //1 finger
	BYTE				m_NFIQ2Buf[IMAGE_BUFFER_COUNT]; //1 finger
	USHORT				m_SpoofBuf[IMAGE_BUFFER_COUNT]; //1 finger

	BYTE				m_NFIQBuf1[IMAGE_BUFFER_COUNT];//4 Hand
	BYTE				m_NFIQBuf2[IMAGE_BUFFER_COUNT];//4 Hand
	BYTE				m_NFIQBuf3[IMAGE_BUFFER_COUNT];//4 Hand
	BYTE				m_NFIQBuf4[IMAGE_BUFFER_COUNT];//4 Hand

	BYTE				m_NFIQ2Buf1[IMAGE_BUFFER_COUNT];//4 Hand
	BYTE				m_NFIQ2Buf2[IMAGE_BUFFER_COUNT];//4 Hand
	BYTE				m_NFIQ2Buf3[IMAGE_BUFFER_COUNT];//4 Hand
	BYTE				m_NFIQ2Buf4[IMAGE_BUFFER_COUNT];//4 Hand

	USHORT				m_SpoofBuf1[IMAGE_BUFFER_COUNT];//4 Hand
	USHORT				m_SpoofBuf2[IMAGE_BUFFER_COUNT];//4 Hand
	USHORT				m_SpoofBuf3[IMAGE_BUFFER_COUNT];//4 Hand
	USHORT				m_SpoofBuf4[IMAGE_BUFFER_COUNT];//4 Hand

	BYTE				m_NFIQTwoBuf1[IMAGE_BUFFER_COUNT];//2 finger
	BYTE				m_NFIQTwoBuf2[IMAGE_BUFFER_COUNT];//2 finger

	BYTE				m_NFIQ2TwoBuf1[IMAGE_BUFFER_COUNT];//2 finger
	BYTE				m_NFIQ2TwoBuf2[IMAGE_BUFFER_COUNT];//2 finger

	USHORT				m_SpoofTwoBuf1[IMAGE_BUFFER_COUNT];//2 finger
	USHORT				m_SpoofTwoBuf2[IMAGE_BUFFER_COUNT];//2 finger
	

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

public:
	CComboBox m_DeviceList;
	CComboBox m_SequenceList;
	CBitmapButton m_btnCapture;
	CBitmapButton m_btnEdit;
	CBitmapButton m_btnSave;
	CBrush m_background;
	CColorCtrl<CBoldCtrl <CStatic> > m_strDeviceList;
	CColorCtrl<CBoldCtrl <CStatic> > m_strCaptureSeq;
	
	afx_msg void OnCbnSelchangeDeviceList();
	afx_msg void OnCbnSelchangeSequenceList();
	afx_msg void OnBnClickedStartCapture();
	afx_msg void OnBnClickedEditCapture();
	afx_msg void OnBnClickedSaveCapture();
	afx_msg void OnBnClickedUseAutoSplit();

	CColorCtrl<CBoldCtrl <CStatic> > m_strView0;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView1;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView2;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView3;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView4;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView5;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView6;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView7;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView8;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView9;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView91;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView92;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView93;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView94;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView95;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView96;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView97;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView98;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView99;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView990;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView991;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView161;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView162;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView163;
	CColorCtrl<CBoldCtrl <CStatic> > m_strView164;
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CString m_SeqName;
	CStatic m_seq_preview;
	CProgressCtrl m_ProgressSave;
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
