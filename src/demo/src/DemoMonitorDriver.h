/* 
 * File:   DemoMonitorDriver.h
 * Author: thunderliu
 *
 * Created on 2012年5月10日, 下午3:10
 */

#ifndef __DEMOMONITORDRIVER_H__
#define	__DEMOMONITORDRIVER_H__

#include "lib64/Attr_API.h"



#define MAX_SUB_ITEM_COUNT 10

namespace DemoMonitorDriver
{

#pragma pack(push, 1)

typedef struct
{
    int iType; // 0 --Server 1--- View
    int iId; //机器id或视图id
    int iAttrId; //属性id
    int iDaysBefore;
    int iFrom;
    int iEnd;
} S_MON_REQ;

typedef struct
{
    uint16_t uSeq;
    uint16_t uPkgType;
    int iId;
    int iAttrId;
    int iDaysBefore;
    int iFrom;
    int iEnd;
    uint32_t adwData[1440]; //每天1440个单点数据
} S_MON_RSP;

typedef struct
{
    uint32_t uAttrId;
    char szAttrName[60];
    uint16_t wAttrType;
} S_ATTR_INFO;

typedef struct
{
    uint32_t adwVal[1440];
} S_ATTR_VAL;

typedef struct
{
    uint8_t cMonWeekDayIndex;
    uint8_t cType;
    uint32_t adwAttrVal[1440];
} S_SUBITEM;

typedef struct
{

    union
    {

        struct
        {
            uint8_t cMonWeekDayIndex1;
            uint8_t cType1;
            uint32_t dwTotalCount[1440];

            uint8_t cMonWeekDayIndex2;
            uint8_t cType2;
            uint32_t dwCallDelay[1440];

            uint8_t cMonWeekDayIndex3;
            uint8_t cType3;
            uint32_t dwFailedCount[1440];

            uint8_t cMonWeekDayIndex4;
            uint8_t cType4;
            uint32_t dwTimeoutCount[1440];

            S_SUBITEM astOtherItems[6];
        };
        S_SUBITEM astItems[MAX_SUB_ITEM_COUNT];
    };
    uint8_t cSubItemCount;

    int FindType(uint8_t cType)
    {
        int i = 0;
        for (i = 0; i < cSubItemCount; i++)
        {
            //fprintf(stderr, "%d/%d, %u/%u\n", i, MAX_SUB_ITEM_COUNT, astItems[i].cType, cType);
            if (astItems[i].cType == cType)
            {
                return i;
            }
        }
        return -1;
    }
    // Reserved

} S_DATA_PER_MIN_MON;

typedef struct
{
    uint32_t dwId;
} S_MON_KEY;

typedef struct
{
    S_DATA_PER_MIN_MON stData;
} S_MON_MAPPED;

typedef struct
{
    uint16_t wCurMinPos;
} S_MON_EXT;

typedef struct
{
    S_DATA_PER_MIN_MON astDataSpec[7];
} S_MON_MAPPED_SPEC;

#pragma pack(pop)

typedef enum
{
    E_AT_CUMULANT = 0,
    E_AT_BPS = 1,
    E_AT_MOMENT = 2
} E_ATTR_TYPE;


// CMyConfigure

class CMyConfigure : public CConfigureFile
{
public:
    CMyConfigure();
    virtual bool OnLoadConfigure();

public:
    CStr m_szIpHost;
    int m_iPortHost;

    CStr m_szIpMon;
    int m_iPortMon;
    int m_iTimeoutMon;

    int m_iHashShmKey;
    uint32_t m_dwHashWidth;
    uint32_t m_dwHashHeight;

    int m_iHashShmKeySpec;

    CStr m_szDbHostMon;
    int m_iDbPortMon;
    CStr m_szDbUserMon;
    CStr m_szDbPassMon;
    CStr m_szDbNameMon;
    CStr m_szDbCharMon;
    CStr m_szDbTableMonAttr;

