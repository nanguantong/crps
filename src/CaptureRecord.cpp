
// CaptureRecord.cpp : Defines the class behaviors for the application.
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
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCaptureRecordApp
BEGIN_MESSAGE_MAP(CCaptureRecordApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CCaptureRecordApp::OnAppAbout)
END_MESSAGE_MAP()

// CCaptureRecordApp construction
CCaptureRecordApp::CCaptureRecordApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

// The one and only CCaptureRecordApp object
CCaptureRecordApp theApp;

// CCaptureRecordApp initialization
BOOL CCaptureRecordApp::InitInstance()
{
    ///////////////////////////////////////////////////////////////////////////////

    // INITIALIZE COM RESOURCE
    //
    {
        HRESULT hr = CoInitialize(NULL);
        if (hr != S_OK) return FALSE;
    }

    ///////////////////////////////////////////////////////////////////////////////

    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    // Initialize OLE libraries
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }
    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("CaptureRecord"));

    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object
    CMainFrame* pFrame = new CMainFrame;
    if (!pFrame)
        return FALSE;
    m_pMainWnd = pFrame;

    // create and load the frame with its resources
    BOOL bLoad = pFrame->LoadFrame(IDR_MAINFRAME,
        WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

    if (!bLoad) {
        return FALSE;
    }

    // The one and only window has been initialized, so show and update it
    pFrame->ResizeWindow();
    pFrame->ShowWindow(SW_SHOW);
    pFrame->UpdateWindow();
    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand
    return TRUE;
}

// CCaptureRecordApp message handlers
int CCaptureRecordApp::ExitInstance()
{
    ///////////////////////////////////////////////////////////////////////////////

    // UNINITIALIZE COM RESOURCE
    //
    {
        CoUninitialize();
    }

    ///////////////////////////////////////////////////////////////////////////////

    return CWinApp::ExitInstance();
}

// CCaptureRecordApp message handlers

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();

public:
    CStatic m_staticProduct;
    CStatic m_staticCopyright;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_PRODUCTNAME, m_staticProduct);
    DDX_Control(pDX, IDC_STATIC_COMPANY_NAME, m_staticCopyright);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString str(CRPS_NAME);
    str += _T(" \n ");
    str += CRPS_VERSION;
    m_staticProduct.SetWindowText(str);

    str = CRPS_COPYRIGHT;
    m_staticCopyright.SetWindowText(str);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

// App command to run the dialog
void CCaptureRecordApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}
