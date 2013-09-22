/* 
 * File:   TSList.inl
 * Author: thunderliu
 *
 * Created on 2012年2月20日, 上午11:19
 */

#ifndef __TSLIST_INL__
#define	__TSLIST_INL__

/*
template <typename DATA>
inline CList<DATA>::CListNode::CListNode()
{
}

template <typename DATA>
inline typename CList<DATA>::CListNode*  CList<DATA>::CListNode::CreateNode()
{
    return new CListNode;
}

template <typename DATA>
inline void CList<DATA>::CListNode::Release()
{
    delete this;
}
 */
template <typename DATA>
inline CList<DATA>::CList()
: m_pHead(NULL)
, m_pTail(NULL)
, m_uCount(0)
{

}

template <typename DATA>
inline CList<DATA>::~CList()
{

}

template <typename DATA>
inline size_t CList<DATA>::GetCount() const
{
    return m_uCount;
}

template <typename DATA>
inline typename CList<DATA>::CListNode* CList<DATA>::GetHeadNode()
{
    return m_pHead;
}

template <typename DATA>
inline typename CList<DATA>::CListNode* CList<DATA>::GetTailNode()
{
    return m_pTail;
}

template <typename DATA>
inline DATA* CList<DATA>::GetHead()
{
    return GetData(m_pHead);
}

template <typename DATA>
inline DATA* CList<DATA>::GetTail()
{
    return GetData(m_pTail);
}

template <typename DATA>
inline DATA* CList<DATA>::GetData(CList<DATA>::CListNode* pPosition)
{
    assert(pPosition);
    return &pPosition->tData;
}

template <typename DATA>
bool CList<DATA>::Init()
{
    if (m_pHead)
    {
        //Release();
        assert(false);
    }
    else
    {
        m_pHead = NULL;
        m_pTail = NULL;
        m_uCount = 0;
    }

    return true;
}

/*
template <typename DATA>
void CList<DATA>::Release()
{
    if (!m_pHead)
    {
        return;
    }
    
    CListNode* pDel;
    CListNode* pNode = m_pHead;
    while (pNode)
    {
        pDel = pNode;
        pNode = pNode->pNext;
        pDel->Release();
    }
    
    m_pHead = NULL;
    m_pTail = NULL;
    m_uCount = 0;
}
 */
template <typename DATA>
typename CList<DATA>::CListNode* CList<DATA>::InsertBefore(const DATA& rData, CListNode* pPosition)
{
    //CListNode* pNode = CListNode::CreateNode();
    CListNode* pNode = new CListNode;
    assert(pNode);
    pNode->tData = rData;

    if (!InsertNodeBefore(pNode, pPosition))
    {
        delete pNode;
        return NULL;
    }
    return pNode;
}

template <typename DATA>
typename CList<DATA>::CListNode* CList<DATA>::InsertAfter(const DATA& rData, CListNode* pPosition)
{
    //CListNode* pNode = CListNode::CreateNode();
    CListNode* pNode = new CListNode;
    assert(pNode);
    pNode->tData = rData;

    if (!InsertNodeAfter(pNode, pPosition))
    {
        delete pNode;
        return NULL;
    }
    return pNode;
}

template <typename DATA>
bool CList<DATA>::InsertNodeBefore(CListNode* pNode, CListNode* pPosition)
{
    pNode->pNext = pPosition;

    if (!m_pHead)
    {
        // 空表
        assert(!pPosition);
        m_pHead = pNode;
        m_pTail = pNode;
        pNode->pPrev = NULL;

        return true;
    }

    // 非空表
    assert(pPosition);

    pNode->pPrev = pPosition->pPrev;

    if (pPosition->pPrev)
    {
        // 参照位置不是头节点
        pPosition->pPrev->pNext = pNode;
    }
    else
    {
        // 参照位置是头节点
        m_pHead = pNode;
    }

    pPosition->pPrev = pNode;

    return true;
}

template <typename DATA>
bool CList<DATA>::InsertNodeAfter(CListNode* pNode, CListNode* pPosition)
{
    pNode->pPrev = pPosition;

    if (!m_pTail)
    {
        // 空表
        assert(!pPosition);
        m_pTail = pNode;
        m_pHead = pNode;
        pNode->pNext = NULL;

        return true;
    }

    // 非空表
    assert(pPosition);

    pNode->pNext = pPosition->pNext;

    if (pPosition->pNext)
    {
        // 参照位置不是尾节点
        pPosition->pNext->pPrev = pNode;
    }
    else
    {
        // 参照位置是尾节点
        m_pTail = pNode;
    }

    pPosition->pNext = pNode;

    return true;
}

template <typename DATA>
bool CList<DATA>::PickNode(CListNode* pPosition)
{
    if (!pPosition || !m_pHead || !m_pTail)
    {
        assert(!pPosition & !m_pHead & !m_pTail);
        return false;
    }


    if (pPosition == m_pHead)
    {
        // pick head
        m_pHead = m_pHead->pNext;
        if (m_pHead)
        {
            m_pHead->pPrev = NULL;
        }
        else
        {
            // 没有剩余节点
            m_pTail = NULL;
        }

        return true;
    }

    if (pPosition == m_pTail)
    {
        // pick tail
        m_pTail = m_pTail->pPrev;
        if (m_pTail)
        {
            m_pTail->pNext = NULL;
        }
        else
        {
            // 没有剩余节点
            m_pHead = NULL;
        }

        return true;
    }

    pPosition->pPrev->pNext = pPosition->pNext;
    pPosition->pNext->pPrev = pPosition->pPrev;

    return true;
}

