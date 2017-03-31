
// PlayDlg.h : header file
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

#define WM_MSG_FILE_TO_LIST    (WM_USER + 2000)
#define WM_MSG_AUTO_PLAY       (WM_USER + 2001)

// CPlayDlg 对话框

class CPlayDlg : public CDialog
{
    DECLARE_DYNAMIC(CPlayDlg)

public:
    CPlayDlg(CWnd* pParent = NULL);   // 标准构造函数
    virtual ~CPlayDlg();

// 对话框数据
    enum { IDD = IDD_DIALOG_PLAY };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()

    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnNMDblclkListFiles(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedCheckAutoPlay();
    afx_msg void OnBnClickedBtnPlayPause();
    afx_msg void OnBnClickedBtnStop();
    afx_msg void OnBnClickedBtnForword();
    afx_msg void OnBnClickedBtnBack();
    afx_msg void OnBnClickedBtnPlayListRefresh();
    afx_msg void OnCbnSelchangeComboSpeedPlayDelta();

    afx_msg LRESULT OnMsgInsertFile2List(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMsgRecordAutoPlay(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedCheckKeepFullscreen();

    afx_msg void OnBnClickedCheckPlayLoc();
    afx_msg void OnEnChangeEditPlayStartHour();
    afx_msg void OnEnChangeEditPlayStartMin();
    afx_msg void OnEnChangeEditPlayStartSecs();
    afx_msg void OnEnChangeEditPlayDuration();

    afx_msg void OnDeltaposSpinPlayStartHour(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposSpinPlayStartMin(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeltaposSpinPlayStartSecs(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMSetfocusListFiles(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMKillfocusListFiles(NMHDR *pNMHDR, LRESULT *pResult);

public:
    virtual BOOL OnInitDialog();

    void ResizePlayWindow(LPRECT lprcPosition = NULL);
    int GetCurPlaySel();
    void GetPlayRect();
    CRect GetScreenRectBeforeFullScreen(int nCurPlaySel);
    CRect GetClientRectBeforeFullScreen(int nCurPlaySel);
    void PlayOrPause(PlayPageSwitch ePageSwitch = PLAY_PAGE_SWITCH_NONE);
    void StartPlayChannels(BOOL bSetPlayLoc);
    BOOL StartPlay(int nItem);
    void StopPlay();
    void StartAutoPlay();

    void InsertItemToList(int iChannel, LPCTSTR lpszFileName);
    void UpdateListFiles(BOOL bInit = TRUE);

    CStatic* GetStaticPlayByChannel(int iChannel);
    CStatic* GetStaticPlayFileNameByChannel(int iChannel);

    ProcessInfo* GetPlayInfoByChannel(int iChannel);

private:
    BOOL UpdatePlayLocStartTimeAndDuration();
    //BOOL UpdatePlayLocDuration();

public:
    CListCtrl       m_listCtrlFiles;
    CStatic         m_staticPlay[CAPTURE_NUM];
    CStatic         m_staticPlayFileName[CAPTURE_NUM];

    CComboBox       m_cmbSpeedPlayDelta;

    CEdit           m_editPlayStartTimeHour;
    CEdit           m_editPlayStartTimeMin;
    CEdit           m_editPlayStartTimeSecs;
    CEdit           m_editPlayDuration;

    CFont           m_fontDefault;
    POINT           m_pointOld;

    ProcessInfo*    m_arrPlays[CAPTURE_NUM];

    BOOL            m_bRefreshingListCtrlFiles;
    CRITICAL_SECTION m_hListCtrlFilesAccessCriticalSection;

    PlayLocation    m_playLoc;
    BOOL            m_bPlayLoc; // 定位并循环播放
    CMyStatic*      m_staticVideoWnd[CAPTURE_NUM];

    BOOL            m_bAutoPlay;
    BOOL            m_bKeepFullscreen;
    BOOL            m_bAutoPlayEnd;
    PlayType        m_ePlayType;

    int             m_nCurPlaySel;

    int             m_iListSelectedItem;

    volatile BOOL   m_bFullScreen;
    CRect           m_rectScreenPlayBeforeFullScreen[CAPTURE_NUM];
    CRect           m_rectClientPlayBeforeFullScreen[CAPTURE_NUM];
};
