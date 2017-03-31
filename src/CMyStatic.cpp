
// MyStatic.cpp : implementation of the CMyStatic class
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
#include "CaptureDlg.h"
#include "ChildView.h"
#include "CMyStatic.h"


CMyStatic::CMyStatic()
{
    m_eDialogType = DialogType_None;
    m_wndSaveParent = NULL;
    m_iIndex = 0;
}

CMyStatic::~CMyStatic()
{
}


BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
    //{{AFX_MSG_MAP(CMyStatic)
    ON_WM_LBUTTONDBLCLK()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_KEYDOWN()
    //ON_WM_PAINT()
    //ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMyStatic::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
    return CWnd::OnEraseBkgnd(pDC);
}

void CMyStatic::OnPaint()
{
    if (IsIconic())
        return;
    else
    {
        CWnd::OnPaint();
    }
}

void CMyStatic::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (DialogType_None == m_eDialogType) {
        return;
    }

    if (DialogType_Capture == m_eDialogType) {
        CCaptureDlg* pCapDlg = (CCaptureDlg*)GetParent();
        CRect rect;

        if (GetParent() != GetDesktopWindow() && !pCapDlg->m_bFullScreen) {
            pCapDlg->m_bFullScreen = TRUE;

            SetParent(GetDesktopWindow());
            GetDesktopWindow()->GetWindowRect(&rect);
            SetWindowPos(&wndTopMost, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);

            m_wndSaveParent = pCapDlg;
            pCapDlg->ShowWindow(SW_HIDE);
            //((CChildView *)(pCapDlg->GetParent()->GetParent()))->UpdateRenderer();
            Invalidate(TRUE);
        }
        else if (m_wndSaveParent/* && pCapDlg->m_bFullScreen*/) {
            pCapDlg = (CCaptureDlg*)m_wndSaveParent;
            pCapDlg->m_bFullScreen = FALSE;

            SetParent(pCapDlg);
            rect = pCapDlg->GetScreenRectBeforeFullScreen(m_iIndex);
            SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
            //((CChildView *)(pCapDlg->GetParent()->GetParent()))->UpdateRenderer();
            pCapDlg->ShowWindow(SW_SHOW);
        }

        CStatic::OnLButtonDblClk(nFlags, point);
    }
    else if (DialogType_Play == m_eDialogType) {
        CPlayDlg* pPlayDlg = (CPlayDlg*)GetParent()->GetParent();
        CRect rect;

        if (GetParent() != GetDesktopWindow() && !pPlayDlg->m_bFullScreen) {
            pPlayDlg->m_bFullScreen = TRUE;

            SetParent(GetDesktopWindow());
            GetDesktopWindow()->GetWindowRect(&rect);
            SetWindowPos(&wndTopMost, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);

            m_wndSaveParent = pPlayDlg;
            pPlayDlg->ResizePlayWindow(&rect);
            pPlayDlg->ShowWindow(SW_HIDE);
            Invalidate(TRUE);
        }
        else if (m_wndSaveParent) {
            pPlayDlg = (CPlayDlg*)m_wndSaveParent;
            pPlayDlg->m_bFullScreen = FALSE;

            CStatic* staticPlay;
            if (m_iIndex >= 1 && m_iIndex <= CAPTURE_NUM) {
                staticPlay = &pPlayDlg->m_staticPlay[m_iIndex - 1];
            }
            else return;

            SetParent(staticPlay);
            rect = pPlayDlg->GetScreenRectBeforeFullScreen(m_iIndex);
            SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
            pPlayDlg->ResizePlayWindow();
            pPlayDlg->ShowWindow(SW_SHOW);
        }

        CStatic::OnLButtonDblClk(nFlags, point);
    }
    else if (DialogType_ComparePlay == m_eDialogType) {
        CComparePlayDlg* pCmpPlayDlg = (CComparePlayDlg*)GetParent()->GetParent();
        CRect rect;

        if (GetParent() != GetDesktopWindow() && !pCmpPlayDlg->m_bFullScreen) {
            pCmpPlayDlg->m_bFullScreen = TRUE;

            SetParent(GetDesktopWindow());
            GetDesktopWindow()->GetWindowRect(&rect);
            SetWindowPos(&wndTopMost, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);

            m_wndSaveParent = pCmpPlayDlg;
            pCmpPlayDlg->ResizePlayWindow(&rect);
            pCmpPlayDlg->ShowWindow(SW_HIDE);
            Invalidate(TRUE);
        }
        else if (m_wndSaveParent) {
            pCmpPlayDlg = (CComparePlayDlg*)m_wndSaveParent;
            pCmpPlayDlg->m_bFullScreen = FALSE;

            CStatic* staticPlay;
            if (m_iIndex >= 1 && m_iIndex <= 2) {
                staticPlay = &pCmpPlayDlg->m_staticPlay[m_iIndex - 1];
            }
            else return;

            SetParent(staticPlay);
            rect = pCmpPlayDlg->GetScreenRectBeforeFullScreen(m_iIndex);
            SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
            pCmpPlayDlg->ResizePlayWindow();
            pCmpPlayDlg->ShowWindow(SW_SHOW);
        }

        CStatic::OnLButtonDblClk(nFlags, point);
    }
}

