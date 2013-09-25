/* 
 * File:   TSNetService.h
 * Author: thunderliu
 *
 * Created on 2011年12月11日, 下午6:47
 */

#ifndef __TSNETSERVICE_H__
#define	__TSNETSERVICE_H__

#include "TSDebug.h"

class CAutoSwitch
{
public:
    CAutoSwitch();
    bool Init(uint32_t dwMaxFailedCount, uint32_t dwMaxTimeoutCount, uint32_t dwFirstDelayMS, uint32_t dwDelayIncStepMS, uint32_t dwMaxDelayMS);
    bool IsAvailable();
    void TellFailed();
    void TellTimeout();
    void Reset();

protected:
    bool m_bIsAvailable;
    uint32_t m_dwMaxFailedCount;
    uint32_t m_dwMaxTimeoutCount;
    uint32_t m_dwFirstDelayUS;
    uint32_t m_dwDelayIncStepUS;
    uint32_t m_dwMaxDelayUS;
    uint32_t m_dwFailedCount;
    uint32_t m_dwTimeoutCount;
    uint32_t m_dwDelayUS;
    CStopWatch m_oSw;
};

class CConnectionStream : public CTcpSocket
{
public:
    CConnectionStream();
    virtual ~CConnectionStream();

    bool Init(CEpollFramework* pEpFw, int iReserved = 0);
    bool IsInUse() const;

    time_t GetLastAccessTime() const;
    void SetLastAccessTime(time_t uLastAccessTime);

protected:
    virtual bool OnInit();
    virtual bool OnClose();

protected:
    time_t m_uLastAccessTime;
};

class CProtocolStream : public CConnectionStream
{
public:
    CProtocolStream();
    
    void ResetRecvInfo();

    bool Init(CEpollFramework* pEpFw, int iPktHdrLen);

protected:
    virtual bool OnRead();
    virtual bool OnInit();
    virtual bool OnClose();
    //virtual void OnSetPacketHeaderLength(int& iPktHdrLen) = 0;
    virtual bool OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen) = 0;
    virtual bool OnReadPacket(CBufferHelper& roPkt) = 0;

protected:
    bool m_bToRcvPktHdr;
    int m_iPktHdrLen;
    int m_iRcvBuffPos; // 包体已接收的长度
    int m_iRcvBuffLen; // 包体总长度
    uint8_t m_acRcvBuff[0x100000];
};

template <typename CONNECTION>
class CSockHash : public CHashMap<int, EMPTY>
{
public:
    //typedef CHashMap<uint32_t, MAPPED> CConnHash;
    typedef typename CHashMap<int, EMPTY>::CHashNode CHashNode;
    CSockHash();
    void SetConnsAddr(CONNECTION* pConns, int iMaxConnectionCount);

    size_t GetNodeInUsed();
    bool CalcNodeInUsed(CHashNode& rNode, void* pParam);

protected:
    virtual bool NodeMatch(const int& rKey, CHashNode& rNode);

protected:
    CONNECTION* m_pConns;
    int m_iMaxConnectionCount;
    size_t m_uCount;
};

template <typename CONNECTION>
class CAcceptorSocket : public CTcpSocket
{
protected:
    typedef CSockHash<CONNECTION> CConnHash;

public:
    CAcceptorSocket();
    bool Init(CEpollFramework* pEpFw, int iMaxConnectionCount, int iConnctionPktHdrLen, char* pAddr, int iPort, bool bReuseAddr = true);

protected:
    virtual bool OnRead();
    virtual bool OnClose();

protected:
    //CONNECTION* m_pConns;
    CConnHash m_oConnHash;
    int m_iMaxConnCount;
    CONNECTION m_oTmpConn;
    CONNECTION* m_pConns;

    int m_iPktHdrLen;
};

class CRecordBlock : public CBlock16
{
public:
    static CRecordBlock* CreateBlock(uint16_t wSize, time_t uTime);
    time_t GetTime() const;

protected:
    CRecordBlock(uint16_t wSize, void* pBuf, time_t uTime);

protected:
    time_t m_uTime;
};

class CRecordWorker : public CWorker
{
public:
    bool Init(const char* pPath, size_t uBlockCount, bool bTextFormat);

protected:
    bool UpdateFileHandle(time_t tmTime = 0);

    virtual bool OnStart();
    virtual bool OnWork(CBlock16* pBlock);
    virtual bool OnWork(CRecordBlock* pBlock);
    virtual bool OnEmpty();
    virtual bool OnStop();

protected:
    FILE* m_pFile;
    uint32_t m_dwDate;
    void SetDate(uint32_t Date);
    uint32_t GetDate() const;
    char m_szPath[1024];
    bool m_bTextFormat;

};

class CServiceBase
{
public:
    virtual ~CServiceBase();

    virtual bool StartService() = 0;
    virtual bool WaitService() = 0;

    static bool IgnoreSignals();

protected:
    virtual bool OnServiceInit();
    virtual bool OnServiceExit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();
};

class CThreadService : public CServiceBase, public CThread
{
public:
    virtual bool StartService();
    virtual bool WaitService();

protected:
    virtual long ThreadProc();

};

class CProcessService : public CServiceBase, public CProcess
{
public:
    virtual bool StartService();
    virtual bool WaitService();

protected:
    virtual int ProcessProc();

};

class CMultiProcessService : public CProcessService
{
public:
    CMultiProcessService();

    virtual bool StartService();
    virtual bool WaitService();
    void SetProcessCount(int iCount);

protected:
    virtual int ProcessProc();

protected:
    int m_iCount;
};

class CProgressValue
{
public:
    CProgressValue(uint64_t ddwMaxValue = 0);
    
    void Init(uint64_t ddwMaxValue = 0);
    uint32_t GetCurPercent() const;
    uint64_t GetSpeedPerSec();
    bool SetPosition(uint64_t ddwValue);
    bool IsEnd() const;
    
protected:
    uint64_t m_ddwPos;
    uint64_t m_ddwMax;
    uint32_t m_dwCur;
    time_t m_uOldTime;
    uint64_t m_ddwOldPos;
};







#include "TSNetService.inl"

#endif	/* __TSNETSERVICE_H__ */

