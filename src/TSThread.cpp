/* 
 * File:   TSThread.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月25日, 上午1:20
 */

#include "TSPlatform.h"
#include <signal.h>

#include "TSThread.h"

bool CThread::Start()
{
    if (m_uTid)
    {
        return false;
    }

    int iRes = pthread_create(&m_uTid, NULL, (void*(*)(void*)) & CThread::ThreadRoutine, this);
    if (iRes < 0)
    {
        return false;
    }

    return true;
}

bool CThread::Wait()
{
    void* pExitCode = NULL;
    int iRet = pthread_join(m_uTid, &pExitCode);
    m_uTid = 0;
    return iRet >= 0;
}

bool CThread::Cancel()
{
    bool bRet = pthread_cancel(m_uTid) >= 0;
    //bool bRet = pthread_kill(m_uTid, SIGKILL) >= 0;
    //CThread::Wait(); // user should call Wait()
    return bRet;
}

void* CThread::ThreadRoutine(CThread* pInstance)
{
    CThread::SetCancelType(CThread::CANCEL_ASYNCHRONOUS);
    void* pExitCode = NULL;
    pthread_cleanup_push((void(*)(void*)) & CThread::CleanUpRoutine, pInstance);
    pExitCode = (void*)pInstance->ThreadProc();
    pInstance->m_pExitCode = pExitCode;
    pthread_exit(pExitCode);
    pthread_cleanup_pop(0); // 不执行就泄露
    return pExitCode;
}

void CThread::CleanUpRoutine(CThread* pInstance)
{
    //void* pExitCode = NULL;
    //pthread_join(pInstance->m_uTid, &pExitCode);
    pthread_join(pInstance->m_uTid, NULL);
    pInstance->OnExit(pInstance->m_pExitCode);
}