void CMyStatic::OnLButtonDown(UINT nFlags, CPoint point) {
    if (DialogType_None == m_eDialogType) {
        return;
    }

    MSG message;
    DWORD st = GetTickCount();

    while (1) {
        if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&message);
            ::DispatchMessage(&message);
            //if (message.message == WM_LBUTTONDBLCLK)
            //    return;
        }
        DWORD et = GetTickCount();
        if (et - st > 200)
            break;
    }

    // single click
    if (DialogType_Capture == m_eDialogType) {
        CCaptureDlg* pCapDlg = (CCaptureDlg*)GetParent();
        CRect rect;

        // not fullsceen
        if (GetParent() != GetDesktopWindow() && !pCapDlg->m_bFullScreen) {
            rect = pCapDlg->GetClientRectBeforeFullScreen(m_iIndex);
        }
        else if (m_wndSaveParent) { // fullscreen
            pCapDlg = (CCaptureDlg*)m_wndSaveParent;
            GetDesktopWindow()->GetWindowRect(&rect);
        }

        if (pCapDlg) {
            ::PostMessage(pCapDlg->GetSafeHwnd(), WM_MSG_MODIFY_STYLE, NULL, m_iIndex);
        }

        CStatic::OnLButtonDown(nFlags, point);
    }
    else if (DialogType_Play == m_eDialogType) {
        CPlayDlg* pPlayDlg = (CPlayDlg*)GetParent()->GetParent();
        CRect rect;

        // not fullsceen
        if (GetParent() != GetDesktopWindow() && !pPlayDlg->m_bFullScreen) {
            rect = pPlayDlg->GetClientRectBeforeFullScreen(m_iIndex);
        }
        else if (m_wndSaveParent) { // fullscreen
            pPlayDlg = (CPlayDlg*)m_wndSaveParent;
            GetDesktopWindow()->GetWindowRect(&rect);
        }

        if (pPlayDlg && message.message != WM_LBUTTONDBLCLK) {
            ProcessInfo* pInfoPlay = pPlayDlg->GetPlayInfoByChannel(m_iIndex);

            if (pInfoPlay && pInfoPlay->avChannelPlay) {
                LONG xPixel = point.x * pInfoPlay->avChannelPlay->m_nFileVideoWidth / rect.Width();
                LONG yPixel = point.y * pInfoPlay->avChannelPlay->m_nFileVideoHeight / rect.Height();

                if (xPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.left &&
                    xPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.right &&
                    yPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.top &&
                    yPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.bottom) {

                    double dSampleTime = (xPixel - pInfoPlay->avChannelPlay->m_rectProgressPixel.left) *
                        pInfoPlay->avChannelPlay->m_dFileTotalDuationTime / pInfoPlay->avChannelPlay->m_nBarProgressWidth;
                    pInfoPlay->avChannelPlay->SeekFile(dSampleTime);
                }
            }
        }

        CStatic::OnLButtonDown(nFlags, point);
    }
    else if (DialogType_ComparePlay == m_eDialogType) {
        CComparePlayDlg* pCmpPlayDlg = (CComparePlayDlg*)GetParent()->GetParent();
        CRect rect;

        // not fullscreen
        if (GetParent() != GetDesktopWindow() && !pCmpPlayDlg->m_bFullScreen) {
            rect = pCmpPlayDlg->GetClientRectBeforeFullScreen(m_iIndex);
        }
        else if (m_wndSaveParent) { // fullscreen
            pCmpPlayDlg = (CComparePlayDlg*)m_wndSaveParent;
            GetDesktopWindow()->GetWindowRect(&rect);
        }

        if (pCmpPlayDlg && message.message != WM_LBUTTONDBLCLK) {
            ProcessInfo* pInfoPlay = NULL;

            if (m_iIndex >= 1 && m_iIndex <= 2) {
                pInfoPlay = pCmpPlayDlg->m_pInfoPlay[m_iIndex - 1];
            }

            if (pInfoPlay && pInfoPlay->avChannelPlay) {
                LONG xPixel = point.x * pInfoPlay->avChannelPlay->m_nFileVideoWidth / rect.Width();
                LONG yPixel = point.y * pInfoPlay->avChannelPlay->m_nFileVideoHeight / rect.Height();

                if (xPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.left && 
                    xPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.right &&
                    yPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.top &&
                    yPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.bottom) {

                    double dSampleTime = (xPixel - pInfoPlay->avChannelPlay->m_rectProgressPixel.left) * 
                        pInfoPlay->avChannelPlay->m_dFileTotalDuationTime / pInfoPlay->avChannelPlay->m_nBarProgressWidth;
                    pInfoPlay->avChannelPlay->SeekFile(dSampleTime);
                }
            }
        }

        CStatic::OnLButtonDown(nFlags, point);
    }

    Dbg(_T("--------OnLButtonDown: %ldx%ld\n"), point.x, point.y);
}

