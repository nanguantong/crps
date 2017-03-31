
// ComparePlayDlg.cpp : implementation of the CComparePlayDlg class
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
#include "ComparePlayDlg.h"

// CComparePlayDlg 对话框

IMPLEMENT_DYNAMIC(CComparePlayDlg, CDialog)

CComparePlayDlg::CComparePlayDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CComparePlayDlg::IDD, pParent)
{
    m_pointOld.x = m_pointOld.y = 0;
    m_bFullScreen = FALSE;
    m_nCurPlaySel = 0;

    for (int i = 0; i < 2; i++) {
        m_pInfoPlay[i] = NULL;
    }
}

CComparePlayDlg::~CComparePlayDlg()
{

}

void CComparePlayDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_CMP_PLAY1, m_staticPlay[0]);
    DDX_Control(pDX, IDC_STATIC_CMP_PLAY2, m_staticPlay[1]);
    DDX_Control(pDX, IDC_EDIT_CMP_PLAY_PATH1, m_editPlayPath[0]);
    DDX_Control(pDX, IDC_EDIT_CMP_PLAY_PATH2, m_editPlayPath[1]);
    DDX_Control(pDX, IDC_COMBO_SPEED_CMP_PLAY_DELTA1, m_cmbSpeedPlayDelta[0]);
    DDX_Control(pDX, IDC_COMBO_SPEED_CMP_PLAY_DELTA2, m_cmbSpeedPlayDelta[1]);
}

BEGIN_MESSAGE_MAP(CComparePlayDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BTN_CMP_PLAY_PATH1, &CComparePlayDlg::OnBnClickedBtnCmpPlayPath1)
    ON_BN_CLICKED(IDC_BTN_CMP_PLAY_PATH2, &CComparePlayDlg::OnBnClickedBtnCmpPlayPath2)
    ON_BN_CLICKED(IDC_BTN_CMP_PLAY_PAUSE1, &CComparePlayDlg::OnBnClickedBtnCmpPlayPause1)
    ON_BN_CLICKED(IDC_BTN_CMP_STOP1, &CComparePlayDlg::OnBnClickedBtnCmpStop1)
    ON_BN_CLICKED(IDC_BTN_CMP_FORWORD1, &CComparePlayDlg::OnBnClickedBtnCmpForword1)
    ON_BN_CLICKED(IDC_BTN_CMP_BACK1, &CComparePlayDlg::OnBnClickedBtnCmpBack1)
    ON_BN_CLICKED(IDC_BTN_CMP_PLAY_PAUSE2, &CComparePlayDlg::OnBnClickedBtnCmpPlayPause2)
    ON_BN_CLICKED(IDC_BTN_CMP_STOP2, &CComparePlayDlg::OnBnClickedBtnCmpStop2)
    ON_BN_CLICKED(IDC_BTN_CMP_FORWORD2, &CComparePlayDlg::OnBnClickedBtnCmpForword2)
    ON_BN_CLICKED(IDC_BTN_CMP_BACK2, &CComparePlayDlg::OnBnClickedBtnCmpBack2)
    ON_CBN_SELCHANGE(IDC_COMBO_SPEED_CMP_PLAY_DELTA2, &CComparePlayDlg::OnCbnSelchangeComboSpeedCmpPlayDelta2)
    ON_CBN_SELCHANGE(IDC_COMBO_SPEED_CMP_PLAY_DELTA1, &CComparePlayDlg::OnCbnSelchangeComboSpeedCmpPlayDelta1)
END_MESSAGE_MAP()


BOOL CComparePlayDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_fontDefault.CreatePointFont(DEFAULT_FONT_SIZE, _T("宋体"));

    CRect rect;
    GetClientRect(&rect);
    m_pointOld.x = rect.Width();
    m_pointOld.y = rect.Height();

    int nSizeSel = 0;
    int nSizeCount = sizeof(g_arrSpeedPlayDelta) / sizeof(g_arrSpeedPlayDelta[0]);
    for (int j = 0; j < 2; j++) {
        CComboBox* cmbSpeedPlayDelta = &m_cmbSpeedPlayDelta[j];

        for (int i = 0; i < nSizeCount; i++) {
            int nItem = cmbSpeedPlayDelta->InsertString(i, g_arrSpeedPlayDelta[i].str);
            cmbSpeedPlayDelta->SetItemDataPtr(nItem, &g_arrSpeedPlayDelta[i].num);

            if (_T("正常速度") == g_arrSpeedPlayDelta[i].str) {
                nSizeSel = nItem;
            }
        }
        cmbSpeedPlayDelta->SetCurSel(nSizeSel);
    }

    SetTimer(TIMER_ID_COMPARE_PLAY_END, 100, NULL);

    return FALSE;
}

