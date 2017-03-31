
// MultiChannelVideo.h : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Common.h"

#define CAPTURE_WIDTH           1920
#define CAPTURE_HEIGHT          1080

#define RECORD_WIDTH            1920
#define RECORD_HEIGHT           1080

#define FRAME_DURATION_60       166667
#define FRAME_DURATION_25       400000
#define FRAME_DURATION_15       666667
#define FRAME_DURATION_1        1000000

#define AUDIO_SAMPLESPERSEC     48000
#define AUDIO_BUFFERED_FRAMES   10

#define RETURN_BOOL_IF_ERROR(x) if (x != QCAP_RS_SUCCESSFUL) return FALSE;

class CVideoChannel
{
public:
    CVideoChannel() {
        ZeroMemory(m_szDeviceName, sizeof(m_szDeviceName));
        ZeroMemory(m_szRecordInputFile, sizeof(m_szRecordInputFile));
        ZeroMemory(m_szRecordOutputFile, sizeof(m_szRecordOutputFile));
        m_hVideoWnd                 = NULL;
        m_dwRecordNotifyThreadID    = 0;
        m_bRecordAudio              = TRUE;
        m_bAudioMonitor             = FALSE;
        m_hVideoDevice              = NULL;

        m_nRecordState              = 0x00000000;
        InitializeCriticalSection(&m_hShareRecordAccessCriticalSection);
        m_nRecordDoneCount          = 0;

        SetRectEmpty(&m_rcPosition);

        m_nVideoWidth               = CAPTURE_WIDTH;
        m_nVideoHeight              = CAPTURE_HEIGHT;
        m_nDeviceVideoWidth         = 0;
        m_nDeviceVideoHeight        = 0;
        m_dDeviceVideoFrameRate     = 0.0;
        m_nBitRate                  = 4 * 1024 * 1024;
        m_bVideoIsInterleaved       = FALSE;
        m_dVideoFrameRate           = 60.0;
        m_nFrameDuration            = FRAME_DURATION_60;
        m_dwVideoFramesPerSecond    = 0;
        m_dwVideoFrameCount         = 0;

        m_nAudioChannels            = 2;
        m_nAudioBitsPerSample       = 16;
        m_nAudioSampleFrequency     = AUDIO_SAMPLESPERSEC;

        m_bNoSignal                 = FALSE;
        m_bOutSignalChanged         = FALSE;
        m_bCanPreview               = FALSE;
        m_bSupportGPU               = FALSE;

        m_nSharpness                = 128;
        m_nSaturation               = 128;
        m_nHue                      = 128;
        m_nConstrast                = 128;
        m_nBrightness               = 128;
    }

    virtual ~CVideoChannel() {
        Destroy();
    }

public:
    BOOL Create(HWND hVideoWnd, int iChannel, int cx, int cy, double nFrameRate, int nFrameDuration, BOOL bHardwareEncode, BOOL bRecordAudio) {
        if (hVideoWnd == NULL)
            return FALSE;

        QRESULT qr;
        TCHAR szLog[MAX_PATH] = { 0 };

        m_hVideoWnd = hVideoWnd;
        m_iChannel = iChannel;
        m_bHardwareEncode = bHardwareEncode;
        m_bRecordAudio = bRecordAudio;
        m_dVideoFrameRate = nFrameRate;
        m_nFrameDuration = nFrameDuration;
        m_nVideoWidth = cx;
        m_nVideoHeight = cy;

        // CALLBACK FUNCTION
        //
        PF_FORMAT_CHANGED_CALLBACK          m_pFormatChangedCB = { 0 };
        PF_VIDEO_PREVIEW_CALLBACK           m_pPreviewVideoCB = { 0 };
        PF_AUDIO_PREVIEW_CALLBACK           m_pPreviewAudioCB = { 0 };
        PF_VIDEO_HARDWARE_ENCODER_CALLBACK  m_pHardwareEncoderVideoCB = { 0 };
        PF_NO_SIGNAL_DETECTED_CALLBACK      m_pNoSignalDetectedCB = { 0 };
        PF_SIGNAL_REMOVED_CALLBACK          m_pSignalRemovedCB = { 0 };
        PF_RECORD_DONE_CALLBACK             m_pRecordDoneCB = { 0 };
        PF_RECORD_FAIL_CALLBACK             m_pRecordFailCB = { 0 };
        PF_SHARE_RECORD_DONE_CALLBACK       m_pShareRecordDoneCB = { 0 };
        PF_SHARE_RECORD_FAIL_CALLBACK       m_pShareRecordFailCB = { 0 };

        // CREATE CAPTURE DEVICE
        //
        qr = QCAP_CREATE("MZ0380 PCI", m_iChannel, m_hVideoWnd, &m_hVideoDevice, TRUE);
        RETURN_BOOL_IF_ERROR(qr);

        if (m_hVideoDevice == NULL) { return FALSE; }

        //ENABLE SUB
        //qr = QCAP_SET_DEVICE_CUSTOM_PROPERTY(m_hVideoDevice, 273, 1);
        //RETURN_BOOL_IF_ERROR(qr);

        // REGISTER FORMAT CHANGED CALLBACK FUNCTION
        //
        m_pFormatChangedCB = OnProcessFormatChanged;
        qr = QCAP_REGISTER_FORMAT_CHANGED_CALLBACK(m_hVideoDevice, m_pFormatChangedCB, this);
        RETURN_BOOL_IF_ERROR(qr);

        // REGISTER PREVIEW VIDEO CALLBACK FUNCTION (Software Encoder)
        //
        m_pPreviewVideoCB = OnProcessVideoPreviewBuffer;
        qr = QCAP_REGISTER_VIDEO_PREVIEW_CALLBACK(m_hVideoDevice, m_pPreviewVideoCB, this);
        RETURN_BOOL_IF_ERROR(qr);

        // REGISTER PREVIEW AUDIO CALLBACK FUNCTION (Software Encoder)
        //
        m_pPreviewAudioCB = OnProcessAudioPreviewBuffer;
        qr = QCAP_REGISTER_AUDIO_PREVIEW_CALLBACK(m_hVideoDevice, m_pPreviewAudioCB, this);
        RETURN_BOOL_IF_ERROR(qr);

        // REGISTER NO SIGNAL DETECTED CALLBACK FUNCTION
        //
        m_pNoSignalDetectedCB = OnProcessNoSignalDetected;
        qr = QCAP_REGISTER_NO_SIGNAL_DETECTED_CALLBACK(m_hVideoDevice, OnProcessNoSignalDetected, this);
        RETURN_BOOL_IF_ERROR(qr);

        // REGISTER SIGNAL REMOVED CALLBACK FUNCTION
        //
        m_pSignalRemovedCB = OnProcessSignalRemoved;
        qr = QCAP_REGISTER_SIGNAL_REMOVED_CALLBACK(m_hVideoDevice, m_pSignalRemovedCB, this);
        RETURN_BOOL_IF_ERROR(qr);

        if (m_bHardwareEncode) {
            ULONG ulValue = 0;
            ULONG nInput = QCAP_INPUT_TYPE_SDI;

            qr = QCAP_GET_DEVICE_CUSTOM_PROPERTY(m_hVideoDevice, 8, &ulValue);
            RETURN_BOOL_IF_ERROR(qr);

            switch (ulValue) {
                case 4:
                    nInput = QCAP_INPUT_TYPE_HDMI;
                    break;
                case 64:
                    nInput = QCAP_INPUT_TYPE_SDI;
                    break;
                default: break;
            }
            qr = QCAP_SET_VIDEO_INPUT(m_hVideoDevice, nInput);
            RETURN_BOOL_IF_ERROR(qr);

            qr = QCAP_SET_VIDEO_HARDWARE_ENCODER_PROPERTY(m_hVideoDevice, 0, QCAP_ENCODER_FORMAT_H264,
                                                          QCAP_RECORD_MODE_CBR, 8000, m_nBitRate, m_dVideoFrameRate, 0, 0, QCAP_DOWNSCALE_MODE_OFF, 0, 0);
            RETURN_BOOL_IF_ERROR(qr);

            // REGISTER RECORD DONE CALLBACK FUNCTION
            //
            m_pRecordDoneCB = OnRecordDone;
            qr = QCAP_REGISTER_RECORD_DONE_CALLBACK(m_hVideoDevice, 0, m_pRecordDoneCB, this);
            RETURN_BOOL_IF_ERROR(qr);

            // REGISTER RECORD DONE CALLBACK FUNCTION
            //
            m_pRecordFailCB = OnRecordFail;
            qr = QCAP_REGISTER_RECORD_FAIL_CALLBACK(m_hVideoDevice, 0, m_pRecordFailCB, this);
            RETURN_BOOL_IF_ERROR(qr);
        }
        else {
            // REGISTER SHARE RECORD DONE CALLBACK FUNCTION
            //
            m_pShareRecordDoneCB = OnShareRecordDone;
            qr = QCAP_REGISTER_SHARE_RECORD_DONE_CALLBACK(m_iChannel, m_pShareRecordDoneCB, this);
            RETURN_BOOL_IF_ERROR(qr);

            // REGISTER SHARE RECORD DONE CALLBACK FUNCTION
            //
            m_pShareRecordFailCB = OnShareRecordFail;
            qr = QCAP_REGISTER_SHARE_RECORD_FAIL_CALLBACK(m_iChannel, m_pShareRecordFailCB, this);
            RETURN_BOOL_IF_ERROR(qr);
        }

#if 0
        // REGISTER VIDEO HARDWARE ENCODER CALLBACK FUNCTION
        //
        m_pHardwareEncoderVideoCB = OnProcessVideoHardwareEncoderBuffer;
        qr = QCAP_REGISTER_VIDEO_HARDWARE_ENCODER_CALLBACK(m_hVideoDevice, 0, m_pHardwareEncoderVideoCB, this);
        RETURN_BOOL_IF_ERROR(qr);

        qr = QCAP_SET_VIDEO_DEINTERLACE(m_hVideoDevice, TRUE);
        RETURN_BOOL_IF_ERROR(qr);

        qr = QCAP_SET_AUDIO_VOLUME(m_hVideoDevice, 100);
        RETURN_BOOL_IF_ERROR(qr);

        ULONG nWidth = 0, nHeight = 0, nColorSpaceType = 0;
        BOOL nIsInterleaved = 0;
        double nFrameRate = 0.0f;
        qr = QCAP_GET_VIDEO_CURRENT_INPUT_FORMAT(m_hVideoDevice, &nColorSpaceType, &nWidth, &nHeight, &nIsInterleaved, &nFrameRate);
        RETURN_BOOL_IF_ERROR(qr);

        qr = QCAP_SET_VIDEO_DEFAULT_OUTPUT_FORMAT(m_hVideoDevice, nColorSpaceType, m_nVideoWidth, m_nVideoHeight, TRUE, m_dVideoFrameRate);
        RETURN_BOOL_IF_ERROR(qr);
#endif

        if (!StartCapture())
            return FALSE;

        _stprintf(m_szDeviceName, _T("MZ0380 PCI %d"), iChannel);

        return TRUE;
    }