    CStr m_szDbHostWm;
    int m_iDbPortWm;
    CStr m_szDbUserWm;
    CStr m_szDbPassWm;
    CStr m_szDbNameWm;
    CStr m_szDbCharWm;
    CStr m_szDbTableWmItems;
    CStr m_szDbTableWmSubItems;

    int m_iViewMon;
    int m_iAttrHb;

    long m_lTimeoutRunSpt;
    long m_lDelayRunSpt;

};


// CShmHash

template <class KEY, class MAPPED, class EXTENTION = EMPTY>
class CShmHash : public CHashMap<KEY, MAPPED, EXTENTION>
{
public:
    //typedef typename CHashMap<KEY, MAPPED, EXTENTION>::CHashHeader CHashHeader;

public:

    void SetShmKey(int iShmKey)
    {
        m_iShmKey = iShmKey;
    }

    bool IsNewShm() const
    {
        return m_bIsNew;
    }

protected:

    bool OpenShareMemery(int iShmKey, size_t uShmSize, const typename CShmHash<KEY, MAPPED, EXTENTION>::CHashHeader& roHdr, bool& bIsNew)
    {
        typename CShmHash::CHashHeader* pHdr = NULL;
        int iRet = OI_GetShm3((void**)&pHdr, iShmKey, uShmSize, 0666 | IPC_CREAT);
        if (iRet < 0)
        {
            // 错误
            LOG_ERR("ERR | OI_GetShm3() < 0, key(0x%08x), size(%lu), %s", iShmKey, uShmSize, strerror(errno));
            return false;
        }
        //CHashMap<KEY, MAPPED, EXTENTION>::SetHeader(pHdr);
        //CShmHash::SetHeader(pHdr);
        if (iRet == 1)
        {
            // 表示新创建
            bIsNew = true;
            // 初始化内存
            //CHashMap<KEY, MAPPED, EXTENTION>::Clear();
            *pHdr = roHdr;
            CShmHash::m_pHeader = pHdr;
            CShmHash::Clear();
        }
        else
        {
            if (pHdr->dwHashNodeSize != roHdr.dwHashNodeSize || pHdr->dwHashHeight != roHdr.dwHashHeight || pHdr->dwHashWidth != roHdr.dwHashWidth || memcmp(&pHdr->tEmptyKey, &roHdr.tEmptyKey, sizeof (roHdr.tEmptyKey)))
            {
                return false;
            }
            bIsNew = false;
            CShmHash::m_pHeader = pHdr;
        }

        return true;
    }

    virtual bool AllocAndInitHashMemory(size_t uSize, const typename CShmHash<KEY, MAPPED, EXTENTION>::CHashHeader& roHdr)
    {
        return OpenShareMemery(m_iShmKey, uSize, roHdr, m_bIsNew);
    }

    virtual bool FreeHeaderMemory()
    {
        CStr szCmd(64);
        szCmd.Format("ipcrm -M 0x%x", m_iShmKey);
        system(szCmd);

        return true;
    }

protected:
    int m_iShmKey;
    bool m_bIsNew;
};


// CMBMonShmHash

template <class MAPPED, class EXTENTION = EMPTY>
class CMBMonShmHash : public CShmHash<S_MON_KEY, MAPPED, EXTENTION>
{
public:
    typedef typename CShmHash<S_MON_KEY, MAPPED, EXTENTION>::CHashNode CHashNode;

protected:

    virtual bool NodeMatch(const S_MON_KEY& rKey, CHashNode& rNode)
    {
        return rKey.dwId == rNode.tKey.dwId;
    }
};


// CMBMonHash

class CMBMonHash : public CMBMonShmHash<S_MON_MAPPED, S_MON_EXT>
{
public:
    //typedef CMBMonHash::CHashNode CHashNode;
    uint16_t GetCurMinPos();
    void SetCurMinPos(uint16_t wCurMinPos);

