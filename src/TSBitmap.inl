/* 
 * File:   TSBitmap.inl
 * Author: thunderliu
 *
 * Created on 2011年11月17日, 下午4:49
 */

#ifndef __TSBITMAP_INL__
#define	__TSBITMAP_INL__

#include "TSBitmap.h"


inline CBitmap::CBitmap()
: m_pBuf(NULL)
, m_uBitCount(0)
{
}

inline CBitmap::CBitmap(size_t uBitCount, void* pBuf)
: m_pBuf((uint8_t*)pBuf)
, m_uBitCount(uBitCount)
{
}

inline CBitmap::~CBitmap()
{
}

inline void CBitmap::Release()
{
    free(m_pBuf);
    delete this;
}

inline bool CBitmap::Init(size_t uBitCount, void* pBuf)
{
    m_pBuf = (uint8_t*)pBuf;
    m_uBitCount = uBitCount;
    return true;
}

inline void CBitmap::Clear()
{
    SetAllBit(false);
}

inline void CBitmap::SetAllBit(bool bIsOn)
{
    memset(m_pBuf, (bIsOn ? 0xFF : 0x00), (m_uBitCount + 7) / 8);
}

#endif	/* __TSBITMAP_INL__ */

