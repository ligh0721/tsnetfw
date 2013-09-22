/* 
 * File:   TSThread.inl
 * Author: thunderliu
 *
 * Created on 2011年12月25日, 上午1:21
 */

#ifndef __TSTHREAD_INL__
#define	__TSTHREAD_INL__


// CThread

inline CThread::CThread()
: m_uTid(ptw32_handle_t())
, m_pExitCode(PTHREAD_CANCELED)
{
}

inline CThread::~CThread()
{
}

inline long CThread::ThreadProc()
{
    return 0;
}

inline void CThread::OnExit(void* pExitCode)
{
}

inline CThread::E_CANCEL_TYPE CThread::SetCancelType(E_CANCEL_TYPE eType)
{
    int iOld = 0;
    pthread_setcanceltype((int)eType, &iOld);
    return (E_CANCEL_TYPE)iOld;
}

#endif	/* __TSTHREAD_INL__ */

