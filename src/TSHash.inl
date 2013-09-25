/* 
 * File:   TSHash.inl
 * Author: thunderliu
 *
 * Created on 2011年12月14日, 下午8:42
 */

#ifndef __TSHASH_INL__
#define	__TSHASH_INL__

#include <string.h>
#include <assert.h>

#include "TSHash.h"
#include "TSDebug.h"
#include "TSMemory.h"
#include "TSPlatform.h"
// CHashBase

template <typename KEY, typename MAPPED, typename EXTENTION>
inline CHashMap<KEY, MAPPED, EXTENTION>::CHashMap()
: HASH_EMPTY_KEY()
, m_pHeader(NULL)
{
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline CHashMap<KEY, MAPPED, EXTENTION>::~CHashMap()
{
}

template <typename KEY, typename MAPPED, typename EXTENTION>
bool CHashMap<KEY, MAPPED, EXTENTION>::Init(uint32_t dwHashWidth, uint32_t dwHashHeight, const KEY& rEmptyKey)
{
    if (m_pHeader)
    {
        return false;
    }

    const_cast<KEY&>(HASH_EMPTY_KEY) = rEmptyKey;
    /*
    m_pHeader->dwHashMaxLine = HASH_MAX_LINE;
    m_pHeader->dwHashNodeSize = HASH_NODE_SIZE;
    m_pHeader->iHashShmKey = iHashShmKey;
    m_pHeader->dwHashHeight = dwHashHeight;
    m_pHeader->dwHashRowCount = dwHashRowCount;
     */

    CHashHeader oTmpHdr;
    oTmpHdr.dwHashMaxHeight = HASH_MAX_HEIGHT;
    oTmpHdr.tEmptyKey = rEmptyKey;
    oTmpHdr.dwHashNodeSize = GetNodeSize();

    oTmpHdr.dwHashHeight = dwHashHeight;
    oTmpHdr.dwHashWidth = dwHashWidth;
    oTmpHdr.dwHashCurHeight = 0;
    oTmpHdr.uHashNodeCount = 0;
    oTmpHdr.uHashNodeUsedCount = 0;
    memset(&oTmpHdr.tExtention, sizeof (oTmpHdr.tExtention), 0);

    // 生成质数序列，质数序列为从小于m_oHash.dwHashRowCount的第一个质数开始降序排列，总共m_oHash.dwHashHeight个
    uint32_t dwIndex = 0;
    uint32_t dwValue;
    bool bIsPrime;
    for (dwValue = oTmpHdr.dwHashWidth; dwValue > 1; dwValue--)
    {
        if (dwValue < 2)
        {
            bIsPrime = false;
        }
        else
        {
            bIsPrime = true;
            for (uint32_t i = 2; i < dwValue && i * i <= dwValue; i++)
            {
                if (dwValue % i == 0)
                {
                    bIsPrime = false;
                    break;
                }
            }
        }

        if (bIsPrime)
        {
            oTmpHdr.adwHashMods[dwIndex] = dwValue;
            oTmpHdr.uHashNodeCount += dwValue;
            dwIndex++;
            if (dwIndex >= oTmpHdr.dwHashHeight)
            {
                break;
            }
        }
    }

    if (dwValue == 1)
    {
        return false;
    }

    if (!AllocAndInitHashMemory(GetHeaderSize() + oTmpHdr.uHashNodeCount * GetHeaderSize(), oTmpHdr))
    {
        return false;
    }

    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline bool CHashMap<KEY, MAPPED, EXTENTION>::Release()
{
    return FreeHeaderMemory();
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline bool CHashMap<KEY, MAPPED, EXTENTION>::NodeMatch(const KEY& rKey, CHashNode& rNode)
{
    return !memcmp(&rKey, &rNode.tKey, sizeof (KEY));
}

template <typename KEY, typename MAPPED, typename EXTENTION>
typename CHashMap<KEY, MAPPED, EXTENTION>::CHashNode* CHashMap<KEY, MAPPED, EXTENTION>::FindNodeToSet(const KEY& rKey, bool* pbNew)
{
    if (!m_pHeader)
    {
        //LOG_POS("");
        return HASH_INVALID_MAPPED_POINTER;
    }

    CHashNode* pRow;
    CHashNode* pNode;
    CHashNode* pEmptyNode = NULL;
    CHashNode* pFoundNode = NULL;
    uint32_t dwKey;

    if (sizeof (KEY) > sizeof (uint32_t))
    {
        dwKey = udc_crc32(0, (const unsigned char*)(&rKey), sizeof (KEY));
    }
    else
    {
        dwKey = 0;
        memmove(&dwKey, &rKey, sizeof (KEY));
    }

    ::gettimeofday(&m_stTv, NULL);
    uint32_t i;
    uint32_t dwHashCurHeight = -1;
    CHashHeader* pHeader = (CHashHeader*)GetHeader();
    for (i = 0, pRow = pHeader->aoHashNodes; i < pHeader->dwHashHeight; pRow += pHeader->adwHashMods[i], i++)
    {
        pNode = pRow + (dwKey % pHeader->adwHashMods[i]);
        if (NodeMatch(rKey, *pNode))
        {
            pFoundNode = pNode;
            break;
        }

        if (!pEmptyNode && NodeMatch(HASH_EMPTY_KEY, *pNode))
        {
            pEmptyNode = pNode;
            dwHashCurHeight = i;
        }
    }

    if (pbNew)
    {
        *pbNew = (!pFoundNode);
    }

    if (!pFoundNode)
    {
        // 空节点to set
        if (!pEmptyNode)
        {
            // FULL错误
            return NULL;
        }
        else
        {
            // 空节点有效
            pHeader->uHashNodeUsedCount++;
            pEmptyNode->tKey = rKey; // 空节点的头部初始化
            dwHashCurHeight++; // 结果为当前查询最深行
            if (dwHashCurHeight > pHeader->dwHashCurHeight)
            {
                // 查询最深行已经超过记载的最深行
                pHeader->dwHashCurHeight = dwHashCurHeight;
            }

            return pEmptyNode;
        }
    }

    return pFoundNode;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline size_t CHashMap<KEY, MAPPED, EXTENTION>::GetNodeIndex(CHashNode* pNode)
{
    return pNode - ((CHashHeader*)GetHeader())->aoHashNodes;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline typename CHashMap<KEY, MAPPED, EXTENTION>::CHashNode* CHashMap<KEY, MAPPED, EXTENTION>::GetNode(size_t uIndex)
{
    CHashHeader* pHeader = (CHashHeader*)GetHeader();
    if (uIndex < 0 || uIndex >= pHeader->uHashNodeCount)
    {
        return NULL;
    }

    return &pHeader->aoHashNodes[uIndex];
}

template <typename KEY, typename MAPPED, typename EXTENTION>
typename CHashMap<KEY, MAPPED, EXTENTION>::CHashNode* CHashMap<KEY, MAPPED, EXTENTION>::FindNode(const KEY& rKey)
{
    if (!m_pHeader)
    {
        //LOG_POS("");
        return HASH_INVALID_MAPPED_POINTER;
    }

    CHashHeader* pHeader = (CHashHeader*)GetHeader();
    CHashNode* pRow;
    CHashNode* pNode;
    uint32_t dwKey;
    if (sizeof (KEY) > sizeof (uint32_t))
    {
        // 如果key的长度大于32bit，则取key的crc作为shortkey
        dwKey = udc_crc32(0, (const unsigned char*)(&rKey), sizeof (KEY));
    }
    else
    {
        // 如果key的长度在32bit之内，shortkey即为key
        dwKey = 0;
        memmove(&dwKey, &rKey, sizeof (KEY));
    }

    ::gettimeofday(&m_stTv, NULL);
    uint32_t i;
    for (i = 0, pRow = pHeader->aoHashNodes; i < pHeader->dwHashCurHeight; pRow += pHeader->adwHashMods[i], i++)
    {
        pNode = pRow + (dwKey % pHeader->adwHashMods[i]);
        if (NodeMatch(rKey, *pNode))
        {
            return pNode;
        }
    }

    return NULL;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
bool CHashMap<KEY, MAPPED, EXTENTION>::Traverse(TRAVERSECALLBACKFUNC TraverseCallback, void* pParam)
{
    if (!m_pHeader || !TraverseCallback)
    {
        //LOG_POS("");
        return false;
    }

    CHashHeader* pHeader = (CHashHeader*)GetHeader();
    uint32_t dwIndex = 0;
    for (uint32_t i = 0; i < pHeader->dwHashHeight; i++)
    {
        for (uint32_t j = 0; j < pHeader->adwHashMods[i]; j++, dwIndex++)
        {
            if (!NodeMatch(HASH_EMPTY_KEY, pHeader->aoHashNodes[dwIndex]))
            {
                if ((this->*TraverseCallback)(pHeader->aoHashNodes[dwIndex], pParam) < 0)
                {
                    //LOG_POS("");
                    return false;
                }
            }
        }
    }
    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
bool CHashMap<KEY, MAPPED, EXTENTION>::Clear()
{
    CHashHeader* pHeader = (CHashHeader*)GetHeader();
    uint32_t dwIndex = 0;
    for (uint32_t i = 0; i < pHeader->dwHashHeight; i++)
    {
        for (uint32_t j = 0; j < pHeader->adwHashMods[i]; j++, dwIndex++)
        {
            pHeader->aoHashNodes[dwIndex].tKey = HASH_EMPTY_KEY;
        }
    }
    pHeader->dwHashCurHeight = 0;
    pHeader->uHashNodeUsedCount = 0;

    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline void* CHashMap<KEY, MAPPED, EXTENTION>::GetHeader()
{
    return m_pHeader;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline void CHashMap<KEY, MAPPED, EXTENTION>::SetHeaderAddress(void* pHdr)
{
    m_pHeader = pHdr;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline void CHashMap<KEY, MAPPED, EXTENTION>::CopyHeaderMemory(const void* pHdr)
{
    memmove(m_pHeader, pHdr, GetHeaderSize());
}

template <typename KEY, typename MAPPED, typename EXTENTION>
bool CHashMap<KEY, MAPPED, EXTENTION>::ClearNode(CHashNode& rNode)
{
    if (!m_pHeader)
    {
        return false;
    }

    if (rNode.tKey != HASH_EMPTY_KEY)
    {
        rNode.tKey = HASH_EMPTY_KEY;
        ((CHashHeader*)GetHeader())->uHashNodeUsedCount--;
        //LOG_DBG("MSG | HashNodeUsedCount: %lu", m_pHeader->uHashNodeUsedCount);
    }

    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline int CHashMap<KEY, MAPPED, EXTENTION>::GetHashNodeUr() const
{
    assert(m_pHeader);
    CHashHeader* pHeader = (CHashHeader*)GetHeader();
    return pHeader->uHashNodeUsedCount * 100 / pHeader->uHashNodeCount;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline int CHashMap<KEY, MAPPED, EXTENTION>::GetHashLineUr() const
{
    assert(m_pHeader);
    CHashHeader* pHeader = (CHashHeader*)GetHeader();
    return pHeader->dwHashCurHeight * 100 / pHeader->dwHashHeight;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline size_t CHashMap<KEY, MAPPED, EXTENTION>::GetHeaderSize() const
{
    return sizeof(CHashHeader);
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline size_t CHashMap<KEY, MAPPED, EXTENTION>::GetNodeSize() const
{
    return sizeof(CHashNode);
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline struct timeval* CHashMap<KEY, MAPPED, EXTENTION>::GetTimeValueFind()
{
    return &m_stTv;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline bool CHashMap<KEY, MAPPED, EXTENTION>::AllocAndInitHashMemory(size_t uSize, const CHashHeader& roHdr)
{
    //LOG_ERR("DBG | Hash malloc memery size(%lu)", uSize);
    void* pBase = malloc(uSize);
    this->SetHeaderAddress(pBase);
    memset(this->GetHeader(), 0, this->GetHeaderSize());
    //assert(CHashMap<KEY, MAPPED, EXTENTION>::m_pHeader);
    *((CHashHeader*)this->GetHeader()) = roHdr;
    //this->CopyHeaderMemory(&roHdr);
    this->Clear();
    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline bool CHashMap<KEY, MAPPED, EXTENTION>::FreeHeaderMemory()
{
    unlikely(!m_pHeader)
    {
        return false;
    }

    free(m_pHeader);
    m_pHeader = NULL;
    return true;
}


// CSimple32Hash

template<typename MAPPED>
inline bool CSimple32Hash<MAPPED>::NodeMatch(const uint32_t& rKey, CHashNode& rNode)
{
    return rKey == rNode.tKey;
}

// CSimple64Hash

template<typename MAPPED>
inline bool CSimple64Hash<MAPPED>::NodeMatch(const uint64_t& rKey, CHashNode& rNode)
{
    return rKey == rNode.tKey;
}


// CShmHash

template <typename KEY, typename MAPPED, typename EXTENTION>
bool CShmHash<KEY, MAPPED, EXTENTION>::Init(uint32_t dwHashWidth, uint32_t dwHashHeight, const KEY& rEmptyKey, key_t dwShmKey)
{
    m_dwShmKey = dwShmKey;
    return CHashMap<KEY, MAPPED, EXTENTION>::Init(dwHashWidth, dwHashHeight, rEmptyKey);
}

template <typename KEY, typename MAPPED, typename EXTENTION>
bool CShmHash<KEY, MAPPED, EXTENTION>::Attach(key_t dwShmKey, bool bReadOnly)
{
    unlikely(!m_oShm.Attach(dwShmKey, bReadOnly))
    {
        return false;
    }
    CHashMap<KEY, MAPPED, EXTENTION>::SetHeaderAddress(m_oShm.GetAddress());
    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline bool CShmHash<KEY, MAPPED, EXTENTION>::AllocAndInitHashMemory(size_t uSize, const CHashHeader& roHdr)
{
    LOG_ERR("DBG | Hash share memery size(%lu)", uSize);
    unlikely(!m_oShm.Init(m_dwShmKey, uSize))
    {
        return false;
    }
    this->SetHeaderAddress(m_oShm.GetAddress());
    memset(this->GetHeader(), 0, this->GetHeaderSize());
    //assert(CHashMap<KEY, MAPPED, EXTENTION>::m_pHeader);
    *((CHashHeader*)this->GetHeader()) = roHdr;
    //this->CopyHeaderMemory(&roHdr);
    this->Clear();
    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline bool CShmHash<KEY, MAPPED, EXTENTION>::FreeHeaderMemory()
{
    if (!CHashMap<KEY, MAPPED, EXTENTION>::m_pHeader)
    {
        return false;
    }

    m_oShm.Close();
    CHashMap<KEY, MAPPED, EXTENTION>::m_pHeader = NULL;
    return true;
}


inline CDataSequenceInterface::CDataSequenceInterface(uint32_t dwSeq)
: _dwSeq(dwSeq)
{
}

inline uint32_t CDataSequenceInterface::NextSequence()
{
    _dwSeq++;
    /*
    if (_dwSeq == 0)
    {
        _dwSeq = 1;
    }
    */
    return _dwSeq;
}

inline bool CDataSequenceInterface::IsNewerThan(const CDataSequenceInterface* pObj) const
{
    return (_dwSeq > pObj->_dwSeq) && (_dwSeq - pObj->_dwSeq < 0x80000000);
}

inline uint32_t CDataSequenceInterface::GetSequence() const
{
    return _dwSeq;
}

inline void CDataSequenceInterface::SetSequence(uint32_t dwSeq)
{
    _dwSeq = dwSeq;
}


// CPersistentHash

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::CPersistentHash()
{
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::Init(uint32_t dwHashWidth, uint32_t dwHashHeight, const KEY& rEmptyKey, key_t dwShmKey)
{
    unlikely ((!CShmHash<KEY, MAPPED, EXTENTION>::Init(dwHashWidth, dwHashHeight, rEmptyKey, dwShmKey)))
    {
        return false;
    }
    
    unlikely (!OnLoadMemory())
    {
        return false;
    }
    printf("After LoadMemory Seq: %u\n", ((CHashHeader*)this->GetHeader())->GetSequence());

    unlikely (!OnLoadBinlog())
    {
        return false;
    }
    printf("After LoadBinlog Seq: %u\n", ((CHashHeader*)this->GetHeader())->GetSequence());
    
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline size_t CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::GetHeaderSize() const
{
    return sizeof(CHashHeader);
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline size_t CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::GetNodeSize() const
{
    return sizeof(CHashNode);
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::HandleData(CDataCell& rData)
{
    // 首先更新seq
    rData.SetSequence(((CHashHeader*)CShmHash<KEY, MAPPED, EXTENTION>::GetHeader())->NextSequence());
    printf("Handle Seq: %u\n", rData.GetSequence());

    // 启用了binlog就向binlog中写
    unlikely (!OnWriteBinlog(rData))
    {
        //LOG_POS("");
        return false;
    }

    unlikely (!OnHandleData(rData))
    {
        //LOG_POS("");
        return false;
    }

    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnHandleData(const CDataCell& rData)
{
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnWriteBinlog(const CDataCell& rData)
{
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnLoadBinlog()
{
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnDumpMemory()
{
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnLoadMemory()
{
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CPersistentHash<KEY, MAPPED, DATA, EXTENTION>::DumpMemory()
{
    return OnDumpMemory();
}


// CNormalPersistentHash::CDumpThread

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline long CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::CDumpThread::ThreadProc()
{
    pthread_detach(pthread_self());
    
    return (long)!m_pHash->OnDumpMemory();
}


// CNormalPersistentHash

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::CNormalPersistentHash()
: m_pBinlog(NULL)
, m_dwDateYYYYMMDD(0)
, m_iDumpOffset(0)
{
    m_oDumpThread.m_pHash = this;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::~CNormalPersistentHash()
{
    if (m_pBinlog)
    {
        fclose(m_pBinlog);
    }
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
bool CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::Init(unsigned int dwHashWidth, unsigned int dwHashHeight, const KEY& rEmptyKey, int dwShmKey, const char* pBinlogPrefix, const char* pBinlogSuffix, const char* pDumpPrefix, const char* pDumpSuffix)
{
    m_sBinlogPrefix = pBinlogPrefix;
    m_sBinlogSuffix = pBinlogSuffix;
    m_sDumpPrefix = pDumpPrefix;
    m_sDumpSuffix = pDumpSuffix;    
    
    return CPersistentHash_::Init(dwHashWidth, dwHashHeight, rEmptyKey, dwShmKey);
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
bool CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::Attach(int dwShmKey, bool bReadOnly, const char* pBinlogPrefix, const char* pBinlogSuffix, const char* pDumpPrefix, const char* pDumpSuffix)
{
    m_sBinlogPrefix = pBinlogPrefix;
    m_sBinlogSuffix = pBinlogSuffix;
    m_sDumpPrefix = pDumpPrefix;
    m_sDumpSuffix = pDumpSuffix;    
    
    return CPersistentHash_::Attach(dwShmKey, bReadOnly);
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::AsyncDumpMemory()
{
    return m_oDumpThread.Start();
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
bool CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnWriteBinlog(const CDataCell& rData)
{
    char szFileName[256] = {};
    uint32_t dwDate = CUtil::GetTodayFileName(m_sBinlogPrefix.c_str(), m_sBinlogSuffix.c_str(), 0, szFileName, sizeof(szFileName));
    if (m_dwDateYYYYMMDD != dwDate)
    {
        m_dwDateYYYYMMDD = dwDate;
        // 日期已变更
        if (m_pBinlog)
        {
            fclose(m_pBinlog);
        }
        m_pBinlog = fopen(szFileName, "wb");
    }
    
    fwrite(&rData, sizeof(rData), 1, m_pBinlog);
    
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
bool CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnLoadBinlog()
{
    CHashHeader* pHeader = (CHashHeader*)CPersistentHash_::GetHeader();
    char szFileName[256] = {};
    FILE* pFile = NULL;
    
    
    // 找到可加载的binlog
    CDataCell oData;
    // 取出binlog中较新的数据单元
    for (;;)
    {
        while (!pFile)
        {
            CUtil::GetTodayFileName(m_sBinlogPrefix.c_str(), m_sBinlogSuffix.c_str(), m_iDumpOffset, szFileName, sizeof(szFileName));
            pFile = fopen(szFileName, "rb");
            if (m_iDumpOffset == 0)
            {
                break;
            }
            ++m_iDumpOffset;
        }
        
        if (!pFile)
        {
            // 直到当天的binlog不存在，
            return true;
        }
                
        // 从Binlog文件中加载一个数据单元
        if (fread(&oData, sizeof (oData), 1, pFile) != 1)
        {
            // 文件读到头了
            fclose(pFile);
            if (m_iDumpOffset < 0)
            {
                // 还有后续binlog，继续加载
                pFile = NULL;
                continue;
            }
            else
            {
                // 读到今天的binlog，且读完了
                return true;
                
            }
            
        }

        // 准备向hash中效验更新，此时seq已经代表了hash中最新的节点seq
        if (oData.IsNewerThan(pHeader))
        {
            // 如果该数据单元比当前hash还新，应该向hash中写入该数据
            unlikely (!this->OnHandleData(oData))
            {
                fclose(pFile);
                return false;
            }
            
            // 并且更新最新seq
            pHeader->SetSequence(oData.GetSequence());
        }
    }
    
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
inline bool CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnDumpMemory()
{
    char szFileName[256] = {};
    CUtil::GetTodayFileName(m_sDumpPrefix.c_str(), m_sDumpSuffix.c_str(), 0, szFileName, sizeof(szFileName));
    FILE* pFile = fopen(szFileName, "wb");
    
    // 写入头部
    CHashHeader* pHeader = (CHashHeader*)this->GetHeader();
    fwrite(pHeader, this->GetHeaderSize(), 1, pFile);
    
    // 写入节点
    uint32_t dwIndex = 0;
    for (uint32_t i = 0; i < pHeader->dwHashHeight; i++)
    {
        for (uint32_t j = 0; j < pHeader->adwHashMods[i]; j++, dwIndex++)
        {
            if (!this->NodeMatch(this->HASH_EMPTY_KEY, pHeader->aoHashNodes[dwIndex]))
            {
                fwrite(&pHeader->aoHashNodes[dwIndex], this->GetNodeSize(), 1, pFile);
            }
        }
    }
    
    fclose(pFile);
    
    return true;
}

template <typename KEY, typename MAPPED, typename DATA, typename EXTENTION>
bool CNormalPersistentHash<KEY, MAPPED, DATA, EXTENTION>::OnLoadMemory()
{
    char szFileName[256] = {};
    
    CUtil::GetTodayFileName(m_sDumpPrefix.c_str(), m_sDumpSuffix.c_str(), 0, szFileName, sizeof(szFileName));
    FILE* pFile = fopen(szFileName, "rb");
    likely (pFile)
    {
        m_iDumpOffset = 0;
    }
    else
    {
        // 当天dump不存在时，尝试打开昨天dump
        CUtil::GetTodayFileName(m_sDumpPrefix.c_str(), m_sDumpSuffix.c_str(), -1, szFileName, sizeof(szFileName));
        pFile = fopen(szFileName, "rb");
        likely (pFile)
        {
            m_iDumpOffset = -1;
        }
    }
    if (!pFile)
    {
        // 没找到可加载的binlog, 不用false
        //RETURN_ERR(-1);
        return true;
    }
    
    
    // 加载hash头部
    CHashHeader oHeader;
    unlikely (fread(&oHeader, sizeof(oHeader), 1, pFile) != 1)
    {
        fclose(pFile);
        return false;
    }
    //memcpy(rHash.GetHeader(), &oHeader, sizeof(oHeader));
    oHeader.dwHashCurHeight = 0;
    ((CHashHeader*)this->GetHeader())->SetSequence(oHeader.GetSequence());
    ((CHashHeader*)this->GetHeader())->tExtention = oHeader.tExtention;
    
    
    // 加载hash节点
    CAutoBuffer oNode(this->GetNodeSize());
    CHashNode* pNode = NULL;
    
    while (!feof(pFile))
    {
        unlikely (fread(oNode.GetBuffer(), oNode.GetSize(), 1, pFile) != 1)
        {
            break;
        }
        
        pNode = (CHashNode*)this->FindNodeToSet(((CHashNode*)oNode.GetBuffer())->tKey, NULL);
        memcpy(pNode, oNode.GetBuffer(), oNode.GetSize());
    }
    fclose(pFile);
    
    return true;
}


#endif	/* __TSHASH_INL__ */

