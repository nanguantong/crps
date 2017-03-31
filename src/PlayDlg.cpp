
// PlayDlg.cpp : implementation of the CPlayDlg class
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "CaptureRecord.h"
#include "ChildView.h"
#include "PlayDlg.h"

// CPlayDlg 对话框

IMPLEMENT_DYNAMIC(CPlayDlg, CDialog)

CPlayDlg::CPlayDlg(CWnd* pParent /*=NULL*/)
: CDialog(CPlayDlg::IDD, pParent)
{
    m_pointOld.x = m_pointOld.y = 0;
    m_bFullScreen = FALSE;
    m_nCurPlaySel = 0;
    m_iListSelectedItem = 0;

    m_bRefreshingListCtrlFiles = FALSE;
    m_bPlayLoc = FALSE;
    m_bAutoPlay = FALSE;
    m_bKeepFullscreen = TRUE;
    m_bAutoPlayEnd = FALSE;
    m_ePlayType = PlayType_None;

    memset(&m_playLoc, 0, sizeof(m_playLoc));
    for (int i = 0; i < CAPTURE_NUM; i++) {
        m_arrPlays[i] = NULL;
        m_staticVideoWnd[i] = NULL;
    }

    InitializeCriticalSection(&m_hListCtrlFilesAccessCriticalSection);
}

CPlayDlg::~CPlayDlg()
{
    DeleteCriticalSection(&m_hListCtrlFilesAccessCriticalSection);
}

void CPlayDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_FILES, m_listCtrlFiles);
    DDX_Control(pDX, IDC_STATIC_PLAY1, m_staticPlay[0]);
    DDX_Control(pDX, IDC_STATIC_PLAY2, m_staticPlay[1]);
    DDX_Control(pDX, IDC_STATIC_PLAY3, m_staticPlay[2]);
    DDX_Control(pDX, IDC_STATIC_PLAY4, m_staticPlay[3]);
    DDX_Control(pDX, IDC_STATIC_PLAY1_FILE_NAME, m_staticPlayFileName[0]);
    DDX_Control(pDX, IDC_STATIC_PLAY2_FILE_NAME, m_staticPlayFileName[1]);
    DDX_Control(pDX, IDC_STATIC_PLAY3_FILE_NAME, m_staticPlayFileName[2]);
    DDX_Control(pDX, IDC_STATIC_PLAY4_FILE_NAME, m_staticPlayFileName[3]);
    DDX_Control(pDX, IDC_COMBO_SPEED_PLAY_DELTA, m_cmbSpeedPlayDelta);
    DDX_Check(pDX, IDC_CHECK_AUTO_PLAY, m_bAutoPlay);
    DDX_Check(pDX, IDC_CHECK_KEEP_FULLSCREEN, m_bKeepFullscreen);
    DDX_Control(pDX, IDC_EDIT_PLAY_START_HOUR, m_editPlayStartTimeHour);
    DDX_Control(pDX, IDC_EDIT_PLAY_START_MIN, m_editPlayStartTimeMin);
    DDX_Control(pDX, IDC_EDIT_PLAY_START_SECS, m_editPlayStartTimeSecs);
    DDX_Control(pDX, IDC_EDIT_PLAY_DURATION, m_editPlayDuration);
    DDX_Check(pDX, IDC_CHECK_PLAY_LOC, m_bPlayLoc);
}


BEGIN_MESSAGE_MAP(CPlayDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILES, &CPlayDlg::OnNMDblclkListFiles)
    ON_BN_CLICKED(IDC_BTN_PLAY_PAUSE, &CPlayDlg::OnBnClickedBtnPlayPause)
    ON_BN_CLICKED(IDC_BTN_STOP, &CPlayDlg::OnBnClickedBtnStop)
    ON_BN_CLICKED(IDC_BTN_FORWORD, &CPlayDlg::OnBnClickedBtnForword)
    ON_BN_CLICKED(IDC_BTN_BACK, &CPlayDlg::OnBnClickedBtnBack)
    ON_BN_CLICKED(IDC_CHECK_AUTO_PLAY, &CPlayDlg::OnBnClickedCheckAutoPlay)
    ON_BN_CLICKED(IDC_CHECK_KEEP_FULLSCREEN, &CPlayDlg::OnBnClickedCheckKeepFullscreen)
    ON_BN_CLICKED(IDC_BTN_PLAY_LIST_REFRESH, &CPlayDlg::OnBnClickedBtnPlayListRefresh)
    ON_CBN_SELCHANGE(IDC_COMBO_SPEED_PLAY_DELTA, &CPlayDlg::OnCbnSelchangeComboSpeedPlayDelta)
    ON_MESSAGE(WM_MSG_FILE_TO_LIST, &CPlayDlg::OnMsgInsertFile2List)
    ON_MESSAGE(WM_MSG_AUTO_PLAY, &CPlayDlg::OnMsgRecordAutoPlay)
    ON_BN_CLICKED(IDC_CHECK_PLAY_LOC, &CPlayDlg::OnBnClickedCheckPlayLoc)
    ON_EN_CHANGE(IDC_EDIT_PLAY_DURATION, &CPlayDlg::OnEnChangeEditPlayDuration)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PLAY_START_HOUR, &CPlayDlg::OnDeltaposSpinPlayStartHour)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PLAY_START_MIN, &CPlayDlg::OnDeltaposSpinPlayStartMin)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PLAY_START_SECS, &CPlayDlg::OnDeltaposSpinPlayStartSecs)
    ON_EN_CHANGE(IDC_EDIT_PLAY_START_HOUR, &CPlayDlg::OnEnChangeEditPlayStartHour)
    ON_EN_CHANGE(IDC_EDIT_PLAY_START_MIN, &CPlayDlg::OnEnChangeEditPlayStartMin)
    ON_EN_CHANGE(IDC_EDIT_PLAY_START_SECS, &CPlayDlg::OnEnChangeEditPlayStartSecs)
    ON_NOTIFY(NM_SETFOCUS, IDC_LIST_FILES, &CPlayDlg::OnNMSetfocusListFiles)
    ON_NOTIFY(NM_KILLFOCUS, IDC_LIST_FILES, &CPlayDlg::OnNMKillfocusListFiles)