void CComparePlayDlg::OnDestroy()
{
    OnBnClickedBtnCmpStop1();
    OnBnClickedBtnCmpStop2();

    for (int i = 0; i < 2; i++) {
        if (m_pInfoPlay[i]) {
#if PLAY_MODE_QCAP
            delete m_pInfoPlay[i]->avChannelPlay;
            delete m_pInfoPlay[i]->cwnd;
            delete m_pInfoPlay[i];
#endif
        }
    }

    KillTimer(TIMER_ID_COMPARE_PLAY_END);

    CDialog::OnDestroy();
}

void CComparePlayDlg::OnPaint()
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

void CComparePlayDlg::OnSize(UINT nType, int cx, int cy)
{
    if (m_pointOld.x == 0 || m_pointOld.y == 0 || cx == 0 || cy == 0)
        return;

    ResizeWindow(this->GetSafeHwnd(), m_pointOld, &m_fontDefault);

    GetPlayRect();

    ResizePlayWindow();
}

HBRUSH CComparePlayDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetBkColor(RGB(255, 255, 255));

        return (HBRUSH)::GetStockObject(WHITE_BRUSH);
        //return CreateSolidBrush(RGB(255, 255, 255));
    }

    return hbr;
}

void CComparePlayDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIMER_ID_COMPARE_PLAY_END) {
        for (int i = 0; i < 2; i++) {
            ProcessInfo* proInfo = m_pInfoPlay[i];

#if PLAY_MODE_QCAP
            if (proInfo && proInfo->avChannelPlay) {
                if (proInfo->avChannelPlay->IsEnd()) {
                    proInfo->avChannelPlay->StopFile();
                }
                else {
                    double dSampleTime;
                    if (proInfo->avChannelPlay->GetFilePosition(&dSampleTime)) {
                        proInfo->avChannelPlay->SetFileOSD(dSampleTime);
                    }
                }
            }
#endif
        }
    }

    CWnd::OnTimer(nIDEvent);
}

void CComparePlayDlg::OnBnClickedBtnCmpPlayPath1()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                    (LPCTSTR)_TEXT("MP4 Files (*.mp4)|*.mp4|All Files (*.*)|*.*||"), NULL);

    INT_PTR ret = dlg.DoModal();
    if (ret == IDOK) {
        m_strPlayPath[0] = dlg.GetPathName();
    }
    else if (ret == IDCANCEL) {
        return;
    }
    else {
        AfxMessageBox(_T("打开文件失败!\n"), MB_OK | MB_ICONSTOP);
        return;
    }

    StartPlay(1);

    m_editPlayPath[0].SetWindowText(m_strPlayPath[0]);
}

void CComparePlayDlg::OnBnClickedBtnCmpPlayPath2()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                    (LPCTSTR)_TEXT("MP4 Files (*.mp4)|*.mp4|All Files (*.*)|*.*||"), NULL);
    
    INT_PTR ret = dlg.DoModal();
    if (ret == IDOK) {
        m_strPlayPath[1] = dlg.GetPathName();
    }
    else if (ret == IDCANCEL) {
        return;
    }
    else {
        AfxMessageBox(_T("打开文件失败!\n"), MB_OK | MB_ICONSTOP);
        return;
    }

    StartPlay(2);

    m_editPlayPath[1].SetWindowText(m_strPlayPath[1]);
}

void CComparePlayDlg::OnBnClickedBtnCmpPlayPause1()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[0] == NULL) {
        StartPlay(1);
    }
    if (m_pInfoPlay[0] && m_pInfoPlay[0]->avChannelPlay) {
        OnCbnSelchangeComboSpeedCmpPlayDelta1();
        m_pInfoPlay[0]->avChannelPlay->PlayOrPauseFile();
    }
#endif
}

void CComparePlayDlg::OnBnClickedBtnCmpStop1()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[0] && m_pInfoPlay[0]->avChannelPlay) {
        //m_pInfoPlay1->avChannelPlay->StopFile();

        delete m_pInfoPlay[0]->avChannelPlay;
        delete m_pInfoPlay[0]->cwnd;
        delete m_pInfoPlay[0];
        m_pInfoPlay[0] = NULL;

        if (m_bFullScreen) {
            m_bFullScreen = FALSE;
            ShowWindow(TRUE);
        }
    }
