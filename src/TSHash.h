/* 
 * File:   TSHash.h
 * Author: thunderliu
 *
 * Created on 2011年12月14日, 下午8:41
 */

#ifndef __TSHASH_H__
#define	__TSHASH_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
using namespace std;

#include "TSUtil.h"


uint32_t udc_crc32(unsigned long crc,
                   const unsigned char* buf,
                   int len);

typedef struct
{
    char _unused[0];
} EMPTY;

#define HASH_MAX_HEIGHT       60
#define HASH_INVALID_MAPPED_POINTER   ((CHashNode*)(-1))

/*
template<typename KEY, typename MAPPED, class NODE = CHashNodeBase<KEY, MAPPED>, typename EXTENTION = EMPTY>
class CHashHeader
{
public:
    uint32_t dwHashMaxHeight; // 最大允许hash行数，静态常成员
    KEY tEmptyKey; // 空key常量
    uint32_t dwHashNodeSize; // 节点尺寸常量

    uint32_t dwHashHeight; // hash行数
    uint32_t dwHashWidth; // hash列数
    size_t uHashNodeCount;
    size_t uHashNodeUsedCount;  // hash已使用结点数
    uint32_t adwHashMods[HASH_MAX_HEIGHT]; // 对应行的素数 ,比m_oHash.dwHashRowCount小的最大素数, 降序排列，总共m_oHash.dwHashLineCount个
    uint32_t dwHashCurHeight; // 当前hash使用行数
    EXTENTION tExtention;
    
    NODE atHashNodes[0]; // hash内存地址，共享内存地址

};

template<typename KEY, typename MAPPED>
class CSeqHashNode : public CDataSequenceInterface, public CHashNodeBase<KEY, MAPPED>
{
public:
    KEY tKey;
    MAPPED tMapped;

};
 */


// CDataStorage KEY：hash键类型，MAPPED：hash映射值类型，EXTENTION：扩展哈希头部数据

template <typename KEY, typename MAPPED, typename EXTENTION = EMPTY>
class CHashMap
{
public:
    //static const uint32_t HASH_MAX_LINE = 60; // 最大允许hash行数，静态常成员
    //const uint32_t HASH_NODE_SIZE; // 节点尺寸常量
    const KEY HASH_EMPTY_KEY; // 空key常量

    struct CHashNode
    {
    public:
        KEY tKey;
        MAPPED tMapped;

    };

    struct CHashHeader
    {
    public:
        uint32_t dwHashMaxHeight; // 最大允许hash行数，静态常成员
        KEY tEmptyKey; // 空key常量
        uint32_t dwHashNodeSize; // 节点尺寸常量
        uint32_t dwHashHeight; // hash行数
        uint32_t dwHashWidth; // hash列数
        size_t uHashNodeCount;
        size_t uHashNodeUsedCount; // hash已使用结点数
        uint32_t adwHashMods[HASH_MAX_HEIGHT]; // 对应行的素数 ,比m_oHash.dwHashRowCount小的最大素数, 降序排列，总共m_oHash.dwHashLineCount个
        uint32_t dwHashCurHeight; // 当前hash使用行数
        EXTENTION tExtention;
        CHashNode aoHashNodes[0]; // hash内存地址，共享内存地址

    };


public:
    typedef KEY TYPE_KEY;
    typedef MAPPED TYPE_MAPPED;
    typedef EXTENTION TYPE_EXTENTION;

    // hash非空节点遍历回调函数类型，rNode：当前遍历到的非空节点，pParam：附带的参数，调用CHashTable::Traverse时传入
    typedef bool (CHashMap::*TRAVERSECALLBACKFUNC)(CHashNode& rNode, void* pParam);

public:
    CHashMap();
    virtual ~CHashMap();

    bool Init(uint32_t dwHashWidth, uint32_t dwHashHeight, const KEY& rEmptyKey);
    bool Release();

    // 查找键为rKey的节点，错误返回-1，没找到返回NULL，注：判断没找到或错误时，不可以用<=0，必须用或运算
    CHashNode* FindNode(const KEY& rKey);

    // 查找键位rKey的节点，错误返回-1，pbNew传回是否为新的空节点，没找到该节点时即返回新的空节点，用于添加hash节点
    CHashNode* FindNodeToSet(const KEY& rKey, bool* pbNew = NULL);

    // 获取节点索引
    size_t GetNodeIndex(CHashNode* pNode);

    // 通过索引直接获取节点
    CHashNode* GetNode(size_t uIndex);

    // hash非空节点遍历，对每一个非空节点都会回调TraverseCallback，pParam为调用是附带的参数，可应用用于hash dump
    bool Traverse(TRAVERSECALLBACKFUNC TraverseCallback, void* pParam = NULL);

    // 用key为m_oHash.HASH_EMPTY_KEY的节点填充共享内存，这样整个hash逻辑上是空的
    bool Clear();

    void* GetHeader();
    void SetHeaderAddress(void* pHdr);
    void CopyHeaderMemory(const void* pHdr);

    bool ClearNode(CHashNode& rNode);

    int GetHashNodeUr() const;
    int GetHashLineUr() const;
    
    virtual size_t GetHeaderSize() const;
    virtual size_t GetNodeSize() const;
    
    struct timeval* GetTimeValueFind();
        
protected:
    virtual bool AllocAndInitHashMemory(size_t uSize, const CHashHeader& roHdr);
    
public:
    virtual bool FreeHeaderMemory();

    // 默认键值匹配回调函数
    virtual bool NodeMatch(const KEY& rKey, CHashNode& rNode);

protected:

