/* 
 * File:   TSSocket.h
 * Author: thunderliu
 *
 * Created on 2011年12月20日, 下午6:21
 */

#ifndef __TSSOCKET_H__
#define	__TSSOCKET_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

class CSockAddr
{
public:
    CSockAddr(const struct sockaddr* pstSockAddr, socklen_t dwSockLen);
    CSockAddr(const CSockAddr& roSockAddr);
    virtual ~CSockAddr();
    operator struct sockaddr*();
    operator const struct sockaddr*() const;
    void SetSockAddr(const struct sockaddr* pstSockAddr);
    CSockAddr& operator =(const CSockAddr& roSockAddr);
    socklen_t GetSockLen() const;

protected:
    struct sockaddr* m_pstSockAddr;
    socklen_t m_dwSockLen;
};

class CSockAddrIn : public CSockAddr
{
public:
    CSockAddrIn(const struct sockaddr* pstSockAddr = NULL);
    CSockAddrIn(const char* pAddr, int iPort);

    void SetSockAddrIn(const char* pAddr, int iPort);

    static void MakeSockAddrInStruct(struct sockaddr* pstSockAddr, const char* pAddr, int iPort);

    operator struct sockaddr_in*();
    operator const struct sockaddr_in*() const;

    char* GetAddr();
    int GetPort();
    static CSockAddrIn* ConvertToSockAddrIn(const struct sockaddr* pstSockAddr);
    static CSockAddrIn* ConvertToSockAddrIn(const char* pAddr, int iPort);


protected:
    static CSockAddrIn m_oConvert;

};

class CSocket : public CIo
{
public:
    CSocket(int iSock = -1);
    virtual ~CSocket();
    bool Bind(const CSockAddr* pSockAddr);
    CSockAddr* GetSockAddr();
    bool SetSockOpt(int iLevel, int iOptName, const void* pOptVal, socklen_t dwOptlen);
    bool GetSockOpt(int iLevel, int iOptName, void* pOptVal, socklen_t* pOptlen) const;
    bool SetRecvTimeout(int iTimeout);
    int GetRecvTimeout() const;
    bool SetSendBuffLen(int iSendBuffLen);
    int GetSendBuffLen();
    bool SetRecvBuffLen(int iRecvBuffLen);
    int GetRecvBuffLen();

    virtual bool Close();
    int Send(const void* pBuf, int iSize);
    int Recv(void* pBuf, int iSize);
    int SendTo(const void* pBuf, int iSize, const CSockAddr* pSockAddr);
    int RecvFrom(void* pBuf, int iSize, CSockAddr* pSockAddr);

protected:
    virtual bool CreateSocket() = 0;


protected:
    CSockAddr* m_pSockAddr;

};

class CIpSocket : public CSocket
{
public:
    CIpSocket(int iSock = -1);
    virtual ~CIpSocket();
    CSockAddrIn* GetSockAddrIn();

    bool Bind(const char* pAddr, int iPort, bool bReuseAddr = false);

};

class CUdpSocket : public CIpSocket
{
public:
    CUdpSocket(int iSock = -1);

    bool EnableBroadcast(bool bEnable = true);

protected:
    virtual bool CreateSocket();

};

class CTcpSocket : public CIpSocket
{
public:
    CTcpSocket(int iSock = -1);

    bool Connect(const CSockAddr* pSockAddr);
    bool Connect(const char* pAddr, int iPort);
    bool Listen(int iBacklog = 5);
    bool Accept(CTcpSocket* pStreamSock);
    bool SendEx(const void* pBuf, int iSize);
    bool RecvEx(void* pBuf, int iSize);

protected:
    virtual bool CreateSocket();
};


#ifdef TSNETFW_FEATURE_PACKET
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <linux/if_arp.h>

// Ethernet Socket

#pragma pack(push, 1)

struct etharphdr
{
    __be16 ar_hrd; /* format of hardware address	*/
    __be16 ar_pro; /* format of protocol address	*/
    unsigned char ar_hln; /* length of hardware address	*/
    unsigned char ar_pln; /* length of protocol address	*/
    __be16 ar_op; /* ARP opcode (command)		*/
    unsigned char ar_sha[ETH_ALEN]; /* sender hardware address	*/
    unsigned char ar_sip[4]; /* sender IP address		*/
    unsigned char ar_tha[ETH_ALEN]; /* target hardware address	*/
    unsigned char ar_tip[4]; /* target IP address		*/
};
//#pragma pack(pop)
#pragma pack()

class CSockAddrLl : public CSockAddr
{
public:
    CSockAddrLl(const struct sockaddr* pstSockAddr = NULL);
    CSockAddrLl(const char* pInterface, const char* pHardAddr);

    void SetSockAddrLl(const char* pInterface, const char* pHardAddr);

    static void MakeSockAddrLlStruct(struct sockaddr* pstSockAddr, const char* pInterface, const char* pHardAddr);

    operator struct sockaddr_ll*();
    operator const struct sockaddr_ll*() const;

    char* GetAddr();
    //int GetPort();

    static CSockAddrLl* ConvertToSockAddrLl(const struct sockaddr* pstSockAddr);
    static CSockAddrLl* ConvertToSockAddrLl(const char* pInterface, const char* pHardAddr);

protected:
    static CSockAddrLl m_oConvert;
};

#define ETH_HADDR_BROADCAST     ((uint8_t*)"\xFF\xFF\xFF\xFF\xFF\xFF")
#define ETH_HADDR_UNKNOWN       ((uint8_t*)"\x00\x00\x00\x00\x00\x00")
//INADDR_ANY INADDR_BROADCAST

class CEthSocket : public CSocket
{
public:
    CEthSocket(int iSock = -1);
    virtual ~CEthSocket();
    bool Bind(const char* pInterface);
    bool EnablePromisc(const char* pInterface, bool bEnable = true);

    static void MakeEthernetHeader(struct ethhdr* pEthHdr, const uint8_t* pDstMac, const uint8_t* pSrcMac, uint16_t wType/* ETH_P_IP or ETH_P_ARP etc.*/);
    static void MakeArpHeader(struct etharphdr* pArpHdr, const uint8_t* pSndrMac, in_addr_t dwSndrIp, const uint8_t* pTrgtMac, in_addr_t dwTrgtIp, uint16_t wOpCode/* ARPOP_REQUEST or ARPOP_REPLY */);

protected:
    virtual bool CreateSocket();
};
#endif // TSNETFW_FEATURE_PACKET


#ifndef ___constant_swab64
#define ___constant_swab64(x) \
    ((uint64_t)( \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0x00000000000000ffULL) << 56) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0x000000000000ff00ULL) << 40) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0x0000000000ff0000ULL) << 24) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0x00000000ff000000ULL) <<  8) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0x000000ff00000000ULL) >>  8) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0x0000ff0000000000ULL) >> 24) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0x00ff000000000000ULL) >> 40) | \
    (uint64_t)(((uint64_t)(x) & (uint64_t)0xff00000000000000ULL) >> 56) ))
#endif // ___constant_swab64

inline uint64_t htonl64(uint64_t host)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return ___constant_swab64(host);
#else
    return host;
#endif
}

inline uint64_t ntohl64(uint64_t net)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return ___constant_swab64(net);
#else
    return host;
#endif    
}


#include "TSSocket.inl"

#endif	/* __TSSOCKET_H__ */

