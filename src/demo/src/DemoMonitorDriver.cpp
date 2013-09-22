/* 
 * File:   DemoMonitorDriver.cpp
 * Author: thunderliu
 * 
 * Created on 2012年5月10日, 下午3:10
 */

#define TSNETFW_FEATURE_MYSQL
#define TSNETFW_FEATURE_READLINE
#define TSNETFW_FEATURE_LUA

#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "lib64/oi_shm.h"
#include "lib64/Attr_API.h"
#include "TSNetFw.h"
#include "DemoMonitorDriver.h"
#include "TSSynch.h"



CMyConfigure g_oCfg;
CDbMySQL g_oDbMon;
CDbMySQL g_oDbWm;
CMBMonHash g_oHash;
CMBMonHashSpec g_oHashSpec;
CDemoMonitorDriver g_oSvc;
CDemoDataStorage g_oSvcSpec;
CDemoScriptJudge g_oSvcJdg;
CDemoScriptDaemon g_oSvcSpt;



// CMyConfigure

CMyConfigure::CMyConfigure()
: m_szIpHost(32)
, m_szIpMon(32)
, m_szDbHostMon(32)
, m_szDbUserMon(32)
, m_szDbPassMon(32)
, m_szDbNameMon(32)
, m_szDbCharMon(32)
, m_szDbTableMonAttr(32)
, m_szDbHostWm(32)
, m_szDbUserWm(32)
, m_szDbPassWm(32)
, m_szDbNameWm(32)
, m_szDbCharWm(32)
, m_szDbTableWmItems(32)
, m_szDbTableWmSubItems(32)
{
}

bool CMyConfigure::OnLoadConfigure()
{
    CStr szCfg(1024);
    if (true || !CCommandLine::GetArgValue(1))
    {
        szCfg.Format("%s.cfg", CCommandLine::GetArgValue(0));
    }
    else
    {
        szCfg.Format("%s", CCommandLine::GetArgValue(1));
    }

    if (!LoadFromFile(szCfg))
    {
        LOG_ERR("ERR | Load cfg(%s) failed", szCfg.GetBuffer());
        return false;
    }

    ReadString("ip_host", m_szIpHost);
    ReadInt("port_host", m_iPortHost);

    ReadString("ip_mon", m_szIpMon);
    ReadInt("port_mon", m_iPortMon);
    ReadInt("timeout_mon", m_iTimeoutMon);

    ReadString("db_host_mon", m_szDbHostMon);
    ReadInt("db_port_mon", m_iDbPortMon);
    ReadString("db_user_mon", m_szDbUserMon);
    ReadString("db_pass_mon", m_szDbPassMon);
    ReadString("db_name_mon", m_szDbNameMon);
    ReadString("db_char_mon", m_szDbCharMon);
    ReadString("db_table_mon_attr", m_szDbTableMonAttr);

    ReadInt("hash_shm_key", m_iHashShmKey);
    ReadUInt32("hash_width", m_dwHashWidth);
    ReadUInt32("hash_height", m_dwHashHeight);

    ReadInt("hash_shm_key_spec", m_iHashShmKeySpec);

    ReadString("db_host_wm", m_szDbHostWm);
    ReadInt("db_port_wm", m_iDbPortWm);
    ReadString("db_user_wm", m_szDbUserWm);
    ReadString("db_pass_wm", m_szDbPassWm);
    ReadString("db_name_wm", m_szDbNameWm);
    ReadString("db_char_wm", m_szDbCharWm);
    ReadString("db_table_wm_items", m_szDbTableWmItems);
    ReadString("db_table_wm_sub_items", m_szDbTableWmSubItems);

    ReadInt("view_mon", m_iViewMon);
    ReadInt("attr_hb", m_iAttrHb);

    ReadLong("timeout_run_script", m_lTimeoutRunSpt);
    ReadLong("delay_run_script", m_lDelayRunSpt);

    return true;
}

uint16_t CMBMonHash::GetCurMinPos()
{
    //CMBMonHash::CHashHeader* pHdr = GetHeader();
    assert(m_pHeader);

    return m_pHeader->tExtention.wCurMinPos;
}

void CMBMonHash::SetCurMinPos(uint16_t wCurMinPos)
{
    //CMBMonHash::CHashHeader* pHdr = GetHeader();
    assert(m_pHeader);
    m_pHeader->tExtention.wCurMinPos = wCurMinPos;
}

bool CMBMonHash::GetNowSubItemVal(uint32_t dwItemId, uint8_t cTypeId, uint32_t& dwVal)
{

    S_MON_KEY stKey;
    stKey.dwId = dwItemId;
    CHashNode* pNode = FindNode(stKey);
    assert(ISNOT_INVALID_NODE(pNode));
    if (!pNode)
    {
        return false;
    }

    int iIndex = pNode->tMapped.stData.FindType(cTypeId);
    if (iIndex < 0)
    {
        return false;
    }

    uint16_t wCurMinPos = GetCurMinPos();
    dwVal = pNode->tMapped.stData.astItems[iIndex].adwAttrVal[wCurMinPos];
    if (!dwVal && wCurMinPos > 0)
    {
        uint32_t dwPrevVal = pNode->tMapped.stData.astItems[iIndex].adwAttrVal[wCurMinPos - 1];
        if (dwPrevVal > 5)
        {
            dwVal = dwPrevVal;
        }
    }

    return true;
}

long CMBMonitorHelper::CHBThread::ThreadProc()
{
    S_ATTR_VAL stAttrVal;
    for (int i = 0;; i++)
    {
        Attr_API_Set(g_oCfg.m_iAttrHb, CMBMonitorHelper::GetMinuteIndex(time(NULL)));
        sleep(1);
        if (i == 1)
        {
            if (g_oSvc.m_oMon.GetValOfAttr(g_oCfg.m_iViewMon, g_oCfg.m_iAttrHb, 0, stAttrVal, g_oCfg.m_iTimeoutMon))
            {
                for (int j = 1439; j >= 0; --j)
                {
                    if (stAttrVal.adwVal[j])
                    {
                        g_oHash.SetCurMinPos(j);
                        break;
                    }
                }
            }
            i = 0;
        }
    }

    return 0;
}

