/* 
 * File:   DemoRecordService.h
 * Author: thunderliu
 *
 * Created on 2011年12月24日, 下午4:48
 */

#ifndef __DEMORECORDSERVICE_H__
#define	__DEMORECORDSERVICE_H__


namespace DemoRecordService
{

#pragma pack(push, 1)

typedef struct
{
    uint16_t wLen;
    uint16_t wCmd;
} S_HDR;

#pragma pack(pop)



typedef char REC_LOG[160];

class CClientConnection : public CProtocolStream
{
protected:
    virtual bool OnReadPacketHeader(CBufferHelper& roPkt, int& iTotalPktLen);
    virtual bool OnReadPacket(CBufferHelper& roPkt);
    bool AddRecord(uint32_t dwTime);
};

class CMyConfigure : public CConfigureFile
{
public:

    CMyConfigure() : m_szHostIp(32), m_szRecPath(1024)
    {
    }

public:
    CStr m_szHostIp;
    int m_iHostPort;

    CStr m_szRecPath;
    int m_iRecThreadCount;
    int m_iRecQueueLength;

    int m_iSvcCount;
    int m_iSvcIndex;
};

class CDemoRecordService : public CThreadService
{
public:
    CDemoRecordService();
    virtual ~CDemoRecordService();

protected:
    virtual bool OnServiceInit();
    virtual bool OnLoadConfigure();
    virtual bool OnEnterLoop();
    //virtual long ThreadProc();

public:
    CMyConfigure m_oCfg;
    CEpollFramework m_oEfw;

    CRecordWorker* m_pRec;

    CAcceptorSocket<CClientConnection> m_oLstn;

};










}

using namespace DemoRecordService;

extern CDemoRecordService g_oSvc;



#endif	/* __DEMORECORDSERVICE_H__ */

