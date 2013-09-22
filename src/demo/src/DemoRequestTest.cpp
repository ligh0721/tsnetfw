/* 
 * File:   DemoRequestTest.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月29日, 下午12:16
 */

#include "TSNetFw.h"
#include "DemoRequestTest.h"


CDemoRequestTest g_oSvc;

bool CTestThread::Init(FILE* pFile)
{
    m_pFile = pFile;
    return true;
}

long CTestThread::TcpProc()
{
    long lConnect;
    long lSend;
    long lRecv;
    long lTotal;

    CTcpSocket oSvr;

    CStopWatch oSw;
    CStopWatch oSwTotal;

    oSw.Start();
    oSwTotal = oSw;

    if (!oSvr.Connect(g_oSvc.m_szSvrIp, g_oSvc.m_iSvrPort))
    {
        fprintf(m_pFile, "ERR | %ld, Connect failed, %s\n", m_uTid, strerror(errno));
        fflush(m_pFile);
        return -1;
    }
    lConnect = oSw.Start();

    if (!oSvr.SendEx(g_oSvc.m_szPktBuf, g_oSvc.m_iPktLen))
    {
        fprintf(m_pFile, "ERR | %ld, Send failed, %s\n", m_uTid, strerror(errno));
        fflush(m_pFile);
        return -1;
    }
    lSend = oSw.Start();

    char szBuf[4096];
    int iPos = 0;
    if (!oSvr.RecvEx(szBuf + iPos, sizeof (S_RSP_H)))
    {
        fprintf(m_pFile, "ERR | %ld, Recv hdr failed, %s\n", m_uTid, strerror(errno));
        fflush(m_pFile);
        return -1;
    }

    iPos += sizeof (S_RSP_H);

    int iLeftLen = ntohs(((S_RSP_H*)szBuf)->stPktH.wLength) - sizeof (S_RSP_H);

    if (!oSvr.RecvEx(szBuf + iPos, iLeftLen))
    {
        fprintf(m_pFile, "ERR | %ld, Recv body failed, %s\n", m_uTid, strerror(errno));
        fflush(m_pFile);
        return -1;
    }
    lRecv = oSw.Start();

    lTotal = oSwTotal.Start();

    fprintf(m_pFile, "MSG | %lu, Finished res(%02X), Total: %ld, Conn: %ld, Send: %ld, Recv: %ld\n", m_uTid, ((S_RSP_H*)szBuf)->cResult, lTotal, lConnect, lSend, lRecv);
    fflush(m_pFile);

    return 0;
}

long CTestThread::UdpProc()
{
    long lSend;
    long lRecv;
    long lTotal;

    CUdpSocket oSvr;

    CStopWatch oSw;
    CStopWatch oSwTotal;

    oSw.Start();
    oSwTotal = oSw;

    oSvr.Bind(NULL, 0, true);

    CSockAddrIn oSa;
    oSa.SetSockAddrIn(g_oSvc.m_szSvrIp, g_oSvc.m_iSvrPort);

    if (!oSvr.SendTo(g_oSvc.m_szPktBuf, g_oSvc.m_iPktLen, &oSa))
    {
        fprintf(m_pFile, "ERR | %ld, SendTo failed\n", m_uTid);
        fflush(m_pFile);
        return -1;
    }

    char szBuf[4096];
    int iPos = 0;

    oSvr.SetRecvTimeout(5000);

    lSend = oSw.Start();
    if (oSvr.RecvFrom(szBuf + iPos, 4096, &oSa) <= 0)
    {
        fprintf(m_pFile, "ERR | %ld, RecvFrom pkt failed\n", m_uTid);
        fflush(m_pFile);
        return -1;
    }

    lRecv = oSw.Start();
    lTotal = oSwTotal.Start();

    fprintf(m_pFile, "MSG | %lu, Finished res(%02X), Total: %ld, SendTo: %ld, Recv: %ld\n", m_uTid, ((S_RSP_H*)szBuf)->cResult, lTotal, lSend, lRecv);
    fflush(m_pFile);

    return 0;
}

long CTestThread::ThreadProc()
{
    if (!strncmp(g_oSvc.m_szProto, "tcp", sizeof (g_oSvc.m_szProto)))
    {
        return TcpProc();
    }
    else if (!strncmp(g_oSvc.m_szProto, "udp", sizeof (g_oSvc.m_szProto)))
    {
        return UdpProc();
    }

    return -1;
}

CDemoRequestTest::CDemoRequestTest()
{
}

CDemoRequestTest::~CDemoRequestTest()
{
}

long CDemoRequestTest::ThreadProc()
{
    CStr szInFile(1024);
    CStr szOutFile(1024);

    if (CCommandLine::GetArgCount() != 7)
    {
        fprintf(stderr, "usage: %s PROTO IP PORT PACKET LOG THREADCOUNT\n", CCommandLine::GetArgValue(0));
        return 0;
    }

    strncpy(m_szProto, CCommandLine::GetArgValue(1), sizeof (m_szProto));
    strncpy(m_szSvrIp, CCommandLine::GetArgValue(2), sizeof (m_szSvrIp));
    CStringHelper::StrToVal(CStr(CCommandLine::GetArgValue(3), 16), &m_iSvrPort, sizeof (m_iSvrPort));
    szInFile.Copy(CCommandLine::GetArgValue(4));
    szOutFile.Copy(CCommandLine::GetArgValue(5));
    CStringHelper::StrToVal(CStr(CCommandLine::GetArgValue(6), 16), &m_iCount, sizeof (m_iCount));

    FILE* pFile = fopen(szInFile, "r");
    m_iPktLen = 0;
    int iRet;
    while (!feof(pFile))
    {
        iRet = fread(m_szPktBuf + m_iPktLen, 1, sizeof (m_szPktBuf), pFile);
        if (iRet <= 0)
        {
            fclose(pFile);
            return -1;
        }
        m_iPktLen += iRet;
    }
    fclose(pFile);

    pFile = fopen(szOutFile, "a");

    CTestThread* pThrd = new CTestThread[m_iCount];
    for (int i = 0; i < m_iCount; i++)
    {
        pThrd[i].Init(pFile);
        pThrd[i].Start();
    }

    for (int i = 0; i < m_iCount; i++)
    {
        pThrd[i].Wait();
    }

    delete[] pThrd;

    fclose(pFile);

    return 0;
}

int main(int argc, char* const* argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();

    if (!g_oSvc.Start())
    {
        return -1;
    }

    g_oSvc.Wait();

    return 0;
}

