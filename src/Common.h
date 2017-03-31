
// Common.h : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include "CIniFile.h"
#include "resource.h"

#define ERR_RETURN                    1
#define CAPTURE_NUM                   4

static CString CONFIG_FILE_NAME =     _T(".\\cfg.ini");
static CString CRPS_APP_NAME =        _T("AppCaptureRecord");
static CString CRPS_NAME =            _T("CRPS");
static CString CRPS_VERSION =         _T("V1.0.2017.03131");
static CString CRPS_COPYRIGHT =       _T("Copyright (C) 2017");

static CString RECORD_VIDEO_PATH =    _T("\\video\\");
static CString RECORD_SNAPSHOT_PATH = _T("\\pic\\");

static int DEFAULT_FONT_SIZE =        150;

//是否启用录制文件未完成时添加临时后缀
#define USE_RECORD_UNFINISH_SUFFIX    0

#define RECORD_FILE_SUFFIX            _T(".mp4")
#define RECORD_UNFINISH_SUFFIX        _T(".tdr")

#define NO_DEVIDE_INFO                _T("未选择设备")

#define TIME_FORMAT                   _T("%Y.%m.%d-%H.%M.%S")
#define TIME_FORMAT_QCAP              _T("$Y.$M.$D-$h.$m.$s")

#define TIMER_ID_SIGNAL_CHANGE        1
#define TIMER_ID_CURTIME              2

#define TIMER_ID_PLAY_END             10
#define TIMER_ID_COMPARE_PLAY_END     20

/////////////////////////////////////////////////////////
#define PLAY_PROCESSOR                _T("fplayer.exe")

#define PROGRESS_BAR_HEIGHT           80
#define PROGRESS_BAR_LEFT_WIDTH       220
#define PROGRESS_BAR_TEXT_WIDTH       80
#define PROGRESS_BAR_TEXT_LEFT_WIDTH  7
#define PROGRESS_BAR_PROGRESS_TOP     4
#define PROGRESS_BAR_BACK_COLOR       0x00C4C4C4
#define PROGRESS_BAR_TEXT_COLOR       0xFF000000
#define PROGRESS_BAR_TEXT_FONT_NAME   "Arial"
#define PROGRESS_BAR_TEXT_FONT_SIZE   25
#define PROGRESS_BAR_ALPHA            200     /* 0~255 */

#define SDL_WIND                      1
#define PLAY_MODE_QCAP                1
//#define PLAY_MODE_FFPLAY 0
#define RECORD_SPLIT_ONTIMER          0

#define PLAY_BASE_HWND_ID             44441
#define CMP_PLAY_BASE_HWND_ID         54441

typedef struct PlayInfo
{
    CString strChannel;
    CString strFileNamePrefix;
    CString strPath;
} PlayInfo;

typedef enum PlayType
{
    PlayType_None,
    PlayType_Auto,
    PlayType_Manual,
} PlayType;

// 定位播放
typedef struct PlayLocation
{
    /* play from the begin of files ---> (iHour == 0 && iMin == 0 && iSecs == 0) 
     * play to the end of the files ---> (iDuration == 0)
     */
    int iHour;
    int iMin;
    int iSec;
    int iDuration;
} PlayLocation;

class CAVChannelPlay;
typedef struct ProcessInfo
{
    int iChannel;
    CString strFileNamePrefix;
    CWnd *cwnd;
#if PLAY_MODE_QCAP
    CAVChannelPlay *avChannelPlay;
#else
    PROCESS_INFORMATION pi;
    HANDLE hPipeWrite, hPipeRead;
#endif
} ProcessInfo;

struct SpeedPlayDelta
{
    double num;
    CString str;
};

static SpeedPlayDelta g_arrSpeedPlayDelta[] = {
    { 0.25, _T("慢两倍") },
    { 0.5, _T("慢一倍") },
    { 1.0, _T("正常速度") },
    { 2.0, _T("快一倍") },
    { 4.0, _T("快两倍") },
};

enum PlayPageSwitch
{
    PLAY_PAGE_SWITCH_NONE,
    PLAY_PAGE_SWITCH_FROM_PLAY_TO_OTHER,
    PLAY_PAGE_SWITCH_FROM_OTHER_TO_PLAY,
};

enum PlayProgressOSD {
    PLAY_PROGRESS_OSD_BAR = 0,
    PLAY_PROGRESS_OSD_PROGRESS,
    PLAY_PROGRESS_OSD_START,
    PLAY_PROGRESS_OSD_END,
};
/////////////////////////////////////////////////////////

typedef struct RecordInfo
{
    int iChannel;
    ULONG ulCurRecordNum;
    CString strFilePath;
} RecordInfo;



