#define _DEBUG
#define TSNETFW_FEATURE_MYSQL

#include "TSNetFw.h"
#include "DemoUdpDispenser.h"

bool CMyUdpSock::OnRead()
{
    char szBuf[4096];
    BH_NEW(szBuf, sizeof (szBuf));
    SESSION stSess;
    CSockAddrIn oSa;

    RecvFrom(szBuf, sizeof (szBuf), &oSa);
    //LOG_DBG_DMP(szBuf, iRes);
    BH_GET_NEXT_RET(S_RSP_H, pRspH, "rsp hdr");
    uint16_t wCmd = ntohs(pRspH->wCommand);
    switch (wCmd)
    {
    case 0x0702:
    {
        ++g_wCtrlAgentOkCount;
        BH_GET_NEXT_RET(S_RSP_B_0x0702, pRspB, "rsp body");
        uint16_t wCtrlAgentVersion = ntohs(pRspB->wAgentVersion);
        if (wCtrlAgentVersion != g_wCtrlAgentVersion)
        {
            fprintf(stdout, "%s: Wrong wCtrlAgentVersion(%u)\n", oSa.GetAddr(), wCtrlAgentVersion);
        }
    }
        break;

    case 0x0902:
    {
        ++g_wCfgAgentOkCount;
        BH_GET_NEXT_RET(S_RSP_B_0x0902, pRspB, "rsp body");
        uint16_t wCfgAgentVersion = ntohs(pRspB->wAgentVersion);
        if (wCfgAgentVersion != g_wCfgAgentVersion)
        {
            fprintf(stdout, "%s: Wrong wCfgAgentVersion(%u)\n", oSa.GetAddr(), wCfgAgentVersion);
        }
    }
        break;

    default:
        LOG_ERR("ERR | Cmd(0x%04X) err", wCmd);
        return true;
    }

    g_oSk.GetSession(ntohl(pRspH->dwSequence), stSess);

    if (wCmd == 0x0702 && (g_wCtrlAgentOkCount + g_wCtrlAgentTimeoutCount == g_wCtrlAgentCount))
    {
        g_bCtrlAgentCheckingFinished = true;
        fprintf(stdout, "CtrlAgent Checking finished\n");
    }

    if (wCmd == 0x0902 && (g_wCfgAgentOkCount + g_wCfgAgentTimeoutCount == g_wCfgAgentCount))
    {
        g_bCfgAgentCheckingFinished = true;
        fprintf(stdout, "CfgAgent Checking finished\n");
    }

    if (g_bCtrlAgentCheckingFinished && g_bCfgAgentCheckingFinished)
    {
        exit(EXIT_SUCCESS);
    }

    return true;
}

int CMyUdpSock::SendTo0x0702(char* pIp, int iPort)
{
    SESSION stSess;
    uint32_t dwSeq;
    CSockAddrIn oSa;
    char szBuf[4096];
    BH_NEW(szBuf, sizeof (szBuf));

    BH_GET_NEXT_RET(S_REQ_H, pReqH, "req hdr");
    memset(pReqH, 0, sizeof (S_REQ_H));
    pReqH->cFlag = 0x02;
    pReqH->wVersion = 0;
    pReqH->wCommand = htons(0x0702);
    pReqH->dwUin = htonl(174209756);
    pReqH->dwUserIp = inet_addr(LISTEN_HOST);
    pReqH->dwReserved = 0;

    BH_GET_NEXT_RET(S_REQ_B_0x0702, pReqB, "req body");
    memset(pReqB, 0, sizeof (S_REQ_B_0x0702));
    pReqB->cSvrFlag = 0;
    pReqB->wAgentVersion = htons(g_wCtrlAgentVersion);

    BH_GET_NEXT_RET(S_WBUF, pWBuf, "wbuf");
    memset(pWBuf, 0, sizeof (S_WBUF));
    pWBuf->wBufLen = htons(0);

    BH_CHECK_NEXT_RET(ntohs(pWBuf->wBufLen), "wbuf");

    BH_GET_NEXT_RET(uint8_t, pEtx, "etx");
    *pEtx = 0x03;

    pReqH->wLength = htons(BH_OBJ.GetBufferPos());

    oSa.SetSockAddrIn(pIp, iPort);
    stSess.stSa = *(struct sockaddr_in*)oSa;
    stSess.stReqH = *pReqH;
    g_oSk.AddSession(stSess, dwSeq, CTime(1, 0));
    pReqH->dwSequence = htonl(dwSeq);

    return g_oUdp.SendTo(BH_BUF, BH_POS, &oSa);
}

