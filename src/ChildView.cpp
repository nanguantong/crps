
// ChildView.cpp : implementation of the CChildView class
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "StringUtils.h"
#include "CaptureRecord.h"
#include "VideoSettingsDlg.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum WindowType {
    WINDOW_TYPE_CAPTURE_RECORD = 0,
    WINDOW_TYPE_PLAY,
    WINDOW_TYPE_COMPARE_PLAY,
    WINDOW_TYPE_SETTINGS,
};

static DWORD WINAPI RecordNotifyThread(void *pArg);

// CChildView
CChildView::CChildView()
{
    m_bStop = FALSE;

#if RECORD_SPLIT_ONTIMER == 0
    m_hRecordNotifyThread = NULL;
    m_dwRecordNotifyThreadID = 0;
#endif

    m_iCurrRecordChannelNum = 0;
    m_ulCurrRecordNum = 0;

    m_tabCtrl = NULL;

    m_cx = CAPTURE_WIDTH;
    m_cy = CAPTURE_HEIGHT;
    m_dFrameRate = 60.0;
    m_nFrameDuration = FRAME_DURATION_60;

    m_iTabSelIndex = 0;

    m_bRecording = FALSE;

    m_bAudioMonitor = FALSE;
    m_bVideoPreview = TRUE;
}

CChildView::~CChildView()
{
    if (m_tabCtrl)
        delete m_tabCtrl;
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_MOVE()
    ON_WM_SETFOCUS()
    ON_WM_ERASEBKGND()
    //ON_WM_TIMER()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TABCTRL, OnTabSelChange)
    ON_NOTIFY(TCN_SELCHANGING, IDC_TABCTRL, OnTabSelChanging)
    ON_COMMAND(ID_FILE_START, &CChildView::OnFileStart)
    ON_COMMAND(ID_FILE_STOP, &CChildView::OnFileStop)
    ON_UPDATE_COMMAND_UI(ID_FILE_START, &CChildView::OnUpdateFileStart)
    ON_UPDATE_COMMAND_UI(ID_FILE_STOP, &CChildView::OnUpdateFileStop)
    ON_COMMAND(ID_FILE_SNAPSHOT, &CChildView::OnFileSnapshot)
    ON_UPDATE_COMMAND_UI(ID_FILE_SNAPSHOT, &CChildView::OnUpdateFileSnapshot)
    //ON_COMMAND(IDM_FILE_PROPERTY, &CChildView::OnFileProperty)
    //ON_COMMAND(IDM_FILE_MONITOR, &CChildView::OnFileMonitor)
    ON_UPDATE_COMMAND_UI(IDM_FILE_MONITOR, &CChildView::OnUpdateFileMonitor)
END_MESSAGE_MAP()

// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL);

    return TRUE;
}