typedef struct EnumFunArg
{
    HWND    hWnd;
    DWORD   dwProcessId;
} EnumFunArg, *LPEnumFunArg;

static BOOL CALLBACK lpEnumFunc(HWND hwnd, LPARAM lParam) {
    EnumFunArg *pArg = (LPEnumFunArg)lParam;
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == pArg->dwProcessId)
    {
        pArg->hWnd = hwnd;
        return FALSE;
    }
    return TRUE;
}

static HWND GetHwndByProcessId(DWORD dwProcessId) {
    EnumFunArg wi;
    wi.dwProcessId = dwProcessId;
    wi.hWnd = NULL;
    EnumWindows(lpEnumFunc, (LPARAM)&wi);
    return wi.hWnd;
}

static HHOOK SetHookEx(HWND hWnd)
{
    if (hWnd == NULL)
        return 0;

    //AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HMODULE hModule = NULL;
    HHOOK hHook = NULL;
    DWORD dwThreadID = GetWindowThreadProcessId(hWnd, NULL);
    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)SetHookEx, &hModule))
    {
        _ASSERT(hModule);

        HOOKPROC HookProc = (HOOKPROC)GetProcAddress(hModule, "HookCallWndProc");

        hHook = ::SetWindowsHookEx(WH_GETMESSAGE, HookProc, hModule, dwThreadID);
        //SetWindowsHookEx(WH_CALLWNDPROC, HookProc, hDll, GetWindowThreadProcessId(hWnd, NULL));
        if (hHook == NULL)
        {
            DWORD dwError = GetLastError();
            CString str;
            str.Format(_T("dwError:%d\r\n"), dwError);
        }
    }

    return hHook;
}

#if 0
void InjectCode(DWORD dwProcId, LPVOID mFunc, LPVOID param, DWORD paramSize)
{
    HANDLE hProcess;//远程句柄
    LPVOID mFuncAddr;//申请函数内存地址   
    LPVOID ParamAddr;//申请参数内存地址
    HANDLE hThread;   //线程句柄
    DWORD NumberOfByte; //辅助返回值
    CString str;

    //打开被注入的进程句柄 
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcId);

    //申请内存
    mFuncAddr = VirtualAllocEx(hProcess, NULL, 128, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    ParamAddr = VirtualAllocEx(hProcess, NULL, paramSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    //写内存
    WriteProcessMemory(hProcess, mFuncAddr, mFunc, 128, &NumberOfByte);
    WriteProcessMemory(hProcess, ParamAddr, param, paramSize, &NumberOfByte);

    //创建远程线程
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)mFuncAddr, ParamAddr, 0, &NumberOfByte);

    WaitForSingleObject(hThread, INFINITE); //等待线程结束

    //释放申请有内存
    VirtualFreeEx(hProcess, mFuncAddr, 128, MEM_RELEASE);
    VirtualFreeEx(hProcess, ParamAddr, paramSize, MEM_RELEASE);

    //释放远程句柄
    CloseHandle(hThread);
    CloseHandle(hProcess);
}
#endif

static VOID Dbg(TCHAR* pszStr, ...)
{
#define DEBUG_FLAG _T("[DEBUG] ")
    static size_t nDebugLen = _tcsclen(DEBUG_FLAG);
    TCHAR szData[MAX_PATH] = DEBUG_FLAG;

    va_list args;
    va_start(args, pszStr);
    _vsntprintf(szData + nDebugLen, _tcsclen(szData) - sizeof(DEBUG_FLAG), pszStr, args);
    va_end(args);

    wcscat(szData, _T("\n"));

    OutputDebugString(szData);
}

static TCHAR g_szCurWorkDir[MAX_PATH] = { 0 };
// 获取工作路径
static CString GetWorkDir()
{
    if (wcslen(g_szCurWorkDir) > 0)
        return g_szCurWorkDir;

    int nPos = GetCurrentDirectory(MAX_PATH, g_szCurWorkDir);
    if (nPos < 0) {
        g_szCurWorkDir[0] = _T('\0');
        return CString(_T(""));
    }

    return g_szCurWorkDir;
}

static TCHAR g_szCurModuleDir[MAX_PATH] = { 0 };
// 获取项目所在的路径
static CString GetModuleDir()
{
    if (wcslen(g_szCurModuleDir) > 0)
        return g_szCurModuleDir;

    HMODULE module = GetModuleHandle(NULL);
    GetModuleFileName(module, g_szCurModuleDir, MAX_PATH);

    CString csFullPath(g_szCurModuleDir);
    //int nPos = csFullPath.ReverseFind(_T('\\'));
    int nPos = csFullPath.Find(_T('.'));
    if (nPos < 0) {
        g_szCurModuleDir[0] = _T('\0');
        return CString(_T(""));
    }

    wcscpy(g_szCurModuleDir, csFullPath.Left(nPos));
    return g_szCurModuleDir;
}