    void Destroy() {
        StopRecord();

        m_bAudioMonitor = FALSE;

        if (m_hVideoDevice) {
            QCAP_STOP(m_hVideoDevice);
            QCAP_DESTROY(m_hVideoDevice);

            m_hVideoDevice = NULL;
        }

        DeleteCriticalSection(&m_hShareRecordAccessCriticalSection);

        m_nRecordDoneCount = 0;
    }

    BOOL SetPosition(LPCRECT lprcPosition, double nFrameRate, int nFrameDuration) {
        int cxCaptureNew = (lprcPosition->right - lprcPosition->left) >> 1 << 1;
        int cyCaptureNew = (lprcPosition->bottom - lprcPosition->top) >> 1 << 1;

        int cxCaptureOld = (m_rcPosition.right - m_rcPosition.left) >> 1 << 1;
        int cyCaptureOld = (m_rcPosition.bottom - m_rcPosition.top) >> 1 << 1;

        if (cxCaptureOld != cxCaptureNew || cyCaptureOld != cyCaptureNew || m_nFrameDuration != nFrameDuration) {
            if (m_nFrameDuration != nFrameDuration) {
                m_dVideoFrameRate = nFrameRate;
                m_nFrameDuration = nFrameDuration;
            }

            m_bOutSignalChanged = TRUE;
        }
        else {
            m_bOutSignalChanged = FALSE;
        }

        CopyRect(&m_rcPosition, lprcPosition);
        m_nVideoWidth = cxCaptureNew;
        m_nVideoHeight = cyCaptureNew;

        return TRUE;
    }

    BOOL StartCapture() {
        if (!m_hVideoDevice) {
            return FALSE;
        }

        QRESULT qr = QCAP_RUN(m_hVideoDevice);
        RETURN_BOOL_IF_ERROR(qr);

        return TRUE;
    }

    BOOL StopCapture() {
        if (!m_hVideoDevice) {
            return FALSE;
        }

        QRESULT qr = QCAP_STOP(m_hVideoDevice);
        RETURN_BOOL_IF_ERROR(qr);

        return TRUE;
    }

    BOOL StartRecord(LPCTSTR lpszFileName, double dSegmentDurationTime, DWORD dwRecordNotifyThreadID, BOOL bEnableRecord, BOOL bRecordAudio) {
        if (!m_hVideoDevice || m_bNoSignal || !m_bCanPreview || lpszFileName == NULL || dwRecordNotifyThreadID == 0)
            return FALSE;

        QRESULT qr;

        CAutoConvertString acs(lpszFileName);
        CHAR *pszFileName = acs;

        m_nRecordDoneCount = 0;

        m_bEnableRecord = bEnableRecord;
        m_bRecordAudio = bRecordAudio;

        if (m_bHardwareEncode) {
            qr = QCAP_SET_VIDEO_RECORD_PROPERTY(m_hVideoDevice, 0, QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK, QCAP_ENCODER_FORMAT_H264,
                                                QCAP_RECORD_MODE_CBR, 8000, m_nBitRate, m_dVideoFrameRate, 0, 0, QCAP_DOWNSCALE_MODE_OFF);
            RETURN_BOOL_IF_ERROR(qr);

            if (m_bRecordAudio) {
                qr = QCAP_SET_AUDIO_RECORD_PROPERTY(m_hVideoDevice, 0, QCAP_ENCODER_TYPE_SOFTWARE, QCAP_ENCODER_FORMAT_AAC);
                RETURN_BOOL_IF_ERROR(qr);
            }

            qr = QCAP_START_RECORD(m_hVideoDevice, 0, pszFileName, QCAP_RECORD_FLAG_FULL, 0.0, 0.0, dSegmentDurationTime);
            RETURN_BOOL_IF_ERROR(qr);
        }
        else {
            // INITIALIZE SHARE RECORDING RESOURCE
            //
            qr = QCAP_SET_VIDEO_SHARE_RECORD_PROPERTY(m_iChannel, QCAP_ENCODER_TYPE_INTEL_MEDIA_SDK, QCAP_ENCODER_FORMAT_H264,
                                                      QCAP_COLORSPACE_TYEP_YV12, m_nVideoWidth, m_nVideoHeight, m_dVideoFrameRate,
                                                      QCAP_RECORD_MODE_CBR, 8000, m_nBitRate, m_dVideoFrameRate, 0, 0, NULL, TRUE, FALSE);

            if (qr != QCAP_RS_SUCCESSFUL) {
                qr = QCAP_SET_VIDEO_SHARE_RECORD_PROPERTY(m_iChannel, QCAP_ENCODER_TYPE_NVIDIA_CUDA, QCAP_ENCODER_FORMAT_H264,
                                                          QCAP_COLORSPACE_TYEP_YV12, m_nVideoWidth, m_nVideoHeight, m_dVideoFrameRate,
                                                          QCAP_RECORD_MODE_CBR, 8000, m_nBitRate, m_dVideoFrameRate, 0, 0, NULL, TRUE, FALSE);

                Dbg(_T("StartRecord %d set intel failed"), m_iChannel);
            }

            if (qr != QCAP_RS_SUCCESSFUL) {
                qr = QCAP_SET_VIDEO_SHARE_RECORD_PROPERTY(m_iChannel, QCAP_ENCODER_TYPE_NVIDIA_NVENC, QCAP_ENCODER_FORMAT_H264,
                                                          QCAP_COLORSPACE_TYEP_YV12, m_nVideoWidth, m_nVideoHeight, m_dVideoFrameRate,
                                                          QCAP_RECORD_MODE_CBR, 8000, m_nBitRate, m_dVideoFrameRate, 0, 0, NULL, TRUE, FALSE);

                Dbg(_T("StartRecord %d set nv cuda failed"), m_iChannel);
            }

            RETURN_BOOL_IF_ERROR(qr);

            if (m_bRecordAudio) {
                qr = QCAP_SET_AUDIO_SHARE_RECORD_PROPERTY(m_iChannel, QCAP_ENCODER_TYPE_SOFTWARE, QCAP_ENCODER_FORMAT_AAC, 
                                                          m_nAudioChannels, m_nAudioBitsPerSample, m_nAudioSampleFrequency);
                RETURN_BOOL_IF_ERROR(qr);
            }

            qr = QCAP_START_SHARE_RECORD(m_iChannel, pszFileName, QCAP_RECORD_FLAG_FULL | QCAP_RECORD_FLAG_VIDEO_USE_MEDIA_TIMER |
                                         QCAP_RECORD_FLAG_AUDIO_USE_MEDIA_TIMER, 0.0, 0.0, dSegmentDurationTime);
        }
        
        if (qr != QCAP_RS_SUCCESSFUL) {
            _tremove(lpszFileName);
            return FALSE;
        }

        m_dwRecordNotifyThreadID = dwRecordNotifyThreadID;

        wcscpy(m_szRecordInputFile, lpszFileName);

        EnterCriticalSection(&m_hShareRecordAccessCriticalSection);
        m_nRecordState = 0x00000001;
        LeaveCriticalSection(&m_hShareRecordAccessCriticalSection);

        m_bOutSignalChanged = FALSE;

        return TRUE;
    }

