/* 
 * File:   TSNetService.inl
 * Author: thunderliu
 *
 * Created on 2011年12月11日, 下午6:48
 */

#ifndef __TSNETSERVICE_INL__
#define	__TSNETSERVICE_INL__

#include "TSNetService.h"
#include "TSHash.h"
#include "TSDebug.h"
#include "TSProcess.h"
#include "TSWorker.h"


// CAutoSwitch

inline CAutoSwitch::CAutoSwitch()
: m_bIsAvailable(true)
, m_dwMaxFailedCount(0)
, m_dwMaxTimeoutCount(0)
, m_dwFirstDelayUS(0)
, m_dwDelayIncStepUS(0)
, m_dwMaxDelayUS(0)
, m_dwFailedCount(0)
, m_dwTimeoutCount(0)
, m_dwDelayUS(0)
{
}

inline void CAutoSwitch::Reset()
{
    m_bIsAvailable = true;
    m_dwFailedCount = 0;
    m_dwTimeoutCount = 0;
    m_dwDelayUS = 0;
}


// CConnectionSocket

inline CConnectionStream::CConnectionStream()
: m_uLastAccessTime(0)
{
}

inline CConnectionStream::~CConnectionStream()
{
}

inline bool CConnectionStream::IsInUse() const
{
    return m_iFd >= 0;
}

inline time_t CConnectionStream::GetLastAccessTime() const
{
    return m_uLastAccessTime;
}

inline void CConnectionStream::SetLastAccessTime(time_t uLastAccessTime)
{
    m_uLastAccessTime = uLastAccessTime;
}

inline bool CConnectionStream::OnInit()
{
    return true;
}

inline bool CConnectionStream::OnClose()
{
    if (m_pEpFw)
    {
        m_pEpFw->UnregisterIo(this);
        m_pEpFw = NULL;
    }
    return true;
}


// CProtocolStream

inline CProtocolStream::CProtocolStream()
: m_bToRcvPktHdr(0)
, m_iPktHdrLen(0)
, m_iRcvBuffPos(0)
, m_iRcvBuffLen(0)
{

}

inline bool CProtocolStream::OnInit()
{
    ResetRecvInfo();
    return true;
}

inline bool CProtocolStream::OnClose()
{
    return CConnectionStream::OnClose();
}

inline void CProtocolStream::ResetRecvInfo()
{
    m_bToRcvPktHdr = true;
    m_iRcvBuffPos = 0;
    //OnSetPacketHeaderLength(m_iRcvBuffLen); // 包体总长度设为包头大小
    m_iRcvBuffLen = m_iPktHdrLen;
}

inline bool CProtocolStream::Init(CEpollFramework* pEpFw, int iPktHdrLen)
{
    m_iPktHdrLen = iPktHdrLen;
    return CConnectionStream::Init(pEpFw);
}

// CSockHash

template <typename CONNECTION>
inline CSockHash<CONNECTION>::CSockHash()
: m_pConns(NULL)
, m_iMaxConnectionCount(0)
{
}

template <typename CONNECTION>
inline void CSockHash<CONNECTION>::SetConnsAddr(CONNECTION* pConns, int iMaxConnectionCount)
{
    m_pConns = pConns;
    m_iMaxConnectionCount = iMaxConnectionCount;
}

template <typename CONNECTION>
inline size_t CSockHash<CONNECTION>::GetNodeInUsed()
{
    m_uCount = 0;
    Traverse((TRAVERSECALLBACKFUNC) & CSockHash::CalcNodeInUsed, NULL);
    return m_uCount;
}

template <typename CONNECTION>
inline bool CSockHash<CONNECTION>::CalcNodeInUsed(CHashNode& rNode, void* pParam)
{
    rNode.tKey != HASH_EMPTY_KEY && m_pConns[GetNodeIndex(&rNode)].IsInUse() && (++m_uCount);
    return true;
}

template <typename CONNECTION>
bool CSockHash<CONNECTION>::NodeMatch(const int& rKey, CHashNode& rNode)
{
    if (GetNodeIndex(&rNode) >= (size_t)m_iMaxConnectionCount)
    {
        // 超过最大连接数限制
        return false;
    }

    if (rNode.tKey != CHashMap<int, EMPTY>::HASH_EMPTY_KEY)
    {
        // 当前欲匹配节点不是空节点
        if (!m_pConns[GetNodeIndex(&rNode)].IsInUse())
        {
            // 当前节点标记为删除，或已超过生存期
            ClearNode(rNode);
            //m_pHash->uHashNodeUsedCount--;
        }
    }
    return rKey == rNode.tKey;
}


