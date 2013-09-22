/* 
 * File:   TSIo.inl
 * Author: thunderliu
 *
 * Created on 2011年11月26日, 下午8:52
 */

#ifndef __TSIO_INL__
#define	__TSIO_INL__

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "TSEventInterface.h"
#include "TSIo.h"




// CIo

inline CIo::CIo(int iFd)
: m_iFd(iFd)
{

}

inline CIo::~CIo()
{
    Close();
}

inline bool CIo::Init(const char* pPath, int iAccess)
{
    Close();

    m_iFd = open(pPath, iAccess);
    if (m_iFd < 0)
    {
        return false;
    }

    return true;
}

inline long CIo::Write(const void* pBuf, size_t uSize)
{
    return write(m_iFd, pBuf, uSize);
}

inline long CIo::Read(void* pBuf, size_t uSize)
{
    return read(m_iFd, pBuf, uSize);
}

inline bool CIo::SetNonBlock()
{
    int iFlag;
    return (iFlag = fcntl(m_iFd, F_GETFL, 0)) >= 0 && fcntl(m_iFd, F_SETFL, iFlag | O_NONBLOCK) >= 0;
}

inline bool CIo::IsInUse() const
{
    return m_iFd >= 0;
}

inline int CIo::GetHandle() const
{
    return m_iFd;
}

inline void CIo::SetHandle(int iFd)
{
    m_iFd = iFd;
}

inline CIo::operator int() const
{
    return m_iFd;
}






#endif	/* __TSIO_INL__ */