    BOOL StopRecord() {
        if (!m_hVideoDevice)
            return FALSE;

        QRESULT qr = QCAP_RS_SUCCESSFUL;
        
        if (m_bHardwareEncode) {
            qr = QCAP_STOP_RECORD(m_hVideoDevice, 0);
        }
        else {
            qr = QCAP_STOP_SHARE_RECORD(m_iChannel);
        }

        RETURN_BOOL_IF_ERROR(qr);

        while (m_nRecordState > 0x00000000) {
            Sleep(10);
        }

        return TRUE;
    }

    void GetPosition(LPRECT lprcPosition) {
        CopyRect(lprcPosition, &m_rcPosition);
    }

    int GetChannelIndex() {
        return m_iChannel;
    }

    LPCTSTR GetDeviceName() {
        return m_szDeviceName;
    }

    LPCTSTR GetRecordInputFile() {
        return m_szRecordInputFile;
    }

    LPCTSTR GetRecordOutputFile() {
        return m_szRecordOutputFile;
    }

    ULONG GetCurRecordNum() {
        return m_nRecordDoneCount;
    }

    void Repaint() {
    }

    void UpdateRendererPosition(int cx, int cy) {
#if 0
        if (m_hVideoDevice) {
            RECT rcDest;
            GetClientRect(m_hWnd, &rcDest);

            // 通过指定目标矩形，可以将视频显示在窗口的部分区域
            // 通过指定源矩形，可以将视频画面剪裁后再显示
        }
#endif
    }

    BOOL IsVideoSignalChanged() {
        // TODO:
        return FALSE;
    }

    void SetAudioMonitor(BOOL bAudioMonitor) {
        m_bAudioMonitor = bAudioMonitor;
    }

    void SetEnableRecord(BOOL bEnableRecord) {
        m_bEnableRecord = bEnableRecord;
    }

    BOOL IsAudioAvailible() {
        return m_hVideoDevice != NULL && !m_bNoSignal;
    }

    void ShowVideoCapturePropertyDialog() {
        if (!m_hVideoDevice)
            return;
    }

    BOOL VideoCaptureCreateSnapShot(LPCTSTR lpszFilePath, int nQuality) {
        if (!lpszFilePath)
            return FALSE;

        if (nQuality <= 0 || nQuality > 100)
            nQuality = 100;

        CAutoConvertString acs(lpszFilePath);
        CHAR *pszFilePath = acs;

        QRESULT qr = QCAP_SNAPSHOT_JPG(m_hVideoDevice, pszFilePath, nQuality);
        RETURN_BOOL_IF_ERROR(qr);

        return TRUE;
    }

    BOOL IsCanRecord() {
        return m_hVideoDevice && !m_bNoSignal && m_bCanPreview && m_bEnableRecord;
    }

protected:
    // FORMAT CHANGED CALLBACK FUNCTION
    //
    static QRETURN OnProcessFormatChanged(PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, ULONG nVideoWidth, ULONG nVideoHeight,
                                          BOOL bVideoIsInterleaved, double dVideoFrameRate, ULONG nAudioChannels, ULONG nAudioBitsPerSample,
                                          ULONG nAudioSampleFrequency, PVOID pUserData)
    {
        if (pUserData == NULL) { return QCAP_RT_OK; }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        Dbg(_T("format changed Detected, %d\n"), pThis->m_iChannel);

        pThis->m_nDeviceVideoWidth = nVideoWidth;
        pThis->m_nDeviceVideoHeight = nVideoHeight;
        pThis->m_bVideoIsInterleaved = bVideoIsInterleaved;
        pThis->m_dDeviceVideoFrameRate = dVideoFrameRate;
        pThis->m_nAudioChannels = nAudioChannels;
        pThis->m_nAudioBitsPerSample = nAudioBitsPerSample;
        pThis->m_nAudioSampleFrequency = nAudioSampleFrequency;

        // OUTPUT FORMAT CHANGED MESSAGE
        //
        CHAR strVideoInput[MAX_PATH] = { 0 };
        CHAR strAudioInput[MAX_PATH] = { 0 };
        CHAR strFrameType[MAX_PATH] = { 0 };
        UINT nVH = 0;

        if (nVideoInput == 0) { sprintf(strVideoInput, "COMPOSITE"); }
        else if (nVideoInput == 1) { sprintf(strVideoInput, "SVIDEO"); }
        else if (nVideoInput == 2) { sprintf(strVideoInput, "HDMI"); }
        else if (nVideoInput == 3) { sprintf(strVideoInput, "DVI_D"); }
        else if (nVideoInput == 4) { sprintf(strVideoInput, "COMPONENTS (YCBCR)"); }
        else if (nVideoInput == 5) { sprintf(strVideoInput, "DVI_A (RGB / VGA)"); }
        else if (nVideoInput == 6) { sprintf(strVideoInput, "SDI"); }
        else if (nVideoInput == 7) { sprintf(strVideoInput, "AUTO"); }

        if (nAudioInput == 0) { sprintf(strAudioInput, "EMBEDDED_AUDIO"); }
        else if (nAudioInput == 1) { sprintf(strAudioInput, "LINE_IN"); }

        if (bVideoIsInterleaved == TRUE) { nVH = nVideoHeight / 2; }
        else { nVH = nVideoHeight; }

        if (bVideoIsInterleaved == TRUE) { sprintf(strFrameType, " I "); }
        else { sprintf(strFrameType, " P "); }

        /*Format(_T("INFO : %d x %d%s @%2.3f FPS , %d CH x %d BITS x %d HZ ,  VIDEO INPUT : %s ,  AUDIO INPUT : %s"),
            nVideoWidth, nVH, strFrameType, dVideoFrameRate,
            nAudioChannels, nAudioBitsPerSample, nAudioSampleFrequency,
            strVideoInput, strAudioInput);*/

        // NO SOURCE
        //
        if (nVideoWidth == 0 || nVideoHeight == 0 || dVideoFrameRate == 0.0 ||
            nAudioChannels == 0 || nAudioBitsPerSample == 0 || nAudioSampleFrequency == 0) {
            pThis->m_bNoSignal = TRUE;
        }
        else {
            pThis->m_bNoSignal = FALSE;
        }

        return QCAP_RT_OK;
    }

    // NO SIGNAL DETEACTED CALLBACK FUNCTION
    //
    static QRETURN OnProcessNoSignalDetected(PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, PVOID pUserData)
    {
        if (pUserData == NULL) { return QCAP_RT_OK; }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        Dbg(_T("No Signal Detected, %d\n"), pThis->m_iChannel);

        pThis->m_bNoSignal = TRUE;

        return QCAP_RT_OK;
    }

    // SIGNAL REMOVED CALLBACK FUNCTION
    //
    static QRETURN OnProcessSignalRemoved(PVOID pDevice, ULONG nVideoInput, ULONG nAudioInput, PVOID pUserData)
    {
        if (pUserData == NULL) { return QCAP_RT_OK; }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        Dbg(_T("Signal Removed, %d\n"), pThis->m_iChannel);

        pThis->m_bNoSignal = TRUE;

        return QCAP_RT_OK;
    }

