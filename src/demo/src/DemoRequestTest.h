/* 
 * File:   DemoRequestTest.h
 * Author: thunderliu
 *
 * Created on 2011年12月29日, 下午12:16
 */

#ifndef __DEMOTCPREQTEST_H__
#define	__DEMOTCPREQTEST_H__

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

class CTestThread : public CThread
{
public:
    bool Init(FILE* pFile);

protected:
    virtual long ThreadProc();
    long TcpProc();
    long UdpProc();
    FILE* m_pFile;
};

class CDemoRequestTest : public CThread
{
public:
    CDemoRequestTest();
    virtual ~CDemoRequestTest();

    long ThreadProc();


public:
    CEpollFramework m_oEpfw;

    char m_szProto[64];
    char m_szSvrIp[64];
    int m_iSvrPort;
    int m_iCount;
    char m_szPktBuf[4096];
    int m_iPktLen;

};

extern CDemoRequestTest g_oSvc;

#endif	/* __DEMOTCPREQTEST_H__ */

