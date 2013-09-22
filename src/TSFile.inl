/* 
 * File:   TSFile.inl
 * Author: thunderliu
 *
 * Created on 2013年6月4日, 下午5:00
 */

#ifndef __TSFILE_INL__
#define	__TSFILE_INL__

#include "TSFile.h"


inline CFileMapping::CFileMapping()
: m_pAddr(MAP_FAILED)
, m_uLen(0)
{
}

inline CFileMapping::operator const void*() const
{
    return m_pAddr;
}

inline CFileMapping::operator void*()
{
    return m_pAddr;
}

inline void* CFileMapping::GetAddress()
{
    return m_pAddr;
}

inline size_t CFileMapping::GetSize() const
{
    return m_uLen;
}
    

#endif	/* __TSFILE_INL__ */