void CChildView::OnPaint()
{
    if (IsIconic())
        return;
    else
    {
        CPaintDC dc(this); // device context for painting

        m_multiAVChannel.Repaint();

        CWnd::OnPaint();
    }
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    CRect rect;
    rect.SetRectEmpty();

    m_tabCtrl = new CTabCtrl;
    if (!m_tabCtrl || !m_tabCtrl->Create(TCS_TABS | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, rect, this, IDC_TABCTRL)) {
        TRACE0("未能创建Tab控件！");
        return -1;
    }

    int i, iTabCount = 0;
    m_capDlg.Create(IDD_DIALOG_CAPTURE, m_tabCtrl);
    m_capDlg.ShowWindow(TRUE);
    m_arrDlg.Add(&m_capDlg);
    m_tabCtrl->InsertItem(iTabCount, _T("录制"));
    iTabCount++;

    if (!m_multiAVChannel.Init()) {
#if ERR_RETURN
        AfxMessageBox(_T("初始化采集失败!\n"), MB_OK | MB_ICONSTOP);
        return -1;
#endif
    }

    CCaptureDlg* dlgCapture = ((CCaptureDlg *)m_arrDlg[WINDOW_TYPE_CAPTURE_RECORD]);
    int iOkNum = 0;
    for (i = 0; i < CAPTURE_NUM; i++) {
        CStatic* staticCapture = dlgCapture->GetStaticCaptureWindow(i);

        if (!m_multiAVChannel.Create(staticCapture->GetSafeHwnd(), i, m_cx, m_cy, m_dFrameRate, m_nFrameDuration)) {
            //TCHAR szTmp[64];
            //_stprintf(szTmp, _T("创建视频通道%d失败!\n"), i + 1);
            //AfxMessageBox(szTmp, MB_OK | MB_ICONSTOP);
            continue;
        }
        iOkNum++;
    }

    if (iOkNum <= 0) {
#if ERR_RETURN
        AfxMessageBox(_T("创建所有视频通道失败!\n"), MB_OK | MB_ICONSTOP);
        return -1;
#endif
    }

    CIniFile iniFile(CONFIG_FILE_NAME);
    CString strSavaFilePath;
    strSavaFilePath = iniFile.ReadString(CRPS_APP_NAME, _T("SavaFilePath"), _T(""));

    // Not found
    if (strSavaFilePath.IsEmpty()) {
        TCHAR szDefaultPath[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, szDefaultPath);

        m_strFilepath = szDefaultPath;
        m_strRecordPath = szDefaultPath;
        m_strRecordPath += RECORD_VIDEO_PATH;
        m_strSnapshotPath = szDefaultPath;
        m_strSnapshotPath += RECORD_SNAPSHOT_PATH;
    }
    else {
        m_strFilepath = strSavaFilePath;
        m_strRecordPath = strSavaFilePath;
        m_strRecordPath += RECORD_VIDEO_PATH;
        m_strSnapshotPath = strSavaFilePath;
        m_strSnapshotPath += RECORD_SNAPSHOT_PATH;
    }

    m_playDlg.Create(IDD_DIALOG_PLAY, m_tabCtrl);
    m_playDlg.ShowWindow(FALSE);
    m_arrDlg.Add(&m_playDlg);
    m_tabCtrl->InsertItem(iTabCount, _T("播放"));
    iTabCount++;
#if 1
    m_comparePlayDlg.Create(IDD_DIALOG_COMPARE_PLAY, m_tabCtrl);
    m_comparePlayDlg.ShowWindow(FALSE);
    m_arrDlg.Add(&m_comparePlayDlg);
    m_tabCtrl->InsertItem(iTabCount, _T("对比播放"));
    iTabCount++;
#endif
    m_configDlg.Create(IDD_DIALOG_VIDEO, m_tabCtrl);
    m_configDlg.ShowWindow(FALSE);
    m_arrDlg.Add(&m_configDlg);
    m_tabCtrl->InsertItem(iTabCount, _T("参数设置"));
    iTabCount++;

    m_tabCtrlFont.CreatePointFont(DEFAULT_FONT_SIZE, _T("宋体"));
    m_tabCtrl->SetFont(&m_tabCtrlFont);
    m_tabCtrl->SetCurSel(0);

    CVideoSettingsDlg* pVideoSettingsDlg = ((CVideoSettingsDlg *)m_arrDlg[WINDOW_TYPE_SETTINGS]);
    TCHAR szDeviceName[MAX_PATH];
    for (i = 0; i < CAPTURE_NUM; i++) {
        LPCTSTR lpszName = m_multiAVChannel.GetChannelDeviceName(i);
        if (lpszName && lpszName[0] != '\0') {
            // Set video device combox
            _stprintf(szDeviceName, _T("%ls video"), lpszName);
            pVideoSettingsDlg->SetVideoComboxItem(i, 0, szDeviceName);
            pVideoSettingsDlg->SetVideoComboxItem(i, 1, NO_DEVIDE_INFO);

            // Set audio device combox
            _stprintf(szDeviceName, _T("%ls audio"), lpszName);
            pVideoSettingsDlg->SetAudioComboxItem(i, 0, szDeviceName);
            pVideoSettingsDlg->SetAudioComboxItem(i, 1, NO_DEVIDE_INFO);
        }
        else {
            pVideoSettingsDlg->SetVideoComboxItem(i, 0, NO_DEVIDE_INFO);

            pVideoSettingsDlg->SetAudioComboxItem(i, 0, NO_DEVIDE_INFO);
        }
    }

    dlgCapture->SetEnableRecordStart(!m_bRecording);

    std::vector<RecordInfo> v;
    m_mapRecordInfos.InsertAt(CAPTURE_NUM - 1, v);

    //SetTimer(TIMER_ID_SIGNAL_CHANGE, 100, NULL);

    return 0;
}

