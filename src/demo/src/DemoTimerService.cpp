/* 
 * File:   DemoTimerService.cpp
 * Author: thunderliu
 * 
 * Created on 2012年4月27日, 下午4:48
 */

#define TSNETFW_FEATURE_MYSQL

#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "./lib64/oi_shm.h"
#include "TSNetFw.h"
#include "DemoTimerService.h"



CDemoTimerService g_oSvc;

CMBMonitorHelper::CMBMonitorHelper()
{
}

bool CMBMonitorHelper::Init(const char* pSvrIp, int iSvrPort, const char* pAttrDbHost, int iAttrDbPort, const char* pAttrDbUser, const char* pAttrDbPass, const char* pAttrDbChar, const char* pAttrDbName, const char* pAttrDbTable)
{
    m_oSa.SetSockAddrIn(pSvrIp, iSvrPort);
    m_oUdp.Bind(NULL, iSvrPort);
    m_oDb.Init(pAttrDbHost, iAttrDbPort, pAttrDbUser, pAttrDbPass, pAttrDbName, pAttrDbChar);
    strncpy(m_szAttrDbTable, pAttrDbTable, sizeof (m_szAttrDbTable));
    return true;
}

bool CMBMonitorHelper::GetAttrInfo(int iAttrId, S_ATTR_INFO& rstAttrInfo)
{
    char szSql[1024] = {0};
    snprintf(szSql, sizeof (szSql), "select Fattr_id, Fattr_name, Fdata_type from t_attr where Fattr_id = %d;", iAttrId);
    m_oDb.Query(szSql);
    CDbMySQLResult* pRes = m_oDb.GetResult();
    pRes->GetNextLine();
    char* pData = pRes->GetData(0);
    rstAttrInfo.uAttrId = atol(pData);
    pData = pRes->GetData(1);
    strncpy(rstAttrInfo.szAttrName, pData, sizeof (rstAttrInfo.szAttrName));
    pData = pRes->GetData(2);
    rstAttrInfo.eAttrType = (E_ATTR_TYPE)atol(pData);
    pRes->Release();
    return true;
}

bool CMBMonitorHelper::GetValOfAttr(int iViewId, int iAttrId, int iDaysBefore, S_ATTR_VAL& rstAttrVal)
{
    S_MON_REQ stReq = {0};
    stReq.iType = htonl(1);
    stReq.iId = htonl(iViewId);
    stReq.iAttrId = htonl(iAttrId);
    stReq.iDaysBefore = htonl(iDaysBefore);
    stReq.iFrom = htonl(0);
    stReq.iEnd = htonl(1440);
    m_oUdp.SendTo(&stReq, sizeof (stReq), &m_oSa);
    S_MON_RSP stRsp = {0};
    CSockAddrIn oSa;
    m_oUdp.RecvFrom(&stRsp, sizeof (stRsp), &oSa);
    for (int i = 0; i < 1440; i++)
    {
        rstAttrVal.adwVal[i] = ntohl(stRsp.adwData[i]);
    }
    fprintf(stderr, "%d\n", GetMinuteIndex(time(NULL)));

    return true;
}

int CMBMonitorHelper::GetMinuteIndex(time_t uTime)
{
    struct tm stTm0 = {0};
    struct tm stTm = {0};
    localtime_r(&uTime, &stTm);
    stTm0 = stTm;
    stTm.tm_hour = 0;
    stTm.tm_min = 0;
    stTm.tm_sec = 0;
    time_t uTime0 = mktime(&stTm);
    return (uTime - uTime0) / 60;
}

bool CQueryTimer::OnTimeout()
{
    fprintf(stderr, "OnTimeout\n");
    return true;
}


// CMyConfigure

