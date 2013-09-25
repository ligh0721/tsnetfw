/* 
 * File:   TSMemory.inl
 * Author: thunderliu
 *
 * Created on 2012年4月16日, 下午4:30
 */

#ifndef __TSMEMORY_INL__
#define	__TSMEMORY_INL__

#include "TSMemory.h"


// CBuffer

inline CBuffer::CBuffer(uint32_t dwSize)
: m_dwSize(dwSize)
{
    m_pBuf = (uint8_t*)malloc(dwSize);
}

inline CBuffer::~CBuffer()
{
    if (m_pBuf)
    {
        free(m_pBuf);
    }
}

inline void* CBuffer::GetBuffer(uint32_t dwPos)
{
    return m_pBuf + dwPos;
}

inline uint32_t CBuffer::GetBufferSize() const
{
    return m_dwSize;
}


// CBufferHelper

inline CBufferHelper::CBufferHelper(void* pBuff, uint32_t dwMaxSize)
: m_pBuf((uint8_t*)pBuff)
, m_dwPos(0)
, m_dwMaxSize(dwMaxSize)
{
}

inline CBufferHelper::CBufferHelper(CBuffer& roBuf)
: m_pBuf((uint8_t*)roBuf.GetBuffer())
, m_dwPos(0)
, m_dwMaxSize(roBuf.GetBufferSize())
{
}

inline void* CBufferHelper::GetNextPointer(uint32_t dwDataSize)
{
    void* pBuff = m_pBuf + m_dwPos;
    m_dwPos += dwDataSize;
    return m_dwPos > m_dwMaxSize ? NULL : pBuff;
}

inline void* CBufferHelper::GetBuffer(uint32_t dwOffset)
{
    return (uint8_t*)m_pBuf + dwOffset;
}

inline uint32_t CBufferHelper::GetBufferPos() const
{
    return m_dwPos;
}

inline uint32_t CBufferHelper::GetMaxBufferSize() const
{
    return m_dwMaxSize;
}

inline void CBufferHelper::Rewind()
{
    m_dwPos = 0;
}

inline void CBufferHelper::Truncate()
{
    m_dwMaxSize = m_dwPos;
}

// CShareMemory

inline CShareMemory::CShareMemory()
: m_pAddr(NULL)
, m_uLen(0)
, m_dwKey(0)
, m_iId(-1)
{
}

inline bool CShareMemory::Detach()
{
    unlikely(shmdt(m_pAddr) < 0)
    {
        return false;
    }
    m_pAddr = NULL;
    return true;
}

inline bool CShareMemory::Close()
{
    unlikely(!Detach() || shmctl(m_iId, IPC_RMID, NULL) < 0)
    {
        return false;
    }
    m_pAddr = NULL;
    m_uLen = 0;
    m_dwKey = 0;
    m_iId = -1;
    return true;
}

inline CShareMemory::operator const void*() const
{
    return m_pAddr;
}

inline CShareMemory::operator  void*()
{
    return m_pAddr;
}

inline size_t CShareMemory::GetSize() const
{
    return m_uLen;
}

inline void* CShareMemory::GetAddress()
{
    return m_pAddr;
}


#endif	/* MEMORY_INL */