CMBMonitorHelper::CMBMonitorHelper()
{
}

bool CMBMonitorHelper::Init(const char* pSvrIp, int iSvrPort, const char* pAttrDbHost, int iAttrDbPort, const char* pAttrDbUser, const char* pAttrDbPass, const char* pAttrDbChar, const char* pAttrDbName, const char* pAttrDbTable)
{
    m_oThrd.Start();
    m_oSa.SetSockAddrIn(pSvrIp, iSvrPort);
    m_oUdp.Bind(NULL, iSvrPort);
    g_oDbMon.Init(pAttrDbHost, iAttrDbPort, pAttrDbUser, pAttrDbPass, pAttrDbName, pAttrDbChar);
    strncpy(m_szAttrDbTable, pAttrDbTable, sizeof (m_szAttrDbTable));
    return true;
}

bool CMBMonitorHelper::GetAttrInfo(int iAttrId, S_ATTR_INFO& rstAttrInfo)
{
    CStr szSql(1024);
    szSql.Format("select Fattr_id, Fattr_name, Fdata_type from t_attr where Fattr_id = %d;", iAttrId);
    g_oDbMon.Query(szSql);
    CDbMySQLResult* pRes = g_oDbMon.GetResult();
    pRes->GetNextLine();
    char* pData = pRes->GetData(0);
    rstAttrInfo.uAttrId = atol(pData);
    pData = pRes->GetData(1);
    strncpy(rstAttrInfo.szAttrName, pData, sizeof (rstAttrInfo.szAttrName));
    pData = pRes->GetData(2);
    rstAttrInfo.wAttrType = (uint16_t)atoi(pData);
    pRes->Release();
    return true;
}

bool CMBMonitorHelper::GetValOfAttr(int iViewId, int iAttrId, int iDaysBefore, S_ATTR_VAL& rstAttrVal, int iTimeout)
{
    CGuard oG(&m_oMtx);
    S_MON_REQ stReq = {0};
    stReq.iType = htonl(1);
    stReq.iId = htonl(iViewId);
    stReq.iAttrId = htonl(iAttrId);
    stReq.iDaysBefore = htonl(iDaysBefore);
    stReq.iFrom = htonl(0);
    stReq.iEnd = htonl(1440);
    int iRet;
    iRet = m_oUdp.SendTo(&stReq, sizeof (stReq), &m_oSa);
    if (iRet < 0)
    {
        LOG_ERR("ERR | %s:%d, %s", m_oSa.GetAddr(), m_oSa.GetPort(), strerror(errno));
        return false;
    }
    S_MON_RSP stRsp;
    CSockAddrIn oSa;
    int iOrgTimeout = m_oUdp.GetRecvTimeout();
    m_oUdp.SetRecvTimeout(iTimeout);
    for (;;)
    {
        memset(&stRsp, 0, sizeof (stRsp));
        iRet = m_oUdp.RecvFrom(&stRsp, sizeof (stRsp), &oSa);
        if (iRet < 0)
        {
            break;
        }

        if (stRsp.iAttrId == stReq.iAttrId && stRsp.iId == stReq.iId && stRsp.iDaysBefore == stReq.iDaysBefore)
        {
            break;
        }
        LOG_ERR("WARNING | Rsp pkt err, iAttrId(%d/%d), iViewId(%d/%d), iDaysBefore(%d/%d)", ntohl(stReq.iAttrId), ntohl(stRsp.iAttrId), ntohl(stReq.iId), ntohl(stRsp.iId), ntohl(stReq.iDaysBefore), ntohl(stRsp.iDaysBefore));
    }
    m_oUdp.SetRecvTimeout(iOrgTimeout);

    if (iRet < 0)
    {
        LOG_ERR("ERR | %s", strerror(errno));
        return false;
    }

    for (int i = 0; i < 1440; i++)
    {
        rstAttrVal.adwVal[i] = ntohl(stRsp.adwData[i]);
    }

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
    CStr szSql(128);

    szSql.Format("select f_Id from %s;", g_oCfg.m_szDbTableWmItems.GetBuffer());

    bool bRet;
    bRet = g_oDbWm.Query(szSql);
    assert(bRet);

    CDbMySQLResult* pItemsRes = g_oDbWm.GetResult();
    assert(pItemsRes);

    const char* pItemId = NULL;
    int iItemId = 0;

    const char* pTmp = NULL;
    int iAttrId = 0;
    int iViewId = 0;
    int iTypeId = 0;
    S_ATTR_VAL stAttrVal;
    S_MON_KEY stKey;
    bool bNew;
    CMBMonHash::CHashNode* pNode;
    uint8_t cSubItemIndex;
    S_SUBITEM* pSubItem;

    CDbMySQLResult* pViewAttrRes = NULL;

    // 遍历所有Item
    while (pItemsRes->GetNextLine())
    {
        pItemId = pItemsRes->GetData(0);
        assert(pItemId);
        iItemId = atoi(pItemId);

        szSql.Format("select f_AttrId, f_ViewId, f_TypeId from %s where f_ItemId=%s order by f_TypeId;", g_oCfg.m_szDbTableWmSubItems.GetBuffer(), pItemId);
        g_oDbWm.Query(szSql);
        pViewAttrRes = g_oDbWm.GetResult();
        assert(pViewAttrRes);

        stKey.dwId = iItemId;
        pNode = g_oHash.FindNodeToSet(stKey, &bNew);
        assert(ISNOT_INVALID_NODE(pNode) && pNode);
        if (bNew)
        {
            //pNode->tMapped.stData.cSubItemCount = 0;
            memset(&pNode->tMapped, 0, sizeof (pNode->tMapped));
            for (int i = 0; i < MAX_SUB_ITEM_COUNT; i++)
            {
                pNode->tMapped.stData.astItems[i].cMonWeekDayIndex = 0xFF;
            }
        }

        // 遍历所有SubItem
        cSubItemIndex = 0;

        while (pViewAttrRes->GetNextLine())
        {
            pSubItem = &pNode->tMapped.stData.astItems[cSubItemIndex];

            pTmp = pViewAttrRes->GetData(0);
            assert(pTmp);
            iAttrId = atoi(pTmp);

            pTmp = pViewAttrRes->GetData(1);
            assert(pTmp);
            iViewId = atoi(pTmp);

            pTmp = pViewAttrRes->GetData(2);
            assert(pTmp);
            iTypeId = atoi(pTmp);

            fprintf(stderr, "Item: %d, SubItem: %d, ViewId: %d, AttrId: %d\n", iItemId, cSubItemIndex, iViewId, iAttrId);
            if (g_oSvc.m_oMon.GetValOfAttr(iViewId, iAttrId, 0, stAttrVal, g_oCfg.m_iTimeoutMon))
            {
                //fprintf(stderr, "Attr: %d, View: %d, SUCC\n", iAttrId, iViewId);
                CheckNewDay(*pSubItem, stAttrVal);

                pSubItem->cType = iTypeId;
                memmove(pSubItem->adwAttrVal, stAttrVal.adwVal, sizeof (stAttrVal.adwVal));
            }
            else
            {
                // ERR
                fprintf(stderr, "Attr: %d, View: %d, FAILED\n", iAttrId, iViewId);
            }
            ++cSubItemIndex;
        }

        pNode->tMapped.stData.cSubItemCount = cSubItemIndex;
        for (int i = cSubItemIndex; i < MAX_SUB_ITEM_COUNT; i++)
        {
            pNode->tMapped.stData.astItems[i].cMonWeekDayIndex = 0xFF;
        }

        pViewAttrRes->Release();
    }

    pItemsRes->Release();

    return true;
}

