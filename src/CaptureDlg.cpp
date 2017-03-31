
// CaptureDlg.cpp : implementation of the CCaptureDlg class
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
#include "MainFrm.h"
#include "CaptureDlg.h"

#define MAX_RECORD_TIME_MINITE  120
#define MAX_RECORD_TIME_SECOND  60

// CCaptureDlg 对话框

IMPLEMENT_DYNAMIC(CCaptureDlg, CDialog)

CCaptureDlg::CCaptureDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CCaptureDlg::IDD, pParent)
{
    m_nMinite = m_nSecond = 0;
    m_bFullScreen = FALSE;

    m_pointOld.x = m_pointOld.y = 0;
}

CCaptureDlg::~CCaptureDlg()
{
}

void CCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_SPIN_MINITE, m_spinMinite);
    DDX_Control(pDX, IDC_SPIN_SECOND, m_spinSecond);

    DDX_Control(pDX, IDC_EDIT_MINITE, m_editMinite);
    DDX_Control(pDX, IDC_EDIT_SECOND, m_editSecond);
    DDX_Control(pDX, IDC_STATIC_CURTIME, m_staicCurTime);
    DDX_Control(pDX, IDC_STATIC_RECORD_TIME, m_staicRecordTime);

    DDX_Control(pDX, IDC_BTN_FULL_SCREEN, m_btnFullSceen);
    DDX_Control(pDX, IDC_BTN_SNAPSHOT, m_btnSnapshot);
    DDX_Control(pDX, IDC_BTN_RECORD_START, m_btnRecordStart);
    DDX_Control(pDX, IDC_BTN_RECORD_STOP, m_btnRecordStop);

    DDX_Control(pDX, IDC_STATIC_CAPTURE1, m_staticCapture[0]);
    DDX_Control(pDX, IDC_STATIC_CAPTURE2, m_staticCapture[1]);
    DDX_Control(pDX, IDC_STATIC_CAPTURE3, m_staticCapture[2]);
    DDX_Control(pDX, IDC_STATIC_CAPTURE4, m_staticCapture[3]);
}


BEGIN_MESSAGE_MAP(CCaptureDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_BTN_SNAPSHOT, &CCaptureDlg::OnBnClickedBtnSnapshot)
    ON_BN_CLICKED(IDC_BTN_FULL_SCREEN, &CCaptureDlg::OnBnClickedBtnFullScreen)
    ON_BN_CLICKED(IDC_BTN_RECORD_START, &CCaptureDlg::OnBnClickedBtnRecordStart)
    ON_BN_CLICKED(IDC_BTN_RECORD_STOP, &CCaptureDlg::OnBnClickedBtnRecordStop)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MINITE, &CCaptureDlg::OnDeltaposSpinMinite)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SECOND, &CCaptureDlg::OnDeltaposSpinSecond)
    //ON_STN_CLICKED(IDC_STATIC_CAPTURE1, &CCaptureDlg::OnStnClickedStaticCapture1)
    //ON_STN_CLICKED(IDC_STATIC_CAPTURE2, &CCaptureDlg::OnStnClickedStaticCapture2)
    //ON_STN_CLICKED(IDC_STATIC_CAPTURE3, &CCaptureDlg::OnStnClickedStaticCapture3)
    //ON_STN_CLICKED(IDC_STATIC_CAPTURE4, &CCaptureDlg::OnStnClickedStaticCapture4)
    ON_EN_CHANGE(IDC_EDIT_MINITE, &CCaptureDlg::OnEnChangeEditMinite)
    ON_EN_CHANGE(IDC_EDIT_SECOND, &CCaptureDlg::OnEnChangeEditSecond)
    ON_MESSAGE(WM_MSG_CURRENT_DATETIME, &CCaptureDlg::OnMsgCurrentDateTime)
    ON_MESSAGE(WM_MSG_RECORD_DATETIME, &CCaptureDlg::OnMsgRecordDateTime)
    ON_MESSAGE(WM_MSG_MODIFY_STYLE, &CCaptureDlg::OnMsgModifyStyle)
END_MESSAGE_MAP()