    // SIGNAL RECORD DONE CALLBACK FUNCTION
    //
    static QRETURN OnRecordDone(PVOID pDevice, UINT iRecNum, CHAR* pszFilePathName, PVOID pUserData)
    {
        if (pUserData == NULL) { return QCAP_RT_OK; }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        CAutoConvertString acs(pszFilePathName ? pszFilePathName : "");
        TCHAR *pszFileName = acs;

        Dbg(_T("Record Done. doneCount:%lu, oldName:%ls, newName:%ls\n"), pThis->m_nRecordDoneCount, pThis->m_szRecordOutputFile, pszFileName);

        //if (_tcscmp(pThis->m_szRecordOutputFile, pszFileName) != 0)
        {
            pThis->m_nRecordDoneCount++;

            RecordInfo* pRecordInfo = new RecordInfo;
            pRecordInfo->iChannel = pThis->m_iChannel;
            pRecordInfo->ulCurRecordNum = pThis->m_nRecordDoneCount;
            pRecordInfo->strFilePath = pszFileName;

            PostThreadMessage(pThis->m_dwRecordNotifyThreadID, WM_MSG_RECORD_DONE, NULL, (LPARAM)pRecordInfo);

            wcscpy(pThis->m_szRecordOutputFile, pszFileName);
        }

        EnterCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);
        pThis->m_nRecordState = 0x00000000;
        LeaveCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

        return QCAP_RT_OK;
    }

    // SIGNAL RECORD FAIL CALLBACK FUNCTION
    //
    static QRETURN OnRecordFail(PVOID pDevice, UINT iRecNum, CHAR* pszFilePathName, QRESULT nErrorStatus, PVOID pUserData)
    {
        if (pUserData == NULL) { return QCAP_RT_OK; }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        CAutoConvertString acs(pszFilePathName ? pszFilePathName : "");
        TCHAR *pszFileName = acs;

        Dbg(_T("Record Fail. doneCount:%lu, oldName:%ls, newName:%ls\n"), pThis->m_nRecordDoneCount, pThis->m_szRecordOutputFile, pszFileName);

        //if (_tcscmp(pThis->m_szRecordOutputFile, pszFileName) != 0)
        {
            pThis->m_nRecordDoneCount++;

            RecordInfo* pRecordInfo = new RecordInfo;
            pRecordInfo->iChannel = pThis->m_iChannel;
            pRecordInfo->ulCurRecordNum = pThis->m_nRecordDoneCount;
            pRecordInfo->strFilePath = pszFileName;

            PostThreadMessage(pThis->m_dwRecordNotifyThreadID, WM_MSG_RECORD_DONE, NULL, (LPARAM)pRecordInfo);

            wcscpy(pThis->m_szRecordOutputFile, pszFileName);
        }

        EnterCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);
        pThis->m_nRecordState = 0x00000000;
        LeaveCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

        return QCAP_RT_OK;
    }

    // SIGNAL SHARED RECORD DONE CALLBACK FUNCTION
    //
    static QRETURN OnShareRecordDone(UINT iRecNum, CHAR* pszFilePathName, PVOID pUserData)
    {
        if (pUserData == NULL) { return QCAP_RT_OK; }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        CAutoConvertString acs(pszFilePathName ? pszFilePathName : "");
        TCHAR *pszFileName = acs;

        Dbg(_T("Record Done. doneCount:%lu, oldName:%ls, newName:%ls\n"), pThis->m_nRecordDoneCount, pThis->m_szRecordOutputFile, pszFileName);

        //if (_tcscmp(pThis->m_szRecordOutputFile, pszFileName) != 0)
        {
            pThis->m_nRecordDoneCount++;

            RecordInfo* pRecordInfo = new RecordInfo;
            pRecordInfo->iChannel = iRecNum;
            pRecordInfo->ulCurRecordNum = pThis->m_nRecordDoneCount;
            pRecordInfo->strFilePath = pszFileName;

            PostThreadMessage(pThis->m_dwRecordNotifyThreadID, WM_MSG_RECORD_DONE, NULL, (LPARAM)pRecordInfo);

            wcscpy(pThis->m_szRecordOutputFile, pszFileName);
        }

        EnterCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);
        pThis->m_nRecordState = 0x00000000;
        LeaveCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

        return QCAP_RT_OK;
    }

    // SIGNAL SHARED RECORD FAIL CALLBACK FUNCTION
    //
    static QRETURN OnShareRecordFail(UINT iRecNum, CHAR* pszFilePathName, QRESULT nErrorStatus, PVOID pUserData)
    {
        if (pUserData == NULL) { return QCAP_RT_OK; }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        CAutoConvertString acs(pszFilePathName ? pszFilePathName : "");
        TCHAR *pszFileName = acs;
        
        Dbg(_T("Record Fail. doneCount:%lu, oldName:%ls, newName:%ls\n"), pThis->m_nRecordDoneCount, pThis->m_szRecordOutputFile, pszFileName);

        //if (_tcscmp(pThis->m_szRecordOutputFile, pszFileName) != 0)
        {
            pThis->m_nRecordDoneCount++;

            RecordInfo* pRecordInfo = new RecordInfo;
            pRecordInfo->iChannel = iRecNum;
            pRecordInfo->ulCurRecordNum = pThis->m_nRecordDoneCount;
            pRecordInfo->strFilePath = pszFileName;

            PostThreadMessage(pThis->m_dwRecordNotifyThreadID, WM_MSG_RECORD_DONE, NULL, (LPARAM)pRecordInfo);

            wcscpy(pThis->m_szRecordOutputFile, pszFileName);
        }

        EnterCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);
        pThis->m_nRecordState = 0x00000000;
        LeaveCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

        return QCAP_RT_OK;
    }

    // PREVIEW VIDEO CALLBACK FUNCTION
    //
    static QRETURN OnProcessVideoPreviewBuffer(PVOID pDevice, double dSampleTime, BYTE* pFrameBuffer, ULONG nFrameBufferLen, PVOID pUserData)
    {
        if (pUserData == NULL || pFrameBuffer == NULL || nFrameBufferLen == 0) {
            return QCAP_RT_OK;
        }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;
        pThis->m_bCanPreview = TRUE;

        if (!pThis->m_bHardwareEncode) {
            EnterCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

            if (pThis->m_nRecordState > 0x00000000) {
                QCAP_SET_VIDEO_SHARE_RECORD_UNCOMPRESSION_BUFFER(pThis->m_iChannel, QCAP_COLORSPACE_TYEP_YV12,
                    pThis->m_nVideoWidth, pThis->m_nVideoHeight,
                    pFrameBuffer, nFrameBufferLen);

                pThis->m_dwVideoFramesPerSecond++;
                pThis->m_dwVideoFrameCount++;
#if 0
                if (pThis->m_dwVideoFramesPerSecond >= pThis->m_dVideoFrameRate) {
                    pThis->m_dwVideoFramesPerSecond = 0;
                }
#endif
            }

            LeaveCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);
        }
        else {
            pThis->m_dwVideoFramesPerSecond++;
            pThis->m_dwVideoFrameCount++;
#if 1
            if (pThis->m_dwVideoFramesPerSecond >= pThis->m_dDeviceVideoFrameRate) {
                pThis->m_dwVideoFramesPerSecond = 0;

                Dbg(_T("video %d preview"), pThis->m_iChannel);
            }
#endif
        }

        return QCAP_RT_OK;
    }

    // PREVIEW AUDIO CALLBACK FUNCTION
    //
    static QRETURN OnProcessAudioPreviewBuffer(PVOID pDevice, double dSampleTime, BYTE* pFrameBuffer, ULONG nFrameBufferLen, PVOID pUserData)
    {
        //Dbg("OnProcessAudioPreviewBuffer( %f, %d )", dSampleTime, nFrameBufferLen);

        if (pUserData == NULL || pFrameBuffer == NULL || nFrameBufferLen == 0) {
            return QCAP_RT_OK;
        }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        if (!pThis->m_bHardwareEncode) {
            EnterCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

            if (pThis->m_nRecordState > 0x00000000) {
                QCAP_SET_AUDIO_SHARE_RECORD_UNCOMPRESSION_BUFFER(pThis->m_iChannel, pFrameBuffer, nFrameBufferLen);

#if 0
                if (pThis->m_nAudioBitsPerSample == 16 && pThis->m_nAudioChannels == 2) {

                    double L = 0, R = 0;

                    SHORT *po = (SHORT *)(pFrameBuffer);
                    ULONG  samples = nFrameBufferLen / 4; // 16BITS L + R

                    for (ULONG i = 0; i < samples; i++) {
                        L += abs(*po++);
                        R += abs(*po++);
                    }
                    L /= samples;
                    R /= samples;

                    double DB_L = 20 * log10(L / 32768.0);
                    double DB_R = 20 * log10(R / 32768.0);

                    pThis->m_dAudioVolumeDbLeft = DB_L;
                    pThis->m_dAudioVolumeDbRight = DB_R;
                }
#endif
            }

            LeaveCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);
        }

        return QCAP_RT_OK;
    }

    // VIDEO HARDWARE ENCODER#0 CALLBACK FUNCTION
    //
    static QRETURN OnProcessVideoHardwareEncoderBuffer(PVOID pDevice, UINT iRecNum, double dSampleTime,
                                                       BYTE* pFrameBuffer, ULONG nFrameBufferLen, BOOL bIsKeyFrame, PVOID pUserData)
    {
        if (pUserData == NULL || pFrameBuffer == NULL || nFrameBufferLen == 0) {
            return QCAP_RT_OK;
        }

        CVideoChannel* pThis = (CVideoChannel *)pUserData;

        EnterCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

        if (pThis->m_nRecordState > 0x00000000) {
            QCAP_SET_VIDEO_SHARE_RECORD_COMPRESSION_BUFFER(iRecNum, pFrameBuffer, nFrameBufferLen, bIsKeyFrame, dSampleTime);
        }

        LeaveCriticalSection(&pThis->m_hShareRecordAccessCriticalSection);

        return QCAP_RT_OK;
    }

    static QRETURN OnVideoShareRecordMediaTimer(UINT iRecNum, double dSampleTime, double dDelayTime, PVOID pUserData)
    {
        return QCAP_RT_OK;
    }