    void* m_pHeader;
    struct timeval m_stTv;

};

#define IS_INVALID_NODE(node) ((long)(node) == -1)
#define ISNOT_INVALID_NODE(node) ((long)(node) != -1)

template <typename MAPPED>
class CSimple32Hash : public CHashMap<uint32_t, MAPPED>
{
public:
    typedef CHashMap<uint32_t, MAPPED> CSimpleHash;
    typedef typename CSimpleHash::CHashNode CHashNode;

protected:
    virtual bool NodeMatch(const uint32_t& rKey, CHashNode& rNode);

};

template <typename MAPPED>
class CSimple64Hash : public CHashMap<uint64_t, MAPPED>
{
public:
    typedef CHashMap<uint64_t, MAPPED> CSimpleHash;
    typedef typename CSimpleHash::CHashNode CHashNode;

protected:
    virtual bool NodeMatch(const uint64_t& rKey, CHashNode& rNode);

};


#include "TSMemory.h"

template <typename KEY, typename MAPPED, typename EXTENTION = EMPTY>
class CShmHash : public CHashMap<KEY, MAPPED, EXTENTION>
{
public:
    typedef typename CHashMap<KEY, MAPPED, EXTENTION>::CHashNode CHashNode;
    typedef typename CHashMap<KEY, MAPPED, EXTENTION>::CHashHeader CHashHeader;
    
public:
    virtual bool Init(uint32_t dwHashWidth, uint32_t dwHashHeight, const KEY& rEmptyKey, key_t dwShmKey);
    virtual bool Attach(key_t dwShmKey, bool bReadOnly);
    
protected:
    virtual bool AllocAndInitHashMemory(size_t uSize, const CHashHeader& roHdr);
    
public:
    virtual bool FreeHeaderMemory();
    
    CShareMemory m_oShm;
    key_t m_dwShmKey;
};


class CDataSequenceInterface
{
public:
    CDataSequenceInterface(uint32_t dwSeq = 0);

    uint32_t NextSequence();
    bool IsNewerThan(const CDataSequenceInterface* poObj) const;
    uint32_t GetSequence() const;
    void SetSequence(uint32_t dwSeq);

protected:
    uint32_t _dwSeq;
};



template <typename KEY, typename MAPPED, typename DATA = MAPPED, typename EXTENTION = EMPTY>
class CPersistentHash : public CShmHash<KEY, MAPPED, EXTENTION>
{
private:
    typedef typename CHashMap<KEY, MAPPED, EXTENTION>::CHashNode CHashNode_;
    typedef typename CHashMap<KEY, MAPPED, EXTENTION>::CHashHeader CHashHeader_;
    
public:
    typedef CPersistentHash CPersistentHash_;
    struct CHashNode : public CHashNode_, public CDataSequenceInterface {};
    struct CHashHeader : public CHashHeader_, public CDataSequenceInterface {};
        
    struct CDataCell : public CDataSequenceInterface
    {
    public:
        KEY tKey;
        DATA tData;
    };
        
    CPersistentHash();

    virtual bool Init(uint32_t dwHashWidth, uint32_t dwHashHeight, const KEY& rEmptyKey, key_t dwShmKey);
    
    virtual size_t GetHeaderSize() const;
    virtual size_t GetNodeSize() const;
        
    bool HandleData(CDataCell& rData);
    
    bool DumpMemory();
    
    virtual bool OnHandleData(const CDataCell& rData);
    
    virtual bool OnWriteBinlog(const CDataCell& rData);
    virtual bool OnLoadBinlog();
    virtual bool OnDumpMemory();
    virtual bool OnLoadMemory();
};


#include "TSThread.h"

template <typename KEY, typename MAPPED, typename DATA = MAPPED, typename EXTENTION = EMPTY>
class CNormalPersistentHash : public CPersistentHash<KEY, MAPPED, DATA, EXTENTION>
{
public:
    typedef CPersistentHash<KEY, MAPPED, DATA, EXTENTION> CPersistentHash_;
    typedef typename CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::CHashHeader CHashHeader;
    typedef typename CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::CHashNode CHashNode;
    typedef typename CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::CDataCell CDataCell;
    
    class CDumpThread : public CThread
    {
    public:
        virtual long ThreadProc();
        
    public:
        CNormalPersistentHash* m_pHash;
    };
    
public:
    CNormalPersistentHash();
    virtual ~CNormalPersistentHash();
    
public:
    virtual bool Init(uint32_t dwHashWidth, uint32_t dwHashHeight, const KEY& rEmptyKey, key_t dwShmKey, const char* pBinlogPrefix, const char* pBinlogSuffix, const char* pDumpPrefix, const char* pDumpSuffix);
    virtual bool Attach(key_t dwShmKey, bool bReadOnly, const char* pBinlogPrefix, const char* pBinlogSuffix, const char* pDumpPrefix, const char* pDumpSuffix);
    bool AsyncDumpMemory();
    
protected:
    virtual bool OnWriteBinlog(const CDataCell& rData);
    virtual bool OnLoadBinlog();
    virtual bool OnDumpMemory();
    virtual bool OnLoadMemory();
    
protected:
    FILE* m_pBinlog;
    string m_sBinlogPrefix;
    string m_sBinlogSuffix;
    string m_sDumpPrefix;
    string m_sDumpSuffix;
    uint32_t m_dwDateYYYYMMDD;
    int m_iDumpOffset;
    CDumpThread m_oDumpThread;
};



#include "TSHash.inl"

#endif	/* __TSHASH_H__ */

