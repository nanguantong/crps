
// CIniFile.h : header file
//
// nanguantong, All Rights Reserved.
//
// CONTACT INFORMATION:
// zww0602jsj@gmail.com
// http://weibo.com/nanguantong/
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

class CIniFile
{
public:
    CIniFile(CString strFileName) : m_strFileName(strFileName)
    {}

    virtual ~CIniFile()
    {}

    // 要删除某个节，只需要将WriteProfileString第二个参数设为NULL即可
    // 而要删除某个键，则只需要将该函数的第三个参数设为 NULL即可
    CString     ReadString(const CString &strSection, const CString &strIdent, const CString &strDefault);
    BOOL        WriteString(const CString &strSection, const CString &strIdent, const CString &strValue);
    uint32_t    ReadInteger(const CString &strSection, const CString &strIdent, int iDefault);
    BOOL        WriteInteger(const CString &strSection, const CString &strIdent, int iValue);

    /*bool      operator == (const CIniFile &a, const CIniFile &b)
    {
        if (b == NULL)
        {
            return a.m_strFileName.Trim() == "";
        }
        return a.m_strFileName.Trim() == b.m_strFileName.Trim();
    }*/

protected:
    CString       m_strFileName;
};
