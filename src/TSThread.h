/* 
 * File:   TSThread.h
 * Author: thunderliu
 *
 * Created on 2011年12月25日, 上午1:20
 */

#ifndef __TSTHREAD_H__
#define	__TSTHREAD_H__

#include <pthread.h>
#include "TSRuntime.h"

class CThread : public CRuntimeInstance
{
public:

    typedef enum
    {
        CANCEL_DEFFERED = PTHREAD_CANCEL_DEFERRED,
        CANCEL_ASYNCHRONOUS = PTHREAD_CANCEL_ASYNCHRONOUS
    } E_CANCEL_TYPE;
public:
    CThread();
    virtual ~CThread();

    virtual bool Start();
    virtual bool Wait();
    bool Cancel();

    //static CThread* CreateThread();
    static E_CANCEL_TYPE SetCancelType(E_CANCEL_TYPE eType);

protected:
    virtual long ThreadProc();
    virtual void OnExit(void* pExitCode);

private:
    static void* ThreadRoutine(CThread* pInstance);
    static void CleanUpRoutine(CThread* pInstance);

protected:
    pthread_t m_uTid;
    void* m_pExitCode;

};




#include "TSThread.inl"

#endif	/* __TSTHREAD_H__ */