BOOL CCaptureDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_fontDefault.CreatePointFont(DEFAULT_FONT_SIZE, _T("宋体"));

    CRect rect;
    GetClientRect(&rect);
    m_pointOld.x = rect.Width();
    m_pointOld.y = rect.Height();

    m_spinMinite.SetBuddy(m_editMinite.GetOwner());
    //m_spinMinite.SetRange(0, MAX_RECORD_TIME_MINITE);
    m_editMinite.SetWindowText(_T("0"));

    m_spinSecond.SetBuddy(m_editSecond.GetOwner());
    //m_spinSecond.SetRange(0, MAX_RECORD_TIME_SECOND);
    m_editSecond.SetWindowText(_T("0"));

    m_staicRecordTime.ModifyStyle(SS_LEFT | SS_RIGHT, SS_CENTER, TRUE);
    m_staicRecordTime.SetWindowText(_T("00:00:00"));
    m_editMinite.ModifyStyle(SS_LEFT | SS_RIGHT, SS_CENTER, TRUE);
    m_editSecond.ModifyStyle(SS_LEFT | SS_RIGHT, SS_CENTER, TRUE);

    m_btnFullSceen.EnableWindow(TRUE);
    m_btnRecordStart.EnableWindow(FALSE);
    m_btnRecordStop.EnableWindow(FALSE);

    for (int i = 0; i < CAPTURE_NUM; i++) {
        m_staticCapture[i].SetDialogTypeAndIndex(DialogType_Capture, i + 1);

        //CBitmap bgBitmap;
        //bgBitmap.LoadBitmap(IDB_BITMAP_BG);
        //HBITMAP hBmp = (HBITMAP)bgBitmap.GetSafeHandle();
        m_staticCapture[i].GetWindowRect(&rect);
        HBITMAP hBmp = (HBITMAP)::LoadImage(0, _T("res\\bg.bmp"), IMAGE_BITMAP, rect.Width(), rect.Height(), LR_LOADFROMFILE);
        m_staticCapture[i].ModifyStyle(NULL, SS_BITMAP);
        m_staticCapture[i].SetBitmap(hBmp);
    }

    m_nCurPlaySel = 0;
    m_staticCurCapture = &m_staticCapture[0];

    SetTimer(TIMER_ID_CURTIME, 500, NULL);

    return FALSE;
}

void CCaptureDlg::OnDestroy()
{
    KillTimer(TIMER_ID_CURTIME);

    CDialog::OnDestroy();
}

void CCaptureDlg::OnPaint()
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

void CCaptureDlg::OnSize(UINT nType, int cx, int cy)
{
    if (m_pointOld.x == 0 || m_pointOld.y == 0 || cx == 0 || cy == 0)
        return;

    ResizeWindow(this->GetSafeHwnd(), m_pointOld, &m_fontDefault);

    GetPlayRect();
}

void CCaptureDlg::OnTimer(UINT_PTR nIDEvent)
{
    CChildView& childView = ((CMainFrame *)AfxGetMainWnd())->m_wndView;

    if (nIDEvent == TIMER_ID_CURTIME) {
        CTime t = CTime::GetCurrentTime();
        CString strCurrDateTime(t.Format(TIME_FORMAT));
        SetCurrentDateTime(strCurrDateTime);

        if (childView.m_bRecording) {
            CString strRecordDateTime;
            CTimeSpan ts = t - childView.m_startRecordTime;
            strRecordDateTime.Format(_T("%02ld:%02ld:%02ld"), ts.GetHours(), ts.GetMinutes(), ts.GetSeconds());
            SetRecordTime(strRecordDateTime);

#if RECORD_SPLIT_ONTIMER
            int nMinite = 0, nSecond = 0;
            GetRecordInterval(nMinite, nSecond);
            if ((nMinite > 0 || nSecond > 0)) {
                CTimeSpan tsFragment = t - childView.m_startRecordTimeForFragment;
                if (tsFragment.GetTotalSeconds() >= (nMinite * 60 + nSecond)) {
                    if (!childView.RestartRecord()) {
                        //AfxMessageBox(_T("录制失败!"), MB_OK | MB_ICONSTOP);
                    }
                }
            }
#endif
        }
    }

    CWnd::OnTimer(nIDEvent);
}

void CCaptureDlg::GetPlayRect() {
    for (int i = 0; i < CAPTURE_NUM; i++) {
        m_staticCapture[i].GetWindowRect(&m_rectScreenPlayBeforeFullScreen[i]);
        ScreenToClient(&m_rectScreenPlayBeforeFullScreen[i]);

        m_staticCapture[i].GetClientRect(&m_rectClientPlayBeforeFullScreen[i]);
    }
}

CRect CCaptureDlg::GetScreenRectBeforeFullScreen(int nCurPlaySel) {
    if (nCurPlaySel >= 1 && nCurPlaySel <= CAPTURE_NUM) {
        return m_rectScreenPlayBeforeFullScreen[nCurPlaySel - 1];
    }
    return m_rectScreenPlayBeforeFullScreen[0];
}