END_MESSAGE_MAP()

BOOL CPlayDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_fontDefault.CreatePointFont(DEFAULT_FONT_SIZE, _T("宋体"));

    CRect rect;
    GetClientRect(&rect);
    m_pointOld.x = rect.Width();
    m_pointOld.y = rect.Height();

    int nSizeSel = 0;
    int nSizeCount = sizeof(g_arrSpeedPlayDelta) / sizeof(g_arrSpeedPlayDelta[0]);
    for (int i = 0; i < nSizeCount; i++) {
        int nItem = m_cmbSpeedPlayDelta.InsertString(i, g_arrSpeedPlayDelta[i].str);
        m_cmbSpeedPlayDelta.SetItemDataPtr(nItem, &g_arrSpeedPlayDelta[i].num);

        if (_T("正常速度") == g_arrSpeedPlayDelta[i].str) {
            nSizeSel = nItem;
        }
    }
    m_cmbSpeedPlayDelta.SetCurSel(nSizeSel);

    m_editPlayStartTimeHour.SetWindowText(_T("00"));
    m_editPlayStartTimeMin.SetWindowText(_T("00"));
    m_editPlayStartTimeSecs.SetWindowText(_T("00"));
    m_editPlayDuration.SetWindowText(_T("0"));

    m_listCtrlFiles.EnableToolTips();
    m_listCtrlFiles.InsertColumn(0, _T("名称"), 0, 500);
    m_listCtrlFiles.InsertColumn(1, _T("大小"), 0, 0);
    m_listCtrlFiles.InsertColumn(2, _T("创建时间"), 0, 0);
    m_listCtrlFiles.InsertColumn(3, _T("路径"), 0, 0);
    m_listCtrlFiles.InsertColumn(4, _T("前缀"), 0, 0);
    m_listCtrlFiles.InsertColumn(5, _T("通道"), 0, 0);

    LONG lStyle;
    lStyle = GetWindowLong(m_listCtrlFiles.m_hWnd, GWL_STYLE);//获取当前窗口style
    lStyle &= ~LVS_TYPEMASK; //清除显示方式位
    lStyle |= LVS_REPORT; //设置style
    SetWindowLong(m_listCtrlFiles.m_hWnd, GWL_STYLE, lStyle);

    DWORD dwStyle = m_listCtrlFiles.GetExtendedStyle();
    dwStyle |= LVS_SHOWSELALWAYS;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    //dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
    //dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
    m_listCtrlFiles.SetExtendedStyle(dwStyle);

    UpdateListFiles();

    SetTimer(TIMER_ID_PLAY_END, 100, NULL);

    return FALSE;
}

void CPlayDlg::OnDestroy()
{
    m_bAutoPlayEnd = TRUE;

    StopPlay();

    KillTimer(TIMER_ID_PLAY_END);

    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo) {
            delete proInfo;
        }

        if (m_staticVideoWnd[i] != NULL) {
            delete m_staticVideoWnd[i];
            m_staticVideoWnd[i] = NULL;
        }

    }

    CDialog::OnDestroy();
}

void CPlayDlg::OnPaint()
{
    if (IsIconic())
        return;
    else
    {
        CPaintDC dc(this);
        CRect rect;
        GetClientRect(rect);
        dc.FillSolidRect(rect, RGB(255, 255, 255));

        CDialog::OnPaint();
    }
}

void CPlayDlg::OnSize(UINT nType, int cx, int cy)
{
    if (m_pointOld.x == 0 || m_pointOld.y == 0 || cx == 0 || cy == 0)
        return;

    ResizeWindow(this->GetSafeHwnd(), m_pointOld, &m_fontDefault);

    GetPlayRect();

    ResizePlayWindow();
}

HBRUSH CPlayDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetBkColor(RGB(255, 255, 255));

        return (HBRUSH)::GetStockObject(WHITE_BRUSH);
        //return CreateSolidBrush(RGB(255, 255, 255));
    }

    return hbr;
}

void CPlayDlg::OnNMDblclkListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    int nItem;

    UpdateData(TRUE);

    ////EnterCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

    POSITION pos = m_listCtrlFiles.GetFirstSelectedItemPosition();
    if (!pos) {
        ////LeaveCriticalSection(&m_hListCtrlFilesAccessCriticalSection);
        return;
    }

    nItem = m_listCtrlFiles.GetNextSelectedItem(pos);

    ////LeaveCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

    m_ePlayType = PlayType_Manual;
    StartPlay(nItem);

    *pResult = 0;
}

void CPlayDlg::OnBnClickedCheckAutoPlay()
{
    UpdateData(TRUE);
}

void CPlayDlg::OnBnClickedCheckKeepFullscreen()
{
    UpdateData(TRUE);
}

void CPlayDlg::OnBnClickedBtnPlayPause()
{
    PlayOrPause();
}

void CPlayDlg::OnBnClickedBtnStop()
{
    StopPlay();
}

void CPlayDlg::OnBnClickedBtnForword()
{
    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo) {
#if PLAY_MODE_QCAP
            if (proInfo->avChannelPlay)
                proInfo->avChannelPlay->SeekFile(TRUE);
#else
            DWORD dwWrite;
            WriteFile(proInfo->hPipeWrite, "r", sizeof("r"), &dwWrite, NULL);

            //::PostMessage(proInfo->cwnd->GetSafeHwnd(), WM_KEYDOWN, VK_RIGHT, 0);
            //Sleep(10);
            //::PostMessage(proInfo->cwnd->GetSafeHwnd(), WM_KEYUP, VK_RIGHT, 0);
#endif
        }
    }
}

