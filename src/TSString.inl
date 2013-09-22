/* 
 * File:   TSString.inl
 * Author: thunderliu
 *
 * Created on 2011年12月27日, 下午4:59
 */

#ifndef __TSSTRING_INL__
#define	__TSSTRING_INL__

#include <assert.h>

#include "TSString.h"




// CStr

inline CStr::CStr(size_t uSize)
: m_pBuf(new char[uSize])
, m_uSize(uSize)
, m_bAllocMem(true)
{
    assert(m_pBuf);
    SetChars(0);
}

inline CStr::CStr(const char* pBuf, size_t uSize, bool bAllocMem)
: m_uSize(uSize)
, m_bAllocMem(bAllocMem)
{
    assert(pBuf);
    if (bAllocMem)
    {
        m_pBuf = new char[uSize];
        memset(m_pBuf, 0, uSize);
        Copy(pBuf);
    }
    else
    {
        m_pBuf = const_cast<char*>(pBuf);
    }
}

inline CStr::~CStr()
{
    assert(m_pBuf);
    if (m_bAllocMem)
    {
        delete[] m_pBuf;
    }
}

inline char* CStr::GetBuffer(size_t uIndex)
{
    assert(m_pBuf);
    assert(uIndex < m_uSize);
    return &m_pBuf[uIndex];
}

inline const char* CStr::GetBuffer(size_t uIndex) const
{
    assert(m_pBuf);
    assert(uIndex < m_uSize);
    return &m_pBuf[uIndex];
}

inline char& CStr::GetChar(size_t uIndex)
{
    assert(m_pBuf);
    assert(uIndex < m_uSize);
    return m_pBuf[uIndex];
}

inline const char& CStr::GetChar(size_t uIndex) const
{
    assert(m_pBuf);
    assert(uIndex < m_uSize);
    return m_pBuf[uIndex];
}

inline size_t CStr::GetSize() const
{
    assert(m_pBuf);
    return m_uSize;
}

inline void CStr::SetChars(char cToSet)
{
    assert(m_pBuf);
    memset(m_pBuf, cToSet, m_uSize);
}

inline size_t CStr::GetLength() const
{
    assert(m_pBuf);
    return strnlen(m_pBuf, m_uSize);
}

inline char* CStr::Copy(const char* pSrcStr)
{
    assert(m_pBuf);
    assert(pSrcStr);
    return strncpy(m_pBuf, pSrcStr, m_uSize);
}

inline int CStr::Compare(const char* pStr)
{
    assert(m_pBuf);
    assert(pStr);
    return strncmp(m_pBuf, pStr, m_uSize);
}

inline char* CStr::Concat(const char* pStr)
{
    assert(m_pBuf);
    assert(pStr);
    return strncat(m_pBuf, pStr, m_uSize);
}

inline CStr::operator char*()
{
    assert(m_pBuf);
    return m_pBuf;
}

inline CStr::operator const char*() const
{
    assert(m_pBuf);
    return m_pBuf;
}


// CIoInterface

inline CIoInterface::~CIoInterface()
{
}



// CStringLine

inline CStringLine::CStringLine(const char* pLine)
: m_pLine(pLine)
, m_pPos(m_pLine)
, m_bIsEnd(!m_pLine)
{
}

inline void CStringLine::Attach(const char* pLine)
{
    m_pLine = pLine;
    m_pPos = m_pLine;
    m_bIsEnd = !m_pLine;
}

inline const char* CStringLine::GetStringAddr() const
{
    return m_pLine;
}

inline bool CStringLine::IsEnd()
{
    return m_bIsEnd;
}


// CCmdStrLn

inline CCmdStrLn::CCmdStrLn(int iArgc, const char** ppArgv)
: m_iArgc(iArgc)
, m_ppArgv(ppArgv)
{
}

inline void CCmdStrLn::SetArg(int iArgc, const char** ppArgv)
{
    m_iArgc = iArgc;
    m_ppArgv = ppArgv;
}

inline int CCmdStrLn::GetArgc() const
{
    return m_iArgc;
}

inline const char** CCmdStrLn::GetArgv() const
{
    return m_ppArgv;
}


#ifdef TSNETFW_FEATURE_READLINE
// CReadline

inline CReadline* CReadline::Instance()
{
    return CReadline::m_pInstance;
}

inline void CReadline::Instance(CReadline* pInstance)
{
    CReadline::m_pInstance = pInstance;
}

inline int CReadline::ReadStringEnd(CStr& roStr)
{
    return CCmdStrLn::ReadString(roStr);
}

inline void CReadline::SetPrompt(const char* pPrompt)
{
    strncpy(m_szPrompt, pPrompt, sizeof (m_szPrompt));
}
#endif // TSNETFW_FEATURE_READLINE

#endif	/* __TSSTRING_INL__ */