CRect CCaptureDlg::GetClientRectBeforeFullScreen(int nCurPlaySel) {
    if (nCurPlaySel >= 1 && nCurPlaySel <= CAPTURE_NUM) {
        return m_rectClientPlayBeforeFullScreen[nCurPlaySel - 1];
    }
    return m_rectClientPlayBeforeFullScreen[0];
}

void CCaptureDlg::SetCurrentDateTime(LPCTSTR lpszDateTime) {
    m_staicCurTime.SetWindowText(lpszDateTime);
    UpdateData(FALSE);
    //m_staicCurTime.UpdateWindow();
}

void CCaptureDlg::SetRecordTime(LPCTSTR lpszTime) {
    m_staicRecordTime.SetWindowText(lpszTime);
    UpdateData(FALSE);
    //m_staicRecordTime.UpdateWindow();
}

void CCaptureDlg::SetEnableFullScreen(BOOL bEnable) {
    m_btnFullSceen.EnableWindow(bEnable);
}

void CCaptureDlg::SetEnableRecordStart(BOOL bEnable) {
    m_btnRecordStart.EnableWindow(bEnable);
}

void CCaptureDlg::SetEnableRecordStop(BOOL bEnable) {
    m_btnRecordStop.EnableWindow(bEnable);
}

void CCaptureDlg::SetEnableRecordTime(BOOL bEnable) {
    m_editMinite.EnableWindow(bEnable);
    m_editSecond.EnableWindow(bEnable);
    m_spinMinite.EnableWindow(bEnable);
    m_spinSecond.EnableWindow(bEnable);
}

void CCaptureDlg::OnBnClickedBtnFullScreen()
{
    ::SendMessage(GetStaticCaptureWindow(m_nCurPlaySel)->GetSafeHwnd(), WM_LBUTTONDBLCLK, NULL, NULL);
}

void CCaptureDlg::OnBnClickedBtnSnapshot()
{
    ((CMainFrame *)AfxGetMainWnd())->m_wndView.VideoCaptureCreateSnapShot();
}

void CCaptureDlg::OnBnClickedBtnRecordStart()
{
    SetRecordTime(_T("00:00:00"));

    BOOL ok = ((CMainFrame *)AfxGetMainWnd())->m_wndView.StartRecord(TRUE);
    m_btnRecordStart.EnableWindow(ok ? FALSE : TRUE);
    m_btnRecordStop.EnableWindow(ok ? TRUE : FALSE);
}

void CCaptureDlg::OnBnClickedBtnRecordStop()
{
    BOOL ok = ((CMainFrame *)AfxGetMainWnd())->m_wndView.StopRecord(TRUE);
    m_btnRecordStart.EnableWindow(ok ? TRUE : FALSE);
    m_btnRecordStop.EnableWindow(ok ? FALSE : TRUE);
}

void CCaptureDlg::OnDeltaposSpinMinite(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    UpdateData(TRUE);
    CString ss;

    m_editMinite.GetWindowText(ss);
    m_nMinite = _tcstol(ss, NULL, 10);

    // up
    if (pNMUpDown->iDelta == -1) {
        if (m_nMinite >= 0)
            m_nMinite++;
    }
    // down
    else if (pNMUpDown->iDelta == 1) {
        if (m_nMinite > 0)
            m_nMinite--;
    }

    ss.Format(_T("%d"), m_nMinite);
    m_editMinite.SetWindowText(ss);

    UpdateData(FALSE);

    *pResult = 0;
}

void CCaptureDlg::OnDeltaposSpinSecond(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    UpdateData(TRUE);
    CString ss;

    m_editSecond.GetWindowText(ss);
    m_nSecond = _tcstol(ss, NULL, 10);

    // up
    if (pNMUpDown->iDelta == -1) {
        if (m_nSecond >= 0)
            m_nSecond++;
    }
    // down
    else if (pNMUpDown->iDelta == 1) {
        if (m_nSecond > 0)
            m_nSecond--;
    }

    ss.Format(_T("%d"), m_nSecond);
    m_editSecond.SetWindowText(ss);

    UpdateData(FALSE);

    *pResult = 0;
}

HBRUSH CCaptureDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetBkColor(RGB(255, 255, 255));

        return (HBRUSH)::GetStockObject(WHITE_BRUSH);
        //return CreateSolidBrush(RGB(255, 255, 255));
    }

    return hbr;
}

