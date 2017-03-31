
// CaptureDlg.h : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CMyStatic.h"

#define WM_MSG_CURRENT_DATETIME    (WM_USER + 1000)
#define WM_MSG_RECORD_DATETIME     (WM_USER + 1001)

#define WM_MSG_RECORD_DONE         (WM_USER + 1002)

#define WM_MSG_MODIFY_STYLE        (WM_USER + 1003)

// CCaptureDlg 对话框

class CCaptureDlg : public CDialog
{
    DECLARE_DYNAMIC(CCaptureDlg)

public:
    CCaptureDlg(CWnd* pParent = NULL);   // 标准构造函数
    virtual ~CCaptureDlg();

// 对话框数据
    enum { IDD = IDD_DIALOG_CAPTURE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()

    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedBtnRecordStart();
    afx_msg void OnBnClickedBtnRecordStop();
    afx_msg void OnDeltaposSpinMinite(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposSpinSecond(NMHDR *pNMHDR, LRESULT *pResult);

    afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
    afx_msg void OnStnClickedStaticCapture1();
    afx_msg void OnStnClickedStaticCapture2();
    afx_msg void OnStnClickedStaticCapture3();
    afx_msg void OnStnClickedStaticCapture4();
    afx_msg void OnBnClickedBtnSnapshot();
    afx_msg void OnBnClickedBtnFullScreen();
    afx_msg void OnEnChangeEditMinite();
    afx_msg void OnEnChangeEditSecond();

    afx_msg LRESULT OnMsgCurrentDateTime(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMsgRecordDateTime(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMsgModifyStyle(WPARAM wParam, LPARAM lParam);

public:
    virtual BOOL OnInitDialog();
    void GetPlayRect();
    CRect GetScreenRectBeforeFullScreen(int nCurPlaySel);
    CRect GetClientRectBeforeFullScreen(int nCurPlaySel);

    void SetCurrentDateTime(LPCTSTR lpszDateTime);
    void SetRecordTime(LPCTSTR lpszTime);
    void SetEnableFullScreen(BOOL bEnable);
    void SetEnableRecordStart(BOOL bEnable);
    void SetEnableRecordStop(BOOL bEnable);
    void SetEnableRecordTime(BOOL bEnable);

    void GetRecordInterval(int &nMinite, int &nSecond);
    int GetCurPlaySel();

    CStatic* GetStaticCaptureWindow(int iChannel);

    void ModifyStaticStyle(int nSel);

private:
    CSpinButtonCtrl m_spinMinite;
    CSpinButtonCtrl m_spinSecond;

    CEdit           m_editMinite;
    CEdit           m_editSecond;

    CStatic         m_staicCurTime;
    CStatic         m_staicRecordTime;

    CButton         m_btnFullSceen;
    CButton         m_btnSnapshot;
    CButton         m_btnRecordStart;
    CButton         m_btnRecordStop;

    friend class CMyStatic;
    CMyStatic       m_staticCapture[CAPTURE_NUM];
    CMyStatic*      m_staticCurCapture;

    CFont           m_fontDefault;
    POINT           m_pointOld;

    volatile BOOL   m_bFullScreen;
    CRect           m_rectScreenPlayBeforeFullScreen[CAPTURE_NUM];
    CRect           m_rectClientPlayBeforeFullScreen[CAPTURE_NUM];

    int             m_nCurPlaySel;

    int             m_nMinite;
    int             m_nSecond;
};
