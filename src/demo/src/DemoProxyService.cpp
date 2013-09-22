/* 
 * File:   DemoProxyService.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月24日, 下午5:43
 */

//#define __MP__

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


//#include "lib64/Attr_API.h"
#include "TSNetFw.h"
#include "DemoProxyService.h"
#include "TSSession.h"



#ifndef __MP__
CDemoProxyService g_oSvc;
#else
CDemoProxyServiceMP g_oSvc;
#endif

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
    //LOG_DBG("MSG | recv from %s:%u %u bytes", GetSockAddrIn()->GetAddr(), GetSockAddrIn()->GetPort(), roPkt.GetMaxBufferSize());
    //LOG_DBG_DMP(roPkt.GetBuffer(), roPkt.GetMaxBufferSize());

    if (*(uint8_t*)roPkt.GetBuffer(roPkt.GetMaxBufferSize() - 1) != 0x03)
    {
        return false;
    }

    //S_REQ_H* pPktH = (S_REQ_H*)roPkt.GetNextPointer(sizeof(S_REQ_H));
    BH_REF(roPkt);
    BH_GET_NEXT_RET(S_REQ_H, pPktH, "req hdr");
    //BH_GET_NEXT_RET(S_REQ_B, pPktB, "req body");

    //CStopWatch oSw;
    //oSw.Start();
    //SendNobody(pPktH, E_SUCCESS);
    //LOG_POS("MSG | SendNobody cost %ld usecs", oSw.Stop());
    //return true;
    uint16_t wCmd = ntohs(pPktH->wCommand);
    //uint32_t dwToUin = ntohl(pPktB->dwToUin);
    //int iGroupIndex = dwToUin % g_oSvc.m_oCfg.m_vecGroupAddr.size();
    int iGroupIndex = 0;
    CServerConnection* pSvr = NULL;
    char* pIp = NULL;
    int iPort = 0;

    if (wCmd == g_oSvc.m_oCfg.m_wCmdSet)
    {
        pSvr = g_oSvc.m_pSvrsSet + iGroupIndex;
        pIp = g_oSvc.m_oCfg.m_vecGroupAddr[iGroupIndex].szIp;
        iPort = g_oSvc.m_oCfg.m_vecGroupAddr[iGroupIndex].iPortSet;
    }
    else if (wCmd == g_oSvc.m_oCfg.m_wCmdGet)
    {
        pSvr = g_oSvc.m_pSvrsGet + iGroupIndex;
        pIp = g_oSvc.m_oCfg.m_vecGroupAddr[iGroupIndex].szIp;
        iPort = g_oSvc.m_oCfg.m_vecGroupAddr[iGroupIndex].iPortGet;
    }
    else
    {
        // unknown cmd
        LOG_ERR("ERR | unknown cmd(0x%04X) err", wCmd);
        SendNobody(pPktH, E_PKGERR);
        return false;
    }

    if (!pSvr->IsInUse())
    {
        if (!pSvr->Connect(pIp, iPort))
        {
            //perror(strerror(errno));
            SendNobody(pPktH, E_BUSY);
            LOG_ERR("ERR | reconnect %s:%d err", pIp, iPort);
            return false;
        }

        pSvr->Init(&g_oSvc.m_oEfw, sizeof (S_RSP_H));
    }

    SESSION stSession;
    stSession.stReqH = *pPktH;
    stSession.pSock = this;

    if (!g_oSvc.m_oSk.AddSession(stSession, pPktH->dwSequence, g_oSvc.m_oCfg.m_oTimeout))
    {
        LOG_ERR("ERR | add session(%u) err", pPktH->dwSequence);
        SendNobody(pPktH, E_FAILD);
        return false;
    }

    //LOG_POS("MSG | add session(%u) succ", pPktH->dwSequence);

    if (!pSvr->SendEx(roPkt.GetBuffer(), roPkt.GetMaxBufferSize()))
    {
        LOG_ERR("ERR | send to %s:%d err", pIp, iPort);
        SendNobody(pPktH, E_BUSY);
        return false;
    }

    //CSockAddrIn* pSa = pSvr->GetSockAddrIn();

    //LOG_POS("MSG | send to %s:%d", pIp, iPort);
    //LOG_DBG_DMP(roPkt.GetBuffer(), roPkt.GetMaxBufferSize());


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


// CServerConnection

bool CServerConnection::OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen)
{
    S_RSP_H* pstPktH = (S_RSP_H*)roPkt.GetNextPointer(sizeof (S_RSP_H));

    if (pstPktH->stPktH.cFlag != 0x02)
    {
        return false;
    }

    iTotalPktLen = ntohs(pstPktH->stPktH.wLength);

    if ((iTotalPktLen <= (int)sizeof (S_RSP_H)) || (iTotalPktLen > 0x1000))
    {
        // ERROR: len err
        return false;
    }

    //uint16_t wCmd = ntohs(pstPktH->wCommand);

    return true;
}

