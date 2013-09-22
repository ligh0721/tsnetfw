/* 
 * File:   TSList.h
 * Author: thunderliu
 *
 * Created on 2012年2月20日, 上午11:18
 */

#ifndef __TSLIST_H__
#define	__TSLIST_H__

template <typename DATA>
class CList
{
public:

    class CListNode
    {
    protected:
        //CListNode();

    public:
        //static CListNode* CreateNode();

    public:
        CListNode* pNext;
        CListNode* pPrev;
        DATA tData;

    public:
        //void Release();
    };

    typedef bool (CList::*TRAVERSECALLBACKFUNC)(CListNode& rNode, void* pParam);

public:
    CList();
    virtual ~CList();
    bool Init();
    //void Release();
    size_t GetCount() const;
    CListNode* GetHeadNode();
    CListNode* GetTailNode();
    DATA* GetHead();
    DATA* GetTail();
    DATA* GetData(CListNode* pPosition);
    CListNode* InsertBefore(const DATA& rData, CListNode* pPosition);
    CListNode* InsertAfter(const DATA& rData, CListNode* pPosition);
    bool InsertNodeBefore(CListNode* pNode, CListNode* pPosition);
    bool InsertNodeAfter(CListNode* pNode, CListNode* pPosition);
    bool PickNode(CListNode* pPosition);
    CListNode* PushHead(const DATA& rData);
    CListNode* PushTail(const DATA& rData);
    DATA* PopHead();
    DATA* PopTail();
    bool PushHeadNode(CListNode* pNode);
    bool PushTailNode(CListNode* pNode);
    CListNode* PopHeadNode();
    CListNode* PopTailNode();

    bool TraverseFromHeadToTail(TRAVERSECALLBACKFUNC TraverseCallback, void* pParam = NULL);
    bool TraverseFromTailToHead(TRAVERSECALLBACKFUNC TraverseCallback, void* pParam = NULL);

protected:
    CListNode* m_pHead;
    CListNode* m_pTail;
    size_t m_uCount;

};

template <typename DATA>
class CResourceManager
{
public:
    typedef CList<DATA> CResList;
    typedef typename CList<DATA>::CListNode CResNode;

public:
    CResourceManager();
    bool Init(size_t uFreeCount);
    bool Release();
    CResNode* AllocFreeRes();
    bool FreeRes(CResNode* pRes);
    CResNode* PeekHeadUsedRes();
    size_t GetFreeCount() const;
    size_t GetUsedCount() const;
    size_t GetTotalCount() const;

protected:
    CResList m_oFreeList;
    CResList m_oUsedList;
    size_t m_uFreeCount;
    size_t m_uUsedCount;
};




#include "TSList.inl"

#endif	/* __TSLIST_H__ */

