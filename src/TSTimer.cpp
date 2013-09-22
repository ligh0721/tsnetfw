/* 
 * File:   TSTimer.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月14日, 下午6:58
 */

#include "TSPlatform.h"
#include "TSEventInterface.h"
#include "TSTimer.h"
#include "TSDebug.h"


// CTimerQueue

bool CTimerQueue::AddTimer(CTimerQueueNode* pTimer, long lInterval, long lDelay)
{
    if (!pTimer)
    {
        return false;
    }
    
    LOG_ERR("MSG | AddTimer(%p, %ld, %ld)", pTimer, lInterval, lDelay);

    Update();

    pTimer->SetExpireLeft(lDelay);
    pTimer->SetInterval(lInterval);

    return AddTimerToQueue(pTimer);
}

bool CTimerQueue::RemoveTimer(CTimerNodeBase* pTimer)
{
    bool bRes = RemoveTimerFromQueue((CTimerQueueNode*)pTimer);
    if (!bRes)
    {
        return false;
    }

    delete pTimer;

    return true;
}
#include <unistd.h>
#include <time.h>
#include <stdio.h>

bool CTimerQueue::Update()
{
    struct timeval stTv(m_stTv);
    gettimeofday(&m_stTv, 0);
    // 距上次经过的毫秒数
    long lMSec = (m_stTv.tv_sec - stTv.tv_sec) * 1000 + (m_stTv.tv_usec - stTv.tv_usec) / 1000;


    CTimerQueueNode* pCur;
    //pCur = m_pHead;
    //long lOffset;



    long lUpdateExpire;

    while (m_pHead)
    {
        pCur = m_pHead;

        //lOffset = pCur->GetExpireLeft() - lMSec;
        //lMSec -= pCur->GetExpireLeft();
        //if (!(time(NULL) % 10)) sleep(4);

        // 计算新终期剩余时长，lMSec：为刚逝去的时间
        lUpdateExpire = pCur->GetExpireLeft() - lMSec;

        // 计算下一轮计算时所需的过去时间
        lMSec -= pCur->GetExpireLeft();

        pCur->SetExpireLeft(lUpdateExpire);

        if (pCur->GetExpireLeft() > 0)
        {
            //pCur->SetExpireLeft(lOffset);
            break;
        }

        // 已经超时
        pCur->SetExpireLeft(0);
        pCur->OnTimeout();
        if (m_pHead == pCur)
        {
            // HandleTimeoutEvent中没有RemoveTimer操作
            //for (CTimerQueueNode* pCur = m_pHead; pCur; pCur = pCur->GetNext()) printf("%ld ", pCur->GetExpireLeft()); printf("update\n");
            RemoveTimerFromQueue(pCur);
            //for (CTimerQueueNode* pCur = m_pHead; pCur; pCur = pCur->GetNext()) printf("%ld ", pCur->GetExpireLeft()); printf("update\n");
            // 重新添加该timer到队列
            //printf("<<<%ld>>>\n", pCur->GetInterval());
            pCur->SetExpireLeft(pCur->GetInterval());
            //for (CTimerQueueNode* pCur = m_pHead; pCur; pCur = pCur->GetNext()) printf("%ld ", pCur->GetExpireLeft()); printf("update\n");
            AddTimerToQueue(pCur);
            //for (CTimerQueueNode* pCur = m_pHead; pCur; pCur = pCur->GetNext()) printf("%ld ", pCur->GetExpireLeft()); printf("update\n");
        }

    }


    return true;
}

bool CTimerQueue::AddTimerToQueue(CTimerQueueNode* pTimer)
{
    //printf("AddToQueue\n");
    CTimerQueueNode* pCur;
    if (!m_pHead)
    {
        pTimer->SetPrev(NULL);
        pTimer->SetNext(NULL);
        m_pHead = pTimer;
        return true;
    }

    for (pCur = m_pHead; pCur; pCur = pCur->GetNext())
    {
        if (pTimer->GetExpireLeft() < pCur->GetExpireLeft())
        {
            pTimer->SetNext(pCur);
            pTimer->SetPrev(pCur->GetPrev());

            if (pCur->GetPrev())
            {
                pCur->GetPrev()->SetNext(pTimer);
            }
            else
            {
                m_pHead = pTimer;
            }

            pCur->SetPrev(pTimer);
            pCur->SetExpireLeft(pCur->GetExpireLeft() - pTimer->GetExpireLeft());

            break;
        }

        pTimer->SetExpireLeft(pTimer->GetExpireLeft() - pCur->GetExpireLeft());

        if (!pCur->GetNext())
        {
            // apend to the tail
            pTimer->SetPrev(pCur);
            pTimer->SetNext(NULL);
            pCur->SetNext(pTimer);

            break;
        }
    }

    return true;
}

bool CTimerQueue::RemoveTimerFromQueue(CTimerQueueNode* pTimer)
{
    //printf("RemoveFromQueue\n");
    if (!pTimer)
    {
        return false;
    }

    if (pTimer->GetPrev())
    {
        pTimer->GetPrev()->SetNext(pTimer->GetNext());
    }
    else
    {
        m_pHead = pTimer->GetNext();
    }

    if (pTimer->GetNext())
    {
        pTimer->GetNext()->SetPrev(pTimer->GetPrev());
        pTimer->GetNext()->SetExpireLeft(pTimer->GetNext()->GetExpireLeft() + pTimer->GetExpireLeft());
    }
    return true;
}


