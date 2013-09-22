/* 
 * File:   DemoRecordService.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月24日, 下午4:48
 */

#include <stdint.h>
#include <stdio.h>
#include <signal.h>
//#include "lib64/Attr_API.h"
#include "TSNetFw.h"
#include "DemoRecordService.h"



CDemoRecordService g_oSvc;








// CMyConnection

bool CClientConnection::OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen)
{
    S_HDR* pstPktH = (S_HDR*)roPkt.GetNextPointer(sizeof (S_HDR));

    iTotalPktLen = ntohs(pstPktH->wLen);

    if ((iTotalPktLen <= (int)sizeof (S_HDR)) || (iTotalPktLen > 0x1000))
    {
        // ERROR: len err
        return false;
    }

    return true;
}

bool CClientConnection::OnReadPacket(CBufferHelper& roPkt)
{
    AddRecord(time(NULL));
    return true;
}

bool CClientConnection::AddRecord(uint32_t dwTime)
{
    static uint32_t dwSeq = 0;
    ++dwSeq;
    CRecordBlock* pBlock = CRecordBlock::CreateBlock(sizeof (REC_LOG), dwTime);
    if (!pBlock)
    {
        exit(0);
        return false;
    }

    snprintf((char*)pBlock->GetBuffer(), sizeof (REC_LOG) - 1, "%u\n", dwTime);
    //strncpy((char*)pBlock->GetBuffer(), pBuff, sizeof(REC_LOG));

    if (!g_oSvc.m_pRec[dwSeq % g_oSvc.m_oCfg.m_iRecThreadCount].AddBlock(pBlock))
    {
        pBlock->Release();
        return false;
    }

    return true;
}









// CDemoRecordService

CDemoRecordService::CDemoRecordService()
{
}

CDemoRecordService::~CDemoRecordService()
{
}

bool CDemoRecordService::OnLoadConfigure()
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

    m_oCfg.ReadString("record_path", m_oCfg.m_szRecPath);
    m_oCfg.ReadInt("record_thread_count", m_oCfg.m_iRecThreadCount);
    m_oCfg.ReadInt("record_queue_length", m_oCfg.m_iRecQueueLength);

    m_oCfg.ReadInt("service_count", m_oCfg.m_iSvcCount);
    m_oCfg.ReadInt("service_index", m_oCfg.m_iSvcIndex);

    return true;
}

bool CDemoRecordService::OnServiceInit()
{
    m_oEfw.Init();

    if (!m_oLstn.Init(&m_oEfw, 1000, sizeof (S_HDR), m_oCfg.m_szHostIp, m_oCfg.m_iHostPort, true))
    {
        LOG_ERR("ERR | %s\n", strerror(errno));
        return false;
    }

    m_pRec = new CRecordWorker[m_oCfg.m_iRecThreadCount];
    char szPath[4096];
    for (int i = 0; i < m_oCfg.m_iRecThreadCount; i++)
    {
        snprintf(szPath, sizeof (szPath), "%s%d", m_oCfg.m_szRecPath.GetBuffer(), i);
        if (!m_pRec[i].Init(szPath, m_oCfg.m_iRecQueueLength, true))
        {
            LOG_ERR("ERR | %s\n", strerror(errno));
            return false;
        }
    }

    return true;
}

bool CDemoRecordService::OnEnterLoop()
{


    for (int i = 0; i < m_oCfg.m_iRecThreadCount; i++)
    {
        if (!m_pRec[i].Start())
        {
            return false;
        }
    }

    if (!m_oLstn.Listen())
    {
        return false;
    }

    m_oEfw.FrameworkLoop();

    return true;
}

int main(int argc, char* const* argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();

    if (!g_oSvc.StartService())
    {
        return -1;
    }

    g_oSvc.WaitService();

    return 0;
}


