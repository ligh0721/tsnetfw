/* 
 * File:   TSSession.inl
 * Author: thunderliu
 *
 * Created on 2011年12月22日, 下午12:29
 */

#ifndef __SESSION_INL__
#define	__SESSION_INL__

#include "TSSession.h"
#include "TSDebug.h"
#include "TSHash.h"


// CSessionTimeroutInterface

inline CSessionTimeroutInterface::~CSessionTimeroutInterface()
{
}


//CSessionTimer

inline CSessionTimer::CSessionTimer(CSessionTimeroutInterface* pTimeoutHandler, void* pSessionNode)
: m_pSessionNode(pSessionNode)
, m_pTimeoutHandler(pTimeoutHandler)
{
}

inline bool CSessionTimer::OnTimeout()
{
    return m_pTimeoutHandler->OnSessionTimeout(m_pSessionNode);
}


// CHashSessionKeeper

template <typename SESSION>
inline CHashSessionKeeper<SESSION>::CHashSessionKeeper()
: m_dwSequence(0)
{

}

template <typename SESSION>
bool CHashSessionKeeper<SESSION>::AddSession(const SESSION& rSession, uint32_t& dwSequence, const CTime& rTimeout)
{
    LOG_DBG("MSG | SessHashNode: %lu/%lu, %u/%u", m_oSessHash.GetHeader()->uHashNodeUsedCount, m_oSessHash.GetHeader()->uHashNodeCount, m_oSessHash.GetHeader()->dwHashCurHeight, m_oSessHash.GetHeader()->dwHashHeight);
    bool bNew;
    typename CSessionHash::CHashNode* pNode = m_oSessHash.FindNodeToSet(m_dwSequence, &bNew);

    if (IS_INVALID_NODE(pNode) || !pNode || !bNew)
    {
        //printf("++++++++%p++++++++++\n", pNode);
        return false;
    }

    dwSequence = m_dwSequence;

    pNode->tMapped.tSession = rSession;
    pNode->tMapped.pTimer = new CSessionTimer(this, pNode);

    if (!m_pEpFw->RegisterTimer(pNode->tMapped.pTimer, rTimeout, rTimeout))
    {
        m_oSessHash.ClearNode(*pNode);
        //printf("--------------------\n");
        return false;
    }

    //dwSequence = m_dwSequence;
    ++m_dwSequence;

    return true;
}

template <typename SESSION>
inline bool CHashSessionKeeper<SESSION>::GetSession(uint32_t dwSequence, SESSION& rSession)
{
    typename CSessionHash::CHashNode* pNode = m_oSessHash.FindNode(dwSequence);
    if (!pNode || IS_INVALID_NODE(pNode))
    {
        return false;
    }

    rSession = pNode->tMapped.tSession;

    ClearSession(pNode);

    return true;
}

template <typename SESSION>
inline bool CHashSessionKeeper<SESSION>::PeekSession(uint32_t dwSequence, SESSION& rSession)
{
    typename CSessionHash::CHashNode* pNode = m_oSessHash.FindNode(dwSequence);
    if (!pNode || IS_INVALID_NODE(pNode))
    {
        return false;
    }

    rSession = pNode->tMapped.tSession;

    return true;
}

template <typename SESSION>
inline bool CHashSessionKeeper<SESSION>::ClearSession(typename CSessionHash::CHashNode* pNode)
{
    m_pEpFw->UnregisterTimer(pNode->tMapped.pTimer);
    m_oSessHash.ClearNode(*pNode);

    return true;
}

template <typename SESSION>
inline bool CHashSessionKeeper<SESSION>::OnSessionTimeout(void* pSessionNode)
{
    typename CSessionHash::CHashNode* pNode = (typename CSessionHash::CHashNode*)pSessionNode;
    bool bRet = OnSessionTimeout(pNode->tMapped.tSession);

    ClearSession(pNode);

    return bRet;
}

template <typename SESSION>
inline bool CHashSessionKeeper<SESSION>::OnSessionTimeout(SESSION& rSession)
{
    return true;
}

template <typename SESSION>
inline bool CHashSessionKeeper<SESSION>::Init(CEpollFramework* pEpFw, uint32_t dwMaxSession)
{
    m_pEpFw = pEpFw;
    m_oSessHash.Init(5000, 20, 0);
    m_dwSequence = time(NULL);

    return true;
}



#endif	/* __SESSION_INL__ */

