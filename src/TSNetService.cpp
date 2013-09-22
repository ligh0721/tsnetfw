/* 
 * File:   TSNetService.cpp
 * Author: thunderliu
 * 
 * Created on 2011年11月27日, 上午10:56
 */

#include "TSPlatform.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include "TSDebug.h"
#include "TSThread.h"
#include "TSProcess.h"
#include "TSWorker.h"
#include "TSEventInterface.h"
#include "TSIo.h"
//#include "Hash.h"
//#include "List.h"
#include "TSTimer.h"
#include "TSEpoll.h"
#include "TSSocket.h"
//#include "lib64/Attr_API.h"
#include "TSNetService.h"


// CAutoSwitch

CAutoSwitch::CAutoSwitch()
: m_bIsAvailable(true)
, m_dwMaxFailedCount(0)
, m_dwMaxTimeoutCount(0)
, m_dwFirstDelayUS(0)
, m_dwDelayIncStepUS(0)
, m_dwMaxDelayUS(0)
, m_dwFailedCount(0)
, m_dwTimeoutCount(0)
, m_dwDelayUS(0)
{

}

bool CAutoSwitch::Init(uint32_t dwMaxFailedCount, uint32_t dwMaxTimeoutCount, uint32_t dwFirstDelayMS, uint32_t dwDelayIncStepMS, uint32_t dwMaxDelayMS)
{
    m_dwMaxFailedCount = dwMaxFailedCount;
    m_dwMaxTimeoutCount = dwMaxTimeoutCount;
    m_dwFirstDelayUS = dwFirstDelayMS * 1000;
    m_dwDelayIncStepUS = dwDelayIncStepMS * 1000;
    m_dwMaxDelayUS = dwMaxDelayMS * 1000;

    m_bIsAvailable = true;
    m_dwFailedCount = 0;
    m_dwTimeoutCount = 0;
    m_dwDelayUS = 0;

    return true;
}

bool CAutoSwitch::IsAvailable()
{
    if (m_bIsAvailable)
    {
        m_dwFailedCount && (m_dwFailedCount = 0);
        m_dwTimeoutCount && (m_dwTimeoutCount = 0);
        if (m_oSw.Peek() >= (long)m_dwDelayUS)
        {
            m_oSw.Start();
            if (m_dwDelayUS >= m_dwFirstDelayUS + m_dwDelayIncStepUS)
            {
                m_dwDelayUS -= m_dwDelayIncStepUS;
            }
            else if (m_dwDelayUS == m_dwFirstDelayUS)
            {
                m_dwDelayUS = 0;
            }
        }
        return true;
    }

    if (m_oSw.Peek() >= (long)m_dwDelayUS)
    {
        m_bIsAvailable = true;
        return true;
    }
    return false;
}

void CAutoSwitch::TellFailed()
{
    m_oSw.Start();
    if (!m_bIsAvailable)
    {
        return;
    }
    ++m_dwFailedCount;

    if (m_dwFailedCount < m_dwMaxFailedCount)
    {
        return;
    }

    if (!m_dwDelayUS)
    {
        m_dwDelayUS = m_dwFirstDelayUS;
    }
    else if (m_dwDelayUS + m_dwDelayIncStepUS <= m_dwMaxDelayUS)
    {
        m_dwDelayUS += m_dwDelayIncStepUS;
    }
    m_dwFailedCount = 0;
    m_bIsAvailable = false;
}

void CAutoSwitch::TellTimeout()
{
    m_oSw.Start();
    if (!m_bIsAvailable)
    {
        return;
    }
    ++m_dwTimeoutCount;

    if (m_dwTimeoutCount < m_dwMaxTimeoutCount)
    {
        return;
    }

    if (!m_dwDelayUS)
    {
        m_dwDelayUS = m_dwFirstDelayUS;
    }
    else if (m_dwDelayUS + m_dwDelayIncStepUS <= m_dwMaxDelayUS)
    {
        m_dwDelayUS += m_dwDelayIncStepUS;
    }
    m_dwTimeoutCount = 0;
    m_bIsAvailable = false;
}

void CAutoSwitch::Reset()
{
    m_bIsAvailable = true;
    m_dwFailedCount = 0;
    m_dwTimeoutCount = 0;
    m_dwDelayUS = 0;
}

// CConnectionSocket

bool CConnectionStream::Init(CEpollFramework* pEpFw, int iReserved)
{
    bool bRet = OnInit();
    if (!bRet)
    {
        return false;
    }

    SetFrameworkHandle(pEpFw);
    bRet = m_pEpFw->RegisterIo(this, CEpollEvent::EV_IN);
    if (!bRet)
    {
        Close();
        m_pEpFw = NULL;
        return false;
    }

    return true;
}



