/* 
 * File:   TSDebug.inl
 * Author: thunderliu
 *
 * Created on 2011年12月29日, 上午11:32
 */

#ifndef __TSDEBUG_INL__
#define	__TSDEBUG_INL__

#include "TSDebug.h"



// CStopWatch

inline CStopWatch::CStopWatch()
{
    gettimeofday(&m_stTv, 0);
}

inline CStopWatch::~CStopWatch()
{
}

inline long CStopWatch::Start()
{
    struct timeval stTv(m_stTv);
    gettimeofday(&m_stTv, 0);
    return (m_stTv.tv_sec - stTv.tv_sec) * 1000000 + m_stTv.tv_usec - stTv.tv_usec;
}

inline long CStopWatch::Peek()
{
    struct timeval stTv;
    gettimeofday(&stTv, 0);
    return (stTv.tv_sec - m_stTv.tv_sec) * 1000000 + stTv.tv_usec - m_stTv.tv_usec;
}


// CLog

inline void CLog::SetLogLevel(E_LOG_LVL eLogLv)
{
    CLog::m_eLogLv = eLogLv;
}

inline void CLog::SetOutFile(FILE* pFile)
{
    if (pFile)
    {
        CLog::m_pFile = pFile;
    }
}

inline FILE* CLog::GetOutFile()
{
    return CLog::m_pFile;
}


#ifdef TSNETFW_FEATURE_PTRACE
// CPtrace

inline CPtrace::CPtrace()
: m_iPid(0)
{
}

inline bool CPtrace::TraceMe()
{
    m_iPid = 0;
    return ptrace(PT_TRACE_ME) >= 0;
}

inline bool CPtrace::Attach(pid_t iPid)
{
    m_iPid = iPid;
    return ptrace(PT_ATTACH, m_iPid) >= 0;
}

inline bool CPtrace::Detach()
{
    return ptrace(PT_DETACH, m_iPid) >= 0;
}

inline bool CPtrace::Continue(int iSignal)
{
    return ptrace(PT_CONTINUE, m_iPid, 0, iSignal) >= 0;
}

inline bool CPtrace::SysCall(int iSignal)
{
    return ptrace(PT_SYSCALL, m_iPid, 0, iSignal) >= 0;
}

inline long CPtrace::ReadText(void* pAddr)
{
    return ptrace(PT_READ_I, m_iPid, pAddr);
}

inline long CPtrace::ReadData(void* pAddr)
{
    return ptrace(PT_READ_D, m_iPid, pAddr);
}

inline bool CPtrace::WriteText(const void* pAddr, long lValue)
{
    return ptrace(PT_WRITE_I, m_iPid, pAddr, lValue) >= 0;
}

inline bool CPtrace::WriteData(const void* pAddr, long lValue)
{
    return ptrace(PT_WRITE_D, m_iPid, pAddr, lValue) >= 0;
}

inline bool CPtrace::ReadRegs(struct user_regs_struct* pRegs)
{
    return ptrace(PT_GETREGS, m_iPid, 0, pRegs) >= 0;
}

inline bool CPtrace::WriteRegs(const struct user_regs_struct* pRegs)
{
    return ptrace(PT_SETREGS, m_iPid, 0, pRegs) >= 0;
}

inline bool CPtrace::ReadFpRegs(struct user_fpregs_struct* pRegs)
{
    return ptrace(PT_GETFPREGS, m_iPid, 0, pRegs) >= 0;
}

inline bool CPtrace::WriteFpRegs(const struct user_fpregs_struct* pRegs)
{
    return ptrace(PT_SETFPREGS, m_iPid, 0, pRegs) >= 0;
}

inline bool CPtrace::Kill()
{
    return ptrace(PT_KILL, m_iPid) >= 0;
}

inline bool CPtrace::SingleStep(int iSignal)
{
    return ptrace(PT_STEP, m_iPid, 0, iSignal) >= 0;
}

inline int CPtrace::Wait()
{
    int iStatus = 0;
    int iRet = wait(&iStatus);
    return iRet < 0 ? iRet : iStatus;
}

inline pid_t CPtrace::GetPid() const
{
    return m_iPid;
}
#endif // TSNETFW_FEATURE_PTRACE




#endif	/* __TSDEBUG_INL__ */