int CMyUdpSock::SendTo0x0902(char* pIp, int iPort)
{
    SESSION stSess;
    uint32_t dwSeq;
    CSockAddrIn oSa;
    char szBuf[4096];
    BH_NEW(szBuf, sizeof (szBuf));

    BH_GET_NEXT_RET(S_REQ_H, pReqH, "req hdr");
    memset(pReqH, 0, sizeof (S_REQ_H));
    pReqH->cFlag = 0x02;
    pReqH->wVersion = 0;
    pReqH->wCommand = htons(0x0902);
    pReqH->dwUin = htonl(174209756);
    pReqH->dwUserIp = inet_addr(LISTEN_HOST);
    pReqH->dwReserved = 0;

    BH_GET_NEXT_RET(S_REQ_B_0x0902, pReqB, "req body");
    memset(pReqB, 0, sizeof (S_REQ_B_0x0902));

    BH_GET_NEXT_RET(S_WBUF, pWBuf, "wbuf");
    memset(pWBuf, 0, sizeof (S_WBUF));
    pWBuf->wBufLen = htons(0);

    BH_CHECK_NEXT_RET(ntohs(pWBuf->wBufLen), "wbuf");

    BH_GET_NEXT_RET(uint8_t, pEtx, "etx");
    *pEtx = 0x03;

    pReqH->wLength = htons(BH_OBJ.GetBufferPos());

    oSa.SetSockAddrIn(pIp, iPort);
    stSess.stSa = *(struct sockaddr_in*)oSa;
    stSess.stReqH = *pReqH;
    g_oSk.AddSession(stSess, dwSeq, CTime(1, 0));
    pReqH->dwSequence = htonl(dwSeq);

    return g_oUdp.SendTo(BH_BUF, BH_POS, &oSa);
}

bool CSessionKeeper::OnSessionTimeout(SESSION& rSession)
{
    uint16_t wCmd = ntohs(rSession.stReqH.wCommand);
    const char* pAgent = "";
    switch (wCmd)
    {
    case 0x0702:
        ++g_wCtrlAgentTimeoutCount;
        pAgent = "CtrlAgent";
        break;

    case 0x0902:
        ++g_wCfgAgentTimeoutCount;
        pAgent = "CfgAgent";
        break;

    default:
        LOG_ERR("ERR | Cmd(0x%04X) err, timeout", wCmd);
        return false;
    }

    fprintf(stdout, "%s: %s Timeout\n", CSockAddrIn::ConvertToSockAddrIn((struct sockaddr*)&rSession.stSa)->GetAddr(), pAgent);

    if (wCmd == 0x0702 && (g_wCtrlAgentOkCount + g_wCtrlAgentTimeoutCount == g_wCtrlAgentCount))
    {
        g_bCtrlAgentCheckingFinished = true;
        fprintf(stdout, "CtrlAgent Checking finished\n");
    }

    if (wCmd == 0x0902 && (g_wCfgAgentOkCount + g_wCfgAgentTimeoutCount == g_wCfgAgentCount))
    {
        g_bCfgAgentCheckingFinished = true;
        fprintf(stdout, "CfgAgent Checking finished\n");
    }

    if (g_bCtrlAgentCheckingFinished && g_bCfgAgentCheckingFinished)
    {
        exit(EXIT_SUCCESS);
    }

    return true;
}

