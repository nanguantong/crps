
// MainFrm.cpp : implementation of the CMainFrame class
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

//#ifdef _DEBUG
//#include <vld.h>
//#define new DEBUG_NEW
//#endif
#include "stdafx.h"
#include "CaptureRecord.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SETFOCUS()
    ON_WM_MOVE()
    ON_WM_GETMINMAXINFO()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    CString strTitle;
    GetWindowText(strTitle);
    strTitle += _T("-");
    strTitle += CRPS_VERSION;
    SetWindowText(strTitle);

    // create a view to occupy the client area of the frame
    if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
    {
        TRACE0("Failed to create view window\n");
        return -1;
    }

    return 0;
}

//static void UTFTOGB(CString &szstr)	// UTF-8转GB2312
//{
//    WCHAR* strSrc = NULL;
//    TCHAR* szRes = NULL;
//    //MultiByteToWideChar()的第四个形参设为-1,即可返回所需的短字符数组空间的大小
//    //UTF8转化成Unicode
//    int i = MultiByteToWideChar(CP_UTF8, 0, szstr.GetBuffer(szstr.GetLength()), -1, NULL, 0);
//    strSrc = new WCHAR[i + 1];
//    ZeroMemory(strSrc, (i + 1) * sizeof(WCHAR));
//    MultiByteToWideChar(CP_UTF8, 0, szstr.GetBuffer(szstr.GetLength()), -1, strSrc, i);
//    szstr.ReleaseBuffer();
//
//    //Unicode 转化成 GB2312
//    //WideCharToMultiByte()的第四个形参设为-1,即可返回所需的短字符数组空间的大小
//    i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
//    szRes = new TCHAR[i + 1];
//    ZeroMemory(szRes, (i + 1) * sizeof(TCHAR));
//    WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);
//    szstr = szRes;
//    if (strSrc != NULL)
//    {
//        delete[] strSrc;
//        strSrc = NULL;
//    }
//    if (szRes != NULL)
//    {
//        delete[] szRes;
//        szRes = NULL;
//    }
//}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;

    CIniFile iniFile(CONFIG_FILE_NAME);
    CString strProductName = iniFile.ReadString(CRPS_APP_NAME, _T("ProductName"), _T(""));
    if (!strProductName.IsEmpty()) {
        // 在项目为unicode情况下：这种方式ini文件必须是unicode方式保存，否则如果是utf-8保存的话出现中文乱码
        CRPS_NAME = strProductName;
    }

    // TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
    cs.style &= ~FWS_ADDTOTITLE;
    cs.lpszName = CRPS_NAME;
    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass(0);
    return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
    // forward focus to the view window
    m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    // let the view have first crack at the command
    if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return TRUE;

    // otherwise, do default handling
    return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::ResizeWindow()
{
    m_wndView.ResizeWindow();
}

void CMainFrame::OnMove(int x, int y)
{
    CFrameWnd::OnMove(x, y);
    m_wndView.UpdateRenderer();
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    CFrameWnd::OnGetMinMaxInfo(lpMMI);

    int cx = GetSystemMetrics(SM_CXFULLSCREEN) - 50;
    int cy = GetSystemMetrics(SM_CYFULLSCREEN) - 50;

    lpMMI->ptMinTrackSize.x = cx;
    lpMMI->ptMinTrackSize.y = cy;
}

void CMainFrame::OnClose()
{
    if (IDOK == AfxMessageBox(_T("确定退出吗?"), MB_OKCANCEL | MB_ICONINFORMATION)) {
        m_wndView.StopRecord();
        CFrameWnd::OnClose();
    }
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
    if (pMsg->message == WM_KEYDOWN)
    {
        switch (pMsg->wParam)
        {
        case VK_ESCAPE :
            break;
#if 0
        case VK_TAB:
            if (m_wndView.HandleKeyDownMsg(pMsg))
                return TRUE;
#endif
        }
    }
    return CFrameWnd::PreTranslateMessage(pMsg);
}