/********************************************************************************
函数 名：GetFileNameByPath(CString filePath)
功能描述：获取文件名
入 参：filePath：指定文件路径 ,
isWithFileSuffix:FALSE不带后缀名，TRUE带后缀名
出 参：无
返 回 值：文件名
*********************************************************************************/
static CString GetFileName(const CString &filePath, BOOL isWithFileSuffix = TRUE)
{
    int pos = filePath.ReverseFind('\\');
    CString fileName = filePath.Right(filePath.GetLength() - pos - 1);
    if (!isWithFileSuffix)
    {
        pos = fileName.Find('.');
        fileName = fileName.Left(pos);
    }
    return fileName;
}

static CString FormatFileSize(ULONGLONG fileS) {
    CString strFileSize;

    if (fileS < 1024) {
        strFileSize.Format(_T("%.2fB"), fileS);
    }
    else if (fileS < 1048576) {
        strFileSize.Format(_T("%.2fK"), (double)fileS / 1024.0);
    }
    else if (fileS < 1073741824) {
        strFileSize.Format(_T("%.2fM"), (double)fileS / 1048576);
    }
    else {
        strFileSize.Format(_T("%.2fG"), (double)fileS / 1073741824);
    }
    return strFileSize;
}

static CString GetDesignatedDiskFreeSpace(const CString &strPath)
{
    ULARGE_INTEGER uiFreeBytesAvailableToCaller;
    ULARGE_INTEGER uiTotalNumberOfBytes;
    ULARGE_INTEGER uiTotalNumberOfFreeBytes;

    if (GetDiskFreeSpaceEx(strPath, &uiFreeBytesAvailableToCaller,
        &uiTotalNumberOfBytes, &uiTotalNumberOfFreeBytes))
    {
        CString strTotal = FormatFileSize(uiTotalNumberOfBytes.QuadPart);
        CString strFree = FormatFileSize(uiFreeBytesAvailableToCaller.QuadPart);
        CString strUsed = FormatFileSize(uiTotalNumberOfBytes.QuadPart - uiFreeBytesAvailableToCaller.QuadPart);

        /*DWORD dwTotalDiskSpace, dwFreeDiskSpace, dwUsedDiskSpace;
        dwTotalDiskSpace = (DWORD)(uiTotalNumberOfBytes.QuadPart / 1024 / 1024);
        dwFreeDiskSpace = (DWORD)(uiFreeBytesAvailableToCaller.QuadPart >> 20);
        dwUsedDiskSpace = dwTotalDiskSpace - dwFreeDiskSpace;*/

        TCHAR szTmp[128];
        _stprintf(szTmp, _T("总空间%ls, 已用%ls, 可用%ls\n"), strTotal, strUsed, strFree);

        return szTmp;
    }

    return _T("");
}

static void ResizeWindow(HWND hWnd, POINT &pointOld, CFont *font)
{
    if (!hWnd)
        return;

    FLOAT fsp[2];
    CWnd *pCWndChild, *pCWndParent = CWnd::FromHandle(hWnd);
    CPoint pointOldTL, pointTL; // left top
    CPoint pointOldBR, pointBR; // right bottom
    POINT pointNew;
    CRect rect, recta;
    ::GetClientRect(hWnd, &recta);
    pointNew.x = recta.Width();
    pointNew.y = recta.Height();
    fsp[0] = (FLOAT)pointNew.x / pointOld.x;
    fsp[1] = (FLOAT)pointNew.y / pointOld.y;

#if 0
    if (font) {
        LOGFONT lf;
        font->GetLogFont(&lf);

        TCHAR szLog[MAX_PATH];
        _stprintf(szLog, _T(" src: %ld, %ld, %f, %f"), lf.lfWeight, lf.lfHeight, fsp[0], fsp[1]);
        OutputDebugString(szLog);

        lf.lfWeight = LONG(lf.lfWeight * fsp[1]);
        lf.lfHeight = LONG(lf.lfHeight * fsp[1]);

        _stprintf(szLog, _T(" dst: %ld, %ld, ppp: %d, %d, %d, %d\n\n"), lf.lfWeight, lf.lfHeight, pointNew.x, pointNew.y, pointOld.x, pointOld.y);
        OutputDebugString(szLog);

        font->DeleteObject();
        font->CreateFontIndirect(&lf);
    }
#endif

    HWND hwndChild = ::GetWindow(hWnd, GW_CHILD);
    while (hwndChild) {
        pCWndChild = CWnd::FromHandle(hwndChild);
        pCWndChild->GetWindowRect(rect);
        pCWndParent->ScreenToClient(&rect);
        pointOldTL = rect.TopLeft();
        pointTL.x = LONG(pointOldTL.x*fsp[0] + 0.5);
        pointTL.y = LONG(pointOldTL.y*fsp[1] + 0.5);
        pointOldBR = rect.BottomRight();
        pointBR.x = LONG(pointOldBR.x*fsp[0] + 0.5);
        pointBR.y = LONG(pointOldBR.y*fsp[1] + 0.5);
        rect.SetRect(pointTL, pointBR);
        pCWndChild->SetWindowPos(NULL, pointTL.x, pointTL.y, pointBR.x - pointTL.x, pointBR.y - pointTL.y, SWP_NOZORDER);
        if (font)
            pCWndChild->SetFont(font);
        hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
    }
    pointOld = pointNew;
}