protected:
    HWND                m_hVideoWnd;
    DWORD               m_dwRecordNotifyThreadID;
    BOOL                m_bHardwareEncode;
    BOOL                m_bRecordAudio;
    BOOL                m_bAudioMonitor;
    RECT                m_rcPosition;
    int                 m_iChannel;
    TCHAR               m_szDeviceName[MAX_PATH];
    TCHAR               m_szRecordInputFile[MAX_PATH];
    TCHAR               m_szRecordOutputFile[MAX_PATH];

    CRITICAL_SECTION    m_hShareRecordAccessCriticalSection;
    volatile ULONG      m_nRecordState;
    ULONG               m_nRecordDoneCount;

    ///////////////////////////////////////////////////////////////////// VIDEO PROPERTIES

    PVOID               m_hVideoDevice;
    ULONG               m_nVideoWidth;              /* dest width  */
    ULONG               m_nVideoHeight;             /* dest height */
    ULONG               m_nDeviceVideoWidth;        /* src width   */
    ULONG               m_nDeviceVideoHeight;       /* src height  */
    double              m_dDeviceVideoFrameRate;    /* src fps  */
    ULONG               m_nBitRate;
    BOOL                m_bVideoIsInterleaved;
    double              m_dVideoFrameRate;
    int                 m_nFrameDuration;
    DWORD               m_dwVideoFramesPerSecond;
    DWORD               m_dwVideoFrameCount;

    BOOL                m_bNoSignal;
    BOOL                m_bOutSignalChanged;
    BOOL                m_bCanPreview;
    BOOL                m_bSupportGPU;
    BOOL                m_bEnableRecord;

    ///////////////////////////////////////////////////////////////////// AUDIO PROPERTIES

    ULONG               m_nAudioChannels;
    ULONG               m_nAudioBitsPerSample;
    ULONG               m_nAudioSampleFrequency;

    double              m_dAudioVolumeDbLeft;
    double              m_dAudioVolumeDbRight;

    ///////////////////////////////////////////////////////////////////// QUALITY PROPERTIES

    ULONG               m_nSharpness;
    ULONG               m_nSaturation;
    ULONG               m_nHue;
    ULONG               m_nConstrast;
    ULONG               m_nBrightness;
};

class CMultiAVChannel
{
public:
    CMultiAVChannel() {
        m_bVideoPreview         = TRUE;
        m_nChannelRecordCount   = 0;
    }

    virtual ~CMultiAVChannel() {
        StopCaptures();
        Destroy();
    }

public:
    BOOL Init() {
        return TRUE;
    }

    BOOL Create(HWND hVideoWnd, int iChannel, int cx, int cy, double nFrameRate, int nFrameDuration, BOOL bHardwareEncode = TRUE, BOOL bRecordAudio = TRUE, BOOL bAudioMonitor = FALSE) {
        if (hVideoWnd == NULL || iChannel < 0) {
            return FALSE;
        }

        CVideoChannel* pChannel = new CVideoChannel();
        if (NULL != pChannel) {
            if (pChannel->Create(hVideoWnd, iChannel, cx, cy, nFrameRate, nFrameDuration, bHardwareEncode, bRecordAudio)) {

                pChannel->SetAudioMonitor(bAudioMonitor);
                m_arrChannels.Add(pChannel);
            }
            else {
                delete pChannel;
                return FALSE;
            }
        }

        return TRUE;
    }

