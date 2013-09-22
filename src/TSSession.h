/* 
 * File:   TSSession.h
 * Author: thunderliu
 *
 * Created on 2011年12月22日, 下午12:27
 */

#ifndef __TSSESSION_H__
#define	__TSSESSION_H__


#include "TSSession.h"

class CSessionTimeroutInterface
{
public:
    virtual ~CSessionTimeroutInterface();

    virtual bool OnSessionTimeout(void* pSessionNode) = 0;
};

template <typename SESSION>
class CSessionKeeperBase : public CSessionTimeroutInterface
{
public:
    //CSessionKeeperBase();

    virtual bool AddSession(const SESSION& rSession, uint32_t& dwSequence, const CTime& rTimeout) = 0;
    virtual bool GetSession(uint32_t dwSequence, SESSION& rSession) = 0;
    virtual bool PeekSession(uint32_t dwSequence, SESSION& rSession) = 0;
};

class CSessionTimer : public CTimerQueueNode
{
public:
    CSessionTimer(CSessionTimeroutInterface* pTimeoutHandler, void* pSessionNode);

    virtual bool OnTimeout();

protected:
    void* m_pSessionNode;
    CSessionTimeroutInterface* m_pTimeoutHandler;
};

template <typename SESSION>
class CHashSessionKeeper : public CSessionKeeperBase<SESSION>
{
protected:

    typedef struct
    {
        SESSION tSession;
        CTimerNodeBase* pTimer;
    } SESSION_MAPPED;

    typedef CSimple32Hash<SESSION_MAPPED> CSessionHash;

public:
    CHashSessionKeeper();
    bool Init(CEpollFramework* pEpFw, uint32_t dwMaxSession = 20000);
    virtual bool AddSession(const SESSION& rSession, uint32_t& dwSequence, const CTime& rTimeout);
    virtual bool GetSession(uint32_t dwSequence, SESSION& rSession);
    virtual bool PeekSession(uint32_t dwSequence, SESSION& rSession);
    virtual bool OnSessionTimeout(void* pSessionNode);

protected:
    virtual bool OnSessionTimeout(SESSION& rSession);




protected:
    bool ClearSession(class CSessionHash::CHashNode* pNode);

protected:
    uint32_t m_dwSequence;
    CSessionHash m_oSessHash;
    CEpollFramework* m_pEpFw;

};



#include "TSSession.inl"

#endif	/* __TSSESSION_H__ */