void CChildView::OnDestroy()
{
    m_bStop = TRUE;

    StopRecord();

    //KillTimer(TIMER_ID_SIGNAL_CHANGE);

    CWnd::OnDestroy();
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    if (m_tabCtrl) {
        m_tabCtrl->MoveWindow(0, 0, cx, cy);

        int c = m_arrDlg.GetCount();
        for (int i = 0; i < c; i++) {
            CDialog *dlg = m_arrDlg[i];
            if (dlg) {
                dlg->MoveWindow(0, 35, cx, cy);
            }
        }
    }

    m_multiAVChannel.UpdateRendererPosition(m_cx, m_cy);
}

void CChildView::OnMove(int x, int y)
{
    CWnd::OnMove(x, y);

    m_multiAVChannel.UpdateRendererPosition(m_cx, m_cy);
}

void CChildView::OnSetFocus(CWnd* pOldWnd)
{
    CWnd::OnSetFocus(pOldWnd);

    OnTabSelChange(NULL, NULL);
}

void CChildView::OnTabSelChanging(NMHDR* pnmhdr, LRESULT* pResult)
{
    if (!m_tabCtrl)
        return;

    int iTabSelIndex = m_tabCtrl->GetCurSel();
    m_arrDlg[iTabSelIndex]->ShowWindow(SW_HIDE);

    *pResult = 0;
}