void CPlayDlg::OnBnClickedBtnBack()
{
    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo) {
#if PLAY_MODE_QCAP
            if (proInfo->avChannelPlay)
                proInfo->avChannelPlay->SeekFile(FALSE);
#else
            DWORD dwWrite;
            WriteFile(proInfo->hPipeWrite, "l", sizeof("l"), &dwWrite, NULL);

            //::PostMessage(proInfo->cwnd->GetSafeHwnd(), WM_KEYDOWN, VK_LEFT, 0);
            //Sleep(10);
            //::PostMessage(proInfo->cwnd->GetSafeHwnd(), WM_KEYUP, VK_LEFT, 0);
#endif
        }
    }
}

void CPlayDlg::OnBnClickedBtnPlayListRefresh()
{
    UpdateListFiles();
}

void CPlayDlg::OnCbnSelchangeComboSpeedPlayDelta()
{
    int index = m_cmbSpeedPlayDelta.GetCurSel();

    double* dSpeed = (double *)m_cmbSpeedPlayDelta.GetItemDataPtr(index);
    if (dSpeed == NULL) {
        return;
    }

    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo) {
#if PLAY_MODE_QCAP
            if (proInfo->avChannelPlay)
                proInfo->avChannelPlay->SetFilePlaySpeed(*dSpeed);
#endif
        }
    }
}

void CPlayDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIMER_ID_PLAY_END) {
        int nCount = 0;
        for (int i = 0; i < CAPTURE_NUM; i++) {
            ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
            if (proInfo) {
#if PLAY_MODE_QCAP
                if (proInfo->avChannelPlay) {
                    nCount++;
                }
                if (proInfo->avChannelPlay && !proInfo->avChannelPlay->IsEnd()) {
                    double dSampleTime;
                    if (proInfo->avChannelPlay->GetFilePosition(&dSampleTime)) {
                        proInfo->avChannelPlay->SetFileOSD(dSampleTime);
                    }
                }
                else if (m_bPlayLoc && proInfo->avChannelPlay && proInfo->avChannelPlay->IsEnd() && 
                    (proInfo->avChannelPlay->m_dStartTime > 0 || proInfo->avChannelPlay->m_dEndTime > 0)) {
                    // loop play
                    proInfo->avChannelPlay->PlayFile();
                    //proInfo->avChannelPlay->PauseFile();
                    proInfo->avChannelPlay->SeekFile(proInfo->avChannelPlay->m_dStartTime);
                }
                else if (!m_bPlayLoc || (proInfo->avChannelPlay && 
                    proInfo->avChannelPlay->m_dStartTime == 0 && proInfo->avChannelPlay->m_dEndTime == 0)) {
                    CStatic* staticPlayFileName = GetStaticPlayFileNameByChannel(proInfo->iChannel);
                    if (staticPlayFileName) {
                        staticPlayFileName->SetWindowText(_T(""));
                    }

                    delete proInfo->avChannelPlay;
                    proInfo->avChannelPlay = NULL;
#else
                DWORD dwExitflag = 0;
                BOOL bExit = GetExitCodeThread(proInfo->pi.hThread, &dwExitflag);
                if (bExit && STILL_ACTIVE != dwExitflag) {
                    PostThreadMessage(proInfo->pi.dwThreadId, WM_QUIT, 0, 0);
                    TerminateProcess(proInfo->pi.hProcess, 1);
                    CloseHandle(proInfo->hPipeRead);
                    CloseHandle(proInfo->hPipeWrite);
                    CloseHandle(proInfo->pi.hThread);
                    CloseHandle(proInfo->pi.hProcess);
#endif

#if SDL_WIND
                    if (!m_bFullScreen) {
                        proInfo->cwnd->ShowWindow(FALSE);
                    }
#endif
                }
            }
        }

        // 播放完毕
        if (nCount > 0) {
            m_ePlayType = PlayType_None;
            if (m_bFullScreen && !m_bKeepFullscreen) {
                m_bFullScreen = FALSE;
                ShowWindow(TRUE);
            }
        }
    }

    CWnd::OnTimer(nIDEvent);
}

LRESULT CPlayDlg::OnMsgInsertFile2List(WPARAM wParam, LPARAM lParam) {
    int iChannel = (int)wParam;
    CString* pStrFileName = (CString *)lParam;

    if (iChannel < 0 || iChannel > CAPTURE_NUM - 1 || pStrFileName == NULL)
        return 0;

    InsertItemToList(iChannel, *pStrFileName);

    delete pStrFileName;

    return 0;
}

LRESULT CPlayDlg::OnMsgRecordAutoPlay(WPARAM wParam, LPARAM lParam) {
    BOOL bAutoPlay = (BOOL)lParam;

    StartAutoPlay();

    return 0;
}

void CPlayDlg::ResizePlayWindow(LPRECT lprcPosition) {
    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo * proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo) {
            CStatic *staticPlay = GetStaticPlayByChannel(proInfo->iChannel);
            if (staticPlay == NULL)
                continue;

            CRect rect;
            if (lprcPosition == NULL)
                staticPlay->GetClientRect(&rect);
            else
                rect = *lprcPosition;

            ::MoveWindow(proInfo->cwnd->GetSafeHwnd(), rect.left, rect.top, rect.Width(), rect.Height(), FALSE);

#if PLAY_MODE_FFPLAY
            DWORD dwWrite;
            char szWh[16];
            sprintf(szWh, "s-%dx%d", rect.Width(), rect.Height());
            WriteFile(proInfo->hPipeWrite, szWh, sizeof(szWh), &dwWrite, NULL);
#endif
        }
    }
}

int CPlayDlg::GetCurPlaySel() {
    return m_nCurPlaySel;
}

