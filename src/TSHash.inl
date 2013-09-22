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
// CHashBase

template <typename KEY, typename MAPPED, typename EXTENTION>
inline CHashMap<KEY, MAPPED, EXTENTION>::CHashMap()
: HASH_NODE_SIZE(sizeof (CHashNode))
, HASH_EMPTY_KEY()
, m_pHeader(NULL)
{
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline CHashMap<KEY, MAPPED, EXTENTION>::~CHashMap()
{
    FreeHeaderMemory();
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
    oTmpHdr.dwHashNodeSize = HASH_NODE_SIZE;

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

    if (!AllocAndInitHashMemory((size_t)sizeof (CHashHeader) + oTmpHdr.uHashNodeCount * HASH_NODE_SIZE, oTmpHdr))
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
    for (i = 0, pRow = m_pHeader->aoHashNodes; i < m_pHeader->dwHashHeight; pRow += m_pHeader->adwHashMods[i], i++)
    {
        pNode = pRow + (dwKey % m_pHeader->adwHashMods[i]);
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
            m_pHeader->uHashNodeUsedCount++;
            pEmptyNode->tKey = rKey; // 空节点的头部初始化
            dwHashCurHeight++; // 结果为当前查询最深行
            if (dwHashCurHeight > m_pHeader->dwHashCurHeight)
            {
                // 查询最深行已经超过记载的最深行
                m_pHeader->dwHashCurHeight = dwHashCurHeight;
            }

            return pEmptyNode;
        }
    }

    return pFoundNode;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline size_t CHashMap<KEY, MAPPED, EXTENTION>::GetNodeIndex(CHashNode* pNode)
{
    return pNode - m_pHeader->aoHashNodes;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline typename CHashMap<KEY, MAPPED, EXTENTION>::CHashNode* CHashMap<KEY, MAPPED, EXTENTION>::GetNode(size_t uIndex)
{
    if (uIndex < 0 || uIndex >= m_pHeader->uHashNodeCount)
    {
        return NULL;
    }

    return &m_pHeader->aoHashNodes[uIndex];
}

template <typename KEY, typename MAPPED, typename EXTENTION>
typename CHashMap<KEY, MAPPED, EXTENTION>::CHashNode* CHashMap<KEY, MAPPED, EXTENTION>::FindNode(const KEY& rKey)
{
    if (!m_pHeader)
    {
        //LOG_POS("");
        return HASH_INVALID_MAPPED_POINTER;
    }

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
    for (i = 0, pRow = m_pHeader->aoHashNodes; i < m_pHeader->dwHashCurHeight; pRow += m_pHeader->adwHashMods[i], i++)
    {
        pNode = pRow + (dwKey % m_pHeader->adwHashMods[i]);
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

    uint32_t dwIndex = 0;
    for (uint32_t i = 0; i < m_pHeader->dwHashHeight; i++)
    {
        for (uint32_t j = 0; j < m_pHeader->adwHashMods[i]; j++, dwIndex++)
        {
            if (!NodeMatch(HASH_EMPTY_KEY, m_pHeader->aoHashNodes[dwIndex]))
            {
                if ((this->*TraverseCallback)(m_pHeader->aoHashNodes[dwIndex], pParam) < 0)
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
    uint32_t dwIndex = 0;
    for (uint32_t i = 0; i < m_pHeader->dwHashHeight; i++)
    {
        for (uint32_t j = 0; j < m_pHeader->adwHashMods[i]; j++, dwIndex++)
        {
            m_pHeader->aoHashNodes[dwIndex].tKey = HASH_EMPTY_KEY;
        }
    }
    m_pHeader->dwHashCurHeight = 0;
    m_pHeader->uHashNodeUsedCount = 0;

    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline typename CHashMap<KEY, MAPPED, EXTENTION>::CHashHeader* CHashMap<KEY, MAPPED, EXTENTION>::GetHeader()
{
    return m_pHeader;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline void CHashMap<KEY, MAPPED, EXTENTION>::SetHeaderAddress(const void* pHdr)
{
    m_pHeader = (typename CHashMap<KEY, MAPPED, EXTENTION>::CHashHeader*)pHdr;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline void CHashMap<KEY, MAPPED, EXTENTION>::CopyHeaderMemory(const void* pHdr)
{
    memmove(m_pHeader, pHdr, sizeof(typename CHashMap<KEY, MAPPED, EXTENTION>::CHashHeader));
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
        m_pHeader->uHashNodeUsedCount--;
        //LOG_DBG("MSG | HashNodeUsedCount: %lu", m_pHeader->uHashNodeUsedCount);
    }

    return true;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline int CHashMap<KEY, MAPPED, EXTENTION>::GetHashNodeUr() const
{
    assert(m_pHeader);
    return m_pHeader->uHashNodeUsedCount * 100 / m_pHeader->uHashNodeCount;
}

template <typename KEY, typename MAPPED, typename EXTENTION>
inline int CHashMap<KEY, MAPPED, EXTENTION>::GetHashLineUr() const
{
    assert(m_pHeader);
    return m_pHeader->dwHashCurHeight * 100 / m_pHeader->dwHashHeight;
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
    CHashMap<KEY, MAPPED, EXTENTION>::SetHeaderAddress(pBase);
    //assert(CHashMap<KEY, MAPPED, EXTENTION>::m_pHeader);
    CHashMap<KEY, MAPPED, EXTENTION>::CopyHeaderMemory(&roHdr);
    CHashMap<KEY, MAPPED, EXTENTION>::Clear();
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
    CHashMap<KEY, MAPPED, EXTENTION>::SetHeaderAddress(m_oShm.GetAddress());
    //assert(CHashMap<KEY, MAPPED, EXTENTION>::m_pHeader);
    CHashMap<KEY, MAPPED, EXTENTION>::CopyHeaderMemory(&roHdr);
    CHashMap<KEY, MAPPED, EXTENTION>::Clear();
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


#endif	/* __TSHASH_INL__ */

