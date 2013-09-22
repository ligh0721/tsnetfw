/* 
 * File:   TSWorker.inl
 * Author: thunderliu
 *
 * Created on 2011年11月14日, 下午11:41
 */

#include "TSWorker.h"
#include "TSThread.h"
#include "TSDebug.h"
#include "TSSynch.h"

#ifndef __TSWORKER_INL__
#define	__TSWORKER_INL__


// CBlock16

inline CBlock16::CBlock16(uint16_t wSize, void* pBuf)
: m_wSize(wSize)
, m_pBuf(pBuf)
, m_pPrev(NULL)
, m_pNext(NULL)
{
}

inline CBlock16::~CBlock16()
{
    free(m_pBuf);
}

inline void CBlock16::Release()
{
    delete this;
}

inline uint16_t CBlock16::GetSize() const
{
    return m_wSize;
}

inline void* CBlock16::GetBuffer()
{
    return m_pBuf;
}

inline CBlock16* CBlock16::GetPrev()
{
    return m_pPrev;
}

inline void CBlock16::SetPrev(CBlock16* pBlock)
{
    m_pPrev = pBlock;
}

inline CBlock16* CBlock16::GetNext()
{
    return m_pNext;
}

inline void CBlock16::SetNext(CBlock16* pBlock)
{
    m_pNext = pBlock;
}


// CQueue

inline CQueue::CQueue(size_t uBlockCount) : m_uBlockMaxCount(uBlockCount), m_uBlockUsedCount(0), m_pHead(NULL), m_pTail(NULL)
{
}

inline CQueue::~CQueue()
{
    //free(m_ppBlocks);
}

inline void CQueue::Release()
{
    delete this;
}

inline size_t CQueue::GetBlockMaxCount() const
{
    return m_uBlockMaxCount;
}

inline size_t CQueue::GetBlockUsedCount() const
{
    return m_uBlockUsedCount;
}

inline bool CQueue::IsEmpty() const
{
    return !m_pTail;
}

inline bool CQueue::IsFull() const
{
    return m_uBlockUsedCount >= m_uBlockMaxCount;
}

/*
inline void CQueue::AdjustBlockUsedCount()
{
    size_t uUsed = 0;
    for (CBlock16* pCur = m_pHead; pCur; pCur = pCur->GetNext(), uUsed++);
    m_uBlockUsedCount = uUsed;
}
 */

inline CBlock16* CQueue::GetHead()
{
    return m_pHead;
}

inline CBlock16* CQueue::GetTail()
{
    return m_pTail;
}

// CBlockQueue

inline CBlockQueue::CBlockQueue(size_t uBlockCount) : CQueue(uBlockCount), m_iRef(1)
{
}

inline CBlockQueue::~CBlockQueue()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_unlock(&m_mutex);
    pthread_mutex_destroy(&m_mutex);
}

inline void CBlockQueue::Release()
{
    bool bRelease = false;
    
    pthread_mutex_lock(&m_mutex);
    ASSERT(m_iRef > 0);
    --m_iRef;
    if (m_iRef <= 0)
    {
        bRelease = true;
    }
    pthread_mutex_unlock(&m_mutex);
    
    if (bRelease)
    {
        delete this;
    }
}

inline bool CBlockQueue::DeQueue(CBlock16*& pBlock)
{
    pthread_mutex_lock(&m_mutex);
    if (IsEmpty())
    {
        if (pthread_cond_wait(&m_cond, &m_mutex) < 0)
        {
            pthread_mutex_unlock(&m_mutex);
            return false;
        }
    }
    bool bRet = CQueue::DeQueue(pBlock);
    pthread_mutex_unlock(&m_mutex);
    return bRet;
}

inline bool CBlockQueue::EnQueue(CBlock16* pBlock)
{
    pthread_mutex_lock(&m_mutex);
    if (IsFull())
    {
        pthread_mutex_unlock(&m_mutex);
        return false;
    }

    bool bRet = CQueue::EnQueue(pBlock);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutex);
    return bRet;
}

inline bool CBlockQueue::Signal()
{
    return pthread_cond_signal(&m_cond) >= 0;
}

// CWorker

inline CWorker::CWorker() : m_pQueue(NULL), m_bRunning(false)
{
}

inline CWorker::~CWorker()
{
    Cancel();
    if (m_pQueue) m_pQueue->Release();
}

inline void CWorker::Stop()
{
    if (m_bRunning)
    {
        m_bRunning = false;
        m_pQueue->Signal();
    }
}

inline void CWorker::Cancel()
{
    if (m_bRunning)
    {
        CThread::Cancel();
        //m_uTid = 0;
        m_bRunning = false;
    }
}

inline bool CWorker::AddBlock(CBlock16* pBlock)
{
    if (!pBlock || !m_pQueue)
    {
        return false;
    }

    return m_pQueue->EnQueue(pBlock);
}

inline bool CWorker::OnStart()
{
    return true;
}

inline bool CWorker::OnWork(CBlock16* pBlock)
{
    return true;
}

inline bool CWorker::OnEmpty()
{
    return true;
}

inline bool CWorker::OnStop()
{
    return true;
}




#endif // __TSWORKER_INL__