    void Destroy() {
        int c = m_arrChannels.GetCount();

        // Destroy video channels
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            delete pChannel;
        }
        m_arrChannels.RemoveAll();
        m_nChannelRecordCount = 0;
    }

    BOOL LayoutVideo(int cx, int cy, double nFrameRate, int nFrameDuration) {
        int c = m_arrChannels.GetCount();
        if (c == 0)
            return FALSE;

        RECT rcChannel;

        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel) {
                rcChannel.left = 0;
                rcChannel.top = 0;
                rcChannel.right = cx;
                rcChannel.bottom = cy;

                if (pChannel->SetPosition(&rcChannel, nFrameRate, nFrameDuration))
                    pChannel->UpdateRendererPosition(cx, cy);
            }
        }

        return TRUE;
    }

    void Repaint() {
        int c = m_arrChannels.GetCount();
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel) {
                pChannel->Repaint();
            }
        }
    }

    void UpdateRendererPosition(int cx, int cy) {
        int c = m_arrChannels.GetCount();
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel) {
                pChannel->UpdateRendererPosition(cx, cy);
            }
        }
    }

    int GetAvailableChannelCount() {
        return m_arrChannels.GetCount();
    }

    BOOL IsCanRecord(int nChannelIndex) {
        CVideoChannel* pChannel = GetVideoChannel(nChannelIndex);

        if (!pChannel)
            return FALSE;

        return pChannel->IsCanRecord();
    }

    int GetChannelRecordCount() {
        int nChannelRecordCount = 0;
        int c = m_arrChannels.GetCount();

        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel && pChannel->IsCanRecord()) {
                nChannelRecordCount++;
            }
        }

        m_nChannelRecordCount = nChannelRecordCount;
        return m_nChannelRecordCount;
    }

    void SetChannelEnableRecord(int nChannelIndex, BOOL bEnableRecord) {
        CVideoChannel* pChannel = GetVideoChannel(nChannelIndex);

        if (!pChannel)
            return ;

        pChannel->SetEnableRecord(bEnableRecord);
    }

    CVideoChannel* GetVideoChannel(int nChannelIndex) {
        int c = m_arrChannels.GetCount();

        CVideoChannel* pChannel = NULL;
        for (int i = 0; i < c; i++) {
            pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel && nChannelIndex == pChannel->GetChannelIndex())
                break;
        }

        return pChannel;
    }

    BOOL StartRecord(int nChannelIndex, LPCTSTR lpszFileName, double dSegmentDurationTime, DWORD dwRecordNotifyThreadID, BOOL bEnableRecord, BOOL bRecordAudio) {
        int c = m_arrChannels.GetCount();
        if (nChannelIndex < 0 || nChannelIndex >= c || !lpszFileName || dwRecordNotifyThreadID == 0)
            return FALSE;

        CVideoChannel* pChannel = GetVideoChannel(nChannelIndex);
        if (!pChannel)
            return FALSE;

        BOOL bRet = pChannel->StartRecord(lpszFileName, dSegmentDurationTime, dwRecordNotifyThreadID, bEnableRecord, bRecordAudio);

        return bRet;
    }

    BOOL StopRecord(CArray<RecordInfo> *arrOkChannels = NULL) {
        int c = m_arrChannels.GetCount();
        int iOkNum = 0;
#if 0
        CArray<RecordInfo> arrOkChannelsTmp;
        if (arrOkChannels == NULL) {
            arrOkChannels = &arrOkChannelsTmp;
        }
#endif
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel && pChannel->StopRecord()) {
                if (arrOkChannels) {
                    RecordInfo recordInfo;
                    recordInfo.iChannel = pChannel->GetChannelIndex();
                    recordInfo.ulCurRecordNum = pChannel->GetCurRecordNum();
#if RECORD_SPLIT_ONTIMER
                    recordInfo.strFilePath = pChannel->GetRecordInputFile();
#else
                    recordInfo.strFilePath = pChannel->GetRecordOutputFile();
#endif
                    arrOkChannels->Add(recordInfo);
                }
                iOkNum++;
            }
        }

        return iOkNum > 0 ? TRUE : FALSE;
    }

    void SetAudioMonitor(BOOL bAudioMonitor) {
        int c = m_arrChannels.GetCount();
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel)
                pChannel->SetAudioMonitor(bAudioMonitor);
        }
    }

    BOOL HasAudioAvailable() {
        int c = m_arrChannels.GetCount();
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel) {
                if (pChannel->IsAudioAvailible())
                    return TRUE;
            }
        }

        return FALSE;
    }

    BOOL IsVideoAvailable(int nChannelIndex) {
        int c = m_arrChannels.GetCount();
        if (nChannelIndex < 0 || nChannelIndex >= c)
            return FALSE;

        CVideoChannel* pChannel = GetVideoChannel(nChannelIndex);
        if (!pChannel)
            return FALSE;

        return TRUE;
    }

    BOOL IsVideoSignalChanged() {
        int c = m_arrChannels.GetCount();
        int nChangedCount = 0;
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (pChannel && pChannel->IsVideoSignalChanged())
                nChangedCount++;
        }

        return nChangedCount > 0 ? TRUE : FALSE;
    }

    void ShowVideoCapturePropertyDialog(int nChannelIndex) {
        int c = m_arrChannels.GetCount();
        if (nChannelIndex < 0 || nChannelIndex >= c)
            return;

        CVideoChannel* pChannel = GetVideoChannel(nChannelIndex);
        if (!pChannel)
            return;

        pChannel->ShowVideoCapturePropertyDialog();
    }

    BOOL VideoCaptureCreateSnapShot(int nChannelIndex, LPCTSTR lpszFilePath, int nQuality) {
        int c = m_arrChannels.GetCount();
        if (nChannelIndex < 0 || nChannelIndex >= c || !lpszFilePath)
            return FALSE;

        CVideoChannel* pChannel = GetVideoChannel(nChannelIndex);
        if (!pChannel)
            return FALSE;

        return pChannel->VideoCaptureCreateSnapShot(lpszFilePath, nQuality);
    }

    LPCTSTR GetChannelDeviceName(int nChannelIndex) {
        int c = m_arrChannels.GetCount();
        if (nChannelIndex < 0 || nChannelIndex >= c)
            return NULL;

        CVideoChannel* pChannel = GetVideoChannel(nChannelIndex);
        if (!pChannel)
            return NULL;

        return pChannel->GetDeviceName();
    }

protected:
    void StopCaptures() {
        TCHAR szLog[MAX_PATH];
        int c = m_arrChannels.GetCount();
        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (!pChannel || !pChannel->StopCapture()) {
                _stprintf(szLog, _T("停止采集视频设备通道%d失败!\n"), i);
                AfxMessageBox(szLog);
            }
        }
    }

    void StartCaptures() {
        TCHAR szLog[MAX_PATH];
        int c = m_arrChannels.GetCount();

        for (int i = 0; i < c; i++) {
            CVideoChannel* pChannel = (CVideoChannel *)m_arrChannels[i];
            if (!pChannel || !pChannel->StartCapture()) {
                //delete pChannel;
                //m_arrChannels.RemoveAt(i);

                _stprintf(szLog, _T("开始采集视频设备通道%d失败!\n"), i);
                AfxMessageBox(szLog);
            }
        }
    }

protected:
    CPtrArray           m_arrChannels;
    BOOL                m_bVideoPreview;
    int                 m_nChannelRecordCount;
};


enum PlayState {
    PLAY_STATE_NONE,
    PLAY_STATE_INIT,
    PLAY_STATE_PLAYING,
    PLAY_STATE_PAUSED,
    PLAY_STATE_END,
    PLAY_STATE_STOP,
};

#define READ_PROGRESS_IMG_FROM_RES       1
#define PROGRESS_BAR_BASE_PIXEL_WIDTH    1280
#define PROGRESS_BAR_BASE_PIXEL_HEIGHT   800

class CAVChannelPlay
{
public:
    CAVChannelPlay() {
        m_nRepeatStatus             = 0;
        m_pFile                     = NULL;
        m_nFileVideoFormat          = 0;
        m_nFileVideoWidth           = 0;
        m_nFileVideoHeight          = 0;
        m_dFileVideoFrameRate       = 0;
        m_nFileAudioFormat          = 0;
        m_nFileAudioChannel         = 0;
        m_nFileAudioBitsPerSample   = 0;
        m_nFileAudioSampleFrequency = 0;
        m_dFileTotalDuationTime     = 0;
        m_nFileTotalVideoFrames     = 0;
        m_nFileTotalAudioFrames     = 0;
        m_nFileTotalMetadataFrames  = 0;

        m_hWnd                      = NULL;
        m_font                      = NULL;
        m_ePlayState                = PLAY_STATE_NONE;
        m_dCurPlayTime              = 0;
        m_dStartTime                = 0;
        m_dEndTime                  = 0;

        m_nBarLeft                  = 0;
        m_nBarWidth                 = 0;
        m_nBarHeight                = 0;
        m_nBarTextLeft              = 0;
        m_nBarTextWidth             = 0;
        m_nBarFontSize              = 0;
        m_nBarProgressWidth         = 0;
        m_nBarProgressTop           = 0;
#if READ_PROGRESS_IMG_FROM_RES
        m_imgProgressBar            = NULL;
        m_imgProgress               = NULL;
#else
        ZeroMemory(m_szProgressBarFile, sizeof(m_szProgressBarFile));
        ZeroMemory(m_szProgressFile, sizeof(m_szProgressFile));
#endif
    }

    virtual ~CAVChannelPlay() {
        DestroyFile();
    }

public:
    BOOL OpenFile(LPCTSTR lpszFileName, HWND hWnd) {
        if (lpszFileName == NULL || hWnd == NULL)
            return FALSE;

        DestroyFile();

        CAutoConvertString acs(lpszFileName);
        CHAR *pszFileName = acs;
        QRESULT qr;

        try {
            qr = QCAP_OPEN_FILE(pszFileName, &m_pFile, QCAP_DECODER_TYPE_NVIDIA_NVENC, &m_nFileVideoFormat,
                                &m_nFileVideoWidth, &m_nFileVideoHeight, &m_dFileVideoFrameRate, &m_nFileAudioFormat,
                                &m_nFileAudioChannel, &m_nFileAudioBitsPerSample, &m_nFileAudioSampleFrequency,
                                &m_dFileTotalDuationTime, &m_nFileTotalVideoFrames, &m_nFileTotalAudioFrames,
                                &m_nFileTotalMetadataFrames, hWnd, TRUE);

            if (qr != QCAP_RS_SUCCESSFUL) {
                Dbg(_T("OpenFile %ls nvenc failed"), lpszFileName);

                qr = QCAP_OPEN_FILE(pszFileName, &m_pFile, QCAP_DECODER_TYPE_INTEL_MEDIA_SDK, &m_nFileVideoFormat,
                    &m_nFileVideoWidth, &m_nFileVideoHeight, &m_dFileVideoFrameRate, &m_nFileAudioFormat,
                    &m_nFileAudioChannel, &m_nFileAudioBitsPerSample, &m_nFileAudioSampleFrequency,
                    &m_dFileTotalDuationTime, &m_nFileTotalVideoFrames, &m_nFileTotalAudioFrames,
                    &m_nFileTotalMetadataFrames, hWnd, TRUE);

                if (qr != QCAP_RS_SUCCESSFUL) {
                    Dbg(_T("OpenFile %ls intel failed"), lpszFileName);
                    goto __ERROR__;
                }
            }

            qr = QCAP_PAUSE_FILE(m_pFile);
            if (qr != QCAP_RS_SUCCESSFUL)
                goto __ERROR__;

            qr = QCAP_SET_AUDIO_FILE_VOLUME(m_pFile, 100);
            if (qr != QCAP_RS_SUCCESSFUL)
                goto __ERROR__;
        }
        catch (char *str) {
            Dbg(_T("OpenFile %ls exception"), lpszFileName);
            goto __ERROR__;
        }

        m_hWnd = hWnd;

        CWnd* cWnd = CWnd::FromHandle(m_hWnd);
        if (cWnd->GetParent())
            m_font = cWnd->GetParent()->GetFont();

        SetFileOSD(0);

        //m_nRepeatStatus = 1;

        QCAP_PAUSE_FILE(m_pFile);

        m_ePlayState = PLAY_STATE_INIT;

        return TRUE;

    __ERROR__:
        if (m_pFile) QCAP_DESTROY_FILE(m_pFile);
        return FALSE;
    }