    bool GetNowSubItemVal(uint32_t dwItemId, uint8_t cTypeId, uint32_t& dwVal);
};

class CMBMonHashSpec : public CMBMonShmHash<S_MON_MAPPED_SPEC>
{
public:
    //typedef CMBMonHashSpec::CHashNode CHashNode;
};


// CMBMonitorHelper

class CMBMonitorHelper
{
protected:

    class CHBThread : public CThread
    {
    public:
    protected:
        virtual long ThreadProc();
    };

public:
    CMBMonitorHelper();

    bool Init(const char* pSvrIp, int iSvrPort, const char* pAttrDbHost, int iAttrDbPort, const char* pAttrDbUser, const char* pAttrDbPass, const char* pAttrDbChar, const char* pAttrDbName, const char* pAttrDbTable);
    bool GetAttrInfo(int iAttrId, S_ATTR_INFO& rstAttrInfo);
    bool GetValOfAttr(int iViewId, int iAttrId, int iDaysBefore, S_ATTR_VAL& rstAttrVal, int iTimeout);

    static int GetMinuteIndex(time_t uTime); // return 0~1440

protected:
    CUdpSocket m_oUdp;
    char m_szAttrDbTable[32];
    CSockAddrIn m_oSa;
    CHBThread m_oThrd;
    CMutex m_oMtx;
};




////////////////////// CDemoMonitorDriver ///////////////////////

// CQueryTimer

class CQueryTimer : public CTimerQueueNode
{
public:
    virtual bool OnTimeout();
    bool CheckNewDay(S_SUBITEM& rSubItem, S_ATTR_VAL& rAttrVals);

};

class CDemoMonitorDriver : public CThreadService
{
public:
    CDemoMonitorDriver();
    virtual ~CDemoMonitorDriver();

protected:
    virtual bool OnServiceInit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();

public:
    CEpollFramework m_oEfw;
    CMBMonHash m_oHash;
    CQueryTimer m_oQt;
    CMBMonitorHelper m_oMon;

};



////////////////////// CDemoDataStorage ///////////////////////

// CPolicyTimer

class CPolicyTimer : public CTimerQueueNode
{
public:
    virtual bool OnTimeout();

    bool CheckNewDay();
};

class CDemoDataStorage : public CThreadService
{
public:
    CDemoDataStorage();
    virtual ~CDemoDataStorage();

protected:
    virtual bool OnServiceInit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();

public:
    CEpollFramework m_oEfw;
    CMBMonHash m_oHash;
    CMBMonHashSpec m_oHashSpec;
    CPolicyTimer m_oPt;

};


////////////////////// CDemoScriptJudge ///////////////////////

// CMonLuaSe

class CMonLuaSe : public CLuaSe
{
public:
    CMonLuaSe();
    CMonLuaSe(lua_State* pL);

protected:
    int m_iItemId;
};

// CDbgLuaSe

class CDbgLuaSe : public CMonLuaSe
{
public:
    CDbgLuaSe();
    CDbgLuaSe(lua_State* pL);
    //virtual ~CDbgLuaSe();

    //static const size_t PREFIX_LEN;

protected:
    virtual void OnRunFileOk(CThread* pThrd, const char* pFile, const void* pParam);
    virtual void OnRunFileError(CThread* pThrd, const char* pFile, const CStr& roErr, const void* pParam);
    virtual void OnRunFileCancel(CThread* pThrd, const char* pFile, const void* pParam);

};


// CRunInst

class CRunInst : public CMutex
{
public:

    typedef enum
    {
        E_RUNNING = 0,
        E_OK = 1,
        E_ERR = 2,
        E_TIMEOUT = 3,
        E_CANCELED = 4,

    } E_RES;

public:
    CLuaSe* pL;
    CThread* pLt;
    CStopWatch oSw;
    long lTimeout;
    int iItemId;
    E_RES eRes;
};


// CRunInstChecker

class CRunInstChecker : public CThread
{
public:
    typedef CList<CRunInst*> CInstList;

public:
    CRunInstChecker();
    bool CreateRunInstance(CLuaSe* pL, const char* pScriptPath, int iItemId);

protected:
    virtual long ThreadProc();

protected:
    CInstList m_oLst;
    CMutex m_oMtx;
};


// CDbCheckerWm

class CDbCheckerWm : public CThread
{
public:

