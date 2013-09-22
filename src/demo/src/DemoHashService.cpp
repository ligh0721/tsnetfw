/* 
 * File:   DemoHashService.cpp
 * Author: thunderliu
 *
 * Created on 2012年3月22日, 下午6:03
 */

#include <stdint.h>
#include <stdio.h>
#include <signal.h>
//#include "lib64/Attr_API.h"
#include "TSNetFw.h"
#include "DemoHashService.h"



CDemoHashService g_oSvc;








// CMyConnection

void CClientConnection::Req2RspHdr(S_REQ_H* pReqH, S_RSP_H* pRspH, uint16_t wRspLen, E_RESULT eResult)
{
    pRspH->stPktH.cFlag = 0x02;
    pRspH->stPktH.wLength = htons(wRspLen);
    pRspH->wVersion = pReqH->wVersion;
    pRspH->wCommand = pReqH->wCommand;
    pRspH->dwSequence = pReqH->dwSequence;
    pRspH->dwUin = pReqH->dwUin;
    pRspH->dwReserved = 0;
    pRspH->cResult = eResult;
}

bool CClientConnection::SendNobody(S_REQ_H* pReqH, E_RESULT eResult)
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

bool CClientConnection::OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen)
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

bool CClientConnection::OnReadPacket(CBufferHelper& roPkt)
{
    LOG_DBG("MSG | recv from %s:%u %u bytes", GetSockAddrIn()->GetAddr(), GetSockAddrIn()->GetPort(), roPkt.GetMaxBufferSize());
    LOG_DBG_DMP(roPkt.GetBuffer(), roPkt.GetMaxBufferSize());

    BH_REF(roPkt);

    BH_GET_NEXT_RET(S_REQ_H, pReqH, "req hdr");

    uint16_t wCmd = ntohs(pReqH->wCommand);

    if (wCmd == g_oSvc.m_oCfg.m_wCmdSet)
    {
        BH_GET_NEXT_RET(S_REQ_B_SET, pReqB, "req body set");
        uint32_t dwUin = ntohl(pReqB->dwUin);

        bool bNew;
        CSimple32Hash<S_MAPPED>::CHashNode* pNode = g_oSvc.m_oHash.FindNodeToSet(dwUin, &bNew);
        if (!pNode || IS_INVALID_NODE(pNode))
        {
            SendNobody(pReqH, E_FAILD);
            return false;
        }

        if (bNew)
        {
            pNode->tMapped.dwCount = 1;
        }
        else
        {
            ++pNode->tMapped.dwCount;
        }

        char szBuf[4096];
        CBufferHelper oBh(szBuf, sizeof (szBuf));

        S_RSP_H* pRspH = (S_RSP_H*)oBh.GetNextPointer(sizeof (S_RSP_H));
        //S_RSP_B_SET* pRspB = (S_RSP_B_SET*)oBh.GetNextPointer(sizeof(S_RSP_B_SET));
        uint8_t* pEtx = (uint8_t*)oBh.GetNextPointer(sizeof (uint8_t));

        Req2RspHdr(pReqH, pRspH, oBh.GetBufferPos(), E_SUCCESS);
        *pEtx = 0x03;

        LOG_DBG("MSG | send to %s:%u %u bytes", GetSockAddrIn()->GetAddr(), GetSockAddrIn()->GetPort(), roPkt.GetMaxBufferSize());
        LOG_DBG_DMP(oBh.GetBuffer(), oBh.GetBufferPos());
        return SendEx(oBh.GetBuffer(), oBh.GetBufferPos());
    }
    else if (wCmd == g_oSvc.m_oCfg.m_wCmdGet)
    {
        BH_GET_NEXT_RET(S_REQ_B_GET, pReqB, "req body get");
        uint32_t dwUin = ntohl(pReqB->dwUin);
        uint32_t dwMSec = ntohl(pReqB->dwMSec);
        srand(time(NULL));
        usleep((dwMSec / 2 + rand() % dwMSec) * 1000);

        CSimple32Hash<S_MAPPED>::CHashNode* pNode = g_oSvc.m_oHash.FindNode(dwUin);
        if (pNode == (CSimple32Hash<S_MAPPED>::CHashNode*)(-1))
        {
            SendNobody(pReqH, E_FAILD);
            return false;
        }

        uint32_t dwCount;
        if (!pNode)
        {
            dwCount = (uint32_t)(-1);
        }
        else
        {
            dwCount = pNode->tMapped.dwCount;
        }

        char szBuf[4096];
        CBufferHelper oBh(szBuf, sizeof (szBuf));

        S_RSP_H* pRspH = (S_RSP_H*)oBh.GetNextPointer(sizeof (S_RSP_H));
        S_RSP_B_GET* pRspB = (S_RSP_B_GET*)oBh.GetNextPointer(sizeof (S_RSP_B_GET));
        uint8_t* pEtx = (uint8_t*)oBh.GetNextPointer(sizeof (uint8_t));

        Req2RspHdr(pReqH, pRspH, oBh.GetBufferPos(), E_SUCCESS);
        pRspB->dwCount = htonl(dwCount);
        *pEtx = 0x03;

        LOG_DBG("MSG | send to %s:%u %u bytes", GetSockAddrIn()->GetAddr(), GetSockAddrIn()->GetPort(), roPkt.GetMaxBufferSize());
        LOG_DBG_DMP(oBh.GetBuffer(), oBh.GetBufferPos());

        return SendEx(oBh.GetBuffer(), oBh.GetBufferPos());
    }
    else
    {
        LOG_ERR("ERR | cmd(0x%04X) unknown err", wCmd);

        SendNobody(pReqH, E_PKGERR);
        return false;
    }

    return true;
}









// CDemoHashService

CDemoHashService::CDemoHashService()
{
}

CDemoHashService::~CDemoHashService()
{
}

bool CDemoHashService::OnLoadConfigure()
{
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

    m_oCfg.ReadInt("hash_shm_key", m_oCfg.m_iHashShmKey);
    m_oCfg.ReadUInt32("hash_width", m_oCfg.m_dwHashWidth);
    m_oCfg.ReadUInt32("hash_height", m_oCfg.m_dwHashHeight);

    return true;
}

bool CDemoHashService::OnServiceInit()
{
    if (!m_oEfw.Init())
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    if (!m_oLstn.Init(&m_oEfw, 2000, sizeof (S_REQ_H), m_oCfg.m_szHostIp, m_oCfg.m_iHostPort, true))
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }
    LOG_DBG("MSG | listen on %s:%d", m_oCfg.m_szHostIp.GetBuffer(), m_oCfg.m_iHostPort);

    if (!m_oHash.Init(m_oCfg.m_dwHashWidth, m_oCfg.m_dwHashHeight, -1))
    {
        LOG_ERR("ERR | hash init err, %s", strerror(errno));
        return false;
    }

    return true;
}

bool CDemoHashService::OnEnterLoop()
{
    if (!m_oLstn.Listen())
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    m_oEfw.FrameworkLoop();

    return true;
}

int main(int argc, char* const* argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();
    //FILE* pFile = fopen("DemoHashService.log", "w");
    //CLog::SetOutFile(pFile);
    CLog::SetLogLevel(E_LL_DBG);

    if (!g_oSvc.StartService())
    {
        return -1;
    }

    g_oSvc.WaitService();

    return 0;
}


