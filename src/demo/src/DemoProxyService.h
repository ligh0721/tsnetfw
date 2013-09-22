/* 
 * File:   DemoProxyService.h
 * Author: thunderliu
 *
 * Created on 2011年12月24日, 下午5:43
 */

#ifndef __DEMOPROXYSERVICE_H__
#define	__DEMOPROXYSERVICE_H__


#include <vector>
using namespace std;

namespace DemoProxyService
{


#pragma pack(push)
#pragma pack(1)

typedef struct
{
    uint8_t cFlag;
    uint16_t wLength;
} S_PKG_H;

typedef struct
{
    S_PKG_H stPktH;
    uint16_t wVersion;
    uint16_t wCommand;
    uint32_t dwSequence;
    uint32_t dwUin;
    uint32_t dwUserIp;
    uint32_t dwReserved;
} S_REQ_H; // 23 Bytes

typedef struct
{
    S_PKG_H stPktH;
    uint16_t wVersion;
    uint16_t wCommand;
    uint32_t dwSequence;
    uint32_t dwUin;
    uint32_t dwReserved;
    uint8_t cResult;
} S_RSP_H; // 20 Bytes

typedef struct
{
    S_PKG_H stPktH;
    uint16_t wVersion;
    uint16_t wCommand;
    uint32_t dwSequence;
    uint32_t dwUin;
    uint32_t dwReserved;
    uint8_t cResult;
    uint8_t cEtxFlag;
} S_RSP_NOBODY; // 21 0x15

typedef struct
{
    uint32_t dwFromUin;
    uint32_t dwToUin;
    uint32_t dwTipType;
} S_REQ_B;


#pragma pack(pop)

typedef enum
{
    E_SUCCESS = 0,
    E_TIMEOUT = 0x67,
    E_FAILD = 0x66,
    E_BUSY = 0x65,
    E_PKGERR = 0x64,
    E_FREQUENT = 0x63,
    E_LOW = 0x62
} E_RESULT;


class CMyConnection;

typedef struct
{
    S_REQ_H stReqH;
    CMyConnection* pSock;
} SESSION;

class CMyConnection : public CProtocolStream
{
public:
    bool SendNobody(S_REQ_H* pReqH, E_RESULT eResult);

protected:
    virtual bool OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen);
    virtual bool OnReadPacket(CBufferHelper& roPkt);

};

class CServerConnection : public CProtocolStream
{
protected:
    virtual bool OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen);
    virtual bool OnReadPacket(CBufferHelper& roPkt);
};

class CSessionKeeper : public CHashSessionKeeper<SESSION>
{
protected:
    virtual bool OnSessionTimeout(SESSION& rSession);
};

typedef struct
{
    char szIp[128];
    int iPortSet;
    int iPortGet;
} S_GROUPADDR;

class CMyConfigure : public CConfigureFile
{
public:

    CMyConfigure() : m_szHostIp(32)
    {
    }

public:
    CStr m_szHostIp;
    int m_iHostPort;

    uint16_t m_wCmdSet;
    uint16_t m_wCmdGet;

    CTime m_oTimeout;

    uint32_t m_uGroupCount;
    vector<S_GROUPADDR> m_vecGroupAddr;

};

class CDemoProxyService : public CThreadService
{
public:
    CDemoProxyService();
    virtual ~CDemoProxyService();

protected:
    virtual bool OnLoadConfigure();
    virtual bool OnServiceInit();
    virtual bool OnEnterLoop();

public:
    CMyConfigure m_oCfg;
    CEpollFramework m_oEfw;

    CAcceptorSocket<CMyConnection> m_oLstn;
    CServerConnection* m_pSvrsSet;
    CServerConnection* m_pSvrsGet;

    CSessionKeeper m_oSk;

};

class CDemoProxyServiceMP : public CMultiProcessService
{
public:
    CDemoProxyServiceMP();
    virtual ~CDemoProxyServiceMP();

    virtual bool OnLoadConfigure();
    virtual bool OnServiceInit();
    virtual bool OnEnterLoop();

public:
    CMyConfigure m_oCfg;
    CEpollFramework m_oEfw;

    CAcceptorSocket<CMyConnection> m_oLstn;
    CServerConnection* m_pSvrsSet;
    CServerConnection* m_pSvrsGet;

    CSessionKeeper m_oSk;

};


}

using namespace DemoProxyService;

#ifndef __MP__
extern CDemoProxyService g_oSvc;
#else
extern CDemoProxyServiceMP g_oSvc;
#endif





#endif	/* __DEMOPROXYSERVICE_H__ */

