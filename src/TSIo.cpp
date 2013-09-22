/* 
 * File:   TSIo.cpp
 * Author: thunderliu
 * 
 * Created on 2011年11月26日, 下午8:46
 */

#include "TSPlatform.h"
#include <stdint.h>
#include "TSEventInterface.h"
#include "TSIo.h"


// CIo

bool CIo::Close()
{
    if (m_iFd < 0)
    {
        return false;
    }

    bool bRet = OnClose();

    if (!bRet)
    {
        return false;
    }

    int iRet = close(m_iFd);
    if (iRet < 0)
    {
        return false;
    }

    /*
    if (m_pEpFw)
    {
        m_pEpFw->UnregisterIo(this);
        m_pEpFw = NULL;
    }
     */

    m_iFd = -1;
    
    return OnClosed();
}