bool CQueryTimer::CheckNewDay(S_SUBITEM& rSubItem, S_ATTR_VAL& rAttrVals)
{
    time_t uNow = time(NULL);
    struct tm stTm = {0};
    localtime_r(&uNow, &stTm);

    if (rSubItem.cMonWeekDayIndex == 0xFF)
    {
        // 代表新加入的条目，
        //rSubItem.cLocalUpdateWeekDayIndex = stTm.tm_wday;//(uint32_t)uNow;
        rSubItem.cMonWeekDayIndex = stTm.tm_wday;
        return false;
    }
    for (int i = 1439; i >= 0; i--)
    {
        // 对明显是新一天的数据进行判断，更新cWeekDayIndex
        if (rAttrVals.adwVal[i] == 0 && rSubItem.adwAttrVal[i] != 0)
        {
            if (stTm.tm_hour == 0)
            {
                //rSubItem.cLocalUpdateWeekDayIndex = (stTm.tm_wday + 6) % 7;
                rSubItem.cMonWeekDayIndex = stTm.tm_wday;
            }
            else
            {
                if (stTm.tm_hour != 23)
                {
                    LOG_ERR("ERR | Wrong minute index(%d), hour(%d)", i, stTm.tm_hour);
                    for (int i = 0; i < 1440; i++)
                    {
                        fprintf(stderr, "%d: %u, %u\n", i, rSubItem.adwAttrVal[i], rAttrVals.adwVal[i]);
                    }
                    return false;
                }

                rSubItem.cMonWeekDayIndex = (stTm.tm_wday + 1) % 7;
                //assert(stTm.tm_hour == 23);
                //rSubItem.cLocalUpdateWeekDayIndex = stTm.tm_wday;

            }

            return true;
        }

        // 对不明显的新一天数据进行补充判断，更新cWeekDayIndex
        if (stTm.tm_hour == 0 && stTm.tm_min > 3 && (stTm.tm_wday > rSubItem.cMonWeekDayIndex || (stTm.tm_wday == 0 && rSubItem.cMonWeekDayIndex == 6)))
        {
            // WARNING 特殊情况发生，需要上报
            rSubItem.cMonWeekDayIndex = stTm.tm_wday;
            return true;
        }
    }

    return false;
}


// CDemoMonitorDriver

CDemoMonitorDriver::CDemoMonitorDriver()
{
}

CDemoMonitorDriver::~CDemoMonitorDriver()
{
}

bool CDemoMonitorDriver::OnLoadConfigure()
{
    return g_oCfg.OnLoadConfigure();
}

bool CDemoMonitorDriver::OnServiceInit()
{
    bool bRet;

    bRet = m_oEfw.Init();
    assert(bRet);

    bRet = m_oMon.Init(g_oCfg.m_szIpMon, g_oCfg.m_iPortMon, g_oCfg.m_szDbHostMon, g_oCfg.m_iDbPortMon, g_oCfg.m_szDbUserMon, g_oCfg.m_szDbPassMon, g_oCfg.m_szDbCharMon, g_oCfg.m_szDbNameMon, g_oCfg.m_szDbTableMonAttr);
    assert(bRet);

    S_MON_KEY stEmptyKey = {0};
    g_oHash.SetShmKey(g_oCfg.m_iHashShmKey);
    bRet = g_oHash.Init(g_oCfg.m_dwHashWidth, g_oCfg.m_dwHashHeight, stEmptyKey);
    assert(bRet);
    g_oHash.Clear();

    bRet = g_oDbWm.Init(g_oCfg.m_szDbHostWm, g_oCfg.m_iDbPortWm, g_oCfg.m_szDbUserWm, g_oCfg.m_szDbPassWm, g_oCfg.m_szDbNameWm, g_oCfg.m_szDbCharWm);
    assert(bRet);

    bRet = m_oEfw.RegisterTimer(&m_oQt, CTime(10, 0), CTime(0, 0));
    assert(bRet);

    return true;
}

bool CDemoMonitorDriver::OnEnterLoop()
{
    m_oEfw.FrameworkLoop();

    return true;
}





// CPolicyTimer