static TCHAR g_szProgressBarFile[MAX_PATH] = { 0 };
static CString GetProgressBarFile() {
    if (g_szProgressBarFile[0] != _T(''))
        return g_szProgressBarFile;

    wcscpy(g_szProgressBarFile, GetModuleDir() + _T("res\\progress-bar.png"));
    return g_szProgressBarFile;
}

static TCHAR g_szProgressFile[MAX_PATH] = { 0 };
static CString GetProgressFile() {
    if (g_szProgressFile[0] != _T(''))
        return g_szProgressFile;

    wcscpy(g_szProgressFile, GetModuleDir() + _T("res\\progress.png"));
    return g_szProgressFile;
}

static CHAR g_szSecsTimeFormat[12] = { 0 };
static CHAR* GetSecsTimeFormat(double dTotalTimeSecs) {
    int nMin = dTotalTimeSecs / 60;

    double dOrig = dTotalTimeSecs;
    int nTemp = (int)dOrig;
    int nResult = nTemp % 60;
    double nSecs = dOrig - nTemp + nResult;

    sprintf_s(g_szSecsTimeFormat, "%02d:%02d", nMin, (int)nSecs);

    return g_szSecsTimeFormat;
}

//////////////////////////////////////////////////////////////////////////
/// 从资源文件中加载图片
/// @param [in] pImage 图片指针
/// @param [in] nResID 资源号
/// @param [in] lpTyp 资源类型
//////////////////////////////////////////////////////////////////////////
static BOOL LoadImageFromResource(IN OUT CImage* pImage, IN UINT nResID, IN LPCTSTR lpTyp)
{
    if (pImage == NULL) return FALSE;

    pImage->Destroy();

    HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
    if (hRsrc == NULL) return FALSE;

    HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
    if (hImgData == NULL) {
        return FALSE;
    }

    // 锁定内存中的指定资源
    LPVOID lpVoid = ::LockResource(hImgData);

    LPSTREAM pStream = NULL;
    DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
    HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
    if (hNew == NULL) {
        ::FreeResource(hImgData);
        return FALSE;
    }
    LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
    ::memcpy(lpByte, lpVoid, dwSize);

    // 从指定内存创建流对象
    HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
    if (ht == S_OK) {
        ht = pImage->Load(pStream);
    }

    // 解除内存中的指定资源
    ::GlobalUnlock(hNew);
    ::GlobalFree(hNew);
    ::FreeResource(hImgData);

    if (ht == S_OK) {
        // 对于CImage在处理png图片时，如果png图片中alpha通道，则加载完后要对透明色做一个处理
        if (pImage->GetBPP() == 32)
        {
            for (int i = 0; i < pImage->GetWidth(); i++)
            {
                for (int j = 0; j < pImage->GetHeight(); j++)
                {
                    unsigned char* pucColor = reinterpret_cast<unsigned char *>(pImage->GetPixelAddress(i, j));
                    pucColor[0] = pucColor[0] * pucColor[3] / 255;
                    pucColor[1] = pucColor[1] * pucColor[3] / 255;
                    pucColor[2] = pucColor[2] * pucColor[3] / 255;
                }
            }
        }
    }

    return ht == S_OK ? TRUE : FALSE;
}

static BOOL StringSplitWith(CString source, CStringArray &dest, TCHAR division)
{
    if (source.IsEmpty())
    {
        return FALSE;
    }

    int pos = source.Find(division);
    if (pos == -1)
    {
        dest.Add(source);
    }
    else
    {
        dest.Add(source.Left(pos));
        source = source.Mid(pos + 1);
        StringSplitWith(source, dest, division);
    }

    return TRUE;
}

static BOOL isFloatZero(float fVal)
{
    return fabsf(fVal) <= 0.00001f;
}
