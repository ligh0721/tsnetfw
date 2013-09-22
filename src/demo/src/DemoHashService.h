/* 
 * File:   DemoHashService.h
 * Author: thunderliu
 *
 * Created on 2012年3月22日, 下午6:03
 */

#ifndef __DEMOHASHSERVICE_H__
#define	__DEMOHASHSERVICE_H__




namespace DemoHashService
{

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
    uint32_t dwUin;
    uint32_t dwTime;
} S_REQ_B_SET;

typedef struct
{
    char _unused[0];
} S_RSP_B_SET;

typedef struct
{
    uint32_t dwUin;
    uint32_t dwMSec;
} S_REQ_B_GET;

typedef struct
{
    uint32_t dwCount;
} S_RSP_B_GET;

#pragma pack(pop)

class CClientConnection : public CProtocolStream
{
public:
    void Req2RspHdr(S_REQ_H* pReqH, S_RSP_H* pRspH, uint16_t wRspLen, E_RESULT eResult);
    bool SendNobody(S_REQ_H* pReqH, E_RESULT eResult);

protected:
    virtual bool OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen);
    virtual bool OnReadPacket(CBufferHelper& roPkt);

};

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

    int m_iHashShmKey;
    uint32_t m_dwHashWidth;
    uint32_t m_dwHashHeight;

};

typedef struct
{
    uint32_t dwCount;
} S_MAPPED;

class CDemoHashService : public CThreadService
{
public:
    CDemoHashService();
    virtual ~CDemoHashService();

protected:
    virtual bool OnServiceInit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();
    //virtual long ThreadProc();

public:
    CMyConfigure m_oCfg;
    CEpollFramework m_oEfw;

    CAcceptorSocket<CClientConnection> m_oLstn;

    CSimple32Hash<S_MAPPED> m_oHash;

};










}

using namespace DemoHashService;

extern CDemoHashService g_oSvc;



#endif	/* __DEMOHASHSERVICE_H__ */