// CProtocolStream

bool CProtocolStream::OnRead()
{
    int iRcvLen = 0; // 每次接收的数据大小
    iRcvLen = Recv(m_acRcvBuff + m_iRcvBuffPos, m_iRcvBuffLen - m_iRcvBuffPos);
    if (iRcvLen <= 0)
    {
        return false;
    }

    m_iRcvBuffPos += iRcvLen;

    // 没有接收完继续接收
    if (m_iRcvBuffPos < m_iRcvBuffLen)
    {
        // 继续接收
        return true;
    }

    bool bRet;
    // 头部接收完毕
    if (m_bToRcvPktHdr)
    {
        CBufferHelper oHdr(m_acRcvBuff, m_iRcvBuffLen);
        bRet = OnReadPacketHeader(oHdr, m_iRcvBuffLen);
        if (!bRet)
        {
            return false;
        }
        m_bToRcvPktHdr = false;
    }

    // 没有接收完继续接收
    if (m_iRcvBuffPos < m_iRcvBuffLen)
    {
        // 继续接收
        return true;
    }

    // 接收完毕，此时 m_iRcvLen == m_iPktLen

    SetLastAccessTime(time(NULL));
    CBufferHelper oPkt(m_acRcvBuff, m_iRcvBuffLen);
    bRet = OnReadPacket(oPkt);
    if (!bRet)
    {
        return false;
    }

    // 为接收新的包体做初始化
    ResetRecvInfo();
    return true;
}


// CRecordBlock

CRecordBlock* CRecordBlock::CreateBlock(uint16_t wSize, time_t uTime)
{
    if (wSize <= 0)
    {
        return NULL;
    }

    void* pBuf = malloc(wSize);
    if (!pBuf)
    {
        return NULL;
    }

    CRecordBlock* pBlock = new CRecordBlock(wSize, pBuf, uTime);
    if (!pBlock)
    {
        free(pBuf);
        return NULL;
    }

    return pBlock;
}


// CRecordWorker

bool CRecordWorker::Init(const char* pPath, size_t uBlockCount, bool bTextFormat)
{
    if (!pPath)
    {
        return false;
    }
    strncpy(m_szPath, pPath, sizeof (m_szPath));

    if (mkdir(m_szPath, S_IREAD | S_IWRITE | S_IEXEC | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0)
    {
        if (errno != EEXIST)
        {
            return false;
        }
    }
    
    m_bTextFormat = bTextFormat;

    return CWorker::Init(uBlockCount);
}

bool CRecordWorker::UpdateFileHandle(time_t tmTime)
{
    if (!tmTime)
    {
        tmTime = time(NULL);
    }
    struct tm stTime;
    localtime_r(&tmTime, &stTime);
    uint32_t dwDate = (uint32_t)((stTime.tm_year + 1900) * 10000 + (stTime.tm_mon + 1) * 100 + stTime.tm_mday);
    if (dwDate == m_dwDate)
    {
        return true;
    }

    //LOG_ITIL("WARNING | NEW FILE");
    SetDate(dwDate);
    char szFileName[1024] = {0};
    snprintf((char*)szFileName, sizeof (szFileName), "%s/rec%u.txt", m_szPath, dwDate);
    FILE* pFile = fopen(szFileName, "a+");
    if (!pFile)
    {
        return false;
    }

    FILE* pOld = m_pFile;
    m_pFile = pFile;
    if (pOld)
    {
        fclose(pOld);
    }

    return true;
}


// CNetService

bool CServiceBase::IgnoreSignals()
{
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    return true;
}


// CThreadService

long CThreadService::ThreadProc()
{
    if (!OnLoadConfigure())
    {
        return -1;
    }

    if (!OnServiceInit())
    {
        return -1;
    }

    OnEnterLoop();

    OnServiceExit();

    return 0;
}


// CProcessService

int CProcessService::ProcessProc()
{
    if (!OnLoadConfigure())
    {
        return -1;
    }

    if (!OnServiceInit())
    {
        return -1;
    }

    OnEnterLoop();

    OnServiceExit();

    return 0;
}

// CMultiProcessService

int CMultiProcessService::ProcessProc()
{
    if (!OnLoadConfigure())
    {
        return -1;
    }

    if (!OnServiceInit())
    {
        return -1;
    }


    CProcessGroup* pPg = CProcess::ForkHere(m_iCount - 1);

    OnEnterLoop();

    OnServiceExit();

    if (pPg)
    {
        pPg->Wait();
        pPg->Release();
    }

    return 0;
}