bool CPolicyTimer::OnTimeout()
{
    CStr szSql(128);

    szSql.Format("select f_Id from %s;", g_oCfg.m_szDbTableWmItems.GetBuffer());

    bool bRet;
    bRet = g_oDbWm.Query(szSql);
    assert(bRet);

    CDbMySQLResult* pItemsRes = g_oDbWm.GetResult();
    assert(pItemsRes);

    const char* pItemId = NULL;
    int iItemId = 0;
    S_MON_KEY stKey;
    CMBMonHash::CHashNode* pNode;
    CMBMonHashSpec::CHashNode* pNodeSpec;
    bool bNew;

    // 遍历所有Item
    while (pItemsRes->GetNextLine())
    {
        pItemId = pItemsRes->GetData(0);
        assert(pItemId);
        iItemId = atoi(pItemId);

        stKey.dwId = iItemId;
        pNode = g_oHash.FindNode(stKey);
        assert(ISNOT_INVALID_NODE(pNode));
        if (!pNode)
        {
            // ERR NOT FOUND
            continue;
        }


        pNodeSpec = g_oHashSpec.FindNodeToSet(stKey, &bNew);
        assert(ISNOT_INVALID_NODE(pNodeSpec));
        assert(pNodeSpec); // FULL

        if (bNew)
        {
            memset(&pNodeSpec->tMapped, 0, sizeof (pNodeSpec->tMapped));
        }

        if (pNode->tMapped.stData.cSubItemCount > 0)
        {
            uint8_t cWeekDayIndex = pNode->tMapped.stData.astItems[0].cMonWeekDayIndex;
            //fprintf(stderr, "Item(%d:%d), %d sub items copied\n", iItemId, cWeekDayIndex, pNode->tMapped.stData.cSubItemCount);
            memmove(&pNodeSpec->tMapped.astDataSpec[cWeekDayIndex], &pNode->tMapped.stData.astItems, sizeof (S_DATA_PER_MIN_MON));
        }

    }

    pItemsRes->Release();

    return true;
}


// CDemoDataStorage

CDemoDataStorage::CDemoDataStorage()
{
}

CDemoDataStorage::~CDemoDataStorage()
{
}

bool CDemoDataStorage::OnLoadConfigure()
{
    return g_oCfg.OnLoadConfigure();
}

bool CDemoDataStorage::OnServiceInit()
{
    bool bRet;

    bRet = m_oEfw.Init();
    assert(bRet);

    S_MON_KEY stEmptyKey = {0};
    g_oHash.SetShmKey(g_oCfg.m_iHashShmKey);
    bRet = g_oHash.Init(g_oCfg.m_dwHashWidth, g_oCfg.m_dwHashHeight, stEmptyKey);
    assert(bRet);

    g_oHashSpec.SetShmKey(g_oCfg.m_iHashShmKeySpec);
    bRet = g_oHashSpec.Init(g_oCfg.m_dwHashWidth, g_oCfg.m_dwHashHeight, stEmptyKey);
    assert(bRet);

    bRet = g_oDbWm.Init(g_oCfg.m_szDbHostWm, g_oCfg.m_iDbPortWm, g_oCfg.m_szDbUserWm, g_oCfg.m_szDbPassWm, g_oCfg.m_szDbNameWm, g_oCfg.m_szDbCharWm);
    assert(bRet);

    bRet = m_oEfw.RegisterTimer(&m_oPt, CTime(10, 0), CTime(0, 0));
    assert(bRet);

    return true;
}

bool CDemoDataStorage::OnEnterLoop()
{
    m_oEfw.FrameworkLoop();

    return true;
}


// CMonLuaSe

CMonLuaSe::CMonLuaSe()
{
}

CMonLuaSe::CMonLuaSe(lua_State* pL)
: CLuaSe(pL)
{
}

// CDbgLuaSe

//const size_t CDbgLuaSe::PREFIX_LEN = strlen("/data/pubadmin/privdata/webmon/dbgscript/");

CDbgLuaSe::CDbgLuaSe()
{
}

CDbgLuaSe::CDbgLuaSe(lua_State* pL)
: CMonLuaSe(pL)
{
}