void CChildView::OnTabSelChange(NMHDR* pnmhdr, LRESULT* pResult)
{
    if (!m_tabCtrl || !pnmhdr || !pResult)
        return;

    RECT rc;
    m_tabCtrl->GetItemRect(0, &rc);

    int iTabSelIndex = m_tabCtrl->GetCurSel();
    m_arrDlg[iTabSelIndex]->SetWindowPos(NULL, rc.left + 5, rc.bottom + 5, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
    m_arrDlg[iTabSelIndex]->SetFocus();

    // 从播放页面切换到其他页面
    if (m_iTabSelIndex == WINDOW_TYPE_PLAY) {
        // 如果当前是暂停状态就不用设置，否则暂停
        ((CPlayDlg *)m_arrDlg[WINDOW_TYPE_PLAY])->PlayOrPause(PLAY_PAGE_SWITCH_FROM_PLAY_TO_OTHER);
    }
    // 从其他页面切换到播放页面
    else if (iTabSelIndex == WINDOW_TYPE_PLAY) {
        // 如果当前是暂停状态就播放，否则跳过
        ((CPlayDlg *)m_arrDlg[WINDOW_TYPE_PLAY])->PlayOrPause(PLAY_PAGE_SWITCH_FROM_OTHER_TO_PLAY);
    }

    // 从播放页面切换到其他页面
    if (m_iTabSelIndex == WINDOW_TYPE_COMPARE_PLAY) {
        // 如果当前是暂停状态就不用设置，否则暂停
        ((CComparePlayDlg *)m_arrDlg[WINDOW_TYPE_COMPARE_PLAY])->PlayOrPause(PLAY_PAGE_SWITCH_FROM_PLAY_TO_OTHER);
    }
    // 从其他页面切换到播放页面
    else if (iTabSelIndex == WINDOW_TYPE_COMPARE_PLAY) {
        // 如果当前是暂停状态就播放，否则跳过
        ((CComparePlayDlg *)m_arrDlg[WINDOW_TYPE_COMPARE_PLAY])->PlayOrPause(PLAY_PAGE_SWITCH_FROM_OTHER_TO_PLAY);
    }

    m_iTabSelIndex = iTabSelIndex;

    if (pResult)
        *pResult = 0;
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
    /*if (nIDEvent == TIMER_ID_SIGNAL_CHANGE) {
        if (m_multiAVChannel.IsVideoSignalChanged()) {
            BOOL bRecording = m_bRecording;
            if (bRecording)
                StopRecord();
            ResizeWindow();
            if (bRecording)
                StartRecord();
        }
    }*/

    CWnd::OnTimer(nIDEvent);
}

void CChildView::UpdateRenderer()
{
    m_multiAVChannel.UpdateRendererPosition(m_cx, m_cy);
}

BOOL CChildView::ResizeWindow(BOOL bHardwareEncode)
{
    CRect rcWnd;

    // TODO
    if (bHardwareEncode) {
        ((CVideoSettingsDlg *)m_arrDlg[WINDOW_TYPE_SETTINGS])->m_bHardwareEncode = FALSE;
    }

    int cx = GetSystemMetrics(SM_CXFULLSCREEN) - 50;
    int cy = GetSystemMetrics(SM_CYFULLSCREEN) - 50;

    rcWnd.SetRect(0, 0, cx, cy);
    AdjustWindowRectEx(&rcWnd, WS_OVERLAPPEDWINDOW, TRUE, WS_EX_CLIENTEDGE);

    AfxGetMainWnd()->SetWindowPos(NULL, 0, 0, rcWnd.Width(), rcWnd.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    AfxGetMainWnd()->CenterWindow();

    if (!m_multiAVChannel.LayoutVideo(m_cx, m_cy, m_dFrameRate, m_nFrameDuration)) {
#if ERR_RETURN
        AfxMessageBox(_T("布局视频失败!\n"), MB_OK | MB_ICONSTOP);
        return FALSE;
#endif
    }

    return TRUE;
}

void CChildView::OnFileStart()
{
    if (m_strRecordPath.IsEmpty()) {
        TCHAR szPath[MAX_PATH] = { 0 };
        LPITEMIDLIST pIDList = NULL;
        SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pIDList);

        BROWSEINFO bi;
        ::ZeroMemory(&bi, sizeof(BROWSEINFO));
        bi.hwndOwner = GetSafeHwnd();
        bi.pidlRoot = pIDList;
        bi.lpszTitle = L"请选择一个文件夹:";
        bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
        bi.lpfn = NULL;
        pIDList = SHBrowseForFolder(&bi);
        if (pIDList)
        {
            SHGetPathFromIDList(pIDList, szPath);
            ::CoTaskMemFree(pIDList);

            m_strRecordPath = szPath;
            m_strRecordPath += RECORD_VIDEO_PATH;
        }
        else {
            AfxMessageBox(_T("打开文件夹失败!\n"), MB_OK | MB_ICONSTOP);
            return;
        }
    }

    StartRecord(TRUE);
}

void CChildView::OnFileStop()
{
    StopRecord(TRUE);
}

BOOL CChildView::StartRecord(BOOL bResetTime)
{
    if (m_strRecordPath.IsEmpty()) {
        AfxMessageBox(_T("请选择录制文件路径!\n"));
        OnFileStart();
        return m_bRecording;
    }

    m_bRecording = TRUE;

    CreateDirectory(m_strRecordPath, NULL);

    CCaptureDlg* captureDlg = ((CCaptureDlg *)m_arrDlg[WINDOW_TYPE_CAPTURE_RECORD]);
    TCHAR szTmp[2];
    int iOkNum = 0;

    if (bResetTime)
        m_startRecordTime = CTime::GetCurrentTime();

#if RECORD_SPLIT_ONTIMER
    // Get current date time
    m_startRecordTimeForFragment = CTime::GetCurrentTime();
    CString strFilePath(m_strRecordPath);
    strFilePath += m_startRecordTimeForFragment.Format(TIME_FORMAT);
#else
    CString strFilePath(m_strRecordPath);
    strFilePath += TIME_FORMAT_QCAP;

    for (int i = 0; i < m_mapRecordInfos.GetCount(); i++)
        m_mapRecordInfos[i].clear();
    m_iCurrRecordChannelNum = 0;
    m_ulCurrRecordNum = 0;
    m_strCurrRecordNamePrefix.Empty();

    if (m_hRecordNotifyThread) {
        CloseHandle(m_hRecordNotifyThread);
    }
    m_hRecordNotifyThread = CreateThread(NULL, 0, RecordNotifyThread, this, NULL, &m_dwRecordNotifyThreadID);
#endif

    CVideoSettingsDlg* pVideoSettingsDlg = ((CVideoSettingsDlg *)m_arrDlg[WINDOW_TYPE_SETTINGS]);
    for (int i = 0; i < CAPTURE_NUM; i++) {
        if (!pVideoSettingsDlg->GetVideoAvailable(i) || !m_multiAVChannel.IsVideoAvailable(i)) {
            m_multiAVChannel.SetChannelEnableRecord(i, FALSE);
            continue;
        }

        _stprintf(szTmp, _T("%d"), i + 1);

        CString strFileName(strFilePath);
        strFileName += _T("_");
        strFileName += szTmp;
#if USE_RECORD_UNFINISH_SUFFIX
        strFileName += RECORD_UNFINISH_SUFFIX;
#endif
        strFileName += RECORD_FILE_SUFFIX;

        int nMinite = 0, nSecond = 0;
        captureDlg->GetRecordInterval(nMinite, nSecond);

        if (m_multiAVChannel.StartRecord(i, strFileName, nMinite * 60 + nSecond, m_dwRecordNotifyThreadID, TRUE, pVideoSettingsDlg->GetAudioAvailable(i)))
            iOkNum++;
    }

    if (iOkNum == 0) {
        AfxMessageBox(_T("开始录制视频失败!\n"), MB_OK | MB_ICONSTOP);
        m_bRecording = FALSE;

#if RECORD_SPLIT_ONTIMER == 0
        if (m_hRecordNotifyThread) {
            CloseHandle(m_hRecordNotifyThread);
            m_hRecordNotifyThread = NULL;
        }
#endif

        return FALSE;
    }

    if (bResetTime) {
        captureDlg->SetEnableRecordStart(FALSE);
        captureDlg->SetEnableRecordStop(TRUE);
    }

#if RECORD_SPLIT_ONTIMER == 0
    captureDlg->SetEnableRecordTime(FALSE);
#endif

    return TRUE;
}

BOOL CChildView::StopRecord(BOOL bResetTime) {
    CCaptureDlg* captureDlg = ((CCaptureDlg *)m_arrDlg[WINDOW_TYPE_CAPTURE_RECORD]);

    if (bResetTime)
        m_startRecordTime = CTime::GetCurrentTime();

#if RECORD_SPLIT_ONTIMER == 0
    captureDlg->SetEnableRecordTime(TRUE);

    if (m_multiAVChannel.StopRecord(NULL)) {
        if (bResetTime) {
            captureDlg->SetEnableRecordStart(TRUE);
            captureDlg->SetEnableRecordStop(FALSE);
        }
    }

    Sleep(2000);

    m_bRecording = FALSE;

    if (m_hRecordNotifyThread) {
        CloseHandle(m_hRecordNotifyThread);
        m_hRecordNotifyThread = NULL;
    }

    TCHAR szLog[MAX_PATH] = {0};
    for (int i = 0; i < m_mapRecordInfos.GetCount(); i++) {
        _stprintf(szLog + _tcsclen(szLog), _T("channel:%d remaining %i "), i + 1, (int)m_mapRecordInfos[i].size());
        m_mapRecordInfos[i].clear();
    }

    Dbg(_T("+++++++++++++ StopRecord currRecordChannelNum:%d, currRecordNum:%lu %ls"), m_iCurrRecordChannelNum, m_ulCurrRecordNum, szLog);
    
    m_iCurrRecordChannelNum = 0;
    m_ulCurrRecordNum = 0;
    m_strCurrRecordNamePrefix.Empty();
#else
    CArray<RecordInfo> arrRecordOkChannels;
    if (m_multiAVChannel.StopRecord(&arrRecordOkChannels)) {
        CPlayDlg* pPlayDlg = ((CPlayDlg *)m_arrDlg[WINDOW_TYPE_PLAY]);
        for (int i = 0; i < arrRecordOkChannels.GetCount(); i++) {
            RecordInfo &recordInfo = arrRecordOkChannels.GetAt(i);
            pPlayDlg->InsertItemToList(recordInfo.iChannel, recordInfo.strFilePath.GetBuffer());
        }
        if (m_iTabSelIndex == WINDOW_TYPE_PLAY) {
            pPlayDlg->StartAutoPlay();
        }

        if (bResetTime) {
            captureDlg->SetEnableRecordStart(TRUE);
            captureDlg->SetEnableRecordStop(FALSE);
        }
    }

    m_bRecording = FALSE;
#endif

    return TRUE;
}

BOOL CChildView::RestartRecord(BOOL bResetTime) {
    if (!m_bRecording)
        return FALSE;

    if (!StopRecord(bResetTime) || !StartRecord(bResetTime)) {
        return FALSE;
    }

    return TRUE;
}

BOOL CChildView::VideoCaptureCreateSnapShot() {
    if (m_strSnapshotPath.IsEmpty()) {
        AfxMessageBox(_T("请选择截图文件路径!\n"));
        OnFileSnapshot();
        return FALSE;
    }

    CreateDirectory(m_strSnapshotPath, NULL);

    int nCurCaptureSel = ((CCaptureDlg *)m_arrDlg[WINDOW_TYPE_CAPTURE_RECORD])->GetCurPlaySel();
    TCHAR szTmp[64];
    _stprintf(szTmp, _T("%d"), nCurCaptureSel);

    // Get current date time
    CTime t = CTime::GetCurrentTime();
    CString strFilePath(m_strSnapshotPath);
    strFilePath += t.Format(TIME_FORMAT);

    CString strFileName(strFilePath);
    strFileName += _T("_");
    strFileName += szTmp;
    strFileName += _T(".jpg");

    if (!m_multiAVChannel.VideoCaptureCreateSnapShot(nCurCaptureSel, strFileName, 100)) {
        _stprintf(szTmp, _T("截图视频%d失败!\n"), nCurCaptureSel + 1);
        AfxMessageBox(szTmp, MB_OK | MB_ICONSTOP);
        return FALSE;
    }
    return TRUE;
}

void CChildView::OnUpdateFileStart(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(!m_bRecording);
}

void CChildView::OnUpdateFileStop(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_bRecording);
}