    BOOL PlayFile() {
        if (m_pFile == NULL)
            return FALSE;

        if (m_ePlayState == PLAY_STATE_PLAYING)
            return TRUE;

        QRESULT qr = QCAP_PLAY_FILE(m_pFile);
        RETURN_BOOL_IF_ERROR(qr);

        m_ePlayState = PLAY_STATE_PLAYING;

        return TRUE;
    }

    BOOL PauseFile() {
        if (m_pFile == NULL)
            return FALSE;

        if (m_ePlayState == PLAY_STATE_PAUSED)
            return TRUE;

        QRESULT qr = QCAP_PAUSE_FILE(m_pFile);
        RETURN_BOOL_IF_ERROR(qr);

        m_ePlayState = PLAY_STATE_PAUSED;

        return TRUE;
    }

    BOOL PlayOrPauseFile() {
        if (m_ePlayState == PLAY_STATE_PLAYING) {
            return PauseFile();
        }
        return PlayFile();
    }

    BOOL SeekFile(BOOL Forward) {
        if (m_pFile == NULL)
            return FALSE;

        if (m_ePlayState == PLAY_STATE_NONE ||
            m_ePlayState == PLAY_STATE_END)
            return FALSE;

        double dCurPlayTime;
        if (GetFilePosition(&dCurPlayTime) && SetFilePosition(
            Forward ? (dCurPlayTime + 3.0 > m_dFileTotalDuationTime ? m_dFileTotalDuationTime : dCurPlayTime + 3.0) :
            (dCurPlayTime < 3.0 ? 0.0 : dCurPlayTime - 3.0)))
            return TRUE;

        return FALSE;
    }

    BOOL SeekFile(double dSampleTime) {
        if (m_pFile == NULL)
            return FALSE;

        if (m_ePlayState == PLAY_STATE_NONE ||
            m_ePlayState == PLAY_STATE_END)
            return FALSE;

        if (SetFilePosition(dSampleTime < 0.0f ? 0.0 : (dSampleTime > m_dFileTotalDuationTime ? m_dFileTotalDuationTime : dSampleTime)))
            return TRUE;

        return FALSE;
    }

    BOOL StopFile() {
        if (m_pFile == NULL)
            return FALSE;

        if (m_ePlayState == PLAY_STATE_STOP)
            return TRUE;

        QRESULT qr = QCAP_STOP_FILE(m_pFile);
        RETURN_BOOL_IF_ERROR(qr);

        BOOL bRet = SetFilePosition(0, FALSE);

        m_ePlayState = PLAY_STATE_STOP;

        return bRet;
    }

    BOOL DestroyFile() {
        if (m_pFile == NULL)
            return FALSE;

        QCAP_STOP_FILE(m_pFile);
        QCAP_DESTROY_FILE(m_pFile);
        m_pFile = NULL;

#if READ_PROGRESS_IMG_FROM_RES
        delete m_imgProgressBar;
        delete m_imgProgress;
#endif

        return TRUE;
    }

    PlayState GetFilePlayState() {
        return m_ePlayState;
    }

    BOOL GetFileTotalDuationTime(double* dTotalDuationTime) {
        *dTotalDuationTime = 0;

        if (m_pFile == NULL)
            return FALSE;

        *dTotalDuationTime = m_dFileTotalDuationTime;

        return TRUE;
    }

    BOOL GetFilePosition(double* dSampleTime) {
        *dSampleTime = 0;

        if (m_pFile == NULL)
            return FALSE;

        QRESULT qr = QCAP_GET_FILE_POSITION(m_pFile, dSampleTime);
        RETURN_BOOL_IF_ERROR(qr);

        m_dCurPlayTime = *dSampleTime;

        if (1) {
            // m_dFileTotalDuationTime
            //ULONG dStopSampleTime = (int)(m_nFileTotalVideoFrames / m_dFileVideoFrameRate);

            if (*dSampleTime >= m_dFileTotalDuationTime) {
                m_ePlayState = PLAY_STATE_END;

                //StopFile();
                //if (m_nRepeatStatus)
                //    PlayFile();
            }
        }

        return TRUE;
    }

    // When user wants to adjust the new video play position, please pause the video first.
    BOOL SetFilePosition(double dSampleTime, BOOL bNeedPlay = TRUE) {
        if (m_pFile == NULL)
            return FALSE;

        if (!PauseFile())
            return FALSE;

        QRESULT qr = QCAP_SET_FILE_POSITION(m_pFile, dSampleTime);
        RETURN_BOOL_IF_ERROR(qr);

        m_dCurPlayTime = dSampleTime;
        SetFileOSD(dSampleTime);

        if (bNeedPlay && !PlayFile())
            return FALSE;

        return TRUE;
    }