template <typename DATA>
inline typename CList<DATA>::CListNode* CList<DATA>::PushHead(const DATA& rData)
{
    return InsertBefore(rData, m_pHead);
}

template <typename DATA>
inline typename CList<DATA>::CListNode* CList<DATA>::PushTail(const DATA& rData)
{
    return InsertAfter(rData, m_pTail);
}

template <typename DATA>
inline DATA* CList<DATA>::PopHead()
{
    DATA* pData = GetHead();
    PickNode(m_pHead);

    return pData;
}

template <typename DATA>
inline DATA* CList<DATA>::PopTail()
{
    DATA* pData = GetTail();
    PickNode(m_pTail);

    return pData;
}

template <typename DATA>
inline bool CList<DATA>::PushHeadNode(CListNode* pNode)
{
    return InsertNodeBefore(pNode, m_pHead);
}

template <typename DATA>
inline bool CList<DATA>::PushTailNode(CListNode* pNode)
{
    return InsertNodeAfter(pNode, m_pTail);
}

template <typename DATA>
inline typename CList<DATA>::CListNode* CList<DATA>::PopHeadNode()
{
    CListNode* pNode = m_pHead;
    PickNode(m_pHead);

    return pNode;
}

template <typename DATA>
inline typename CList<DATA>::CListNode* CList<DATA>::PopTailNode()
{
    CListNode* pNode = m_pTail;
    PickNode(m_pTail);

    return pNode;
}

template <typename DATA>
bool CList<DATA>::TraverseFromHeadToTail(TRAVERSECALLBACKFUNC TraverseCallback, void* pParam)
{
    for (CListNode* pNode = m_pHead; pNode; pNode = pNode->pNext)
    {
        if (!(this->*TraverseCallback)(*pNode, pParam))
        {
            return false;
        }
    }

    return true;
}

template <typename DATA>
bool CList<DATA>::TraverseFromTailToHead(TRAVERSECALLBACKFUNC TraverseCallback, void* pParam)
{
    for (CListNode* pNode = m_pTail; pNode; pNode = pNode->pPrev)
    {
        if (!(this->*TraverseCallback)(*pNode, pParam))
        {
            return false;
        }
    }

    return true;
}


// CResourceManager

template <typename DATA>
inline CResourceManager<DATA>::CResourceManager()
: m_uFreeCount(0)
, m_uUsedCount(0)
{
}

template <typename DATA>
bool CResourceManager<DATA>::Init(size_t uFreeCount)
{
    if (!m_oFreeList.Init())
    {
        return false;
    }

    if (!m_oUsedList.Init())
    {
        return false;
    }

    CResNode* pNode;
    for (size_t i = 0; i < uFreeCount; i++)
    {
        pNode = new CResNode;
        assert(pNode);
        if (!m_oFreeList.InsertNodeAfter(pNode, m_oFreeList.GetTailNode()))
        {
            delete pNode;
            continue;
        }
        m_uFreeCount++;
    }

    //m_uFreeCount = uFreeCount;
    m_uUsedCount = 0;
}

template <typename DATA>
bool CResourceManager<DATA>::Release()
{
    if (!GetTotalCount())
    {
        return false;
    }

    CResNode* pNode;
    while (pNode = m_oFreeList.PopHeadNode())
    {
        delete pNode;
    }
    m_uFreeCount = 0;

    while (pNode = m_oUsedList.PopHeadNode())
    {
        delete pNode;
    }
    m_uUsedCount = 0;

    return true;
}

template <typename DATA>
inline typename CResourceManager<DATA>::CResNode* CResourceManager<DATA>::AllocFreeRes()
{
    if (!m_uFreeCount)
    {
        return NULL;
    }

    CResNode* pNode = m_oFreeList.PopHeadNode();
    m_oUsedList.PushTailNode(pNode);
    --m_uFreeCount;
    ++m_uUsedCount;

    return pNode;
}

template <typename DATA>
inline bool CResourceManager<DATA>::FreeRes(CResNode* pRes)
{
    if (!m_uUsedCount)
    {
        return false;
    }

    m_oUsedList.PickNode(pRes);
    bool bRes = m_oFreeList.PushTailNode(pRes);
    --m_uUsedCount;
    ++m_uFreeCount;

    return bRes;
}

template <typename DATA>
inline typename CResourceManager<DATA>::CResNode* CResourceManager<DATA>::PeekHeadUsedRes()
{
    return m_oUsedList.GetHeadNode();
}

template <typename DATA>
inline size_t CResourceManager<DATA>::GetFreeCount() const
{
    return m_uFreeCount;
}

template <typename DATA>
inline size_t CResourceManager<DATA>::GetUsedCount() const
{
    return m_uUsedCount;
}

template <typename DATA>
inline size_t CResourceManager<DATA>::GetTotalCount() const
{
    return m_uFreeCount + m_uUsedCount;
}






#endif	/* __TSLIST_INL__ */

