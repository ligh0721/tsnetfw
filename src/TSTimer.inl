/* 
 * File:   TSTimer.inl
 * Author: thunderliu
 *
 * Created on 2011年12月14日, 下午6:59
 */

#ifndef __TSTIMER_INL__
#define	__TSTIMER_INL__

#include "TSTimer.h"



// CTime

inline CTime::CTime()
{
    SetTime(0, 0);
}

inline CTime::CTime(long lSec, long lMSec)
{
    SetTime(lSec, lMSec);
}

inline CTime::CTime(const struct timeval& stTv)
{
    SetTime(stTv);
}

inline void CTime::SetTime(long lSec, long lMSec)
{
    this->lSec = lSec;
    this->lMSec = lMSec;
}

inline void CTime::SetTime(const struct timeval& stTv)
{
    lSec = stTv.tv_sec;
    lMSec = stTv.tv_usec / 1000;
}

inline long CTime::CalcMilliSecond() const
{
    return lSec * 1000 + lMSec;
}

// CTimerNodeBase

inline void CTimerNodeBase::SetInterval(const CTime& rInterval)
{
    SetInterval(rInterval.CalcMilliSecond());
}

inline void CTimerNodeBase::SetInterval(long lInterval)
{
    m_lInterval = lInterval;
    ;
}

inline long CTimerNodeBase::GetInterval() const
{
    return m_lInterval;
}

inline void CTimerNodeBase::SetExpireLeft(long lExpireLeft)
{
    m_lExpireLeft = lExpireLeft;
}

inline long CTimerNodeBase::GetExpireLeft() const
{
    return m_lExpireLeft;
}


// CTimerManagerBase

inline CTimerManagerBase::~CTimerManagerBase()
{
}


// CTimerQueueNode

inline void CTimerQueueNode::SetNext(CTimerQueueNode* pNext)
{
    m_pNext = pNext;
}

inline CTimerQueueNode* CTimerQueueNode::GetNext()
{
    return m_pNext;
}

inline void CTimerQueueNode::SetPrev(CTimerQueueNode* pPrev)
{
    m_pPrev = pPrev;
}

inline CTimerQueueNode* CTimerQueueNode::GetPrev()
{
    return m_pPrev;
}

// CTimerQueue

inline CTimerQueue::CTimerQueue()
: m_pHead(NULL)
{
    gettimeofday(&m_stTv, 0);
}

inline CTimerQueue::~CTimerQueue()
{
    for (CTimerQueueNode* pCur = m_pHead; pCur; pCur = pCur->GetNext())
    {
        delete pCur;
    }
}

inline bool CTimerQueue::AddTimer(CTimerNodeBase* pTimer, const CTime& rInterval, const CTime& rDelay)
{
    return AddTimer((CTimerQueueNode*)pTimer, rInterval.CalcMilliSecond(), rDelay.CalcMilliSecond());
}

inline bool CTimerQueue::PeekTimeout(long* pMSec)
{
    if (!m_pHead)
    {
        return false;
    }

    if (pMSec)
    {
        *pMSec = m_pHead->GetExpireLeft();
    }

    return true;
}

#endif	/* __TSTIMER_INL__ */