void CDbgLuaSe::OnRunFileOk(CThread* pThrd, const char* pFile, const void* pParam)
{
    CRunInst* pInst = (CRunInst*)pParam;
    bool bLock = pInst->Lock();

    fprintf(stderr, "======OK======\n");

    CStr szSql(1024);
    szSql.Format("update %s set f_DbgStat = %d where f_Id = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), CDbCheckerWm::E_OK, pInst->iItemId);
    g_oDbWm.Query(szSql);
    szSql.Format("update %s set f_DbgEcho = \"PASSED! Elapsed time: %.2f ms\" where f_Id = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), pInst->oSw.Peek() / 1000.0, pInst->iItemId);
    g_oDbWm.Query(szSql);
    pInst->eRes = CRunInst::E_OK;
    if (bLock)
    {
        pInst->Unlock();
    }
}

void CDbgLuaSe::OnRunFileError(CThread* pThrd, const char* pFile, const CStr& roErr, const void* pParam)
{
    CRunInst* pInst = (CRunInst*)pParam;
    bool bLock = pInst->Lock();

    fprintf(stderr, "======ERR: %s, %s======\n", roErr.GetBuffer(), strerror(errno));

    CStr szSql(1024);
    szSql.Format("update %s set f_DbgStat = %d where f_Id = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), CDbCheckerWm::E_FAILED, pInst->iItemId);
    g_oDbWm.Query(szSql);

    const char* pPos = strcasestr(roErr, ".lua:");
    if (pPos)
    {
        for (; pPos != roErr.GetBuffer() && *(pPos - 1) != '/'; --pPos);
    }
    else
    {
        pPos = (char*)roErr.GetBuffer();
    }
    szSql.Format("update %s set f_DbgEcho = \"%s\" where f_Id = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), pPos, pInst->iItemId);


    LOG_ERR("MSG | %s", szSql.GetBuffer());
    g_oDbWm.Query(szSql);
    pInst->eRes = CRunInst::E_ERR;

    if (bLock)
    {
        pInst->Unlock();
    }
}

void CDbgLuaSe::OnRunFileCancel(CThread* pThrd, const char* pFile, const void* pParam)
{
    CRunInst* pInst = (CRunInst*)pParam;
    bool bLock = pInst->TryLock();

    fprintf(stderr, "======Cancel: ======\n");

    CStr szSql(1024);
    szSql.Format("update %s set f_DbgStat = %d where f_Id = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), CDbCheckerWm::E_FAILED, pInst->iItemId);
    g_oDbWm.Query(szSql);
    szSql.Format("update %s set f_DbgEcho = \"TIMEOUT! Elapsed time: %.2f ms\" where f_Id = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), pInst->oSw.Peek() / 1000.0, pInst->iItemId);
    LOG_ERR("MSG | %s", szSql.GetBuffer());
    g_oDbWm.Query(szSql);
    pInst->eRes = CRunInst::E_CANCELED;

    if (bLock)
    {
        pInst->Unlock();
    }
}


// CRunInstChecker

CRunInstChecker::CRunInstChecker()
{
    m_oLst.Init();
}

bool CRunInstChecker::CreateRunInstance(CLuaSe* pL, const char* pScriptPath, int iItemId)
{
    CGuard oG(&m_oMtx);
    assert(pL);
    assert(pScriptPath);

    CRunInst* pInst = new CRunInst;
    pInst->pL = pL;
    pInst->pL->Init();
    LuaRegCFunc(*pInst->pL);
    pInst->lTimeout = g_oCfg.m_lTimeoutRunSpt * 1000000;
    pInst->iItemId = iItemId;
    pInst->eRes = CRunInst::E_RUNNING;
    pInst->oSw.Start();
    pInst->pLt = pInst->pL->AsyncRunFile(pScriptPath, pInst); //ONCE+//

    return m_oLst.PushTail(pInst) != NULL;
}

long CRunInstChecker::ThreadProc()
{
    CInstList::CListNode* pNode;
    CInstList::CListNode* pDel;
    CRunInst* pInst;
    for (;;)
    {
        m_oMtx.Lock();
        pNode = m_oLst.GetHeadNode();
        while (pNode)
        {
            pInst = pNode->tData;
            pInst->Lock();

            CInstList::CListNode* pTmp = m_oLst.GetHeadNode();
            while (pTmp)
            {
                fprintf(stderr, "<%p %p %p>  ", pTmp->pPrev, pTmp, pTmp->pNext);
                pTmp = pTmp->pNext;
            }
            fprintf(stderr, "\n");

            switch (pInst->eRes)
            {
            case CRunInst::E_RUNNING:
                if (pInst->oSw.Peek() > pInst->lTimeout)
                {
                    // 超时预处理
                    pInst->eRes = CRunInst::E_TIMEOUT;
                    LOG_ERR("MSG | Set timeout");
                }
                break;

            case CRunInst::E_TIMEOUT:
                LOG_ERR("MSG | To cancel");
                pInst->pLt->Cancel(); // ONCE +//
                //pInst->pLt->Wait();
                //pInst->eRes = CRunInst::E_CANCELED; //ONCE-//
                break;

            case CRunInst::E_OK:
            case CRunInst::E_ERR:
            case CRunInst::E_CANCELED:


                break;
            }

            pInst->Unlock();
            if (pInst->eRes == CRunInst::E_OK || pInst->eRes == CRunInst::E_ERR || pInst->eRes == CRunInst::E_CANCELED)
            {
                LOG_ERR("MSG | Pick node(%p), %d release inst", pNode, pInst->eRes);
                pDel = pNode;
                pNode = pNode->pNext;
                m_oLst.PickNode(pDel);
                delete pDel->tData->pLt; //ONCE +//
                delete pDel->tData->pL;
                delete pDel->tData;
                delete pDel;
            }
            else
            {
                pNode = pNode->pNext;
            }
        }
        m_oMtx.Unlock();
        sleep(1);
    }
    return 0;
}


// CDbCheckerWm

long CDbCheckerWm::ThreadProc()
{
    CStr szSql(1024);
    CDbMySQLResult* pItemRes;
    int iItemId;
    int iDbgStat;
    char* pDbgScriptPath;
    //CStr szErr(1024);

    for (;;)
    {
        szSql.Format("select f_Id, f_DbgStat, f_DbgScriptPath from %s;", g_oCfg.m_szDbTableWmItems.GetBuffer());
        if (!g_oDbWm.Query(szSql))
        {
            //assert(g_oDbWm.GetErrorString());
            //fprintf(stderr, "%s\n", g_oDbWm.GetErrorString());
            assert(false);
        }
        pItemRes = g_oDbWm.GetResult();
        assert(pItemRes);
        while (pItemRes->GetNextLine())
        {
            iItemId = (int)pItemRes->GetLong(0);
            iDbgStat = (int)pItemRes->GetLong(1);
            switch (iDbgStat)
            {
            case E_WAITING:
                pDbgScriptPath = pItemRes->GetData(2);
                szSql.Format("update %s set f_DbgStat = %d where f_Id = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), E_CHECKING, iItemId);
                g_oDbWm.Query(szSql);
                g_oSvcJdg.m_oInstChkr.CreateRunInstance(new CDbgLuaSe, pDbgScriptPath, iItemId);

                break;
            }
        }
        pItemRes->Release();
        sleep(1);
    }
    return 0;
}


// CCmdLnThread

long CCmdLnThread::ThreadProc()
{
    return LuaCmdLn();
}


// CDemoScriptJudge

CDemoScriptJudge::CDemoScriptJudge()
{
}

CDemoScriptJudge::~CDemoScriptJudge()
{
}

bool CDemoScriptJudge::OnLoadConfigure()
{
    return g_oCfg.OnLoadConfigure();
}

bool CDemoScriptJudge::OnServiceInit()
{
    bool bRet;

    bRet = m_oEfw.Init();
    assert(bRet);

    S_MON_KEY stEmptyKey = {0};
    g_oHash.SetShmKey(g_oCfg.m_iHashShmKey);
    bRet = g_oHash.Init(g_oCfg.m_dwHashWidth, g_oCfg.m_dwHashHeight, stEmptyKey);
    assert(bRet);

    g_oHashSpec.SetShmKey(g_oCfg.m_iHashShmKeySpec);
    bRet = g_oHashSpec.Init(g_oCfg.m_dwHashWidth, g_oCfg.m_dwHashHeight, stEmptyKey);
    assert(bRet);

    bRet = g_oDbWm.Init(g_oCfg.m_szDbHostWm, g_oCfg.m_iDbPortWm, g_oCfg.m_szDbUserWm, g_oCfg.m_szDbPassWm, g_oCfg.m_szDbNameWm, g_oCfg.m_szDbCharWm);
    assert(bRet);

    CStr szSql(1024);
    szSql.Format("update %s set f_DbgStat = %d where f_DbgStat = %d;", g_oCfg.m_szDbTableWmItems.GetBuffer(), CDbCheckerWm::E_WAITING, CDbCheckerWm::E_CHECKING);
    g_oDbWm.Query(szSql);

    if (CCommandLine::GetArgCount() == 2 && !strcmp(CCommandLine::GetArgValue(1), "-l"))
    {
        return true;
    }

    bRet = m_oInstChkr.Start();
    assert(bRet);

    bRet = m_oDbChkrWm.Start();
    assert(bRet);

    //bRet = m_oEfw.RegisterTimer(&m_oPt, CTime(10, 0), CTime(0, 0));
    //assert(bRet);

    return true;
}

bool CDemoScriptJudge::OnEnterLoop()
{
    CCmdLnThread oThrd;
    bool bCmdThrd = false;
    CCommandLine::GetArgCount() == 2 && !strcmp(CCommandLine::GetArgValue(1), "-l") && (bCmdThrd = true);

    bCmdThrd && oThrd.Start();

    m_oEfw.FrameworkLoop();

    bCmdThrd && oThrd.Wait();

    return true;
}


// CDbCheckerWmSpt

long CDbCheckerWmSpt::ThreadProc()
{
    CStr szSql(1024);
    CDbMySQLResult* pItemRes;
    int iItemId;
    char* pScriptPath;

    for (;;)
    {
        szSql.Format("select f_Id, f_ScriptPath from %s;", g_oCfg.m_szDbTableWmItems.GetBuffer());
        if (!g_oDbWm.Query(szSql))
        {
            //assert(g_oDbWm.GetErrorString());
            //fprintf(stderr, "%s\n", g_oDbWm.GetErrorString());
            assert(false);
        }
        pItemRes = g_oDbWm.GetResult();
        assert(pItemRes);
        while (pItemRes->GetNextLine())
        {
            pScriptPath = pItemRes->GetData(1);
            assert(pScriptPath);
            if (strstr(pScriptPath, ".lua"))
            {
                iItemId = (int)pItemRes->GetLong(0);
                g_oSvcSpt.m_oInstChkr.CreateRunInstance(new CPerLuaSe, pScriptPath, iItemId);
            }

        }
        pItemRes->Release();

        sleep(g_oCfg.m_lDelayRunSpt);
    }
    return 0;
}


// CPerLuaSe

CPerLuaSe::CPerLuaSe()
{
}

CPerLuaSe::CPerLuaSe(lua_State* pL)
: CMonLuaSe(pL)
{
}

void CPerLuaSe::OnRunFileOk(CThread* pThrd, const char* pFile, const void* pParam)
{
    CRunInst* pInst = (CRunInst*)pParam;
    bool bLock = pInst->Lock();

    pInst->eRes = CRunInst::E_OK;

    if (bLock)
    {
        pInst->Unlock();
    }
}

void CPerLuaSe::OnRunFileError(CThread* pThrd, const char* pFile, const CStr& roErr, const void* pParam)
{
    CRunInst* pInst = (CRunInst*)pParam;
    bool bLock = pInst->Lock();

    pInst->eRes = CRunInst::E_ERR;

    if (bLock)
    {
        pInst->Unlock();
    }
}

void CPerLuaSe::OnRunFileCancel(CThread* pThrd, const char* pFile, const void* pParam)
{
    CRunInst* pInst = (CRunInst*)pParam;
    bool bLock = pInst->TryLock();

    pInst->eRes = CRunInst::E_CANCELED;

    if (bLock)
    {
        pInst->Unlock();
    }
}


// CDemoScriptDaemon

CDemoScriptDaemon::CDemoScriptDaemon()
{
}

CDemoScriptDaemon::~CDemoScriptDaemon()
{
}

bool CDemoScriptDaemon::OnLoadConfigure()
{
    return g_oCfg.OnLoadConfigure();
}

bool CDemoScriptDaemon::OnServiceInit()
{
    bool bRet;

    bRet = m_oEfw.Init();
    assert(bRet);

    S_MON_KEY stEmptyKey = {0};
    g_oHash.SetShmKey(g_oCfg.m_iHashShmKey);
    bRet = g_oHash.Init(g_oCfg.m_dwHashWidth, g_oCfg.m_dwHashHeight, stEmptyKey);
    assert(bRet);

    g_oHashSpec.SetShmKey(g_oCfg.m_iHashShmKeySpec);
    bRet = g_oHashSpec.Init(g_oCfg.m_dwHashWidth, g_oCfg.m_dwHashHeight, stEmptyKey);
    assert(bRet);

    bRet = g_oDbWm.Init(g_oCfg.m_szDbHostWm, g_oCfg.m_iDbPortWm, g_oCfg.m_szDbUserWm, g_oCfg.m_szDbPassWm, g_oCfg.m_szDbNameWm, g_oCfg.m_szDbCharWm);
    assert(bRet);

    bRet = m_oInstChkr.Start();
    assert(bRet);

    bRet = m_oDbChkrWmSpt.Start();
    assert(bRet);

    return true;
}

bool CDemoScriptDaemon::OnEnterLoop()
{
    m_oEfw.FrameworkLoop();

    return true;
}


// Global Functions

DEF_LUACFUNC(GetCurTime)
{
    CLuaSe oL(L);

    oL.PushInteger((lua_Integer)time(NULL));
    return 1;
}

DEF_LUACFUNC(GetMinuteOfDay)
{
    CLuaSe oL(L);
    time_t uTime = oL.CheckInteger(1);

    oL.PushInteger(CMBMonitorHelper::GetMinuteIndex(uTime));
    return 1;
}

DEF_LUACFUNC(GetCurWeekDayIndex)
{
    CLuaSe oL(L);
    time_t uNow = time(NULL);
    struct tm stTm = {0};
    localtime_r(&uNow, &stTm);

    oL.PushInteger(stTm.tm_wday);
    return 1;
}

DEF_LUACFUNC(GetCurMonMinuteIndex)
{
    CLuaSe oL(L);
    CMBMonHash::CHashHeader* pHdr = g_oHash.GetHeader();
    if (!pHdr)
    {
        return oL.ThrowError(true, "GetHeader err");
    }

    oL.PushInteger(pHdr->tExtention.wCurMinPos);
    return 1;
}

DEF_LUACFUNC(GetCountOfSubItem)
{
    CLuaSe oL(L);
    int iItemId = oL.CheckInteger(1);
    int iWeekDayIndex = oL.CheckInteger(2);

    if (iWeekDayIndex < 0 || iWeekDayIndex >= 7)
    {
        return oL.ThrowError(true, "WeekDayIndex(%d) err", iWeekDayIndex);
    }

    CMBMonHashSpec::CHashNode* pNodeSpec;
    S_MON_KEY stKey;
    stKey.dwId = iItemId;
    pNodeSpec = g_oHashSpec.FindNode(stKey);

    if (IS_INVALID_NODE(pNodeSpec) || !pNodeSpec)
    {
        return oL.ThrowError(true, "Find Item(%d), pNode(%d) err", stKey.dwId, pNodeSpec);
    }

    uint8_t cCount = pNodeSpec->tMapped.astDataSpec[iWeekDayIndex].cSubItemCount;
    oL.PushInteger(cCount);
    return 1;
}

DEF_LUACFUNC(GetValOfSubItemByIndex)
{
    CLuaSe oL(L);
    int iItemId = oL.CheckInteger(1);
    int iSubItemIndex = oL.CheckInteger(2);
    int iMinuteIndex = oL.CheckInteger(3);
    int iWeekDayIndex = oL.CheckInteger(4);

    if (iWeekDayIndex < 0 || iWeekDayIndex >= 7)
    {
        return oL.ThrowError(true, "WeekDayIndex(%d) err", iWeekDayIndex);
    }

    if (iMinuteIndex < 0 || iMinuteIndex >= 1440)
    {
        return oL.ThrowError(true, "MinuteIndex(%d) err", iMinuteIndex);
    }

    CMBMonHashSpec::CHashNode* pNodeSpec;
    S_MON_KEY stKey;
    stKey.dwId = iItemId;
    pNodeSpec = g_oHashSpec.FindNode(stKey);

    if (IS_INVALID_NODE(pNodeSpec) || !pNodeSpec)
    {
        return oL.ThrowError(true, "Find Item(%u) err", stKey.dwId);
    }

    uint8_t cCount = pNodeSpec->tMapped.astDataSpec[iWeekDayIndex].cSubItemCount;
    if (iSubItemIndex < 0 || iSubItemIndex >= cCount)
    {
        return oL.ThrowError(true, "SubItemIndex(%d) err", iSubItemIndex);
    }
    oL.PushInteger(pNodeSpec->tMapped.astDataSpec[iWeekDayIndex].astItems[iSubItemIndex].adwAttrVal[iMinuteIndex]);
    return 1;
}

DEF_LUACFUNC(GetValOfSubItemByType)
{
    CLuaSe oL(L);
    int iItemId = oL.CheckInteger(1);
    int iSubItemType = oL.CheckInteger(2);
    int iMinuteIndex = oL.CheckInteger(3);
    int iWeekDayIndex = oL.CheckInteger(4);

    if (iWeekDayIndex < 0 || iWeekDayIndex >= 7)
    {
        return oL.ThrowError(true, "WeekDayIndex(%d) err", iWeekDayIndex);
    }

    if (iMinuteIndex < 0 || iMinuteIndex >= 1440)
    {
        return oL.ThrowError(true, "MinuteIndex(%d) err", iMinuteIndex);
    }

    CMBMonHashSpec::CHashNode* pNodeSpec;
    S_MON_KEY stKey;
    stKey.dwId = iItemId;
    pNodeSpec = g_oHashSpec.FindNode(stKey);

    if (IS_INVALID_NODE(pNodeSpec) || !pNodeSpec)
    {
        return oL.ThrowError(true, "Find Item(%u) err", stKey.dwId);
    }

    int iSubItemIndex = pNodeSpec->tMapped.astDataSpec[iWeekDayIndex].FindType(iSubItemType);

    if (iSubItemIndex < 0)
    {
        return oL.ThrowError(true, "Type(%d) not found err", iSubItemType);
    }
    oL.PushInteger(pNodeSpec->tMapped.astDataSpec[iWeekDayIndex].astItems[iSubItemIndex].adwAttrVal[iMinuteIndex]);
    return 1;
}

DEF_LUACFUNC(GetTypeOfSubItemByIndex)
{
    CLuaSe oL(L);
    int iItemId = oL.CheckInteger(1);
    int iSubItemIndex = oL.CheckInteger(2);
    int iWeekDayIndex = oL.CheckInteger(3);

    if (iWeekDayIndex < 0 || iWeekDayIndex >= 7)
    {
        return oL.ThrowError(true, "WeekDayIndex(%d) err", iWeekDayIndex);
    }

    CMBMonHashSpec::CHashNode* pNodeSpec;
    S_MON_KEY stKey;
    stKey.dwId = iItemId;
    pNodeSpec = g_oHashSpec.FindNode(stKey);

    if (IS_INVALID_NODE(pNodeSpec) || !pNodeSpec)
    {
        return oL.ThrowError(true, "Find Item(%u) err", stKey.dwId);
    }

    uint8_t cCount = pNodeSpec->tMapped.astDataSpec[iWeekDayIndex].cSubItemCount;
    if (iSubItemIndex < 0 || iSubItemIndex >= cCount)
    {
        return oL.ThrowError(true, "SubItemIndex(%d) err", iSubItemIndex);
    }
    oL.PushInteger(pNodeSpec->tMapped.astDataSpec[iWeekDayIndex].astItems[iSubItemIndex].cType);
    return 1;
}

DEF_LUACFUNC(System)
{
    CLuaSe oL(L);
    const char* pCmdLn = oL.CheckString(1);
    if (!pCmdLn)
    {
        return oL.ThrowError(true, "Command is NULL");
    }

    system(pCmdLn);

    return 0;
}

DEF_LUACFUNC(AttrAdd)
{
    DEF_LUAOBJ(CLuaSe, oL);
    int iAttrId = oL.CheckInteger(1);
    int iVal = oL.CheckInteger(2);
    if (Attr_API(iAttrId, iVal) != 0)
    {
        return oL.ThrowError(true, "Attr_API(%d, %d) err", iAttrId, iVal);
    }

    return 0;
}

DEF_LUACFUNC(AttrSet)
{
    DEF_LUAOBJ(CLuaSe, oL);
    int iAttrId = oL.CheckInteger(1);
    int iVal = oL.CheckInteger(2);
    if (Attr_API_Set(iAttrId, iVal) != 0)
    {
        return oL.ThrowError(true, "Attr_API_Set(%d, %d) err", iAttrId, iVal);
    }

    return 0;
}

DEF_LUACFUNC(AttrGet)
{
    DEF_LUAOBJ(CLuaSe, oL);
    int iAttrId = oL.CheckInteger(1);
    int iVal = 0;
    if (Attr_API_Get(iAttrId, &iVal) != 0)
    {
        return oL.ThrowError(true, "Attr_API_Get(%d, %d) err", iAttrId, iVal);
    }

    oL.PushInteger(iVal);
    return 1;
}

#define LUACFUNC_PAIR(func) { #func, func }
S_REG_LUA_CFUNC_PAIR g_astRegPair[] = {
    { "GetCurTime", GetCurTime},
    { "GetMinuteOfDay", GetMinuteOfDay},
    { "GetCurWeekDayIndex", GetCurWeekDayIndex},
    { "GetCurMonMinuteIndex", GetCurMonMinuteIndex},
    { "GetCountOfSubItem", GetCountOfSubItem},
    { "GetValOfSubItemByIndex", GetValOfSubItemByIndex},
    { "GetValOfSubItemByType", GetValOfSubItemByType},
    { "GetTypeOfSubItemByIndex", GetTypeOfSubItemByIndex},
    { "System", System},
    { "AttrAdd", AttrAdd},
    { "AttrSet", AttrSet},
    //    { "AttrGet", AttrGet },
    LUACFUNC_PAIR(AttrGet),
};

int LuaRegCFunc(CLuaSe& roL, CReadline* pRl)
{
    int iCount = (int)(sizeof (g_astRegPair) / sizeof (S_REG_LUA_CFUNC_PAIR));
    for (int i = 0; i < iCount; i++)
    {
        roL.RegisterCFunction(g_astRegPair[i].pName, g_astRegPair[i].pFunc);
        if (pRl)
        {
            pRl->AddToReadline(g_astRegPair[i].pName);
        }
    }

    return iCount;
}

int LuaCmdLn()
{
    CLuaSe oL;
    oL.Init();
    CStr oErr(128);

    CStr oBuf(256);

    CReadline oRl;
    oRl.SetPrompt("> ");

    LuaRegCFunc(oL, &oRl);

    bool bRet;
    const char* pStr;

    while (oRl.Readline(oBuf) >= 0)
    {
        pStr = oBuf;
        bRet = oL.RunString(pStr, &oErr);
        if (!bRet)
        {
            while (!bRet && CLuaSe::HasErrorEofMark(oErr))
            {
                oL.PushString(pStr);
                if (oRl.Readline(oBuf, ">> ") < 0)
                {
                    return 0;
                }
                oL.PushString(oBuf);
                oL.PushString("\n");
                oL.Insert(-2);
                oL.Concat(3);
                pStr = oL.ToString(CLuaSe::STACK_POS_TOP);
                oL.Pop(1);
                bRet = oL.RunString(pStr, &oErr);
            }
            if (!bRet)
            {
                fprintf(stderr, "%s\n", oErr.GetBuffer());
            }
        }
    }

    return 0;
}

int main(int argc, char* const* argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();
    CLog::SetLogLevel(E_LL_DBG);
    CLog::SetOutFile(stderr);
    const char* pName = CCommandLine::GetArgValue(0);

    if (strstr(pName, "/DemoMonitorDriver"))
    {
        if (!g_oSvc.StartService())
        {
            return -1;
        }

        g_oSvc.WaitService();
    }
    else if (strstr(pName, "/DemoDataStorage"))
    {
        if (!g_oSvcSpec.StartService())
        {
            return -1;
        }

        g_oSvcSpec.WaitService();
    }
    else if (strstr(pName, "/DemoScriptJudge"))
    {
        if (!g_oSvcJdg.StartService())
        {
            return -1;
        }

        g_oSvcJdg.WaitService();
    }
    else if (strstr(pName, "/DemoScriptDaemon"))
    {
        if (!g_oSvcSpt.StartService())
        {
            return -1;
        }

        g_oSvcSpt.WaitService();
    }

    return 0;
}


