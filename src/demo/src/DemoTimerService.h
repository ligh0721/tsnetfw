/* 
 * File:   DemoTimerService.h
 * Author: thunderliu
 *
 * Created on 2012年4月27日, 下午4:48
 */

#ifndef __DEMOTIMERSERVICE_H__
#define	__DEMOTIMERSERVICE_H__


namespace DemoTimerService
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

#pragma pack(pop)

typedef enum
{
    E_AT_CUMULANT = 0,
    E_AT_BPS = 1,
    E_AT_MOMENT = 2
} E_ATTR_TYPE;

typedef struct
{
    uint32_t uAttrId;
    char szAttrName[60];
    E_ATTR_TYPE eAttrType;
} S_ATTR_INFO;

typedef struct
{
    uint32_t adwVal[1440];
} S_ATTR_VAL;

typedef struct
{
    uint32_t dwRetVal;
    uint32_t dwRetTimes;
} S_RET;

typedef struct
{
    uint32_t dwRetDelay;
    uint32_t dwCallTimes;
    S_RET astRetTimes[10];
} S_DATA_PER_MIN_MON;

typedef struct
{
    uint32_t dwId;
} S_MON_KEY;

typedef struct
{
    S_DATA_PER_MIN_MON astData[1440];
} S_MON_MAPPED;

typedef struct
{
    S_DATA_PER_MIN_MON astSpecData[7][1440];
} S_MON_SPEC_MAPPED;

template <class KEY, class MAPPED, class EXTENTION = EMPTY>
class CShmHash : public CHashMap<KEY, MAPPED, EXTENTION>
{
public:
    typedef class CHashMap<KEY, MAPPED, EXTENTION>::CHashHeader CHashHeader;

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

    bool OpenShareMemery(int iShmKey, size_t uShmSize, bool& bIsNew)
    {
        void* pHdr = NULL;
        int iRet = OI_GetShm3(&pHdr, iShmKey, uShmSize, 0666 | IPC_CREAT);
        if (iRet < 0)
        {
            // 错误
            LOG_ERR("ERR | OI_GetShm3() < 0, %s", strerror(errno));
            return false;
        }
        CHashMap<KEY, MAPPED, EXTENTION>::SetHeaderAddress(pHdr);
        if (iRet == 1)
        {
            // 表示新创建
            bIsNew = true;
            // 初始化内存
            CHashMap<KEY, MAPPED, EXTENTION>::Clear();
        }
        else
        {
            bIsNew = false;
        }

        return true;
    }

    virtual bool AllocHeaderMemory(size_t uSize)
    {
        return OpenShareMemery(m_iShmKey, uSize, m_bIsNew);
    }

    virtual bool FreeHeaderMemory()
    {
        char szCmd[64];
        snprintf(szCmd, sizeof (szCmd), "ipcrm -M %d", m_iShmKey);
        system(szCmd);

        return true;
    }

protected:
    int m_iShmKey;
    bool m_bIsNew;
};

template <class MAPPED, class EXTENTION = EMPTY>
class CMBMonShmHash : public CShmHash<S_MON_KEY, MAPPED, EXTENTION>
{
public:
    typedef class CShmHash<S_MON_KEY, MAPPED, EXTENTION>::CHashNode CHashNode;

protected:

    virtual bool NodeMatch(const S_MON_KEY& rKey, CHashNode& rNode)
    {
        return rKey.dwId == rNode.tKey.dwId;
    }
};

class CMBMonHash : public CMBMonShmHash<S_MON_MAPPED>
{
};

class CMBMonSpecHash : public CMBMonShmHash<S_MON_SPEC_MAPPED>
{
};

class CMBMonitorHelper
{
public:
    CMBMonitorHelper();

    bool Init(const char* pSvrIp, int iSvrPort, const char* pAttrDbHost, int iAttrDbPort, const char* pAttrDbUser, const char* pAttrDbPass, const char* pAttrDbChar, const char* pAttrDbName, const char* pAttrDbTable);
    bool GetAttrInfo(int iAttrId, S_ATTR_INFO& rstAttrInfo);
    bool GetValOfAttr(int iViewId, int iAttrId, int iDaysBefore, S_ATTR_VAL& rstAttrVal);

    static int GetMinuteIndex(time_t uTime); // return 0~1440

protected:
    CUdpSocket m_oUdp;
    CDbMySQL m_oDb;
    char m_szAttrDbTable[32];
    CSockAddrIn m_oSa;
};

class CQueryTimer : public CTimerQueueNode
{
public:
    virtual bool OnTimeout();
    int m_iC;
    time_t m_tm;

};

class CMyConfigure : public CConfigureFile
{
public:

    CMyConfigure() : m_szHostIp(32), m_szSvrIp(32), m_szAttrDbHost(32), m_szAttrDbUser(32), m_szAttrDbPass(32), m_szAttrDbName(32), m_szAttrDbChar(32), m_szAttrDbTable(32)
    {
    }
    virtual bool OnLoadConfigure();

public:
    CStr m_szHostIp;
    int m_iHostPort;

    CStr m_szSvrIp;
    int m_iSvrPort;

    int m_iHashShmKey;
    uint32_t m_dwHashWidth;
    uint32_t m_dwHashHeight;

    int m_iHashSpecShmKey;
    uint32_t m_dwHashSpecWidth;
    uint32_t m_dwHashSpecHeight;

    CStr m_szAttrDbHost;
    int m_iAttrDbPort;
    CStr m_szAttrDbUser;
    CStr m_szAttrDbPass;
    CStr m_szAttrDbName;
    CStr m_szAttrDbChar;
    CStr m_szAttrDbTable;

};

class CDemoTimerService : public CThreadService
{
public:
    CDemoTimerService();
    virtual ~CDemoTimerService();

protected:
    virtual bool OnServiceInit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();
    //virtual long ThreadProc();

public:
    CMyConfigure m_oCfg;
    CEpollFramework m_oEfw;
    CMBMonHash m_oHash;
    CMBMonSpecHash m_oSpecHash;
    CQueryTimer m_oQt;
    CMBMonitorHelper m_oMon;

};










}

using namespace DemoTimerService;

extern CDemoTimerService g_oSvc;



#endif	/* __DEMOTIMERSERVICE_H__ */

