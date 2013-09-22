/* 
 * File:   TSRuntime.inl
 * Author: thunderliu
 *
 * Created on 2011年12月27日, 下午9:01
 */

#ifndef __TSRUNTIME_INL__
#define	__TSRUNTIME_INL__

#include "TSRuntime.h"



// CCommandLine

inline CCommandLine::CCommandLine(int iArgc, char* const* ppArgv)
: m_iArgc(iArgc)
, m_ppArgv(ppArgv)
{
}

inline int CCommandLine::GetArgCount()
{
    if (!m_pInstance)
    {
        return 0;
    }

    return m_pInstance->m_iArgc;
}

inline const char* CCommandLine::GetArgValue(int iIndex)
{
    if (!m_pInstance)
    {
        return NULL;
    }

    if (iIndex < 0 || iIndex >= m_pInstance->m_iArgc)
    {
        return NULL;
    }

    return m_pInstance->m_ppArgv[iIndex];
}

inline char* const* CCommandLine::GetArgValues()
{
    if (!m_pInstance)
    {
        return NULL;
    }

    return m_pInstance->m_ppArgv;
}

inline CCommandLine* CCommandLine::CreateCommandLine(int iArgc, char* const* ppArgv)
{
    if (m_pInstance)
    {
        return m_pInstance;
    }

    if (iArgc <= 0 || !ppArgv)
    {
        return NULL;
    }

    m_pInstance = new CCommandLine(iArgc, ppArgv);

    return m_pInstance;
}


// CRuntimeUnit

inline CRuntimeInstance::~CRuntimeInstance()
{
}



#endif	/* __TSRUNTIME_INL__ */

