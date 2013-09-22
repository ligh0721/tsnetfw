/* 
 * File:   TSProcess.inl
 * Author: thunderliu
 *
 * Created on 2011年12月30日, 下午4:10
 */

#ifndef __TSPROCESS_INL__
#define	__TSPROCESS_INL__

#include "TSProcess.h"


// CProcess

inline CProcess::CProcess()
: m_iPid(0)
{
}

inline CProcess::CProcess(pid_t iPid)
: m_iPid(iPid)
{
}

inline CProcess::~CProcess()
{
}

inline CProcess* CProcess::Attach(pid_t iPid)
{
    if (!iPid)
    {
        return NULL;
    }
    return new CProcess(iPid);
}

inline bool CProcess::Wait()
{
    if (!m_iPid)
    {
        return false;
    }

    return waitpid(m_iPid, NULL, 0) != -1;
}

inline bool CProcess::IsMe() const
{
    return !m_iPid;
}

inline pid_t CProcess::GetPid() const
{
    return m_iPid ? m_iPid : getpid();
}

inline bool CProcess::Release()
{
    if (!m_iPid)
    {
        return false;
    }
    delete this;
}

inline int CProcess::ProcessProc()
{
    return 0;
}


// CProcessGroup

inline CProcessGroup::~CProcessGroup()
{
    if (m_pProcess)
    {
        delete[] m_pProcess;
    }
}

inline bool CProcessGroup::Release()
{
    delete this;
    return true;
}

#endif	/* __TSPROCESS_INL__ */

