
// CMyStatic.h : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once

typedef enum DialogType {
    DialogType_None = -1,
    DialogType_Capture,
    DialogType_Play,
    DialogType_ComparePlay,
} DialogType;

static const DWORD g_dwFlag = WS_EX_DLGMODALFRAME | WS_EX_OVERLAPPEDWINDOW;

// CMyStatic window

class CMyStatic : public CStatic
{
public:
    CMyStatic();

    // Implementation
public:
    virtual ~CMyStatic();

    void SetDialogTypeAndIndex(DialogType eDialogType, int iIndex);

    void OnStnClicked(CPoint point);

    // Generated message map functions
protected:
    //{{AFX_MSG(CMyStatic)
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

private:
    CWnd*       m_wndSaveParent;
    DialogType  m_eDialogType;
    int         m_iIndex;
};