bool CMyConfigure::OnLoadConfigure()
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

    if (!LoadFromFile(szCfg))
    {
        LOG_ERR("ERR | Load cfg(%s) failed", szCfg);
        return false;
    }

    ReadString("host_ip", m_szHostIp);
    ReadInt("host_port", m_iHostPort);

    ReadString("svr_ip", m_szSvrIp);
    ReadInt("svr_port", m_iSvrPort);

    ReadInt("hash_shm_key", m_iHashShmKey);
    ReadUInt32("hash_width", m_dwHashWidth);
    ReadUInt32("hash_height", m_dwHashHeight);

    ReadInt("hash_spec_shm_key", m_iHashSpecShmKey);
    //ReadUInt32("hash_spec_width", m_dwHashSpecWidth);
    //ReadUInt32("hash_spec_height", m_dwHashSpecHeight);
    ReadUInt32("hash_width", m_dwHashSpecWidth);
    ReadUInt32("hash_height", m_dwHashSpecHeight);

    ReadString("attr_db_host", m_szAttrDbHost);
    ReadInt("attr_db_port", m_iAttrDbPort);
    ReadString("attr_db_user", m_szAttrDbUser);
    ReadString("attr_db_pass", m_szAttrDbPass);
    ReadString("attr_db_name", m_szAttrDbName);
    ReadString("attr_db_char", m_szAttrDbChar);
    ReadString("attr_db_table", m_szAttrDbTable);



    return true;
}




// CDemoTimerService

CDemoTimerService::CDemoTimerService()
{
}

CDemoTimerService::~CDemoTimerService()
{
}

bool CDemoTimerService::OnLoadConfigure()
{
    return m_oCfg.OnLoadConfigure();
}

bool CDemoTimerService::OnServiceInit()
{
    m_oEfw.Init();

    m_oMon.Init(m_oCfg.m_szSvrIp, m_oCfg.m_iSvrPort, m_oCfg.m_szAttrDbHost, m_oCfg.m_iAttrDbPort, m_oCfg.m_szAttrDbUser, m_oCfg.m_szAttrDbPass, m_oCfg.m_szAttrDbChar, m_oCfg.m_szAttrDbName, m_oCfg.m_szAttrDbTable);
    S_ATTR_VAL stAttrVal;
    memset(&stAttrVal, 0, sizeof (stAttrVal));
    CCommandLine::GetArgValue(2);
    int iDaysBefore = 0;
    CStringHelper::StrToVal(CStr(CCommandLine::GetArgValue(2), 1024), &iDaysBefore, sizeof (iDaysBefore));
    //oMon.GetValOfAttr(2616, 148347, iDaysBefore, stAttrVal);
    //for (int i = 0; i < 1440; i++)
    //{
    //        fprintf(stderr, "%d, %u\n", i, stAttrVal.adwVal[i]);
    //    }

    S_ATTR_INFO stAttrInfo;
    memset(&stAttrVal, 0, sizeof (stAttrInfo));
    //oMon.GetAttrInfo(148348, stAttrInfo);

    //fprintf(stderr, "%d, %s, %d\n", stAttrInfo.uAttrId, stAttrInfo.szAttrName, (int)stAttrInfo.eAttrType);

    S_MON_KEY stEmptyKey = {0};
    m_oHash.Init(m_oCfg.m_dwHashWidth, m_oCfg.m_dwHashHeight, stEmptyKey);
    m_oSpecHash.Init(m_oCfg.m_dwHashSpecWidth, m_oCfg.m_dwHashSpecHeight, stEmptyKey);


    m_oEfw.RegisterTimer(&m_oQt, CTime(10, 0), CTime(0, 0));

    return true;
}

bool CDemoTimerService::OnEnterLoop()
{
    m_oEfw.FrameworkLoop();

    return true;
}

int main(int argc, char* const* argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();
    CLog::SetLogLevel(E_LL_DBG);
    CLog::SetOutFile(stderr);

    if (!g_oSvc.StartService())
    {
        return -1;
    }

    g_oSvc.WaitService();

    return 0;
}