// CAcceptorSocket

template<typename CONNECTION>
inline CAcceptorSocket<CONNECTION>::CAcceptorSocket()
: m_iMaxConnCount(0)
, m_pConns(NULL)
{
}

// CAcceptorSocket

template<typename CONNECTION>
bool CAcceptorSocket<CONNECTION>::Init(CEpollFramework* pEpFw, int iMaxConnectionCount, int iConnctionPktHdrLen, char* pAddr, int iPort, bool bReuseAddr)
{
    if (m_oConnHash.GetHeader() || iMaxConnectionCount <= 0)
    {
        return false;
    }

    bool bRet = Bind(pAddr, iPort, bReuseAddr);
    if (!bRet)
    {
        return false;
    }

    iMaxConnectionCount *= 2;
    static const int CONST_MAX_HASH_HEIGHT = 5;
    if (!m_oConnHash.Init(iMaxConnectionCount * 2 / CONST_MAX_HASH_HEIGHT, CONST_MAX_HASH_HEIGHT, -1))
    {
        Close();
        LOG_ERR("ERR | conn hash init err");
        return false;
    }

    iMaxConnectionCount = (int)m_oConnHash.GetHeader()->uHashNodeCount;
    LOG_ERR("DBG | connections malloc memory size(%lu * %d = %lu)", sizeof(CONNECTION), iMaxConnectionCount, sizeof(CONNECTION) * iMaxConnectionCount);
    m_pConns = new CONNECTION[iMaxConnectionCount];
    m_oConnHash.SetConnsAddr(m_pConns, iMaxConnectionCount);

    SetFrameworkHandle(pEpFw);
    bRet = m_pEpFw->RegisterIo(this, CEpollEvent::EV_IN);
    if (!bRet)
    {
        Close();
        delete[] m_pConns;
        m_oConnHash.Release();
        m_pEpFw = NULL;
        return false;
    }

    m_iMaxConnCount = iMaxConnectionCount;
    m_iPktHdrLen = iConnctionPktHdrLen;

    return true;
}

template<typename CONNECTION>
bool CAcceptorSocket<CONNECTION>::OnRead()
{
    bool bRet = Accept(&m_oTmpConn);
    UNLIKELY_RET (!bRet, true, "ERR | Accept(%s:%d) err(%s)", m_oTmpConn.GetSockAddrIn()->GetAddr(), m_oTmpConn.GetSockAddrIn()->GetPort(), strerror(errno));
    //LOG_ERR("DBG | Accept %s:%d\n", m_oTmpConn.GetSockAddrIn()->GetAddr(), m_oTmpConn.GetSockAddrIn()->GetPort());

    //LOG_DBG("MSG | ConnHashNode: %lu/%lu/%lu, %u/%u, %d", m_oConnHash.GetNodeInUsed(), m_oConnHash.GetHeader()->uHashNodeUsedCount, m_oConnHash.GetHeader()->uHashNodeCount, m_oConnHash.GetHeader()->dwHashCurHeight, m_oConnHash.GetHeader()->dwHashHeight, m_oTmpConn.GetHandle());
    bool bNew;
    typename CConnHash::CHashNode* pNode = m_oConnHash.FindNodeToSet(m_oTmpConn.GetHandle(), &bNew);
    if (!pNode || IS_INVALID_NODE(pNode) || !bNew)
    {
        // Hash err full? but need ret true
        m_oTmpConn.Close(); // Close TmpConn
        LOG_ERR("ERR | conn hash err or full");
        return true;
    }

    //LOG_ERR("DBG | conn index(%lu)", m_oConnHash.GetNodeIndex(pNode));
    CONNECTION* pConn = m_pConns + m_oConnHash.GetNodeIndex(pNode);
    *pConn = m_oTmpConn;
    pConn->SetLastAccessTime(time(NULL));
    UNLIKELY_RET(!pConn->Init(m_pEpFw, m_iPktHdrLen), true, "ERR | clt conn init err(%s)", strerror(errno));

    return true;
}

template<typename CONNECTION>
bool CAcceptorSocket<CONNECTION>::OnClose()
{
    m_oConnHash.Release();
    //delete[] m_pConns;
    //m_pConns = NULL;
    m_iMaxConnCount = 0;

    if (m_pEpFw)
    {
        m_pEpFw->UnregisterIo(this);
        m_pEpFw = NULL;
    }

    return true;
}