#endif
}

void CComparePlayDlg::OnBnClickedBtnCmpForword1()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[0] && m_pInfoPlay[0]->avChannelPlay) {
        m_pInfoPlay[0]->avChannelPlay->SeekFile(TRUE);
    }
#endif
}

void CComparePlayDlg::OnBnClickedBtnCmpBack1()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[0] && m_pInfoPlay[0]->avChannelPlay) {
        m_pInfoPlay[0]->avChannelPlay->SeekFile(FALSE);
    }
#endif
}

void CComparePlayDlg::OnCbnSelchangeComboSpeedCmpPlayDelta1()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[0] && m_pInfoPlay[0]->avChannelPlay) {
        int index = m_cmbSpeedPlayDelta[0].GetCurSel();

        double* dSpeed = (double *)m_cmbSpeedPlayDelta[0].GetItemDataPtr(index);
        if (dSpeed == NULL) {
            return;
        }

        m_pInfoPlay[0]->avChannelPlay->SetFilePlaySpeed(*dSpeed);
    }
#endif
}

void CComparePlayDlg::OnBnClickedBtnCmpPlayPause2()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[1] == NULL) {
        StartPlay(2);
    }
    if (m_pInfoPlay[1] && m_pInfoPlay[1]->avChannelPlay) {
        OnCbnSelchangeComboSpeedCmpPlayDelta2();
        m_pInfoPlay[1]->avChannelPlay->PlayOrPauseFile();
    }
#endif
}

void CComparePlayDlg::OnBnClickedBtnCmpStop2()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[1] && m_pInfoPlay[1]->avChannelPlay) {
        //m_pInfoPlay2->avChannelPlay->StopFile();

        delete m_pInfoPlay[1]->avChannelPlay;
        delete m_pInfoPlay[1]->cwnd;
        delete m_pInfoPlay[1];
        m_pInfoPlay[1] = NULL;

        if (m_bFullScreen) {
            m_bFullScreen = FALSE;
            ShowWindow(TRUE);
        }
    }
#endif
}

void CComparePlayDlg::OnBnClickedBtnCmpForword2()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[1] && m_pInfoPlay[1]->avChannelPlay) {
        m_pInfoPlay[1]->avChannelPlay->SeekFile(TRUE);
    }
#endif
}

void CComparePlayDlg::OnBnClickedBtnCmpBack2()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[1] && m_pInfoPlay[1]->avChannelPlay) {
        m_pInfoPlay[1]->avChannelPlay->SeekFile(FALSE);
    }
#endif
}

void CComparePlayDlg::OnCbnSelchangeComboSpeedCmpPlayDelta2()
{
#if PLAY_MODE_QCAP
    if (m_pInfoPlay[1] && m_pInfoPlay[1]->avChannelPlay) {
        int index = m_cmbSpeedPlayDelta[1].GetCurSel();

        double* dSpeed = (double *)m_cmbSpeedPlayDelta[1].GetItemDataPtr(index);
        if (dSpeed == NULL) {
            return;
        }

        m_pInfoPlay[1]->avChannelPlay->SetFilePlaySpeed(*dSpeed);
    }
#endif
}

void CComparePlayDlg::StartPlay(int iChannel) {
    if (iChannel != 1 && iChannel != 2)
        return;

    CRect rect;
    UINT nID;
    ProcessInfo *proInfo;
    CStatic *staticPlay;
    CString *strPlayPath;
    int iIndex = iChannel - 1;

#if PLAY_MODE_QCAP
    if (iChannel == 1) {
        //OnBnClickedBtnCmpStop1();
    }
    else if (iChannel == 2) {
        //OnBnClickedBtnCmpStop2();
    }

    if (m_pInfoPlay[iIndex]) {
        delete m_pInfoPlay[iIndex]->avChannelPlay;
        delete m_pInfoPlay[iIndex]->cwnd;
        delete m_pInfoPlay[iIndex];
        m_pInfoPlay[iIndex] = NULL;

        //m_editPlayPath[iIndex].SetWindowText(_T(""));
        //m_strPlayPath[iIndex].Empty();
    }

    nID = CMP_PLAY_BASE_HWND_ID + iIndex;
    proInfo = m_pInfoPlay[iIndex];
    staticPlay = &m_staticPlay[iIndex];
    strPlayPath = &m_strPlayPath[iIndex];

    staticPlay->GetClientRect(&rect);
    CMyStatic *staticVideoWnd = new CMyStatic;
    staticVideoWnd->SetDialogTypeAndIndex(DialogType_ComparePlay, iChannel);
    staticVideoWnd->Create(NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | /*WS_EX_CONTROLPARENT | */SS_NOTIFY, rect, staticPlay, nID);
    staticVideoWnd->SetActiveWindow();
    //staticVideoWnd->SetWindowPos(&wndNoTopMost, rect.left, rect.right, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_DRAWFRAME);

    CAVChannelPlay* avChannelCmpPlay = new CAVChannelPlay;
    if (avChannelCmpPlay->OpenFile(*strPlayPath, staticVideoWnd->GetSafeHwnd())) {
        proInfo = new ProcessInfo;
        proInfo->iChannel = iChannel;
        proInfo->cwnd = staticVideoWnd;
        proInfo->avChannelPlay = avChannelCmpPlay;
        m_pInfoPlay[iIndex] = proInfo;
    }
    else {
        delete staticVideoWnd;
        delete avChannelCmpPlay;
    }
#endif
}

