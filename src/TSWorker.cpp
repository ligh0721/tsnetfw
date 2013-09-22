/* 
 * File:   TSWorker.cpp
 * Author: thunderliu
 *
 * Created on 2011年11月14日, 下午11:41
 */

#include "TSPlatform.h"
#include "TSWorker.h"


// CBlock16

CBlock16* CBlock16::CreateBlock(uint16_t wSize)
{
    if (wSize <= 0)
    {
        return NULL;
    }

    void* pBuf = malloc(wSize);
    if (!pBuf)
    {
        return NULL;
    }

    CBlock16* pBlock = new CBlock16(wSize, pBuf);
    if (!pBlock)
    {
        free(pBuf);
        return NULL;
    }

    return pBlock;
}

// CQueue

CQueue* CQueue::CreateQueue(size_t uBlockCount)
{
    if (uBlockCount <= 0)
    {
        return NULL;
    }

    /*
    CBlock16** ppBlocks = (CBlock16**)malloc(sizeof(CBlock16*) * uBlockCount);
    if (!ppBlocks)
    {
        return NULL;
    }
     */

    CQueue* pQueue = new CQueue(uBlockCount);
    if (!pQueue)
    {
        //free(ppBlocks);
        return NULL;
    }

    return pQueue;
}

bool CQueue::DeQueue(CBlock16*& pBlock)
{
    if (!m_pHead)
    {
        return false;
    }

    pBlock = m_pHead;
    m_pHead = m_pHead->GetNext();

    if (!m_pHead)
    {
        m_pTail = NULL;
    }
    else
    {
        m_pHead->SetPrev(NULL);
    }

    --m_uBlockUsedCount;

    if (!m_uBlockUsedCount && m_pHead == m_pTail)
    {
        m_pHead = m_pTail = NULL;
    }

    pBlock->SetPrev(NULL);
    pBlock->SetNext(NULL);

    return true;
}

bool CQueue::EnQueue(CBlock16* pBlock)
{
    if (!pBlock)
    {
        return false;
    }
    /*
    if (IsFull())
    {
        printf("EnQueue: but FULL(%lu/%lu)\n", m_uBlockUsedCount, m_uBlockMaxCount);
        return false;
    }
     */

    ++m_uBlockUsedCount;

    if (!m_pTail)
    {
        m_pHead = pBlock;
        m_pTail = pBlock;
        m_pHead->SetPrev(NULL);
    }
    else
    {
        m_pTail->SetNext(pBlock);
        pBlock->SetPrev(m_pTail);
        m_pTail = pBlock;
    }

    return true;
}

// CBlockQueue

CBlockQueue* CBlockQueue::CreateBlockQueue(size_t uBlockCount)
{
    if (uBlockCount <= 0)
    {
        return NULL;
    }

    CBlockQueue* pQueue = new CBlockQueue(uBlockCount);
    if (!pQueue)
    {
        //free(ppBlocks);
        return NULL;
    }

    int iRet = pthread_mutex_init(&pQueue->m_mutex, NULL);
    //ASSERT(iRet >= 0);
    if (iRet < 0)
    {
        delete pQueue;
        return NULL;
    }

    iRet = pthread_cond_init(&pQueue->m_cond, NULL);
    //ASSERT(iRet >= 0);
    if (iRet < 0)
    {
        pthread_mutex_destroy(&pQueue->m_mutex);
        delete pQueue;
        return NULL;
    }
    return pQueue;
}

CBlockQueue* CBlockQueue::ReferenceBlockQueue(CBlockQueue* pQueue)
{
    pthread_mutex_lock(&pQueue->m_mutex);
    ASSERT(pQueue->m_iRef > 0);
    ++pQueue->m_iRef;
    pthread_mutex_unlock(&pQueue->m_mutex);
    return pQueue;
}

// CWorker

bool CWorker::Init(size_t uBlockCount)
{
    if (m_bRunning)
    {
        return false;
    }

    if (uBlockCount <= 0)
    {
        return false;
    }

    if (m_pQueue)
    {
        m_pQueue->Release();
    }
    m_pQueue = CBlockQueue::CreateBlockQueue(uBlockCount);
    if (!m_pQueue)
    {
        return false;
    }

    return true;
}

bool CWorker::Start()
{
    if (!m_pQueue)
    {
        return false;
    }

    if (m_bRunning)
    {
        return false;
    }

    if (!OnStart())
    {
        return false;
    }

    m_bRunning = true;

    if (!CThread::Start())
    {
        return false;
    }

    return true;
}

long CWorker::ThreadProc()
{
    CBlock16* pBlock;
    bool bRet;
    while (m_bRunning)
    {
        bRet = m_pQueue->DeQueue(pBlock);
        if (!bRet)
        {
            break;
        }

        bRet = OnWork(pBlock);
        pBlock->Release();
        if (!bRet)
        {
            // false
            break;
        }
        if (m_pQueue->IsEmpty())
        {
            bRet = OnEmpty();
            if (!bRet)
            {
                // false
                break;
            }
        }

    }

    OnStop();
    m_bRunning = false;
    //m_pthrd = 0;

    return 0;
}

// CMirrorWorker

bool CMirrorWorker::InitWithWorker(CWorker* pWorker)
{
    if (m_bRunning)
    {
        return false;
    }

    if (m_pQueue)
    {
        m_pQueue->Release();
    }
    m_pQueue = CBlockQueue::ReferenceBlockQueue(pWorker->m_pQueue);
    if (!m_pQueue)
    {
        return false;
    }
    return true;
}