void CChildView::OnFileSnapshot()
{
    if (m_strSnapshotPath.IsEmpty())
    {
        TCHAR szPath[MAX_PATH] = { 0 };
        LPITEMIDLIST pIDList = NULL;
        SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pIDList);

        BROWSEINFO bi;
        ::ZeroMemory(&bi, sizeof(BROWSEINFO));
        bi.hwndOwner = GetSafeHwnd();
        bi.pidlRoot = pIDList;
        bi.lpszTitle = L"请选择一个文件夹:";
        bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
        bi.lpfn = NULL;
        pIDList = SHBrowseForFolder(&bi);
        if (pIDList)
        {
            SHGetPathFromIDList(pIDList, szPath);
            ::CoTaskMemFree(pIDList);

            m_strSnapshotPath = szPath;
            m_strSnapshotPath += RECORD_SNAPSHOT_PATH;
        }
        else {
            AfxMessageBox(_T("打开文件夹失败!\n"), MB_OK | MB_ICONSTOP);
            return;
        }
    }

    VideoCaptureCreateSnapShot();
}

void CChildView::OnUpdateFileSnapshot(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void CChildView::OnFileProperty()
{
    m_multiAVChannel.ShowVideoCapturePropertyDialog(((CCaptureDlg *)m_arrDlg[WINDOW_TYPE_CAPTURE_RECORD])->GetCurPlaySel());
}

void CChildView::OnFileMonitor()
{
    m_bAudioMonitor = !m_bAudioMonitor;
    m_multiAVChannel.SetAudioMonitor(m_bAudioMonitor);
}

void CChildView::OnUpdateFileMonitor(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_multiAVChannel.HasAudioAvailable());
    pCmdUI->SetCheck(m_bAudioMonitor);
}

