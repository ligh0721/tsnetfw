/* 
 * File:   TSMemory.h
 * Author: thunderliu
 *
 * Created on 2012年4月16日, 下午4:29
 */

#ifndef __TSMEMORY_H__
#define	__TSMEMORY_H__


#include <sys/types.h>
#include <sys/shm.h>


class CAutoBuffer
{
public:
    CAutoBuffer(size_t uSize);
    ~CAutoBuffer();
    operator void*();
    operator const void*() const;
    void* GetBuffer(size_t uOffset = 0);
    size_t GetSize() const;
    
protected:
    void* m_pBuf;
    size_t m_uSize;
};

class CBuffer
{
public:
    CBuffer(uint32_t dwSize);
    ~CBuffer();
    void* GetBuffer(uint32_t dwPos = 0);
    uint32_t GetBufferSize() const;
    
protected:
    uint8_t* m_pBuf;
    uint32_t m_dwSize;
};

class CBufferHelper
{
public:
    CBufferHelper(void* pBuff, uint32_t dwMaxSize);
    CBufferHelper(CBuffer& roBuf);

    void* GetNextPointer(uint32_t dwDataSize);
    void* GetBuffer(uint32_t dwOffset = 0);
    uint32_t GetBufferPos() const;
    uint32_t GetMaxBufferSize() const;
    void Rewind();
    void Truncate();

protected:
    uint8_t* m_pBuf;
    uint32_t m_dwPos;
    uint32_t m_dwMaxSize;
};

#define BH_OBJ __oBh_1DC65F21__
#define BH_BUF (BH_OBJ.GetBuffer())
#define BH_POS (BH_OBJ.GetBufferPos())
#define BH_SIZE (BH_OBJ.GetMaxBufferSize())

#define BH_NEW(buff, size) CBufferHelper BH_OBJ((buff), (size))
#define BH_REF(bf) CBufferHelper& BH_OBJ = bf

#define BH_GET_NEXT_RET(type, var, text) \
type* var = (type*)BH_OBJ.GetNextPointer(sizeof(type)); \
do \
{ \
    unlikely(!var) \
    { \
        LOG_ERR("ERR | pkg len(%d) read(%d) err, "text, (int)BH_OBJ.GetMaxBufferSize(), (int)sizeof(type)); \
        return false; \
    } \
} while (false)

#define BH_CHECK_NEXT_RET(size, text) \
do \
{ \
    unlikely(!BH_OBJ.GetNextPointer((size))) \
    { \
        LOG_ERR("ERR | pkg len(%d) read(%d) err, "text, (int)BH_OBJ.GetMaxBufferSize(), (int)(size)); \
        return false; \
    } \
} while (false)

#define BH_GET_NEXT_RET_EX(bh, type, var, text) \
type* var = (type*)bh.GetNextPointer(sizeof(type)); \
do \
{ \
    unlikely(!var) \
    { \
        LOG_ERR("ERR | pkg len(%d) read(%d) err, "text, (int)bh.GetMaxBufferSize(), (int)sizeof(type)); \
        return false; \
    } \
} while (false)

#define BH_CHECK_NEXT_RET_EX(bh, size, text) \
do \
{ \
    unlikely(!bh.GetNextPointer((size))) \
    { \
        LOG_ERR("ERR | pkg len(%d) read(%d) err, "text, (int)bh.GetMaxBufferSize(), (int)(size)); \
        return false; \
    } \
} while (false)

#define BH_GET_NEXT(type, var) type* var = (type*)BH_OBJ.GetNextPointer(sizeof(type))
#define BH_CHECK_NEXT(size) BH_OBJ.GetNextPointer((size))

#define BH_GET(type, var, base, off) type* var = (type*)((size_t)(base) + (off))



class CShareMemory
{  
public:  
    CShareMemory();
    
    bool Init(key_t dwKey, size_t uSize);
    bool Attach(key_t dwKey, bool bReadOnly = false);
    bool Detach();
    bool Close();
    
    operator const void*() const;
    operator void*();
    size_t GetSize() const;
    void* GetAddress();
    
protected:
    void* m_pAddr;
    size_t m_uLen;
    key_t m_dwKey;
    int m_iId;
};


#include "TSMemory.inl"

#endif	/* MEMORY_H */

