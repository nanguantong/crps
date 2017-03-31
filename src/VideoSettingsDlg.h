
// VideoSettingsDlg.h : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"

// CVideoSettingsDlg dialog

class CVideoSettingsDlg : public CDialog
{
    DECLARE_DYNAMIC(CVideoSettingsDlg)

public:
    CVideoSettingsDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CVideoSettingsDlg();

    // Dialog Data
    enum { IDD = IDD_DIALOG_VIDEO };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedCheckHardwareEncode();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtnModifyPath();
    afx_msg void OnBnClickedCheckRecordMp4();
    afx_msg void OnCbnSelchangeComboVideoDevice1();
    afx_msg void OnCbnSelchangeComboVideoDevice2();
    afx_msg void OnCbnSelchangeComboVideoDevice3();
    afx_msg void OnCbnSelchangeComboVideoDevice4();

    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

public:
    void SetVideoComboxItem(int nChannel, int nIndex, LPCTSTR lpszValue);
    void SetAudioComboxItem(int nChannel, int nIndex, LPCTSTR lpszValue);

    void SetEnableAVCombox(BOOL bEnable);
    BOOL GetVideoAvailable(int nChannel);
    BOOL GetAudioAvailable(int nChannel);

public:
    CComboBox       m_cmbVD[CAPTURE_NUM];
    CComboBox       m_cmbAD[CAPTURE_NUM];

    CComboBox       m_cmbVideoSize;
    CComboBox       m_cmbVideoRate;
    CEdit           m_editRecordPath;
    CButton         m_btnModifyPath;
    CStatic         m_staticDirSize;

    CFont           m_fontDefault;
    POINT           m_pointOld;

    BOOL            m_bHardwareEncode;

    SIZE            m_siDefault;
    double          m_fDefaultRate;

    BOOL            m_bRecordMp4;
    CString         m_strFilepath;
    CString         m_strRecordPath;
    CString         m_strSnapshotPath;

    int             m_nVDSel[CAPTURE_NUM];
    int             m_nADSel[CAPTURE_NUM];
};
