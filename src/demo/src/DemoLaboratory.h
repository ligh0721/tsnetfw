/* 
 * File:   DemoLaboratory.h
 * Author: thunderliu
 *
 * Created on 2011年12月31日, 下午2:52
 */

#ifndef __DEMOLABORATORY_H__
#define	__DEMOLABORATORY_H__


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


#pragma pack(pop)

typedef enum
{
    E_SUCCESS = 0,
    E_TIMEOUT = 0x67,
    E_FAILED = 0x66,
    E_BUSY = 0x65,
    E_PKGERR = 0x64,
    E_FREQUENT = 0x63,
    E_LOW = 0x62
} E_RESULT;

class CMyConnection : public CProtocolStream
{
public:
    bool SendNobody(S_REQ_H* pReqH, E_RESULT eResult);

protected:
    virtual bool OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen);
    virtual bool OnReadPacket(CBufferHelper& roPkt);

};




#endif	/* __DEMOLABORATORY_H__ */

