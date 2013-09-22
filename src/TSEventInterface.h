/* 
 * File:   TSEventInterface.h
 * Author: thunderliu
 *
 * Created on 2011年12月20日, 下午6:33
 */

#ifndef __TSEVENTINTERFACE_H__
#define	__TSEVENTINTERFACE_H__


class CEpollFramework;

class CEventInterface
{
public:
    CEventInterface();
    virtual ~CEventInterface();

    virtual bool OnRead();
    virtual bool OnWrite();
    virtual bool OnError();
    virtual bool OnClose();
    virtual bool OnClosed();
    virtual bool OnTimeout();

    CEpollFramework* GetFrameworkHandle();
    void SetFrameworkHandle(CEpollFramework* pEpFw);

protected:
    CEpollFramework* m_pEpFw;
};


#include "TSEventInterface.inl"

#endif	/* __TSEVENTINTERFACE_H__ */

