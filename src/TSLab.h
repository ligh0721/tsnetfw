/* 
 * File:   TSLab.h
 * Author: thunderliu
 *
 * Created on 2012年6月13日, 下午3:13
 */

#ifndef __TSLAB_H__
#define	__TSLAB_H__

#include <unistd.h>
#include <stdint.h>
#include "TSThread.h"

class CMachine
{
protected:

    class CInnerThread : public CThread
    {
    protected:
        virtual long ThreadProc();
        virtual void OnExit(void* pExitCode);
    };

public:
    CMachine();
    virtual ~CMachine();

    bool Init(uint64_t ddwHz);
    void SetHz(uint64_t ddwHz);
    uint64_t GetMaxHz();
    bool Work();

protected:
    virtual bool OnWork();

protected:
    useconds_t m_uUsec;
    uint64_t m_ddwHz;
};

// CMachine

long CMachine::CInnerThread::ThreadProc()
{
    pthread_detach(m_uTid);
    return true;
}

void CMachine::CInnerThread::OnExit(void* pExitCode)
{

}

inline CMachine::CMachine()
: m_uUsec(0)
{
}

inline CMachine::~CMachine()
{
}

inline bool CMachine::OnWork()
{
    return true;
}

bool CMachine::Init(uint64_t ddwHz)
{
    return true;
}

void CMachine::SetHz(uint64_t ddwHz)
{
    m_ddwHz = ddwHz;
}

bool CMachine::Work()
{

    OnWork();
    usleep(m_uUsec);
    return true;
}

#endif	/* __TSLAB_H__ */

