/* 
 * File:   TSDebug.h
 * Author: thunderliu
 *
 * Created on 2011年12月29日, 上午11:32
 */

#ifndef __TSDEBUG_H__
#define	__TSDEBUG_H__

#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>

class CDebug
{
public:
    static void SimpleDump(const void* pBuf, size_t uSize);
};

class CStopWatch
{
public:
    CStopWatch();
    virtual ~CStopWatch();

    long Start();
    long Peek();

protected:
    struct timeval m_stTv;
};

typedef enum
{
    E_LL_NONE = 0,
    E_LL_ERR = 1,
    E_LL_DBG = 2
} E_LOG_LVL;

class CLog
{
public:
    static void SetLogLevel(E_LOG_LVL eLogLv);
    static void SetOutFile(FILE* pFile);
    static FILE* GetOutFile();

public:
    static E_LOG_LVL m_eLogLv;
    static FILE* m_pFile;
};




#define LOG_OUT(LOG_LVL, fmt, args...) \
do \
{ \
    if (LOG_LVL != E_LL_NONE && LOG_LVL <= CLog::m_eLogLv) \
    { \
        fprintf(CLog::GetOutFile(), (fmt), ##args); \
    } \
} while (false)

#define LOG_DMP(LOG_LVL, buf, size) \
do \
{ \
    if (LOG_LVL != E_LL_NONE && LOG_LVL <= CLog::m_eLogLv) \
    { \
        CDebug::SimpleDump((buf), (size)); \
    } \
} while (false)

//#define LOG_ERR(fmt, args...) LOG_OUT(E_LL_ERR, ("[ "fmt" | %s | %s: %d ]\n"), ##args, __PRETTY_FUNCTION__, __FILE__, __LINE__)
//#define LOG_DBG(fmt, args...) LOG_OUT(E_LL_DBG, ("[ "fmt" | %s | %s: %d ]\n"), ##args, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#ifdef USE_GLOG
#include <glog/logging.h>
#define LOG_ERR(fmt, args...) \
do \
{ \
    char szTmp[128]; \
    snprintf(szTmp, sizeof(szTmp), fmt, ##args); \
    LOG(ERROR) << szTmp; \
} while (false)

#define LOG_DBG(fmt, args...) \
do \
{ \
    char szTmp[128]; \
    snprintf(szTmp, sizeof(szTmp), fmt, ##args); \
    LOG(INFO) << szTmp; \
} while (false)
#else
#define LOG_ERR(fmt, args...) LOG_OUT(E_LL_ERR, ("[ "fmt" | %s | %s: %d ]\n"), ##args, __FUNCTION__, __FILE__, __LINE__)
#define LOG_DBG(fmt, args...) LOG_OUT(E_LL_DBG, ("[ "fmt" | %s | %s: %d ]\n"), ##args, __FUNCTION__, __FILE__, __LINE__)
#define LOG_ERR_DMP(buf, size) LOG_DMP(E_LL_ERR, buf, size)
#define LOG_DBG_DMP(buf, size) LOG_DMP(E_LL_DBG, buf, size)
#endif // USE_GLOG


#ifdef _DEBUG
#define ASSERT(b) do { unlikely(!b) abort(); } while (false)
#else
#define ASSERT(b) static_cast<void>((b))
#endif

#define VERTIFY(b) do { unlikely(b) abort(); } while (false)

#define UNUSED_ARG(a) do {/* null */} while (&a == 0)

#ifdef TSNETFW_FEATURE_PTRACE
#include <sys/ptrace.h>
#include <sys/user.h>

class CPtrace
{
public:
    CPtrace();

    bool TraceMe();
    bool Attach(pid_t iPid);
    bool Detach();
    bool Continue(int iSignal = 0);
    bool SysCall(int iSignal = 0);
    long ReadText(void* pAddr);
    long ReadData(void* pAddr);
    bool WriteText(const void* pAddr, long lValue);
    bool WriteData(const void* pAddr, long lValue);
    bool ReadRegs(struct user_regs_struct* pRegs);
    bool WriteRegs(const struct user_regs_struct* pRegs);
    bool ReadFpRegs(struct user_fpregs_struct* pRegs);
    bool WriteFpRegs(const struct user_fpregs_struct* pRegs);
    bool Kill();
    bool SingleStep(int iSignal = 0);

    int Wait();
    pid_t GetPid() const;

protected:
    pid_t m_iPid;
};

#endif // TSNETFW_FEATURE_PTRACE

#define UNLIKELY_RET(b, ret, fmt, args...) \
do \
{ \
    unlikely ((b)) \
    { \
        LOG_ERR(fmt, ##args); \
        return (ret); \
    } \
} while (false)

#include "TSDebug.inl"

#endif	/* __TSDEBUG_H__ */

