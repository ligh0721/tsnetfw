/* 
 * File:   DemoLaboratory.cpp
 * Author: thunderliu
 *
 * Created on 2011年12月31日, 下午2:38
 */



/*
 * 
 */

#include "TSNetFw.h"
#include "DemoLaboratory.h"


#if 0
// CMyConnection

bool CMyConnection::OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen)
{
    S_REQ_H* pPktH = (S_REQ_H*)roPkt.GetNextPointer(sizeof (S_REQ_H));

    iTotalPktLen = ntohs(pPktH->stPktH.wLength);

    if (*(uint8_t*)roPkt.GetBuffer(0) != 0x02 || iTotalPktLen <= (int)sizeof (S_REQ_H) || iTotalPktLen > 0x1000)
    {
        // ERROR: len err
        return false;
    }

    return true;
}

bool CMyConnection::OnReadPacket(CBufferHelper& roPkt)
{
    //Attr_API(ATTR_REQUEST, 1);
    //LOG_DUMP(roPkt.GetBuffer(), roPkt.GetMaxBufferSize());

    if (*(uint8_t*)roPkt.GetBuffer(roPkt.GetMaxBufferSize() - 1) != 0x03)
    {
        return false;
    }

    S_REQ_H* pPktH = (S_REQ_H*)roPkt.GetNextPointer(sizeof (S_REQ_H));

    CStopWatch oSw;
    oSw.Start();
    SendNobody(pPktH, E_SUCCESS);
    LOG_ERR("MSG | SendNobody cost %u usecs", oSw.Stop());
    return true;

    if (!g_oSvc.m_oSvr.IsInUse())
    {
        if (!g_oSvc.m_oSvr.Connect(g_oSvc.m_oCfg.m_szServerIp, g_oSvc.m_oCfg.m_iServerPort))
        {
            SendNobody(pPktH, E_BUSY);
            return false;
        }

        g_oSvc.m_oSvr.Init(&g_oSvc.m_oEfw, sizeof (S_RSP_H));
    }

    SESSION stSession;
    stSession.stReqH = *pPktH;
    stSession.pSock = this;

    if (!g_oSvc.m_oSk.AddSession(stSession, pPktH->dwSequence, g_oSvc.m_oCfg.m_oTimeout))
    {
        LOG_ERR("ERR | add session(%u) failed", pPktH->dwSequence);
        return false;
    }

    LOG_ERR("MSG | add session(%u) succ", pPktH->dwSequence);

    if (!g_oSvc.m_oSvr.SendEx(roPkt.GetBuffer(), roPkt.GetMaxBufferSize()))
    {
        LOG_ERR("ERR | send to svr failed");
        return false;
    }

    CSockAddrIn* pSa = (CSockAddrIn*)g_oSvc.m_oSvr.GetSockAddr();

    LOG_ERR("send %s:%d\n", pSa->GetAddr(), pSa->GetPort());

    LOG_DUMP(roPkt.GetBuffer(), roPkt.GetMaxBufferSize());

    return true;
}

bool CMyConnection::SendNobody(S_REQ_H* pReqH, E_RESULT eResult)
{
    static S_RSP_NOBODY stRspBody;
    memset(&stRspBody, 0, sizeof (S_RSP_NOBODY));
    stRspBody.stPktH.cFlag = 0x02;
    stRspBody.wVersion = pReqH->wVersion;
    stRspBody.wCommand = pReqH->wCommand;
    stRspBody.dwSequence = pReqH->dwSequence;
    stRspBody.dwUin = pReqH->dwUin;
    stRspBody.cResult = eResult;
    stRspBody.cEtxFlag = 0x03;
    stRspBody.stPktH.wLength = htons(sizeof (S_RSP_NOBODY));

    return SendEx(&stRspBody, sizeof (S_RSP_NOBODY));
}


// CMultiProcess

int CMultiProcess::ProcessProc()
{
    return 0;
}

#endif

int main(int argc, char** argv)
{
    return 0;
}