void CPlayDlg::GetPlayRect() {
    for (int i = 0; i < CAPTURE_NUM; i++) {

        m_staticPlay[i].GetWindowRect(&m_rectScreenPlayBeforeFullScreen[i]);
        ScreenToClient(&m_rectScreenPlayBeforeFullScreen[i]);
        m_staticPlay[i].GetClientRect(&m_rectClientPlayBeforeFullScreen[i]);
    }
}

CRect CPlayDlg::GetScreenRectBeforeFullScreen(int nCurPlaySel) {
    if (nCurPlaySel >= 1 && nCurPlaySel <= CAPTURE_NUM) {
        return m_rectScreenPlayBeforeFullScreen[nCurPlaySel - 1];
    }
    return m_rectScreenPlayBeforeFullScreen[0];
}

CRect CPlayDlg::GetClientRectBeforeFullScreen(int nCurPlaySel) {
    if (nCurPlaySel >= 1 && nCurPlaySel <= CAPTURE_NUM) {
        return m_rectClientPlayBeforeFullScreen[nCurPlaySel - 1];
    }
    return m_rectClientPlayBeforeFullScreen[0];
}

void CPlayDlg::PlayOrPause(PlayPageSwitch ePageSwitch) {
    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo * proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo) {
#if PLAY_MODE_QCAP
            if (proInfo->avChannelPlay) {
                if (ePageSwitch == PLAY_PAGE_SWITCH_NONE) {
                    proInfo->avChannelPlay->PlayOrPauseFile();
                }
                else if (ePageSwitch == PLAY_PAGE_SWITCH_FROM_PLAY_TO_OTHER) {
                    proInfo->avChannelPlay->PauseFile();
                }
                else if (ePageSwitch == PLAY_PAGE_SWITCH_FROM_OTHER_TO_PLAY) {
                    proInfo->avChannelPlay->PlayFile();
                }
            }
#else
            DWORD dwWrite;
            WriteFile(proInfo->hPipeWrite, "p", sizeof("p"), &dwWrite, NULL);

            //::PostMessage(proInfo->cwnd->GetSafeHwnd(), WM_KEYDOWN, VK_SPACE, 0);
            //Sleep(10);
            //::PostMessage(proInfo->cwnd->GetSafeHwnd(), WM_KEYUP, VK_SPACE, 0);
#endif
        }
    }
}

void CPlayDlg::StartPlayChannels(BOOL bSetPlayLoc) {
    CStringArray arrStr;
    CString str;
    int i;

    for (i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo && proInfo->avChannelPlay) {
            // first load
            if (m_playLoc.iHour == 0 && m_playLoc.iMin == 0 && m_playLoc.iSec == 0) {
                str = proInfo->strFileNamePrefix; // e.g. 2016.06.03-10.16.08

                if (StringSplitWith(str, arrStr, _T('-')) && arrStr.GetSize() == 2) {
                    CString srcStartTime(arrStr[1]);

                    arrStr.RemoveAll();

                    if (StringSplitWith(srcStartTime, arrStr, _T('.')) && arrStr.GetSize() == 3) {
                        m_editPlayStartTimeHour.SetWindowText(arrStr[0]);
                        m_editPlayStartTimeMin.SetWindowText(arrStr[1]);
                        m_editPlayStartTimeSecs.SetWindowText(arrStr[2]);
                    }
                }
            }

            if (m_bPlayLoc && !(m_playLoc.iHour == 0 && m_playLoc.iMin == 0 && m_playLoc.iSec == 0)) {
                // e.g. filename: 2016.06.03-10.16.08_4
                int iDiffSecs = 0, bOk = FALSE;

                if (arrStr.GetSize() == 0) {
                    str = proInfo->strFileNamePrefix; // e.g. 2016.06.03-10.16.08
                    if (StringSplitWith(str, arrStr, _T('-')) && arrStr.GetSize() == 2) {
                        CString srcStartTime(arrStr[1]);

                        arrStr.RemoveAll();

                        if (StringSplitWith(srcStartTime, arrStr, _T('.')) && arrStr.GetSize() == 3) {
                            bOk = TRUE;
                        }
                    }
                }
                else {
                    bOk = TRUE;
                }

                if (bOk) {
                    int iSrcTotalSecs = _ttoi(arrStr[0]) * 60 * 60 + _ttoi(arrStr[1]) * 60 + _ttoi(arrStr[2]);
                    int iDestTotalSecs = m_playLoc.iHour * 60 * 60 + m_playLoc.iMin * 60 + m_playLoc.iSec;
                    iDiffSecs = iDestTotalSecs - iSrcTotalSecs;
                }

                proInfo->avChannelPlay->SetFileStartTime(iDiffSecs);
                proInfo->avChannelPlay->SetFileEndTime((iDiffSecs > 0 ? iDiffSecs : 0) + m_playLoc.iDuration);

                // 没有重新设置定位参数时不要seek
                if (!bSetPlayLoc && proInfo->avChannelPlay->m_dStartTime > 0 || proInfo->avChannelPlay->m_dEndTime > 0) {
                    proInfo->avChannelPlay->SeekFile((double)iDiffSecs);
                }
            }

            // 没有重新设置定位参数时不要play
            if (!bSetPlayLoc)
                proInfo->avChannelPlay->PlayFile();
        }
    }
}