DWORD WINAPI RecordNotifyThread(void *pArg)
{
    if (pArg == NULL)
        return 0;

#if RECORD_SPLIT_ONTIMER == 0
    CChildView* childView = (CChildView*)pArg;
    CPlayDlg* pPlayDlg = ((CPlayDlg *)childView->m_arrDlg[WINDOW_TYPE_PLAY]);
    DWORD dwRet;
    MSG msg;

    while (!childView->m_bStop && childView->m_bRecording) {
        dwRet = ::MsgWaitForMultipleObjects(1, &childView->m_hRecordNotifyThread, FALSE, INFINITE, QS_ALLINPUT);
        if (dwRet == WAIT_OBJECT_0 + 1) {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                //get the message from queue and dispatch it to specific window
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_MSG_RECORD_DONE) {
                    RecordInfo* pRecordInfo = (RecordInfo *)msg.lParam;
                    if (pRecordInfo == NULL)
                        continue;

                    RecordInfo recordInfo = *pRecordInfo;
                    delete pRecordInfo;

                    Dbg(_T("+++++++++++++ RecordNotify chl:%d curNum:%d %ls"), recordInfo.iChannel, recordInfo.ulCurRecordNum, recordInfo.strFilePath);

                    childView->m_mapRecordInfos[recordInfo.iChannel].push_back(recordInfo);

                    if (childView->m_ulCurrRecordNum == 0) {
                        childView->m_ulCurrRecordNum = 1;
                    }

                    auto& it = childView->m_mapRecordInfos[recordInfo.iChannel];
                    auto itv = it.begin();
                    while (itv != it.end()) {
                        if (itv->ulCurRecordNum != childView->m_ulCurrRecordNum) {
                            itv++;
                            continue;
                        }

                        CString& strFileName = itv->strFilePath;
                        CString* pStrNewFileName = new CString(strFileName);

                        int iPosUnderline = itv->strFilePath.Find(_T("_"));

                        // 根据当前次的第一个生成的文件名来修改后面文件名不一样的文件
                        if (childView->m_strCurrRecordNamePrefix.IsEmpty()) {
                            childView->m_strCurrRecordNamePrefix = itv->strFilePath.Left(iPosUnderline);
                        }

                        // rename file with unfinished suffix and make start record time the same of all channels.
                        if (strFileName.GetLength() > 0) {
#if USE_RECORD_UNFINISH_SUFFIX
                            pStrNewFileName->Replace(RECORD_UNFINISH_SUFFIX, _T(""));// PathFindExtension _wsplitpath
#endif
                            // file prefix name is diff, then modify it
                            if (_tcsncmp(*pStrNewFileName, childView->m_strCurrRecordNamePrefix, iPosUnderline) != 0) {
                                pStrNewFileName->Replace(pStrNewFileName->Left(iPosUnderline), childView->m_strCurrRecordNamePrefix.Left(iPosUnderline));
                            }

                            while (PathFileExists(strFileName)) {
                                int ret = _trename(strFileName, pStrNewFileName->GetBuffer()); // CFile::Rename

                                Dbg(_T("rename %ls to %ls ---- ret %d\n"), strFileName, *pStrNewFileName, ret);

                                if (ret == 0) {
                                    break;
                                }
                                else {
                                    //Sleep(5);
                                    break; // TODO
                                }
                            }

                            PostMessage(pPlayDlg->GetSafeHwnd(), WM_MSG_FILE_TO_LIST, recordInfo.iChannel, (LPARAM)pStrNewFileName);

                            childView->m_iCurrRecordChannelNum++;
                        }

                        itv = it.erase(itv);
                    }

                    BOOL bIsAll = childView->m_iCurrRecordChannelNum == childView->m_multiAVChannel.GetChannelRecordCount();
                    if (bIsAll) {
                        childView->m_iCurrRecordChannelNum = 0;
                        childView->m_ulCurrRecordNum++;
                        childView->m_strCurrRecordNamePrefix.Empty();

                        if (childView->m_iTabSelIndex == WINDOW_TYPE_PLAY) {
                            PostMessage(pPlayDlg->GetSafeHwnd(), WM_MSG_AUTO_PLAY, NULL, (LPARAM)TRUE);
                        }
                    }
                }
            }
        }
        else { // WAIT_OBJECT_0 收到结束线程信号
            break;
        }
    }
#endif

    return 0;
}

BOOL CChildView::HandleKeyDownMsg(MSG* pMsg)
{
    if (pMsg->wParam == VK_TAB && GetKeyState(VK_CONTROL) < 0) {
        BOOL shift_down = GetKeyState(VK_SHIFT) < 0;
        if (!m_tabCtrl)
            return FALSE;

        int old_sel = m_tabCtrl->GetCurSel();
        int num = m_tabCtrl->GetItemCount();
        int new_sel = shift_down ? (old_sel + (num - 1)) % num : (old_sel + 1) % num;

        RECT r;
        m_tabCtrl->GetItemRect(new_sel, &r);
        m_tabCtrl->SendMessage(WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(r.left, r.top));
        return TRUE;
    }

    if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
        return TRUE;

    return FALSE;
}
