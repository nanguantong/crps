
// CIniFile.cpp : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <assert.h>
#include <stdio.h>
#include <locale.h>
#include "CIniFile.h"

#define MAX_BUFFER_SIZE                 1024

CString CIniFile::ReadString(const CString &strSection, const CString &strIdent, const CString &strDefault)
{
    //char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
    //setlocale(LC_CTYPE, "chs");

    TCHAR szBuf[MAX_BUFFER_SIZE] = { 0 };
    uint32_t nLen = ::GetPrivateProfileString(strSection, strIdent, strDefault, szBuf, MAX_BUFFER_SIZE, m_strFileName);

    //setlocale(LC_CTYPE, old_locale);
    //free(old_locale);

    assert((nLen != MAX_BUFFER_SIZE - 1) || (nLen != MAX_BUFFER_SIZE - 2));

    return szBuf;
}

BOOL CIniFile::WriteString(const CString &strSection, const CString &strIdent, const CString &strValue)
{
    BOOL bRet = ::WritePrivateProfileString(strSection, strIdent, strValue, m_strFileName);
    //AssertV(bRet == 0, ::GetLastError());
    return bRet;
}

uint32_t CIniFile::ReadInteger(const CString &strSection, const CString &strIdent, int iDefault)
{
    return ::GetPrivateProfileInt(strSection, strIdent, iDefault, m_strFileName);
}

BOOL CIniFile::WriteInteger(const CString &strSection, const CString &strIdent, int iValue)
{
#ifdef UNICODE
    TCHAR szBuf[32] = { 0 };
    ::swprintf(szBuf, L"%d", iValue);
#else
    CHAR szBuf[32] = { 0 };
    ::sprintf(szBuf, "%d", Value);
#endif

    return WriteString(strSection, strIdent, szBuf);
}