BOOL CPlayDlg::StartPlay(int nItem) {
    CString& strFileName = m_listCtrlFiles.GetItemText(nItem, 0);
    CArray<PlayInfo> arrPlayInfo;

    int iCount, i;
    iCount = m_listCtrlFiles.GetItemCount();
    int nStart = max(nItem - 4, 0), nEnd = min(nItem + 4, iCount);
    for (i = nStart; i < nEnd; i++) {
        CString& strNamePrefix = m_listCtrlFiles.GetItemText(i, 4);
        if (strFileName.Find(strNamePrefix) >= 0) {
            PlayInfo pi;
            pi.strChannel = m_listCtrlFiles.GetItemText(i, 5);
            pi.strFileNamePrefix = m_listCtrlFiles.GetItemText(i, 4);
            pi.strPath = m_listCtrlFiles.GetItemText(i, 3);
            arrPlayInfo.Add(pi);
        }
    }

    iCount = arrPlayInfo.GetCount();
    if (iCount <= 0)
        return FALSE;

    StopPlay();

    CRect rect;
    UINT nID = PLAY_BASE_HWND_ID;

    // open all files with the same name(without "_channelID") to play
    for (i = 0; i < iCount; i++) {
        PlayInfo& playInfo = arrPlayInfo.GetAt(i);
        int iChannel = _tstoi(playInfo.strChannel);

        CStatic* staticPlay = GetStaticPlayByChannel(iChannel);
        CStatic* staticPlayFileName = GetStaticPlayFileNameByChannel(iChannel);
        if (staticPlay == NULL || staticPlayFileName == NULL)
            continue;

#if SDL_WIND
        CMyStatic* staticVideoWnd = NULL;
        int iCurChl = -1;
        staticPlay->GetClientRect(&rect);

        if (iChannel > 0 && iChannel <= CAPTURE_NUM) {
            iCurChl = iChannel - 1;
        }
        if (iCurChl != -1) {
            if (m_staticVideoWnd[iCurChl] == NULL) {
                CMyStatic* sVideoWnd = new CMyStatic;
                sVideoWnd->SetDialogTypeAndIndex(DialogType_Play, iChannel);
                sVideoWnd->Create(NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | /*WS_EX_CONTROLPARENT | */SS_NOTIFY, rect, staticPlay, nID++);

                m_staticVideoWnd[iCurChl] = sVideoWnd;
            }
            staticVideoWnd = m_staticVideoWnd[iCurChl];
            staticVideoWnd->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
            staticVideoWnd->SetActiveWindow();
        }
#else
        staticPlay->GetWindowRect(&rect);
        ScreenToClient(&rect);
#endif

#if PLAY_MODE_QCAP
        CAVChannelPlay *avChannelPlay = new CAVChannelPlay;
        if (staticVideoWnd != NULL && avChannelPlay->OpenFile(playInfo.strPath, staticVideoWnd->GetSafeHwnd())) {
            ProcessInfo* proInfo;
            if (m_arrPlays[iCurChl] == NULL) {
                proInfo = new ProcessInfo;
                proInfo->iChannel = iChannel;
                proInfo->cwnd = staticVideoWnd;
                m_arrPlays[iCurChl] = proInfo;
            }
            else {
                proInfo = (ProcessInfo*)m_arrPlays[iCurChl];
            }
            proInfo->strFileNamePrefix = playInfo.strFileNamePrefix;
            proInfo->avChannelPlay = avChannelPlay;

            staticPlayFileName->SetWindowText(GetFileName(playInfo.strPath));
        }
        else
        {
            delete avChannelPlay;
            staticPlayFileName->SetWindowText(_T(""));
        }

#else
        TCHAR szTmp[64];
        CString strCmd(PLAY_PROCESSOR);
        strCmd += _T(" -i \"");
        strCmd += playInfo.strPath;
        strCmd += _T("\" -autoexit -nostats -x ");
        _stprintf(szTmp, _T("%d"), rect.Width() >> 1 << 1);
        strCmd += szTmp;
        strCmd += _T(" -y ");
        _stprintf(szTmp, _T("%d"), rect.Height() >> 1 << 1);
        strCmd += szTmp;
#if SDL_WIND
        strCmd += _T(" -sdlwnd \"");
        _stprintf(szTmp, _T("SDL_WINDOWID=0x%lx\""), staticVideoWnd->GetSafeHwnd());
        strCmd += szTmp;
#endif

        HANDLE hPipeWrite, hPipeRead;
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        //SetHandleInformation(hPipeW, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT); //使得子进程可以继承这个句柄
        BOOL ok = CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0);
        if (!ok)
            continue;

        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        //子进程的标准输入句柄为父进程管道的读数据句柄
        si.hStdInput = hPipeRead;
        //子进程的标准输出句柄为父进程管道的写数据句柄
        si.hStdOutput = hPipeWrite;
        //子进程的标准错误处理句柄和父进程的标准错误处理句柄一致
        //si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

        ok = CreateProcess(PLAY_PROCESSOR, strCmd.GetBuffer(), NULL, NULL, TRUE/*FALSE*/, HIGH_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        if (ok) {
            Sleep(100);

            staticPlayFileName->SetWindowText(GetFileName(playInfo.strPath));

#if SDL_WIND
            ProcessInfo *proInfo;
            if (m_arrPlays[iCurChl] == NULL) {
                proInfo = new ProcessInfo;
                proInfo->iChannel = iChannel;
                proInfo->cwnd = staticVideoWnd;
                m_arrPlays[iCurChl] = proInfo;
            }
            else {
                proInfo = (ProcessInfo*)m_arrPlays[iCurChl];
            }
            proInfo->strFileNamePrefix = playInfo.strFileNamePrefix;
            proInfo->pi = pi;
            proInfo->hPipeRead = hPipeRead;
            proInfo->hPipeWrite = hPipeWrite;
#else
            HWND hwnd = GetHwndByProcessId(pi.dwProcessId);
            if (hwnd) {
                LONG style = ::GetWindowLong(hwnd, GWL_STYLE);
                style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUPWINDOW | WS_DLGFRAME);
                style |= (WS_CHILD | WS_CLIPSIBLINGS);
                ::SetWindowLong(hwnd, GWL_STYLE, style);

                /*style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
                style &= ~WS_EX_OVERLAPPEDWINDOW;
                ::SetWindowLong(hwnd, GWL_EXSTYLE, style);*/

                ::SetParent(hwnd, this->GetSafeHwnd());
                ::SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);

                ProcessInfo *proInfo;
                if (m_arrPlays[iCurChl] == NULL) {
                    proInfo->iChannel = iChannel;
                    proInfo = new ProcessInfo;
                    m_arrPlays[iCurChl] = proInfo;
                }
                else {
                    proInfo = (ProcessInfo*)m_arrPlays[iCurChl];
                }
                proInfo->pi = pi;
                proInfo->cwnd = CWnd::FromHandle(hwnd);
            }
#endif
        }