void CCaptureDlg::GetRecordInterval(int &nMinite, int &nSecond) {
    nMinite = m_nMinite;
    nSecond = m_nSecond;
}

int CCaptureDlg::GetCurPlaySel() {
    return m_nCurPlaySel;
}

CStatic* CCaptureDlg::GetStaticCaptureWindow(int iChannel) {
    if (iChannel < 0 || iChannel > 3)
        return &m_staticCapture[0];

    return &m_staticCapture[iChannel];
}

void CCaptureDlg::OnStnClickedStaticCapture1()
{
    ModifyStaticStyle(0);
}

void CCaptureDlg::OnStnClickedStaticCapture2()
{
    ModifyStaticStyle(1);
}

void CCaptureDlg::OnStnClickedStaticCapture3()
{
    ModifyStaticStyle(2);
}

void CCaptureDlg::OnStnClickedStaticCapture4()
{
    ModifyStaticStyle(3);
}

void CCaptureDlg::OnEnChangeEditMinite()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    CString ss;
    m_editMinite.GetWindowText(ss);
    m_nMinite = _tcstol(ss, NULL, 10);
}

void CCaptureDlg::OnEnChangeEditSecond()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    CString ss;
    m_editSecond.GetWindowText(ss);
    m_nSecond = _tcstol(ss, NULL, 10);
}

LRESULT CCaptureDlg::OnMsgCurrentDateTime(WPARAM wParam, LPARAM lParam)
{
    CString *strCurrDateTime = (CString *)lParam;
    if (!strCurrDateTime)
        return 0;

    SetCurrentDateTime(*strCurrDateTime);
    delete strCurrDateTime;

    return 0;
}

LRESULT CCaptureDlg::OnMsgRecordDateTime(WPARAM wParam, LPARAM lParam)
{
    CString *strRecordDateTime = (CString *)lParam;
    if (!strRecordDateTime)
        return 0;

    SetRecordTime(*strRecordDateTime);
    delete strRecordDateTime;

    return 0;
}

LRESULT CCaptureDlg::OnMsgModifyStyle(WPARAM wParam, LPARAM lParam)
{
    int iIndex = (LPARAM)lParam;
    ModifyStaticStyle(iIndex);

    return 0;
}

void CCaptureDlg::ModifyStaticStyle(int nSel)
{
    if (nSel == 1) {
        m_staticCurCapture = &m_staticCapture[0];

        m_staticCurCapture->ModifyStyleEx(m_staticCurCapture->GetSafeHwnd(), 0, g_dwFlag, SWP_FRAMECHANGED);
        m_staticCapture[1].ModifyStyleEx(m_staticCapture[1].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[2].ModifyStyleEx(m_staticCapture[2].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[3].ModifyStyleEx(m_staticCapture[3].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
    }
    else if (nSel == 2) {
        m_staticCurCapture = &m_staticCapture[1];

        m_staticCurCapture->ModifyStyleEx(m_staticCurCapture->GetSafeHwnd(), 0, g_dwFlag, SWP_FRAMECHANGED);
        m_staticCapture[0].ModifyStyleEx(m_staticCapture[0].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[2].ModifyStyleEx(m_staticCapture[2].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[3].ModifyStyleEx(m_staticCapture[3].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
    }
    else if (nSel == 3) {
        m_staticCurCapture = &m_staticCapture[2];

        m_staticCurCapture->ModifyStyleEx(m_staticCurCapture->GetSafeHwnd(), 0, g_dwFlag, SWP_FRAMECHANGED);
        m_staticCapture[0].ModifyStyleEx(m_staticCapture[0].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[1].ModifyStyleEx(m_staticCapture[1].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[3].ModifyStyleEx(m_staticCapture[3].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
    }
    else if (nSel == 4) {
        m_staticCurCapture = &m_staticCapture[3];

        m_staticCurCapture->ModifyStyleEx(m_staticCurCapture->GetSafeHwnd(), 0, g_dwFlag, SWP_FRAMECHANGED);
        m_staticCapture[0].ModifyStyleEx(m_staticCapture[0].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[1].ModifyStyleEx(m_staticCapture[1].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
        m_staticCapture[2].ModifyStyleEx(m_staticCapture[2].GetSafeHwnd(), g_dwFlag, 0, SWP_FRAMECHANGED);
    }
    else
        return;

    //CPoint point;
    //GetCursorPos(&point);
    //ScreenToClient(&point);
    //m_staticCurCapture->OnStnClicked(point);

    m_nCurPlaySel = nSel;
    m_staticCurCapture->SetFocus();
    UpdateData(FALSE);
}
