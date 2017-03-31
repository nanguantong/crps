
// CaptureRecord.h : main header file for the CaptureRecord application
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "MainFrm.h"

// CCaptureRecordApp:
// See CaptureRecord.cpp for the implementation of this class
//

class CCaptureRecordApp : public CWinApp
{
public:
    CCaptureRecordApp();


    // Overrides
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    // Implementation
public:
    afx_msg void OnAppAbout();

    DECLARE_MESSAGE_MAP()
};

extern CCaptureRecordApp theApp;