bool CServerConnection::OnReadPacket(CBufferHelper& roPkt)
{
    //LOG_POS("MSG | recv from %s:%d", GetSockAddrIn()->GetAddr(), GetSockAddrIn()->GetPort());
    //LOG_DBG_DMP(roPkt.GetBuffer(), roPkt.GetMaxBufferSize());

    S_RSP_H* pRspH = (S_RSP_H*)roPkt.GetNextPointer(sizeof (S_RSP_H));
    if (!pRspH)
    {
        LOG_ERR("ERR | len(%u) err rsp hdr", roPkt.GetMaxBufferSize());
        return false;
    }

    SESSION stSession;
    if (!g_oSvc.m_oSk.GetSession(pRspH->dwSequence, stSession))
    {
        LOG_ERR("ERR | get session(%u) failed", pRspH->dwSequence);
        return false;
    }

    pRspH->dwSequence = stSession.stReqH.dwSequence;
    //LOG_POS("MSG | send to %s:%d", stSession.pSock->GetSockAddrIn()->GetAddr(), stSession.pSock->GetSockAddrIn()->GetPort());
    stSession.pSock->SendEx(roPkt.GetBuffer(), roPkt.GetMaxBufferSize());

    return true;
}

bool CSessionKeeper::OnSessionTimeout(SESSION& rSession)
{
    return rSession.pSock->SendNobody(&rSession.stReqH, E_TIMEOUT);
}


// CDemoProxyService

CDemoProxyService::CDemoProxyService()
{
}

CDemoProxyService::~CDemoProxyService()
{
}

bool CDemoProxyService::OnLoadConfigure()
{
    //strncpy(m_oCfg.m_szHostIp, HOST_ADDRESS, sizeof(m_oCfg.m_szHostIp));
    //m_oCfg.m_iHostPort = HOST_PORT;
    char szCfg[1024];
    if (!CCommandLine::GetArgValue(1))
    {
        snprintf(szCfg, sizeof (szCfg), "%s.cfg", CCommandLine::GetArgValue(0));
    }
    else
    {
        snprintf(szCfg, sizeof (szCfg), "%s", CCommandLine::GetArgValue(1));
    }

    if (!m_oCfg.LoadFromFile(szCfg))
    {
        LOG_ERR("ERR | Load cfg(%s) failed", szCfg);
        return false;
    }
    m_oCfg.ReadString("host_ip", m_oCfg.m_szHostIp);
    m_oCfg.ReadInt("host_port", m_oCfg.m_iHostPort);

    m_oCfg.ReadUInt16("cmd_set", m_oCfg.m_wCmdSet);
    m_oCfg.ReadUInt16("cmd_get", m_oCfg.m_wCmdGet);

    long lSec = 0;
    long lMSec = 0;
    m_oCfg.ReadLong("session_time_out_sec", lSec);
    m_oCfg.ReadLong("session_time_out_msec", lMSec);
    m_oCfg.m_oTimeout.SetTime(lSec, lMSec);

    int iCount = 0;
    m_oCfg.ReadInt("group_count", iCount);
    CStr szTmp(1024);
    CStr szTmp2(1024);
    S_GROUPADDR stAddr;

    for (int i = 0; i < iCount; i++)
    {
        sprintf(szTmp, "group_m%d", i);
        m_oCfg.ReadString(szTmp, szTmp2);
        sscanf(szTmp2, "%[^:]:%d:%d", stAddr.szIp, &stAddr.iPortSet, &stAddr.iPortGet);
        m_oCfg.m_vecGroupAddr.push_back(stAddr);
    }



    return true;
}

