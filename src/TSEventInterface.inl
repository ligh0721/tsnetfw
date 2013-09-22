/* 
 * File:   TSEventInterface.inl
 * Author: thunderliu
 *
 * Created on 2011年12月20日, 下午7:01
 */

#ifndef __TSEVENTINTERFACE_INL__
#define	__TSEVENTINTERFACE_INL__

#include <stddef.h>

// CEventInterface

inline CEventInterface::CEventInterface()
: m_pEpFw(NULL)
{
}

inline CEventInterface::~CEventInterface()
{
}

inline bool CEventInterface::OnRead()
{
    return true;
}

inline bool CEventInterface::OnWrite()
{
    return true;
}

inline bool CEventInterface::OnError()
{
    return true;
}

inline bool CEventInterface::OnClose()
{
    return true;
}

inline bool CEventInterface::OnClosed()
{
    return true;
}

inline bool CEventInterface::OnTimeout()
{
    return true;
}

inline CEpollFramework* CEventInterface::GetFrameworkHandle()
{
    return m_pEpFw;
}

inline void CEventInterface::SetFrameworkHandle(CEpollFramework* pEpFw)
{
    //if (pEpFw->)
    m_pEpFw = pEpFw;
}



#endif	/* __TSEVENTINTERFACE_INL__ */

