// VideoSettingsDlg.cpp : implementation file
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
#include "VideoSettingsDlg.h"

#define DISK_SPACE_INFO _T("* 该目录所在磁盘")

static SIZE g_arrSupportSize[] = {
  { 320, 240 }, { 352, 240 }, { 352, 288 }, { 360, 240 }, { 360, 288 }, { 512, 384 },
  { 640, 480 }, { 704, 480 }, { 704, 576 }, { 720, 480 }, { 720, 576 }, { 768, 576 },
  { 800, 600 }, { 1024, 768 }, { 1152, 864 }, 
  { 1280, 720 }, { 1280, 800 }, { 1280, 1024 },
  { 1360, 768 }, { 1366, 768 },
  { 1440, 900 }, { 1600, 1200 }, { 1680, 1050 }, { 1920, 1080 }, { 1920, 1200 }
};

//static SIZE g_arrSupportSize[] = {
//    { 480, 270 },
//    { 960, 540 }, 
//    { 1280, 720 },
//    { 1920, 1080 }
//};

static double g_arrSupportRate[] = {
    5.00,
    10.00,
    15.00,
    20.00,
    25.00,
    29.97,
    30.00,
    50.00,
    59.94,
    60.00
};

// CVideoSettingsDlg dialog

IMPLEMENT_DYNAMIC(CVideoSettingsDlg, CDialog)

CVideoSettingsDlg::CVideoSettingsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CVideoSettingsDlg::IDD, pParent)
{
    m_pointOld.x = m_pointOld.y = 0;

    m_siDefault.cx = 1920;
    m_siDefault.cy = 1080;
    m_fDefaultRate = 60;

    m_bHardwareEncode = TRUE;
    m_bRecordMp4 = TRUE;

    m_nVD1Sel = 0;
    m_nVD2Sel = 0;
    m_nVD3Sel = 0;
    m_nVD4Sel = 0;
    m_nAD1Sel = 0;
    m_nAD2Sel = 0;
    m_nAD3Sel = 0;
    m_nAD4Sel = 0;
}

CVideoSettingsDlg::~CVideoSettingsDlg()
{
}

void CVideoSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_VIDEO_DEVICE1, m_cmbVD1);
    DDX_Control(pDX, IDC_COMBO_VIDEO_DEVICE2, m_cmbVD2);
    DDX_Control(pDX, IDC_COMBO_VIDEO_DEVICE3, m_cmbVD3);
    DDX_Control(pDX, IDC_COMBO_VIDEO_DEVICE4, m_cmbVD4);
    DDX_Control(pDX, IDC_COMBO_AUDIO_DEVICE1, m_cmbAD1);
    DDX_Control(pDX, IDC_COMBO_AUDIO_DEVICE2, m_cmbAD2);
    DDX_Control(pDX, IDC_COMBO_AUDIO_DEVICE3, m_cmbAD3);
    DDX_Control(pDX, IDC_COMBO_AUDIO_DEVICE4, m_cmbAD4);

    DDX_Check(pDX, IDC_CHECK_HARDWARE_ENCODE, m_bHardwareEncode);
    DDX_Check(pDX, IDC_CHECK_RECORD_MP4, m_bRecordMp4);
    DDX_Control(pDX, IDC_COMBO_SIZE, m_cmbVideoSize);
    DDX_Control(pDX, IDC_COMBO_FRAMERATE, m_cmbVideoRate);
    DDX_Control(pDX, IDC_EDIT_RECORD_PATH, m_editRecordPath);
    DDX_Control(pDX, IDC_BTN_MODIFY_PATH, m_btnModifyPath);
    DDX_Control(pDX, IDC_STATIC_DIR_SPACE, m_staticDirSize);
}


BEGIN_MESSAGE_MAP(CVideoSettingsDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_CHECK_HARDWARE_ENCODE, &CVideoSettingsDlg::OnBnClickedCheckHardwareEncode)
    ON_BN_CLICKED(IDOK, &CVideoSettingsDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTN_MODIFY_PATH, &CVideoSettingsDlg::OnBnClickedBtnModifyPath)
    ON_BN_CLICKED(IDC_CHECK_RECORD_MP4, &CVideoSettingsDlg::OnBnClickedCheckRecordMp4)
    ON_CBN_SELCHANGE(IDC_COMBO_VIDEO_DEVICE1, &CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice1)
    ON_CBN_SELCHANGE(IDC_COMBO_VIDEO_DEVICE2, &CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice2)
    ON_CBN_SELCHANGE(IDC_COMBO_VIDEO_DEVICE3, &CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice3)
    ON_CBN_SELCHANGE(IDC_COMBO_VIDEO_DEVICE4, &CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice4)