#endif /* PLAY_MODE_QCAP */
    }

#if PLAY_MODE_QCAP
    OnCbnSelchangeComboSpeedPlayDelta();

    StartPlayChannels(FALSE);
#endif

    return TRUE;
}

void CPlayDlg::StopPlay() {
    int nCount = 0;
    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo) {
#if PLAY_MODE_FFPLAY
            PostThreadMessage(proInfo->pi.dwThreadId, WM_QUIT, 0, 0);
            TerminateProcess(proInfo->pi.hProcess, 1);
            CloseHandle(proInfo->hPipeRead);
            CloseHandle(proInfo->hPipeWrite);
            CloseHandle(proInfo->pi.hThread);
            CloseHandle(proInfo->pi.hProcess);
#endif
            CStatic* staticPlayFileName = GetStaticPlayFileNameByChannel(proInfo->iChannel);
            if (staticPlayFileName)
                staticPlayFileName->SetWindowText(_T(""));

#if PLAY_MODE_QCAP
            if (proInfo->avChannelPlay) {
                nCount++;
                delete proInfo->avChannelPlay;
                proInfo->avChannelPlay = NULL;
            }
#endif
#if SDL_WIND
            if (!m_bFullScreen) {
                proInfo->cwnd->ShowWindow(FALSE);
            }
#endif
        }
    }

    // 停止时重置定位时间
    m_playLoc.iHour = m_playLoc.iMin = m_playLoc.iSec = m_playLoc.iDuration = 0;

    if (nCount > 0) {
        m_ePlayType = PlayType_None;
        if (m_bFullScreen && !m_bKeepFullscreen) {
            m_bFullScreen = FALSE;
            ShowWindow(TRUE);
        }
    }
}

void CPlayDlg::StartAutoPlay() {
    if (!m_bAutoPlay || m_ePlayType != PlayType_None || m_bPlayLoc)
        return;

    m_ePlayType = PlayType_Auto;
    StartPlay(0);
}

void CPlayDlg::InsertItemToList(int iChannel, LPCTSTR lpszFileName) {
    if (!lpszFileName)
        return;

    CFileStatus fileStatus;

    if (CFile::GetStatus(lpszFileName, fileStatus)) {

        EnterCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

        int iIndex = m_listCtrlFiles.InsertItem(0xffff, _T(""));

        CString strFileName(fileStatus.m_szFullName);
        int iPos = strFileName.ReverseFind('\\');
        strFileName = strFileName.Right(strFileName.GetLength() - iPos - 1);

        m_listCtrlFiles.SetItemText(iIndex, 0, strFileName);

        CString str = FormatFileSize(fileStatus.m_size);
        m_listCtrlFiles.SetItemText(iIndex, 1, str);

        str = fileStatus.m_ctime.Format(_T("%c"));
        m_listCtrlFiles.SetItemText(iIndex, 2, str);

        m_listCtrlFiles.SetItemText(iIndex, 3, lpszFileName);

        iPos = strFileName.ReverseFind(_T('_'));
        strFileName = strFileName.Left(iPos);
        m_listCtrlFiles.SetItemText(iIndex, 4, strFileName);

        str.Format(_T("%d"), iChannel + 1);
        m_listCtrlFiles.SetItemText(iIndex, 5, str);

        LeaveCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

        UpdateData(FALSE);
    }
}

