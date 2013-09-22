/* 
 * File:   TSEpoll.inl
 * Author: thunderliu
 *
 * Created on 2011年12月20日, 下午6:23
 */

#ifndef __TSEPOLL_INL__
#define	__TSEPOLL_INL__

#include "TSEpoll.h"
#include "TSTimer.h"

// CIoContext

inline bool CIoContext::IsInUse() const
{
    return m_pIo != NULL;
}

inline void CIoContext::Clear()
{
    m_pIo = NULL;
    //m_uCreateTime = 0;
    //m_uLastAccessTime = 0;
}

inline CIo* CIoContext::GetIo()
{
    return m_pIo;
}

inline void CIoContext::SetIo(CIo* pIo)
{
    m_pIo = pIo;
}

inline time_t CIoContext::GetCreateTime() const
{
    return m_uCreateTime;
}

inline void CIoContext::SetCreateTime(time_t uCreateTime)
{
    m_uCreateTime = uCreateTime;
}

inline time_t CIoContext::GetLastAccessTime() const
{
    return m_uLastAccessTime;
}

inline void CIoContext::SetLastAccessTime(time_t uLastAccessTime)
{
    m_uLastAccessTime = uLastAccessTime;
}


// CEpollEvent

inline uint32_t CEpollEvent::GetEvents() const
{
    return events;
}

inline void CEpollEvent::SetEvents(uint32_t dwEvents)
{
    events = dwEvents;
}

inline int CEpollEvent::GetHandle() const
{
    return data.fd;
}

inline void CEpollEvent::SetHandle(int iFd)
{
    data.fd = iFd;
}


// CEpoll

inline CEpoll::CEpoll(int iFd)
: CIo(iFd)
{

}

inline CEpoll::~CEpoll()
{

}

inline bool CEpoll::CreateEpoll()
{
    if (m_iFd >= 0)
    {
        return false;
    }

    m_iFd = epoll_create(1);
    if (m_iFd < 0)
    {
        return false;
    }

    return true;
}

inline int CEpoll::Wait(CEpollEvent* pEvents, int iMaxEventCount, int iTimeout)
{
    //ASSERT(m_iFd >= 0);
    return epoll_wait(m_iFd, pEvents, iMaxEventCount, iTimeout);
}

inline bool CEpoll::Control(CONTROL eControl, int iFd, uint32_t dwEvents)
{
    //ASSERT(m_iFd >= 0);
    CEpollEvent oEvent;
    oEvent.SetEvents(dwEvents);
    oEvent.SetHandle(iFd);
    return epoll_ctl(m_iFd, (int)eControl, iFd, &oEvent) == 0;
}


// CEpollFramework

inline CEpollFramework::CEpollFramework()
{

}

inline CEpollFramework::~CEpollFramework()
{

}

inline bool CEpollFramework::RegisterTimer(CTimerNodeBase* pTimer, const CTime& rInterval, const CTime& rDelay)
{
    return m_oTm.AddTimer(pTimer, rInterval, rDelay);
}

inline bool CEpollFramework::UnregisterTimer(CTimerNodeBase* pTimer)
{
    return m_oTm.RemoveTimer(pTimer);
}


#endif	/* __TSEPOLL_INL__ */

