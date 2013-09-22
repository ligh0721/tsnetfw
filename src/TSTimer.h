/* 
 * File:   TSTimer.h
 * Author: thunderliu
 *
 * Created on 2011年12月14日, 下午6:58
 */

#ifndef __TSTIMER_H__
#define	__TSTIMER_H__

class CTime
{
public:
    CTime();
    CTime(long lSec, long lMSec);
    CTime(const struct timeval& stTv);

    void SetTime(long lSec, long lMSec);
    void SetTime(const struct timeval& stTv);
    long CalcMilliSecond() const;

    long lSec;
    long lMSec;
};

class CTimerNodeBase : public CEventInterface
{
public:
    void SetInterval(const CTime& rInterval);
    void SetInterval(long lInterval);
    long GetInterval() const;

    void SetExpireLeft(long lExpireLeft);
    long GetExpireLeft() const;

protected:
    long m_lInterval;
    long m_lExpireLeft;

};

class CTimerManagerBase
{
public:
    virtual ~CTimerManagerBase();

    virtual bool AddTimer(CTimerNodeBase* pTimer, const CTime& rInterval, const CTime& rDelay) = 0;
    virtual bool RemoveTimer(CTimerNodeBase* pTimer) = 0;
    virtual bool Update() = 0;
    virtual bool PeekTimeout(long* pMSec) = 0;
};

class CTimerQueueNode : public CTimerNodeBase
{
public:
    void SetNext(CTimerQueueNode* pNext);
    CTimerQueueNode* GetNext();

    void SetPrev(CTimerQueueNode* pPrev);
    CTimerQueueNode* GetPrev();

protected:
    CTimerQueueNode* m_pPrev;
    CTimerQueueNode* m_pNext;

};

class CTimerQueue : public CTimerManagerBase
{
public:
    CTimerQueue();
    virtual ~CTimerQueue();
    bool Init();

    virtual bool AddTimer(CTimerNodeBase* pTimer, const CTime& rInterval, const CTime& rDelay);
    virtual bool RemoveTimer(CTimerNodeBase* pTimer);
    virtual bool Update();
    virtual bool PeekTimeout(long* pMSec);

    void SetHead(CTimerQueueNode* pHead);
    CTimerQueueNode* GetHead();

protected:
    bool AddTimer(CTimerQueueNode* pTimer, long lInterval, long lDelay);
    bool AddTimerToQueue(CTimerQueueNode* pTimer);
    bool RemoveTimerFromQueue(CTimerQueueNode* pTimer);
    CTimerQueueNode* m_pHead;

protected:
    struct timeval m_stTv;

};
















#include "TSTimer.inl"

#endif	/* __TSTIMER_H__ */

