/* 
 * File:   TSWorker.h
 * Author: thunderliu
 *
 * Created on 2011年11月14日, 下午11:41
 */

#ifndef __TSWORKER_H__
#define	__TSWORKER_H__

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include "TSThread.h"

class CBlock16
{
protected:
    CBlock16(uint16_t wSize, void* pBuf);
    virtual ~CBlock16();

public:
    static CBlock16* CreateBlock(uint16_t wSize);
    void Release();
    uint16_t GetSize() const;
    void* GetBuffer();
    CBlock16* GetPrev();
    void SetPrev(CBlock16* pBlock);
    CBlock16* GetNext();
    void SetNext(CBlock16* pBlock);

protected:
    uint16_t m_wSize;
    void* m_pBuf;
    CBlock16* m_pPrev;
    CBlock16* m_pNext;
};

class CQueue
{
protected:
    CQueue(size_t uBlockCount);
    virtual ~CQueue();

public:
    static CQueue* CreateQueue(size_t uBlockCount);
    virtual void Release();
    size_t GetBlockMaxCount() const;
    size_t GetBlockUsedCount() const;
    virtual bool DeQueue(CBlock16*& pBlock);
    virtual bool EnQueue(CBlock16* pBlock);
    bool IsEmpty() const;
    bool IsFull() const;
    //void AdjustBlockUsedCount();
    CBlock16* GetHead();
    CBlock16* GetTail();

protected:
    size_t m_uBlockMaxCount;
    size_t m_uBlockUsedCount;
    CBlock16* m_pHead;
    CBlock16* m_pTail;

};

class CBlockQueue : public CQueue
{
protected:
    CBlockQueue(size_t uBlockCount);
    virtual ~CBlockQueue();

public:
    static CBlockQueue* CreateBlockQueue(size_t uBlockCount);
    static CBlockQueue* ReferenceBlockQueue(CBlockQueue* pQueue);
    virtual void Release();
    virtual bool DeQueue(CBlock16*& pBlock);
    virtual bool EnQueue(CBlock16* pBlock);
    bool Signal();

protected:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    int m_iRef;

};

class CWorker : public CThread
{
    friend class CMirrorWorker;
    
public:
    CWorker();
    virtual ~CWorker();

public:
    bool Init(size_t uBlockCount);
    bool Start();
    void Stop();
    void Cancel();
    //void Wait();

    bool AddBlock(CBlock16* pBlock);

protected:
    long ThreadProc();

protected:
    virtual bool OnStart();
    virtual bool OnWork(CBlock16* pBlock); // donot release pBlock in this call
    virtual bool OnEmpty();
    virtual bool OnStop();

protected:
    CBlockQueue* m_pQueue;
    //pthread_t m_pthrd;
    bool m_bRunning;
};

class CMirrorWorker : public CWorker
{
public:
    bool InitWithWorker(CWorker* pWorker);

};
    
#include "TSWorker.inl"





#endif	/* __TSWORKER_H__ */

