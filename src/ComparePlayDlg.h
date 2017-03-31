
// ComparePlayDlg.h : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Common.h"

// CComparePlayDlg 对话框

class CComparePlayDlg : public CDialog
{
    DECLARE_DYNAMIC(CComparePlayDlg)

public:
    CComparePlayDlg(CWnd* pParent = NULL);   // 标准构造函数
    virtual ~CComparePlayDlg();

// 对话框数据
    enum { IDD = IDD_DIALOG_COMPARE_PLAY };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()

    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedBtnCmpPlayPath1();
    afx_msg void OnBnClickedBtnCmpPlayPath2();
    afx_msg void OnBnClickedBtnCmpPlayPause1();
    afx_msg void OnBnClickedBtnCmpStop1();
    afx_msg void OnBnClickedBtnCmpForword1();
    afx_msg void OnBnClickedBtnCmpBack1();
    afx_msg void OnCbnSelchangeComboSpeedCmpPlayDelta1();
    afx_msg void OnBnClickedBtnCmpPlayPause2();
    afx_msg void OnBnClickedBtnCmpStop2();
    afx_msg void OnBnClickedBtnCmpForword2();
    afx_msg void OnBnClickedBtnCmpBack2();
    afx_msg void OnCbnSelchangeComboSpeedCmpPlayDelta2();

public:
    virtual BOOL OnInitDialog();

    void ResizePlayWindow(LPRECT lprcPosition = NULL);
    int GetCurPlaySel();
    void GetPlayRect();
    CRect GetScreenRectBeforeFullScreen(int nCurPlaySel);
    CRect GetClientRectBeforeFullScreen(int nCurPlaySel);

    void StartPlay(int iChannel);
    void PlayOrPause(PlayPageSwitch ePageSwitch = PLAY_PAGE_SWITCH_NONE);

public:
    CStatic         m_staticPlay[2];
    CEdit           m_editPlayPath[2];
    CComboBox       m_cmbSpeedPlayDelta[2];

    CFont           m_fontDefault;
    POINT           m_pointOld;

    CString         m_strPlayPath[2];

    ProcessInfo*    m_pInfoPlay[2];

    int             m_nCurPlaySel;
    volatile BOOL   m_bFullScreen;
    CRect           m_rectScreenPlayBeforeFullScreen[2];
    CRect           m_rectClientPlayBeforeFullScreen[2];
};