bool CDemoProxyService::OnServiceInit()
{
    if (!m_oEfw.Init())
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    if (!m_oSk.Init(&m_oEfw, 4000))
    {
        LOG_ERR("ERR | Session Keeper Init Failed");
        return false;
    }

    m_pSvrsSet = new CServerConnection[m_oCfg.m_vecGroupAddr.size()];
    if (!m_pSvrsSet)
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    m_pSvrsGet = new CServerConnection[m_oCfg.m_vecGroupAddr.size()];
    if (!m_pSvrsGet)
    {
        delete[] m_pSvrsSet;
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    int iCount = m_oCfg.m_vecGroupAddr.size();
    for (int i = 0; i < iCount; i++)
    {
        if (!m_pSvrsSet[i].Connect(m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortSet))
        {
            LOG_ERR("ERR | connecting to %s:%d, %s", m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortSet, strerror(errno));
            return false;
        }

        if (!m_pSvrsSet[i].Init(&m_oEfw, sizeof (S_RSP_H)))
        {
            LOG_ERR("ERR | %s", strerror(errno));
            return false;
        }

        if (!m_pSvrsGet[i].Connect(m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortGet))
        {
            LOG_ERR("ERR | connecting to %s:%d, %s", m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortGet, strerror(errno));
            return false;
        }

        if (!m_pSvrsGet[i].Init(&m_oEfw, sizeof (S_RSP_H)))
        {
            LOG_ERR("ERR | %s", strerror(errno));
            return false;
        }
    }

    if (!m_oLstn.Init(&m_oEfw, 2000, sizeof (S_REQ_H), m_oCfg.m_szHostIp, m_oCfg.m_iHostPort, true))
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    return true;
}

bool CDemoProxyService::OnEnterLoop()
{

    if (!m_oLstn.Listen())
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    m_oEfw.FrameworkLoop();

    return true;
}


// CDemoProxyServiceMP

CDemoProxyServiceMP::CDemoProxyServiceMP()
{
}

CDemoProxyServiceMP::~CDemoProxyServiceMP()
{
}

bool CDemoProxyServiceMP::OnLoadConfigure()
{
    //strncpy(m_oCfg.m_szHostIp, HOST_ADDRESS, sizeof(m_oCfg.m_szHostIp));
    //m_oCfg.m_iHostPort = HOST_PORT;
    char szCfg[1024];
    if (!CCommandLine::GetArgValue(1))
    {
        snprintf(szCfg, sizeof (szCfg), "%s.cfg", CCommandLine::GetArgValue(0));
    }
    else
    {
        snprintf(szCfg, sizeof (szCfg), "%s", CCommandLine::GetArgValue(1));
    }

    if (!m_oCfg.LoadFromFile(szCfg))
    {
        LOG_ERR("ERR | Load cfg(%s) failed", szCfg);
        return false;
    }
    m_oCfg.ReadString("host_ip", m_oCfg.m_szHostIp);
    m_oCfg.ReadInt("host_port", m_oCfg.m_iHostPort);

    m_oCfg.ReadUInt16("cmd_set", m_oCfg.m_wCmdSet);
    m_oCfg.ReadUInt16("cmd_get", m_oCfg.m_wCmdGet);

    long lSec = 0;
    long lMSec = 0;
    m_oCfg.ReadLong("session_time_out_sec", lSec);
    m_oCfg.ReadLong("session_time_out_msec", lMSec);
    m_oCfg.m_oTimeout.SetTime(lSec, lMSec);

    int iProcessCount;
    m_oCfg.ReadInt("process_count", iProcessCount);
    SetProcessCount(iProcessCount);

    int iCount = 0;
    m_oCfg.ReadInt("group_count", iCount);
    CStr szTmp(1024);
    CStr szTmp2(1024);
    S_GROUPADDR stAddr;

    for (int i = 0; i < iCount; i++)
    {
        sprintf(szTmp, "group_m%d", i);
        m_oCfg.ReadString(szTmp, szTmp2);
        sscanf(szTmp2, "%[^:]:%d:%d", stAddr.szIp, &stAddr.iPortSet, &stAddr.iPortGet);
        m_oCfg.m_vecGroupAddr.push_back(stAddr);
    }



    return true;
}

bool CDemoProxyServiceMP::OnServiceInit()
{
    if (!m_oEfw.Init())
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }
    LOG_DBG("MSG | listening %s:%d", m_oCfg.m_szHostIp.GetBuffer(), m_oCfg.m_iHostPort);
    if (!m_oLstn.Init(&m_oEfw, 2000, sizeof (S_REQ_H), m_oCfg.m_szHostIp, m_oCfg.m_iHostPort, true))
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    if (!m_oLstn.Listen())
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    m_pSvrsSet = new CServerConnection[m_oCfg.m_vecGroupAddr.size()];
    if (!m_pSvrsSet)
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    m_pSvrsGet = new CServerConnection[m_oCfg.m_vecGroupAddr.size()];
    if (!m_pSvrsGet)
    {
        delete[] m_pSvrsSet;
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    return true;
}

bool CDemoProxyServiceMP::OnEnterLoop()
{
    if (!m_oSk.Init(&m_oEfw, 4000))
    {
        LOG_ERR("ERR | Session Keeper Init Failed");
        return false;
    }

    int iCount = m_oCfg.m_vecGroupAddr.size();
    for (int i = 0; i < iCount; i++)
    {
        LOG_DBG("MSG | connect %s:%d", m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortSet);
        if (!m_pSvrsSet[i].Connect(m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortSet))
        {
            LOG_ERR("ERR | %s", strerror(errno));
            return false;
        }

        if (!m_pSvrsSet[i].Init(&m_oEfw, sizeof (S_RSP_H)))
        {
            LOG_ERR("ERR | %s", strerror(errno));
            return false;
        }

        LOG_DBG("MSG | connect %s:%d", m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortGet);
        if (!m_pSvrsGet[i].Connect(m_oCfg.m_vecGroupAddr[i].szIp, m_oCfg.m_vecGroupAddr[i].iPortGet))
        {
            LOG_ERR("ERR | %s", strerror(errno));
            return false;
        }

        if (!m_pSvrsGet[i].Init(&m_oEfw, sizeof (S_RSP_H)))
        {
            LOG_ERR("ERR | %s", strerror(errno));
            return false;
        }
    }

    m_oEfw.FrameworkLoop();

    return true;
}

int main(int argc, char* const* argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();
    //FILE* pFile = fopen("DemoProxyService.log", "w");
    //CLog::SetOutFile(pFile);
    CLog::SetLogLevel(E_LL_DBG);

    if (!g_oSvc.StartService())
    {
        return -1;
    }

    g_oSvc.WaitService();


    return 0;
}