void CComparePlayDlg::PlayOrPause(PlayPageSwitch ePageSwitch) {
#if PLAY_MODE_QCAP
    if (ePageSwitch == PLAY_PAGE_SWITCH_NONE) {
        OnBnClickedBtnCmpPlayPause1();
        OnBnClickedBtnCmpPlayPause2();
    }
    else if (ePageSwitch == PLAY_PAGE_SWITCH_FROM_PLAY_TO_OTHER) {
        if (m_pInfoPlay[0] && m_pInfoPlay[0]->avChannelPlay) {
            OnCbnSelchangeComboSpeedCmpPlayDelta1();
            m_pInfoPlay[0]->avChannelPlay->PauseFile();
        }

        if (m_pInfoPlay[1] && m_pInfoPlay[1]->avChannelPlay) {
            OnCbnSelchangeComboSpeedCmpPlayDelta2();
            m_pInfoPlay[1]->avChannelPlay->PauseFile();
        }
    }
    else if (ePageSwitch == PLAY_PAGE_SWITCH_FROM_OTHER_TO_PLAY) {
        if (m_pInfoPlay[0] && m_pInfoPlay[0]->avChannelPlay) {
            OnCbnSelchangeComboSpeedCmpPlayDelta1();
            m_pInfoPlay[0]->avChannelPlay->PlayFile();
        }

        if (m_pInfoPlay[1] && m_pInfoPlay[1]->avChannelPlay) {
            OnCbnSelchangeComboSpeedCmpPlayDelta2();
            m_pInfoPlay[1]->avChannelPlay->PlayFile();
        }
    }
#endif
}

void CComparePlayDlg::ResizePlayWindow(LPRECT lprcPosition) {
    for (int i = 0; i < 2; i++) {
        ProcessInfo* proInfo = m_pInfoPlay[i];
        CStatic *staticPlay = &m_staticPlay[i];

        if (proInfo) {
            CRect rect;
            if (lprcPosition == NULL)
                staticPlay->GetClientRect(&rect);
            else
                rect = *lprcPosition;
            ::MoveWindow(proInfo->cwnd->GetSafeHwnd(), rect.left, rect.top, rect.Width(), rect.Height(), FALSE);
        }
    }
}

int CComparePlayDlg::GetCurPlaySel() {
    return m_nCurPlaySel;
}

void CComparePlayDlg::GetPlayRect() {
    for (int i = 0; i < 2; i++) {
        m_staticPlay[i].GetWindowRect(&m_rectScreenPlayBeforeFullScreen[i]);
        ScreenToClient(&m_rectScreenPlayBeforeFullScreen[i]);

        m_staticPlay[i].GetClientRect(&m_rectClientPlayBeforeFullScreen[i]);
    }
}

CRect CComparePlayDlg::GetScreenRectBeforeFullScreen(int nCurCmpPlaySel) {
    m_nCurPlaySel = nCurCmpPlaySel;

    if (m_nCurPlaySel >= 1 && m_nCurPlaySel <= 2) {
        return m_rectScreenPlayBeforeFullScreen[m_nCurPlaySel - 1];
    }

    return m_rectScreenPlayBeforeFullScreen[0];
}

CRect CComparePlayDlg::GetClientRectBeforeFullScreen(int nCurPlaySel) {
    if (m_nCurPlaySel >= 1 && m_nCurPlaySel <= 2) {
        return m_rectClientPlayBeforeFullScreen[m_nCurPlaySel - 1];
    }

    return m_rectClientPlayBeforeFullScreen[0];
}