    BOOL SetFileOSD(double dCurFileTime) {
        if (m_pFile == NULL || m_hWnd == NULL/* || m_font == NULL*/)
            return FALSE;

        if (m_ePlayState == PLAY_STATE_NONE || m_ePlayState == PLAY_STATE_STOP)
            return FALSE;

        QRESULT qr;

        /*LOGFONT lf;
        CWnd* cWnd;
        int nFontHeight;

        cWnd = CWnd::FromHandle(m_hWnd);
        m_font->GetLogFont(&lf);
        nFontHeight = abs(lf.lfHeight);*/

#if READ_PROGRESS_IMG_FROM_RES
        if (m_imgProgressBar == NULL) {
            delete m_imgProgressBar;
            delete m_imgProgress;
            m_imgProgressBar = new CImage;
            m_imgProgress = new CImage;
            if (!LoadImageFromResource(m_imgProgressBar, IDB_PROGRESS_BAR, _T("RTPNG")) || 
                !LoadImageFromResource(m_imgProgress, IDB_PROGRESS, _T("RTPNG")) ||
                m_imgProgressBar == NULL || m_imgProgressBar->IsNull() ||
                m_imgProgress    == NULL || m_imgProgress->IsNull()) {

                delete m_imgProgressBar; m_imgProgressBar = NULL;
                delete m_imgProgress; m_imgProgress = NULL;

                return FALSE;
            }
#else
        if (strlen(m_szProgressBarFile) == 0) {
            CString str = GetProgressBarFile();
            CAutoConvertString acsBar(str);
            CHAR *pszProgressBarFileName = acsBar;

            str = GetProgressFile();
            CAutoConvertString acsProgress(str);
            CHAR *pszProgressFileName = acsProgress;
#endif
            double dWidthRatio  = (double)m_nFileVideoWidth / (double)PROGRESS_BAR_BASE_PIXEL_WIDTH;
            double dHeightRatio = (double)m_nFileVideoHeight / (double)PROGRESS_BAR_BASE_PIXEL_HEIGHT;

            m_nBarLeft          = dWidthRatio*PROGRESS_BAR_LEFT_WIDTH;
            m_nBarWidth         = m_nFileVideoWidth - m_nBarLeft * 2;
            m_nBarHeight        = dHeightRatio * PROGRESS_BAR_HEIGHT;
            m_nBarTextLeft      = dWidthRatio * PROGRESS_BAR_TEXT_LEFT_WIDTH;
            m_nBarTextWidth     = dWidthRatio * PROGRESS_BAR_TEXT_WIDTH;
            m_nBarFontSize      = dHeightRatio * PROGRESS_BAR_TEXT_FONT_SIZE;
            m_nBarProgressWidth = m_nBarWidth - (m_nBarTextWidth + m_nBarTextLeft * 2) * 2;
            m_nBarProgressTop   = dHeightRatio * PROGRESS_BAR_PROGRESS_TOP;

            m_rectProgressPixel.left   = m_nBarLeft + m_nBarTextWidth + m_nBarTextLeft * 2;
            m_rectProgressPixel.top    = m_nFileVideoHeight - m_nBarHeight + (m_nBarHeight - m_nBarFontSize) / 2 + m_nBarProgressTop;
            m_rectProgressPixel.right  = m_rectProgressPixel.left + m_nBarProgressWidth - m_nBarProgressTop;
            m_rectProgressPixel.bottom = m_rectProgressPixel.top + m_nBarFontSize;

#if READ_PROGRESS_IMG_FROM_RES
            qr = QCAP_SET_OSD_FILE_BUFFER(m_pFile, PLAY_PROGRESS_OSD_BAR,
                                          m_nBarLeft, m_nFileVideoHeight - m_nBarHeight,
                                          m_nBarWidth, m_nBarHeight,
                                          QCAP_COLORSPACE_TYEP_ABGR32, (BYTE*)m_imgProgressBar->GetBits(),
                                          m_imgProgressBar->GetWidth(), m_imgProgressBar->GetHeight(), m_imgProgressBar->GetPitch(),
                                          PROGRESS_BAR_ALPHA, 0xFFFFFFFF, 25, 2, 1, NULL, QCAP_SEQUENCE_STYLE_FOREMOST);
#else
            qr = QCAP_SET_OSD_FILE_PICTURE(m_pFile, PLAY_PROGRESS_OSD_BAR,
                                           m_nBarLeft, m_nFileVideoHeight - m_nBarHeight,
                                           m_nBarWidth, m_nBarHeight,
                                           pszProgressBarFileName, PROGRESS_BAR_ALPHA, QCAP_SEQUENCE_STYLE_FOREMOST);
#endif
            qr = QCAP_SET_OSD_FILE_TEXT(m_pFile, PLAY_PROGRESS_OSD_END,
                                        m_nFileVideoWidth - m_nBarLeft - m_nBarTextWidth - m_nBarTextLeft,
                                        m_nFileVideoHeight - m_nBarHeight + (m_nBarHeight - m_nBarFontSize) / 2,
                                        m_nBarTextWidth, m_nBarFontSize, GetSecsTimeFormat(m_dFileTotalDuationTime),
                                        PROGRESS_BAR_TEXT_FONT_NAME, QCAP_FONT_STYLE_REGULAR, m_nBarFontSize,
                                        PROGRESS_BAR_TEXT_COLOR, PROGRESS_BAR_BACK_COLOR, PROGRESS_BAR_ALPHA, 0, 0, QCAP_SEQUENCE_STYLE_FOREMOST);
#if READ_PROGRESS_IMG_FROM_RES == 0
            strcpy(m_szProgressBarFile, pszProgressBarFileName);
            strcpy(m_szProgressFile, pszProgressFileName);
#endif
        }
#if READ_PROGRESS_IMG_FROM_RES
        qr = QCAP_SET_OSD_FILE_BUFFER(m_pFile, PLAY_PROGRESS_OSD_PROGRESS,
                                      dCurFileTime == 0.0 ? 0 : m_rectProgressPixel.left, 
                                      dCurFileTime == 0.0 ? 0 : m_rectProgressPixel.top,
                                      dCurFileTime == 0.0 ? 0 : dCurFileTime * m_nBarProgressWidth / m_dFileTotalDuationTime, 
                                      dCurFileTime == 0.0 ? 0 : m_nBarFontSize,
                                      QCAP_COLORSPACE_TYEP_ABGR32, (BYTE*)m_imgProgress->GetBits(),
                                      m_imgProgress->GetWidth(), m_imgProgress->GetHeight(), m_imgProgress->GetPitch(), 
                                      PROGRESS_BAR_ALPHA, 0xFFFFFFFF, 25, 2, 1, NULL, QCAP_SEQUENCE_STYLE_FOREMOST);
#else
        qr = QCAP_SET_OSD_FILE_PICTURE(m_pFile, PLAY_PROGRESS_OSD_PROGRESS,
                                       dCurFileTime == 0.0 ? 0 : m_rectProgressPixel.left, 
                                       dCurFileTime == 0.0 ? 0 : m_rectProgressPixel.top,
                                       dCurFileTime == 0.0 ? 0 : dCurFileTime * m_nBarProgressWidth / m_dFileTotalDuationTime, 
                                       dCurFileTime == 0.0 ? 0 : m_nBarFontSize,
                                       m_szProgressFile, PROGRESS_BAR_ALPHA, QCAP_SEQUENCE_STYLE_FOREMOST);
#endif
        qr = QCAP_SET_OSD_FILE_TEXT(m_pFile, PLAY_PROGRESS_OSD_START,
                                    m_nBarLeft + m_nBarTextLeft,
                                    m_nFileVideoHeight - m_nBarHeight + (m_nBarHeight - m_nBarFontSize) / 2,
                                    m_nBarTextWidth, m_nBarFontSize, GetSecsTimeFormat(dCurFileTime),
                                    PROGRESS_BAR_TEXT_FONT_NAME, QCAP_FONT_STYLE_REGULAR, m_nBarFontSize,
                                    PROGRESS_BAR_TEXT_COLOR, PROGRESS_BAR_BACK_COLOR, PROGRESS_BAR_ALPHA, 0, 0, QCAP_SEQUENCE_STYLE_FOREMOST);

        return TRUE;
    }

    // Specify the playback speed, range 0.0 to 4.0
    BOOL SetFilePlaySpeed(double dSpeed) {
        if (m_pFile == NULL)
            return FALSE;

        QRESULT qr = QCAP_SET_FILE_PLAYBACK_SPEED(m_pFile, dSpeed);
        RETURN_BOOL_IF_ERROR(qr);

        return TRUE;
    }

    BOOL SetFileStartTime(double dSampleTime) {
        if (m_pFile == NULL) {
            return FALSE;
        }

        if (dSampleTime >= m_dFileTotalDuationTime || dSampleTime < 0)
            m_dStartTime = 0;
        else
            m_dStartTime = dSampleTime;

        return TRUE;
    }

    BOOL SetFileEndTime(double dSampleTime) {
        if (m_pFile == NULL) {
            return FALSE;
        }

        if (dSampleTime >= m_dFileTotalDuationTime || dSampleTime < 0)
            m_dEndTime = 0;
        else
            m_dEndTime = dSampleTime;

        return TRUE;
    }

    BOOL IsEnd() {
        if (m_pFile == NULL || m_ePlayState == PLAY_STATE_END || (m_dEndTime > 0 && m_dCurPlayTime >= m_dEndTime)) {
            return TRUE;
        }

        return FALSE;
    }

public:
    ULONG           m_nRepeatStatus;
    PVOID           m_pFile;
    CString         m_strFilePath;
    ULONG           m_nFileVideoFormat;
    ULONG           m_nFileVideoWidth;
    ULONG           m_nFileVideoHeight;
    double          m_dFileVideoFrameRate;
    ULONG           m_nFileAudioFormat;
    ULONG           m_nFileAudioChannel;
    ULONG           m_nFileAudioBitsPerSample;
    ULONG           m_nFileAudioSampleFrequency;
    double          m_dFileTotalDuationTime;
    ULONG           m_nFileTotalVideoFrames;
    ULONG           m_nFileTotalAudioFrames;
    ULONG           m_nFileTotalMetadataFrames;

    HWND            m_hWnd;
    CFont*          m_font;
    PlayState       m_ePlayState;
    double          m_dCurPlayTime;
    double          m_dStartTime;
    double          m_dEndTime;
#if READ_PROGRESS_IMG_FROM_RES
    CImage*         m_imgProgressBar;
    CImage*         m_imgProgress;
#else
    CHAR            m_szProgressBarFile[MAX_PATH];
    CHAR            m_szProgressFile[MAX_PATH];
#endif
    int             m_nBarLeft;
    int             m_nBarWidth;
    int             m_nBarHeight;
    int             m_nBarTextLeft;
    int             m_nBarTextWidth;
    int             m_nBarFontSize;
    int             m_nBarProgressWidth;
    int             m_nBarProgressTop;
    CRect           m_rectProgressPixel;
};