int main(int argc, char* const* argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();

    CLog::SetLogLevel(E_LL_ERR);

    bool bRes;

    bRes = g_oCtrlDb.Init("localhost", 3306, "root", "", "mb_web_cmd_control", "utf8");
    assert(bRes);

    bRes = g_oCfgDb.Init("localhost", 3306, "root", "", "mb_web_cfg_mgr", "utf8");
    assert(bRes);

    CDbMySQLResult* pRes;

    fprintf(stdout, "Read wCtrlAgentVersion from db ... ");
    g_oCtrlDb.Query("select f_Ver from tbl_agent_status limit 1;");
    pRes = g_oCtrlDb.GetResult();
    assert(pRes);
    pRes->GetNextLine();
    char* pCtrlAgentVersion = pRes->GetData(0);
    assert(pCtrlAgentVersion);
    g_wCtrlAgentVersion = atoi(pCtrlAgentVersion);
    pRes->Release();
    fprintf(stdout, "%u\n", g_wCtrlAgentVersion);

    fprintf(stdout, "Read wCfgAgentVersion from db ... ");
    g_oCfgDb.Query("select f_Ver from tbl_agent_status limit 1;");
    pRes = g_oCfgDb.GetResult();
    assert(pRes);
    pRes->GetNextLine();
    char* pCfgAgentVersion = pRes->GetData(0);
    assert(pCfgAgentVersion);
    g_wCfgAgentVersion = atoi(pCfgAgentVersion);
    pRes->Release();
    fprintf(stdout, "%u\n", g_wCfgAgentVersion);

    fprintf(stdout, "Read wAgentCount from db ... ");
    g_oCtrlDb.Query("select f_Ip from tbl_mach_info;");
    pRes = g_oCtrlDb.GetResult();
    assert(pRes);

    g_oEpfw.Init(1);
    g_oUdp.Bind(LISTEN_HOST, LISTEN_PORT, true);
    g_oEpfw.RegisterIo(&g_oUdp, CEpollEvent::EV_IN);
    g_oSk.Init(&g_oEpfw, 2048);

    g_wCtrlAgentCount = pRes->GetLineCount();
    g_wCfgAgentCount = g_wCtrlAgentCount;

    fprintf(stdout, "%u\n", g_wCtrlAgentCount);

    g_wCtrlAgentOkCount = 0;
    g_wCtrlAgentTimeoutCount = 0;
    g_bCtrlAgentCheckingFinished = false;

    g_wCfgAgentOkCount = 0;
    g_wCfgAgentTimeoutCount = 0;
    g_bCfgAgentCheckingFinished = false;

    char* pIp;

    while (pRes->GetNextLine())
    {
        pIp = pRes->GetData(0);
        assert(pIp);

        g_oUdp.SendTo0x0702(pIp, REMOTE_PORT_0x0702);
        g_oUdp.SendTo0x0902(pIp, REMOTE_PORT_0x0902);
    }
    pRes->Release();

    for (;;)
    {
        g_oEpfw.FrameworkLoop();
    }

    return 0;
}

CDbMySQL g_oCtrlDb;
CDbMySQL g_oCfgDb;
CEpollFramework g_oEpfw;
CMyUdpSock g_oUdp;
CSessionKeeper g_oSk;
uint16_t g_wCtrlAgentOkCount;
uint16_t g_wCtrlAgentTimeoutCount;
uint16_t g_wCtrlAgentCount;
uint16_t g_wCtrlAgentVersion;
bool g_bCtrlAgentCheckingFinished;
uint16_t g_wCfgAgentOkCount;
uint16_t g_wCfgAgentTimeoutCount;
uint16_t g_wCfgAgentCount;
uint16_t g_wCfgAgentVersion;
bool g_bCfgAgentCheckingFinished;