    enum
    {
        E_ORG = 0,
        E_WAITING = 1,
        E_OK = 2,
        E_FAILED = 3,
        E_CANCELING = 4,
        E_CANCELEDSUCC = 5,
        E_CANCELEDFAILED = 6,
        E_CHECKING = 7,
    };

protected:
    virtual long ThreadProc();

};


// CCmdLnThread

class CCmdLnThread : public CThread
{
protected:
    virtual long ThreadProc();
};


// CDemoScriptJudge

class CDemoScriptJudge : public CThreadService
{
public:
    CDemoScriptJudge();
    virtual ~CDemoScriptJudge();

protected:
    virtual bool OnServiceInit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();

public:
    CEpollFramework m_oEfw;

    CRunInstChecker m_oInstChkr;
    CDbCheckerWm m_oDbChkrWm;

};


// CDbCheckerWmSpt

class CDbCheckerWmSpt : public CThread
{
public:

    enum
    {
        E_ORG = 0,
        E_CHECKING = 1,
        E_OK = 2,
        E_FAILED = 3,
        E_CANCELING = 4,
        E_CANCELEDSUCC = 5,
        E_CANCELEDFAILED = 6,
        E_WAITING = 7,
    };

protected:
    virtual long ThreadProc();

};


// CPerLuaSe

class CPerLuaSe : public CMonLuaSe
{
public:
    CPerLuaSe();
    CPerLuaSe(lua_State* pL);

protected:
    virtual void OnRunFileOk(CThread* pThrd, const char* pFile, const void* pParam);
    virtual void OnRunFileError(CThread* pThrd, const char* pFile, const CStr& roErr, const void* pParam);
    virtual void OnRunFileCancel(CThread* pThrd, const char* pFile, const void* pParam);

};


// CDemoScriptJudge

class CDemoScriptDaemon : public CThreadService
{
public:
    CDemoScriptDaemon();
    virtual ~CDemoScriptDaemon();

protected:
    virtual bool OnServiceInit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();

public:
    CEpollFramework m_oEfw;

    CRunInstChecker m_oInstChkr;
    CDbCheckerWmSpt m_oDbChkrWmSpt;

};









}

using namespace DemoMonitorDriver;

extern CMyConfigure g_oCfg;
extern CDbMySQL g_oDbMon;
extern CDbMySQL g_oDbWm;
extern CMBMonHash g_oHash;
extern CMBMonHashSpec g_oHashSpec;
extern CDemoMonitorDriver g_oSvc;
extern CDemoDataStorage g_oSvcSpec;
extern CDemoScriptJudge g_oSvcJdg;
extern CDemoScriptDaemon g_oSvcSpt;

int main(int argc, char* const* argv);

int LuaCmdLn();

// Global Functions

DEF_LUACFUNC(GetCurTime);
DEF_LUACFUNC(GetMinuteOfDay);
DEF_LUACFUNC(GetCurWeekDayIndex);
DEF_LUACFUNC(GetCurMonMinuteIndex);

DEF_LUACFUNC(GetCountOfSubItem);
DEF_LUACFUNC(GetValOfSubItemByIndex);
DEF_LUACFUNC(GetValOfSubItemByType);
DEF_LUACFUNC(GetTypeOfSubItemByIndex);

DEF_LUACFUNC(System);
DEF_LUACFUNC(AttrAdd);
DEF_LUACFUNC(AttrSet);
DEF_LUACFUNC(AttrGet);

int LuaRegCFunc(CLuaSe& roL, CReadline* pRl = NULL);
int LuaCmdLn();

int main(int argc, char* const* argv);

typedef struct
{
    const char* pName;
    lua_CFunction pFunc;
} S_REG_LUA_CFUNC_PAIR;



#endif	/* __DEMOMONITORDRIVER_H__ */