void CMyStatic::OnMouseMove(UINT nFlags, CPoint point) {

    if (DialogType_None == m_eDialogType) {
        return;
    }

    if (DialogType_Play == m_eDialogType) {
        CPlayDlg* pPlayDlg = (CPlayDlg*)GetParent()->GetParent();
        CRect rect;

        // not fullsceen
        if (GetParent() != GetDesktopWindow() && !pPlayDlg->m_bFullScreen) {
            rect = pPlayDlg->GetClientRectBeforeFullScreen(m_iIndex);
        }
        else if (m_wndSaveParent) { // fullscreen
            pPlayDlg = (CPlayDlg*)m_wndSaveParent;
            GetDesktopWindow()->GetWindowRect(&rect);
        }

        if (pPlayDlg) {
            ProcessInfo* pInfoPlay = pPlayDlg->GetPlayInfoByChannel(m_iIndex);

            if (pInfoPlay && pInfoPlay->avChannelPlay) {
                LONG xPixel = point.x * pInfoPlay->avChannelPlay->m_nFileVideoWidth / rect.Width();
                LONG yPixel = point.y * pInfoPlay->avChannelPlay->m_nFileVideoHeight / rect.Height();

                if (xPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.left &&
                    xPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.right &&
                    yPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.top &&
                    yPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.bottom) {

                    ::SetCursor(LoadCursor(NULL, IDC_HAND));
                }
                else {
                    ::SetCursor(LoadCursor(NULL, IDC_ARROW));
                }
            }
        }

        CStatic::OnMouseHover(nFlags, point);
    }
    else if (DialogType_ComparePlay == m_eDialogType) {
        CComparePlayDlg* pCmpPlayDlg = (CComparePlayDlg*)GetParent()->GetParent();
        CRect rect;

        // not fullscreen
        if (GetParent() != GetDesktopWindow() && !pCmpPlayDlg->m_bFullScreen) {
            rect = pCmpPlayDlg->GetClientRectBeforeFullScreen(m_iIndex);
        }
        else if (m_wndSaveParent) { // fullscreen
            pCmpPlayDlg = (CComparePlayDlg*)m_wndSaveParent;
            GetDesktopWindow()->GetWindowRect(&rect);
        }

        if (pCmpPlayDlg) {
            ProcessInfo* pInfoPlay = NULL;

            if (m_iIndex >= 1 && m_iIndex <= 2) {
                pInfoPlay = pCmpPlayDlg->m_pInfoPlay[m_iIndex - 1];
            }

            if (pInfoPlay && pInfoPlay->avChannelPlay) {
                LONG xPixel = point.x * pInfoPlay->avChannelPlay->m_nFileVideoWidth / rect.Width();
                LONG yPixel = point.y * pInfoPlay->avChannelPlay->m_nFileVideoHeight / rect.Height();

                if (xPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.left &&
                    xPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.right &&
                    yPixel >= pInfoPlay->avChannelPlay->m_rectProgressPixel.top &&
                    yPixel <= pInfoPlay->avChannelPlay->m_rectProgressPixel.bottom) {

                    ::SetCursor(LoadCursor(NULL, IDC_HAND));
                }
                else {
                    ::SetCursor(LoadCursor(NULL, IDC_ARROW));
                }
            }
        }

        CStatic::OnMouseHover(nFlags, point);
    }
}

void CMyStatic::SetDialogTypeAndIndex(DialogType eDialogType, int iIndex)
{
    m_eDialogType = eDialogType;
    m_iIndex = iIndex;
}

void CMyStatic::OnStnClicked(CPoint point)
{
    OnLButtonDown(-1, point);
}