END_MESSAGE_MAP()

// CVideoSettingsDlg message handlers

BOOL CVideoSettingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_fontDefault.CreatePointFont(DEFAULT_FONT_SIZE, _T("宋体"));

    CRect rect;
    GetClientRect(&rect);
    m_pointOld.x = rect.Width();
    m_pointOld.y = rect.Height();

    int nSizeSel = 0;
    int nSizeCount = sizeof(g_arrSupportSize) / sizeof(g_arrSupportSize[0]);
    for (int i = 0; i < nSizeCount; i ++) {
        CString strText;
        strText.Format(_T(" %d x %d"), g_arrSupportSize[i].cx, g_arrSupportSize[i].cy);
        int nItem = m_cmbVideoSize.InsertString(i, strText);
        m_cmbVideoSize.SetItemData(nItem, i);

        if (m_siDefault.cx == g_arrSupportSize[i].cx && m_siDefault.cy == g_arrSupportSize[i].cy) {
            nSizeSel = nItem;
        }
    }
    m_cmbVideoSize.SetCurSel(nSizeSel);

    int nRateSel = 0;
    int nRateCount = sizeof(g_arrSupportRate) / sizeof(g_arrSupportRate[0]);
    for (int i = 0; i < nRateCount; i ++) {
        CString strText;
        strText.Format(_T(" %2.2f FPS"), g_arrSupportRate[i]);
        int nItem = m_cmbVideoRate.InsertString(i, strText);
        m_cmbVideoRate.SetItemData(nItem, i);

        double fOffset = (g_arrSupportRate[i] - m_fDefaultRate);
        if (fOffset < 0.01 && fOffset > -0.01) {
            nRateSel = nItem;
        }
    }
    m_cmbVideoRate.SetCurSel(nRateSel);

    ((CButton *)GetDlgItem(IDC_CHECK_HARDWARE_ENCODE))->EnableWindow(FALSE);
    m_cmbVideoSize.EnableWindow(!m_bHardwareEncode);
    m_cmbVideoRate.EnableWindow(!m_bHardwareEncode);

    m_editRecordPath.EnableWindow(m_bRecordMp4);
    m_btnModifyPath.EnableWindow(m_bRecordMp4);

    CChildView& childView = ((CMainFrame *)AfxGetMainWnd())->m_wndView;

    m_strFilepath = childView.m_strFilepath;
    m_strRecordPath = childView.m_strRecordPath;
    m_strSnapshotPath = childView.m_strSnapshotPath;

    m_editRecordPath.SetWindowText(m_strFilepath);
    m_staticDirSize.SetWindowText(CString(DISK_SPACE_INFO) + GetDesignatedDiskFreeSpace(m_strFilepath));

    // Save file path
    CIniFile iniFile(CONFIG_FILE_NAME);
    iniFile.WriteString(CRPS_APP_NAME, _T("SavaFilePath"), m_strFilepath);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CVideoSettingsDlg::OnPaint()
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

void CVideoSettingsDlg::OnSize(UINT nType, int cx, int cy)
{
    if (m_pointOld.x == 0 || m_pointOld.y == 0 || cx == 0 || cy == 0)
        return;

    ResizeWindow(this->GetSafeHwnd(), m_pointOld, &m_fontDefault);
}

HBRUSH CVideoSettingsDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetBkColor(RGB(255, 255, 255));

        return (HBRUSH)::GetStockObject(WHITE_BRUSH);
        //return CreateSolidBrush(RGB(255, 255, 255));
    }

    return hbr;
}

void CVideoSettingsDlg::OnBnClickedCheckHardwareEncode()
{
    UpdateData();

    m_cmbVideoSize.EnableWindow(!m_bHardwareEncode);
    m_cmbVideoRate.EnableWindow(!m_bHardwareEncode);

    if (!m_bHardwareEncode) {
        int nSizeSel = 0;
        int nSizeCount = sizeof(g_arrSupportSize) / sizeof(g_arrSupportSize[0]);
        for (int i = 0; i < nSizeCount; i ++) {
            if (m_siDefault.cx == g_arrSupportSize[i].cx && m_siDefault.cy == g_arrSupportSize[i].cy) {
                nSizeSel = i;
            }
        }
        m_cmbVideoSize.SetCurSel(nSizeSel);

        int nRateSel = 0;
        int nRateCount = sizeof(g_arrSupportRate) / sizeof(g_arrSupportRate[0]);
        for (int i = 0; i < nRateCount; i ++) {
            double fOffset = (g_arrSupportRate[i] - m_fDefaultRate);
            if (fOffset < 0.01 && fOffset > -0.01) {
                nRateSel = i;
            }
        }
        m_cmbVideoRate.SetCurSel(nRateSel);
    }
}

