/* 
 * File:   DemoUdpDispenser.h
 * Author: thunderliu
 *
 * Created on 2012年7月19日, 下午7:06
 */

#ifndef __DEMOUDPDISPENSER_H__
#define	__DEMOUDPDISPENSER_H__


#define LISTEN_HOST             "10.169.130.240"
#define LISTEN_PORT             0
#define REMOTE_PORT_0x0702      27701
#define REMOTE_PORT_0x0902      27901





#pragma pack(push, 1)

typedef struct
{
    uint8_t cFlag;
    uint16_t wLength;
} S_PKG_H;

typedef struct
{

    union
    {
        S_PKG_H stPktH;

        struct
        {
            uint8_t cFlag;
            uint16_t wLength;
        };
    };
    uint16_t wVersion;
    uint16_t wCommand;
    uint32_t dwSequence;
    uint32_t dwUin;
    uint32_t dwUserIp;
    uint32_t dwReserved;
} S_REQ_H;

typedef struct
{
    uint16_t wBufLen;
    uint8_t acBuf[0];
} S_WBUF;

typedef struct
{

    union
    {
        S_PKG_H stPktH;

        struct
        {
            uint8_t cFlag;
            uint16_t wLength;
        };
    };
    uint16_t wVersion;
    uint16_t wCommand;
    uint32_t dwSequence;
    uint32_t dwUin;
    uint32_t dwReserved;
    uint8_t cResult;
} S_RSP_H;

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
    uint8_t cSvrFlag;
    uint16_t wAgentVersion;
    uint8_t acReserved[14];
} S_REQ_B_0x0702; // CtrlAgent GetInfo

typedef struct
{
    uint8_t cSvrFlag;
    uint32_t dwCtrlSeq;
    uint32_t dwCtrlSeqMod;
    uint16_t wCtrlLvl;
    uint16_t wAgentVersion;
    uint8_t acReserved[14];
} S_RSP_B_0x0702; // CtrlAgent GetInfo

typedef struct
{
    uint8_t acReserved[16];
} S_REQ_B_0x0902; // CfgAgent GetInfo

typedef struct
{
    uint16_t wAgentVersion;
    uint8_t acReserved[16];
} S_RSP_B_0x0902; // CfgAgent GetInfo


#pragma pack(pop)

class CMyUdpSock : public CUdpSocket
{
public:
    virtual bool OnRead();

    int SendTo0x0702(char* pIp, int iPort);
    int SendTo0x0902(char* pIp, int iPort);

};

typedef struct
{
    S_REQ_H stReqH;
    struct sockaddr_in stSa;
} SESSION;

class CSessionKeeper : public CHashSessionKeeper<SESSION>
{
protected:
    virtual bool OnSessionTimeout(SESSION& rSession);
};


extern CDbMySQL g_oCtrlDb;
extern CDbMySQL g_oCfgDb;
extern CEpollFramework g_oEpfw;
extern CMyUdpSock g_oUdp;
extern CSessionKeeper g_oSk;
extern uint16_t g_wCtrlAgentOkCount;
extern uint16_t g_wCtrlAgentTimeoutCount;
extern uint16_t g_wCtrlAgentCount;
extern uint16_t g_wCtrlAgentVersion;
extern bool g_bCtrlAgentCheckingFinished;
extern uint16_t g_wCfgAgentOkCount;
extern uint16_t g_wCfgAgentTimeoutCount;
extern uint16_t g_wCfgAgentCount;
extern uint16_t g_wCfgAgentVersion;
extern bool g_bCfgAgentCheckingFinished;


#endif	/* __DEMOUDPDISPENSER_H__ */

