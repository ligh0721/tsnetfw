/* 
 * File:   TSIo.h
 * Author: thunderliu
 *
 * Created on 2011年11月26日, 下午8:46
 */

#ifndef __TSIO_H__
#define	__TSIO_H__



//#include "Epoll.h"
#include "stdlib.h"
#include "TSEventInterface.h"


#define INFINITE ((int)(-1))

class CIo : public CEventInterface
{
public:
    CIo(int iFd = -1);
    virtual ~CIo();

    bool Init(const char* pPath, int iAccess);
    bool Close();
    long Write(const void* pBuf, size_t uSize);
    long Read(void* pBuf, size_t uSize);
    bool SetNonBlock();


    bool IsInUse() const;

    int GetHandle() const;
    void SetHandle(int iFd);
    operator int() const;


protected:
    int m_iFd;

};





#include "TSIo.inl"

#endif	/* __TSIO_H__ */