void CVideoSettingsDlg::OnBnClickedCheckRecordMp4()
{
    UpdateData();

    m_editRecordPath.EnableWindow(m_bRecordMp4);
    m_btnModifyPath.EnableWindow(m_bRecordMp4);
}

void CVideoSettingsDlg::OnBnClickedOk()
{
    CChildView& childView = ((CMainFrame *)AfxGetMainWnd())->m_wndView;
    BOOL bChanged = FALSE;

    UpdateData();

    int nSizeSel = m_cmbVideoSize.GetCurSel();
    if (nSizeSel == -1)
        return;
    m_siDefault = g_arrSupportSize[nSizeSel];

    int nRateSel = m_cmbVideoRate.GetCurSel();
    if (nRateSel == -1)
        return;
    m_fDefaultRate = g_arrSupportRate[nRateSel];

    if ((!m_bHardwareEncode && (childView.m_cx != m_siDefault.cx ||
        childView.m_cy != m_siDefault.cy || 
        childView.m_dFrameRate != m_fDefaultRate)) || 
        childView.m_strRecordPath != m_strRecordPath ||
        childView.m_strSnapshotPath != m_strSnapshotPath || 
        m_nVD1Sel != m_cmbVD1.GetCurSel() || m_nAD1Sel != m_cmbAD1.GetCurSel() ||
        m_nVD2Sel != m_cmbVD2.GetCurSel() || m_nAD2Sel != m_cmbAD2.GetCurSel() ||
        m_nVD3Sel != m_cmbVD3.GetCurSel() || m_nAD3Sel != m_cmbAD3.GetCurSel() ||
        m_nVD4Sel != m_cmbVD4.GetCurSel() || m_nAD4Sel != m_cmbAD4.GetCurSel() ) {

        if (childView.m_bRecording) {
            if (IDOK != AfxMessageBox(_T("正在录制中，是否重新设置参数?"), MB_OKCANCEL | MB_ICONINFORMATION)) {
                return;
            }
        }
        childView.m_cx = m_siDefault.cx;
        childView.m_cy = m_siDefault.cy;
        childView.m_dFrameRate = m_fDefaultRate;
        childView.m_nFrameDuration = (int)((float)10000000 / m_fDefaultRate);

        childView.m_strRecordPath = m_strRecordPath;
        childView.m_strSnapshotPath = m_strSnapshotPath;

        m_nVD1Sel = m_cmbVD1.GetCurSel();
        m_nVD2Sel = m_cmbVD2.GetCurSel();
        m_nVD3Sel = m_cmbVD3.GetCurSel();
        m_nVD4Sel = m_cmbVD4.GetCurSel();
        m_nAD1Sel = m_cmbAD1.GetCurSel();
        m_nAD2Sel = m_cmbAD2.GetCurSel();
        m_nAD3Sel = m_cmbAD3.GetCurSel();
        m_nAD4Sel = m_cmbAD4.GetCurSel();

        // Save file path
        CIniFile iniFile(CONFIG_FILE_NAME);
        iniFile.WriteString(CRPS_APP_NAME, _T("SavaFilePath"), m_strFilepath);

        childView.ResizeWindow(FALSE);
        childView.RestartRecord();
    }
}

void CVideoSettingsDlg::OnBnClickedBtnModifyPath()
{
    TCHAR szPath[MAX_PATH];
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
    }
    else {
        AfxMessageBox(_T("打开文件夹失败!\n"), MB_OK | MB_ICONSTOP);
        return;
    }

    m_strRecordPath = szPath;
    m_strRecordPath += RECORD_VIDEO_PATH;
    m_strSnapshotPath = szPath;
    m_strSnapshotPath += RECORD_SNAPSHOT_PATH;

    m_strFilepath = szPath;
    m_editRecordPath.SetWindowText(m_strFilepath);

    m_staticDirSize.SetWindowText(CString(DISK_SPACE_INFO) + GetDesignatedDiskFreeSpace(szPath));
}

void CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice1()
{
}

void CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice2()
{
}

void CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice3()
{
}

void CVideoSettingsDlg::OnCbnSelchangeComboVideoDevice4()
{
}

void CVideoSettingsDlg::SetVideoComboxItem(int nChannel, int nIndex, LPCTSTR lpszValue) {
    if (nChannel < 0 || nIndex < 0 || !lpszValue)
        return;

    switch (nChannel)
    {
    case 0:
        m_cmbVD1.InsertString(nIndex, lpszValue);
        m_cmbVD1.SetCurSel(0);
        break;
    case 1:
        m_cmbVD2.InsertString(nIndex, lpszValue);
        m_cmbVD2.SetCurSel(0);
        break;
    case 2:
        m_cmbVD3.InsertString(nIndex, lpszValue);
        m_cmbVD3.SetCurSel(0);
        break;
    case 3:
        m_cmbVD4.InsertString(nIndex, lpszValue);
        m_cmbVD4.SetCurSel(0);
        break;
    default:
        break;
    }
}

void CVideoSettingsDlg::SetAudioComboxItem(int nChannel, int nIndex, LPCTSTR lpszValue) {
    if (nChannel < 0 || nIndex < 0 || !lpszValue)
        return;

    switch (nChannel)
    {
    case 0:
        m_cmbAD1.InsertString(nIndex, lpszValue);
        m_cmbAD1.SetCurSel(0);
        break;
    case 1:
        m_cmbAD2.InsertString(nIndex, lpszValue);
        m_cmbAD2.SetCurSel(0);
        break;
    case 2:
        m_cmbAD3.InsertString(nIndex, lpszValue);
        m_cmbAD3.SetCurSel(0);
        break;
    case 3:
        m_cmbAD4.InsertString(nIndex, lpszValue);
        m_cmbAD4.SetCurSel(0);
        break;
    default:
        break;
    }
}

void CVideoSettingsDlg::SetEnableAVCombox(BOOL bEnable) {
    m_cmbVD1.EnableWindow(bEnable);
    m_cmbVD2.EnableWindow(bEnable);
    m_cmbVD3.EnableWindow(bEnable);
    m_cmbVD4.EnableWindow(bEnable);

    m_cmbAD1.EnableWindow(bEnable);
    m_cmbAD2.EnableWindow(bEnable);
    m_cmbAD3.EnableWindow(bEnable);
    m_cmbAD4.EnableWindow(bEnable);
}

BOOL CVideoSettingsDlg::GetVideoAvailable(int nChannel) {
    if (nChannel < 0)
        return FALSE;

    CString str;
    switch (nChannel)
    {
    case 0:
        m_cmbVD1.GetWindowText(str);
        //m_cmbVD1.GetLBText(m_cmbVD1.GetCurSel(), str);
        break;
    case 1:
        m_cmbVD2.GetWindowText(str);
        //m_cmbVD2.GetLBText(m_cmbVD2.GetCurSel(), str);
        break;
    case 2:
        m_cmbVD3.GetWindowText(str);
        //m_cmbVD3.GetLBText(m_cmbVD3.GetCurSel(), str);
        break;
    case 3:
        m_cmbVD4.GetWindowText(str);
        //m_cmbVD4.GetLBText(m_cmbVD4.GetCurSel(), str);
        break;
    default:
        break;
    }

    if (str.IsEmpty() || str.Compare(NO_DEVIDE_INFO) == 0)
        return FALSE;

    return TRUE;
}

BOOL CVideoSettingsDlg::GetAudioAvailable(int nChannel) {
    if (nChannel < 0)
        return FALSE;

    CString str;
    switch (nChannel)
    {
    case 0:
        m_cmbAD1.GetWindowText(str);
        //m_cmbAD1.GetLBText(m_cmbAD1.GetCurSel(), str);
        break;
    case 1:
        m_cmbAD2.GetWindowText(str);
        //m_cmbAD2.GetLBText(m_cmbAD2.GetCurSel(), str);
        break;
    case 2:
        m_cmbAD3.GetWindowText(str);
        //m_cmbAD3.GetLBText(m_cmbAD3.GetCurSel(), str);
        break;
    case 3:
        m_cmbAD4.GetWindowText(str);
        //m_cmbAD4.GetLBText(m_cmbAD4.GetCurSel(), str);
        break;
    default:
        break;
    }

    if (str.IsEmpty() || str.Compare(NO_DEVIDE_INFO) == 0)
        return FALSE;

    return TRUE;
}