void CPlayDlg::UpdateListFiles(BOOL bInit)
{
    CString strFilePath(((CMainFrame *)AfxGetMainWnd())->m_wndView.m_strRecordPath);
    if (strFilePath.GetLength() == 0)
        return;

    CString strFileFilter(strFilePath + _T("\*")RECORD_FILE_SUFFIX);

    int iIndex;
    int iPos;
    CTime tempTime;
    CString str, strFileLen;

    CFileFind finder;
    BOOL bWorking = finder.FindFile(strFileFilter);

    EnterCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

    m_listCtrlFiles.DeleteAllItems();

    while (bWorking) {
        bWorking = finder.FindNextFile();

        if (!finder.IsDirectory() && !finder.IsDots()) {
            //SHFILEINFO info;
            //SHGetFileInfo(strFilePath + finder.GetFileName(), 0, &info, sizeof(&info), SHGFI_DISPLAYNAME | SHGFI_ICON);

            CString& strFileName = finder.GetFileName();

#if USE_RECORD_UNFINISH_SUFFIX
            iPos = strFileName.Find(RECORD_UNFINISH_SUFFIX);
            if (iPos > 0)
                continue;
#else
            CString& strFilePath = finder.GetFilePath();
            HANDLE handle = CreateFile(strFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (INVALID_HANDLE_VALUE == handle) {
                continue; //(_T("文件已被占用,请先关闭"));
            }
            CloseHandle(handle);
#endif

            iPos = strFileName.ReverseFind(_T('_'));
            if (iPos < 0)
                continue;

            str = strFileName.Right(strFileName.GetLength() - iPos - 1);
            int iPosDot = str.Find(_T("."));
            str = str.Left(iPosDot);

            if (str.IsEmpty())
                continue;

            int iChannel = _tstoi(str);
            if (iChannel < 0 || iChannel > CAPTURE_NUM)
                continue;

            iIndex = m_listCtrlFiles.InsertItem(0xffff, _T(""));

            m_listCtrlFiles.SetItemText(iIndex, 5, str);
            str = strFileName.Left(iPos);
            m_listCtrlFiles.SetItemText(iIndex, 4, str);

            m_listCtrlFiles.SetItemText(iIndex, 0, strFileName);

            strFileLen = FormatFileSize(finder.GetLength());
            m_listCtrlFiles.SetItemText(iIndex, 1, strFileLen);
            if (finder.GetCreationTime(tempTime))
                str = tempTime.Format(_T("%c"));
            m_listCtrlFiles.SetItemText(iIndex, 2, str);
            m_listCtrlFiles.SetItemText(iIndex, 3, finder.GetFilePath());
        }
    }
    finder.Close();

    LeaveCriticalSection(&m_hListCtrlFilesAccessCriticalSection);
}

CStatic* CPlayDlg::GetStaticPlayByChannel(int iChannel) {
    if (iChannel >= 1 && iChannel <= CAPTURE_NUM) {
        return &m_staticPlay[iChannel - 1];
    }

    return NULL;
}

CStatic* CPlayDlg::GetStaticPlayFileNameByChannel(int iChannel) {
    if (iChannel >= 1 && iChannel <= CAPTURE_NUM) {
        return &m_staticPlayFileName[iChannel - 1];
    }

    return NULL;
}

ProcessInfo* CPlayDlg::GetPlayInfoByChannel(int iChannel) {
    for (int i = 0; i < CAPTURE_NUM; i++) {
        ProcessInfo* proInfo = (ProcessInfo *)m_arrPlays[i];
        if (proInfo && proInfo->avChannelPlay && iChannel == proInfo->iChannel) {
            return proInfo;
        }
    }

    return NULL;
}

BOOL CPlayDlg::UpdatePlayLocStartTimeAndDuration() {
    CString str;
#if 0
    CStringArray arrStr;

    m_editPlayStartTime.GetWindowText(str);

    if (!StringSplitWith(str, arrStr, _T('.'))) {
        goto FAIL;
    }

    // e.g. 2016.06.03-10.16.08_4

    if (arrStr.GetSize() != 3) {
        AfxMessageBox(_T("开始时间格式错误"));
        goto FAIL;
    }

    int iHour = _ttoi(arrStr[0]);
    if (iHour < 0 || iHour > 23) {
        AfxMessageBox(_T("开始时间(小时0~23)格式错误"));
        goto FAIL;
    }

    int iMin = _ttoi(arrStr[1]);
    if (iMin < 0 || iMin > 59) {
        AfxMessageBox(_T("开始时间(分钟0~59)格式错误"));
        goto FAIL;
    }

    int iSecs = _ttoi(arrStr[2]);
    if (iSecs < 0 || iSecs > 59) {
        AfxMessageBox(_T("开始时间(秒数0~59)格式错误"));
        goto FAIL;
    }

    m_playLoc.iHour = iHour;
    m_playLoc.iMin = iMin;
    m_playLoc.iSec = iSecs;
#else
    m_editPlayStartTimeHour.GetWindowText(str);
    int iHour = _ttoi(str);
    if (iHour < 0 || iHour > 23) {
        AfxMessageBox(_T("开始时间(小时0~23)格式错误"));
        goto FAIL;
    }

    m_editPlayStartTimeMin.GetWindowText(str);
    int iMin = _ttoi(str);
    if (iMin < 0 || iMin > 59) {
        AfxMessageBox(_T("开始时间(分钟0~59)格式错误"));
        goto FAIL;
    }

    m_editPlayStartTimeSecs.GetWindowText(str);
    int iSecs = _ttoi(str);
    if (iSecs < 0 || iSecs > 59) {
        AfxMessageBox(_T("开始时间(秒数0~59)格式错误"));
        goto FAIL;
    }

    m_editPlayDuration.GetWindowText(str);
    m_playLoc.iDuration = _ttoi(str);

    m_playLoc.iHour = iHour;
    m_playLoc.iMin = iMin;
    m_playLoc.iSec = iSecs;
#endif

    // all of var are zero, then return.
    if (m_playLoc.iHour == 0 && m_playLoc.iMin == 0 && m_playLoc.iSec == 0 && m_playLoc.iDuration == 0) {
        AfxMessageBox(_T("开始时间和播放时长不能同时为0"));
        goto FAIL;
    }

    return TRUE;

FAIL:
    m_bPlayLoc = FALSE;
    UpdateData(FALSE);
    m_editPlayStartTimeHour.SetFocus();
    return FALSE;
}

#if 0
BOOL CPlayDlg::UpdatePlayLocDuration()
{
    CString str;

    m_editPlayDuration.GetWindowText(str);
    m_playLoc.iDuration = _ttoi(str);

    // all of var are zero, then return.
    if (m_playLoc.iHour == 0 && m_playLoc.iMin == 0 && m_playLoc.iSec == 0 && m_playLoc.iDuration == 0) {
        AfxMessageBox(_T("开始时间和播放时长不能同时为0"));
        goto FAIL;
    }

    return TRUE;

FAIL:
    m_bPlayLoc = FALSE;
    UpdateData(FALSE);
    m_editPlayDuration.SetFocus();
    return FALSE;
}
#endif

void CPlayDlg::OnBnClickedCheckPlayLoc()
{
    if (m_bPlayLoc) {
        m_bPlayLoc = !m_bPlayLoc;
        return;
    }

    if (!UpdatePlayLocStartTimeAndDuration()){
        return;
    }

    m_bPlayLoc = !m_bPlayLoc;

    // 重新设置播放中的channel 开始时间和结束时间
    StartPlayChannels(TRUE);
}

void CPlayDlg::OnEnChangeEditPlayStartHour()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    CString ss;
    m_editPlayStartTimeHour.GetWindowText(ss);
    int iHour = _tcstol(ss, NULL, 10);

    if (iHour < 0 || iHour > 23) {
        AfxMessageBox(_T("开始时间(分钟0~23)格式错误"));

        _itow(m_playLoc.iHour, ss.GetBuffer(), 10);
        m_editPlayStartTimeHour.SetWindowText(ss);
        m_bPlayLoc = FALSE;
        UpdateData(FALSE);
        m_editPlayStartTimeHour.SetFocus();
        return;
    }

    m_playLoc.iHour = iHour;

    static BOOL bFirst = TRUE;
    // don't check when app startup
    if (!bFirst) {
        UpdatePlayLocStartTimeAndDuration();
    }
    bFirst = FALSE;
}