// CRecordBlock

inline CRecordBlock::CRecordBlock(uint16_t wSize, void* pBuf, time_t uTime)
: CBlock16(wSize, pBuf)
, m_uTime(uTime)
{
}

inline time_t CRecordBlock::GetTime() const
{
    return m_uTime;
}


// CRecordWorker

#define ATTR_QUEUE_USERD        155266
#define ATTR_REQUEST            155263

inline bool CRecordWorker::OnStart()
{
    m_pFile = NULL;
    m_dwDate = 0;
    UpdateFileHandle();
    return true;
}

inline bool CRecordWorker::OnWork(CBlock16* pBlock)
{
    return OnWork((CRecordBlock*)pBlock);
}

inline bool CRecordWorker::OnWork(CRecordBlock* pBlock)
{
    //Attr_API_Set(ATTR_QUEUE_USERD, m_pQueue->GetBlockUsedCount());
    UpdateFileHandle(pBlock->GetTime());
    if (m_bTextFormat)
    {
        fputs(((char*)pBlock->GetBuffer()), m_pFile);
    }
    else
    {
        fwrite(pBlock->GetBuffer(), 1, pBlock->GetSize(), m_pFile);
    }
    
    return true;
}

inline bool CRecordWorker::OnEmpty()
{
    fflush(m_pFile);
    return true;
}

inline bool CRecordWorker::OnStop()
{
    fclose(m_pFile);
    return true;
}

inline void CRecordWorker::SetDate(uint32_t Date)
{
    m_dwDate = Date;
}

inline uint32_t CRecordWorker::GetDate() const
{
    return m_dwDate;
}


// CServiceBase

inline CServiceBase::~CServiceBase()
{
}

inline bool CServiceBase::OnServiceInit()
{
    return true;
}

inline bool CServiceBase::OnServiceExit()
{
    return true;
}

inline bool CServiceBase::OnLoadConfigure()
{
    return true;
}

inline bool CServiceBase::OnEnterLoop()
{
    return true;
}


// CThreadService

inline bool CThreadService::StartService()
{
    if (!Start())
    {
        return false;
    }

    return true;
}

inline bool CThreadService::WaitService()
{
    return Wait();
}


// CProcessService

inline bool CProcessService::StartService()
{
    if (!Start())
    {
        return false;
    }

    return true;
}

inline bool CProcessService::WaitService()
{
    return Wait();
}

// CMultiProcessService

inline CMultiProcessService::CMultiProcessService()
: m_iCount(0)
{
}

inline void CMultiProcessService::SetProcessCount(int iCount)
{
    if (iCount < 1)
    {
        m_iCount = 1;
        return;
    }
    m_iCount = iCount;
}

inline bool CMultiProcessService::StartService()
{
    if (!Start())
    {
        return false;
    }

    return true;
}

inline bool CMultiProcessService::WaitService()
{
    return Wait();
}

// CProgressValue

inline CProgressValue::CProgressValue(uint64_t ddwMaxValue)
: m_ddwPos(0)
, m_ddwMax(ddwMaxValue)
, m_dwCur(0)
, m_uOldTime(0)
, m_ddwOldPos(0)
{
}

inline void CProgressValue::Init(uint64_t ddwMaxValue)
{
    m_ddwPos = 0;
    m_ddwMax = ddwMaxValue;
    m_dwCur = 0;
    m_uOldTime = 0;
    m_ddwOldPos = 0;
}

inline uint32_t CProgressValue::GetCurPercent() const
{
    return m_dwCur;
}

inline uint64_t CProgressValue::GetSpeedPerSec()
{
    unlikely (!m_uOldTime)
    {
        m_ddwOldPos = m_ddwPos;
        m_uOldTime = time(NULL);
        return 0;
    }
    
    uint32_t dwRet = (m_ddwPos - m_ddwOldPos) / (time(NULL) - m_uOldTime);
    m_ddwOldPos = m_ddwPos;
    m_uOldTime = time(NULL);
    return dwRet;
}

inline bool CProgressValue::SetPosition(uint64_t ddwValue)
{
    m_ddwPos = ddwValue;
    uint32_t dwPer = ddwValue * 100 / m_ddwMax;
    if (dwPer > m_dwCur)
    {
        m_dwCur = dwPer;
        return true;
    }
    return false;
}

inline bool CProgressValue::IsEnd() const
{
    return m_ddwPos >= m_ddwMax;
}

#endif	/* __TSNETSERVICE_INL__ */