void CPlayDlg::OnEnChangeEditPlayStartMin()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    CString ss;
    m_editPlayStartTimeMin.GetWindowText(ss);
    int iMin = _tcstol(ss, NULL, 10);

    if (iMin < 0 || iMin > 59) {
        AfxMessageBox(_T("开始时间(分钟0~59)格式错误"));

        _itow(m_playLoc.iMin, ss.GetBuffer(), 10);
        m_editPlayStartTimeMin.SetWindowText(ss);
        m_bPlayLoc = FALSE;
        UpdateData(FALSE);
        m_editPlayStartTimeMin.SetFocus();
        return;
    }

    m_playLoc.iMin = iMin;

    static BOOL bFirst = TRUE;
    // don't check when app startup
    if (!bFirst) {
        UpdatePlayLocStartTimeAndDuration();
    }
    bFirst = FALSE;
}

void CPlayDlg::OnEnChangeEditPlayStartSecs()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    CString ss;
    m_editPlayStartTimeSecs.GetWindowText(ss);
    int iSec = _tcstol(ss, NULL, 10);

    if (iSec < 0 || iSec > 59) {
        AfxMessageBox(_T("开始时间(分钟0~59)格式错误"));

        _itow(m_playLoc.iSec, ss.GetBuffer(), 10);
        m_editPlayStartTimeSecs.SetWindowText(ss);
        m_bPlayLoc = FALSE;
        UpdateData(FALSE);
        m_editPlayStartTimeSecs.SetFocus();
        return;
    }

    m_playLoc.iSec = iSec;

    static BOOL bFirst = TRUE;
    // don't check when app startup
    if (!bFirst) {
        UpdatePlayLocStartTimeAndDuration();
    }
    bFirst = FALSE;
}

void CPlayDlg::OnEnChangeEditPlayDuration()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    static BOOL bFirst = TRUE;

    // don't check when app startup
    if (!bFirst) {
        UpdatePlayLocStartTimeAndDuration();
    }

    bFirst = FALSE;
}

void CPlayDlg::OnDeltaposSpinPlayStartHour(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    UpdateData(TRUE);
    CString ss;

    m_editPlayStartTimeHour.GetWindowText(ss);
    m_playLoc.iHour = _tcstol(ss, NULL, 10);

    // up
    if (pNMUpDown->iDelta == -1) {
        if (m_playLoc.iHour >= 0 && m_playLoc.iHour < 23)
            m_playLoc.iHour++;
        else if (m_playLoc.iHour >= 23)
            m_playLoc.iHour = 0;
    }
    // down
    else if (pNMUpDown->iDelta == 1) {
        if (m_playLoc.iHour > 0)
            m_playLoc.iHour--;
        else
            m_playLoc.iHour = 23;
    }

    ss.Format(_T("%d"), m_playLoc.iHour);
    m_editPlayStartTimeHour.SetWindowText(ss);

    UpdateData(FALSE);

    *pResult = 0;
}

void CPlayDlg::OnDeltaposSpinPlayStartMin(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    UpdateData(TRUE);
    CString ss;

    m_editPlayStartTimeMin.GetWindowText(ss);
    m_playLoc.iMin = _tcstol(ss, NULL, 10);

    // up
    if (pNMUpDown->iDelta == -1) {
        if (m_playLoc.iMin >= 0 && m_playLoc.iMin < 59)
            m_playLoc.iMin++;
        else if (m_playLoc.iMin >= 59)
            m_playLoc.iMin = 0;
    }
    // down
    else if (pNMUpDown->iDelta == 1) {
        if (m_playLoc.iMin > 0)
            m_playLoc.iMin--;
        else
            m_playLoc.iMin = 59;
    }

    ss.Format(_T("%d"), m_playLoc.iMin);
    m_editPlayStartTimeMin.SetWindowText(ss);

    UpdateData(FALSE);

    *pResult = 0;
}

void CPlayDlg::OnDeltaposSpinPlayStartSecs(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    UpdateData(TRUE);
    CString ss;

    m_editPlayStartTimeSecs.GetWindowText(ss);
    m_playLoc.iSec = _tcstol(ss, NULL, 10);

    // up
    if (pNMUpDown->iDelta == -1) {
        if (m_playLoc.iSec >= 0 && m_playLoc.iSec < 59)
            m_playLoc.iSec++;
        else if (m_playLoc.iSec >= 59)
            m_playLoc.iSec = 0;
    }
    // down
    else if (pNMUpDown->iDelta == 1) {
        if (m_playLoc.iSec > 0)
            m_playLoc.iSec--;
        else
            m_playLoc.iSec = 59;
    }

    ss.Format(_T("%d"), m_playLoc.iSec);
    m_editPlayStartTimeSecs.SetWindowText(ss);

    UpdateData(FALSE);

    *pResult = 0;
}

void CPlayDlg::OnNMSetfocusListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
    EnterCriticalSection(&m_hListCtrlFilesAccessCriticalSection);
    m_listCtrlFiles.SetItemState(m_iListSelectedItem, FALSE, LVIF_STATE);
    LeaveCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

    *pResult = 0;
}

void CPlayDlg::OnNMKillfocusListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
    EnterCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

    for (int i = 0; i < m_listCtrlFiles.GetItemCount(); i++) {
        if (m_listCtrlFiles.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED) {
            m_listCtrlFiles.SetItemState(i, LVIS_DROPHILITED, LVIS_DROPHILITED);
            m_iListSelectedItem = i;

            LeaveCriticalSection(&m_hListCtrlFilesAccessCriticalSection);
            return;
        }
    }

    LeaveCriticalSection(&m_hListCtrlFilesAccessCriticalSection);

    *pResult = 0;
}
