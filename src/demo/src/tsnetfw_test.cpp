/* 
 * File:   tsnetfw_test.cpp
 * Author: thunderliu
 *
 * Created on 2011年5月26日, 下午6:38
 */


//#include "lib64/Attr_API.h"
//#define TSNETFW_FEATURE_READLINE
//#define TSNETFW_FEATURE_LUA
#include "TSNetFw.h"
#include "tsnetfw_test.h"
#include "TSMemory.h"

int empty()
{
    return 0;
}

class CTimer : public CTimerQueueNode
{
public:
    CTimer();
    virtual bool OnTimeout();
    int m_iC;
    time_t m_tm;

};

CTimer::CTimer()
: m_iC(0)
, m_tm(time(NULL))
{

}

bool CTimer::OnTimeout()
{
    time_t tm0 = time(NULL);
    printf("expire(%ld %ld) %d, %ld\n", m_lInterval, m_lExpireLeft, m_iC++, tm0 - m_tm);
    m_tm = tm0;
    return true;
}

class CIntList : public CList<int>
{
public:

    void ShowValues()
    {
        TraverseFromHeadToTail((TRAVERSECALLBACKFUNC) & CIntList::ShowValue, NULL);
        printf("\n");
        TraverseFromTailToHead((TRAVERSECALLBACKFUNC) & CIntList::ShowValue, NULL);
        printf("\n");
    }

protected:

    bool ShowValue(CListNode& rNode, void* pParam)
    {
        printf("%d ", rNode.tData);
        return true;
    }
};

#ifdef TSNETFW_FEATURE_PACKET
#include <linux/if_arp.h>

//192.168.1.115
#define ETH_HADDR_ME_WLAN0     (uint8_t*)"\x00\x08\xca\x66\xbd\x51"

//192.168.1.104
#define ETH_HADDR_192_168_1_104     (uint8_t*)"\xc8\x3a\x35\xcb\x1b\xb9"

class CSendArpThrd : public CThread
{
protected:

    virtual long ThreadProc()
    {
        CEthSocket oEthSocket;
        oEthSocket.Bind("wlan0");
        oEthSocket.EnablePromisc("wlan0", true);
        char szBuf[65535];

        sleep(1);
        uint32_t dwStart = ntohl(inet_addr("10.72.130.1"));
        uint32_t dwEnd = ntohl(inet_addr("10.72.150.255"));
        for (uint32_t i = dwStart; i < dwEnd; i++)
        {
            BH_NEW(szBuf, sizeof (szBuf));
            BH_GET_NEXT_RET(struct ethhdr, pEthHdr, "eth hdr");

            CEthSocket::MakeEthernetHeader(pEthHdr, ETH_HADDR_BROADCAST, ETH_HADDR_ME_WLAN0, ETH_P_ARP);
            //CEthSocket::MakeEthernetHeader(pEthHdr, ETH_HADDR_192_168_1_104, ETH_HADDR_192_168_1_115, ETH_P_ARP);

            BH_GET_NEXT_RET(struct etharphdr, pArpHdr, "arp hdr");

            CEthSocket::MakeArpHeader(pArpHdr, ETH_HADDR_ME_WLAN0, inet_addr("0.0.0.0"), ETH_HADDR_UNKNOWN, htonl(i), ARPOP_REQUEST);
            //CEthSocket::MakeArpHeader(pArpHdr, ETH_HADDR_192_168_1_115, inet_addr("192.168.1.1"), ETH_HADDR_UNKNOWN, INADDR_ANY, ARPOP_REPLY);

            //LOG_ERR_DMP(BH_OBJ.GetBuffer(), BH_OBJ.GetBufferPos());
            oEthSocket.Send(BH_OBJ.GetBuffer(), BH_OBJ.GetBufferPos());
            usleep(100000);
        }
        return 0;
    }
};

int get_mac()
{
    FILE* pFile = NULL;
    if (pFile)
    {
        CLog::SetOutFile(pFile);
    }
    else
    {
        CLog::SetOutFile(stderr);
    }

    CEthSocket oEthSocket;
    oEthSocket.Bind("wlan0");
    oEthSocket.EnablePromisc("wlan0", true);
    char szBuf[65535];
    int iRet;

    CSendArpThrd oThrd;
    oThrd.Start();

    while ((iRet = oEthSocket.Recv(szBuf, sizeof (szBuf))) > 0)
    {
        //LOG_ERR_DMP(szBuf, iRet);
        BH_NEW(szBuf, iRet);
        //printf("%d Bytes\n", iRet);
        BH_GET_NEXT_RET(struct ethhdr, pEthHdr, "eth hdr");
        //fprintf(stderr, "%lu %u\n", sizeof(struct ethhdr), pEthHdr->h_proto);
        uint16_t wType = ntohs(pEthHdr->h_proto);
        if (wType != ETH_P_ARP)
        {
            continue;
        }

        BH_GET_NEXT_RET(struct etharphdr, pArpHdr, "arp hdr");
        if (!memcmp(pArpHdr->ar_sha, ETH_HADDR_ME_WLAN0, sizeof (pArpHdr->ar_sha)))
        {
            continue;
        }

        fprintf(stderr, "from %-16s %02X:%02X:%02X:%02X:%02X:%02X %s\n", inet_ntoa(*(in_addr*)pArpHdr->ar_sip), pArpHdr->ar_sha[0], pArpHdr->ar_sha[1], pArpHdr->ar_sha[2], pArpHdr->ar_sha[3], pArpHdr->ar_sha[4], pArpHdr->ar_sha[5], ntohs(pArpHdr->ar_op) == ARPOP_REQUEST ? "REQUEST" : "REPLY");
    }

    return 0;
}
#endif // TSNETFW_FEATURE_PTRACE

#include <dlfcn.h>

int get_socket_addr()
{
    void* pDl = dlopen("libc.so.6", RTLD_LAZY);
    assert(pDl);
    void* pFunc = dlsym(pDl, "socket");
    printf("%p\n", pFunc);
    printf("%p\n", socket);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    close(s);
    assert(pFunc);
    return 0;
}


#ifdef TSNETFW_FEATURE_PTRACE
#include <sys/user.h>

int test_ptrace()
{
    pid_t iPid = 0;
    void* pAddr = NULL;
    CStringHelper::StrToVal(CStr(CCommandLine::GetArgValue(1), 1024), &iPid, sizeof (iPid));
    CStringHelper::StrToVal(CStr(CCommandLine::GetArgValue(2), 1024), &pAddr, sizeof (pAddr));
    CPtrace p;
    p.Attach(iPid);
    p.Wait();
    struct user_regs_struct regs;
    struct user_fpregs_struct fpregs;
    struct user usr;
    memset(&regs, 0, sizeof (regs));
    memset(&fpregs, 0, sizeof (fpregs));
    memset(&usr, 0, sizeof (usr));
    long lRet = ptrace(PT_GETREGS, p.GetPid(), 0, &regs);

    //long lRet = p.ReadData(pAddr);
    printf("lRet: 0x%08lx, %s\n", lRet, strerror(errno));
    //printf("0x%08lx\n", regs.rbp);

    lRet = ptrace(PT_READ_U, p.GetPid(), 32, &usr);
    printf("lRet: 0x%08lx, %s\n", lRet, strerror(errno));
    //printf("0x%08lx\n", usr.regs.r15);

    return 0;
}
#endif // TSNETFW_FEATURE_PTRACE

#ifdef TSNETFW_FEATURE_LUA
CThread* g_pT = NULL;

class CMyLuaSe : public CLuaSe
{
public:

    CMyLuaSe()
    {
    }

    CMyLuaSe(lua_State* pL) : CLuaSe(pL)
    {
    }

protected:

    virtual void OnRunFileOk(CThread* pThrd, const char* pFile, const void* pParam)
    {
        fprintf(stderr, "======OK======\n");
        //delete this;
    }

    virtual void OnRunFileError(CThread* pThrd, const char* pFile, const CStr& roErr, const void* pParam)
    {
        fprintf(stderr, "======ERR: %s======\n", roErr.GetBuffer());
        //delete this;
    }

    virtual void OnRunFileCancel(CThread* pThrd, const char* pFile, const void* pParam)
    {
        fprintf(stderr, "======Cancel: ======\n");
        //delete this;
    }

};

int l_test(lua_State* L)
{
    CLuaSe oL(L);
    int n = oL.CheckInteger(1);
    const char* s = oL.CheckString(2);
    if (n > 10)
    {
        return oL.ThrowError(true, "n(%d) is too large!", n);
    }
    for (int i = 0; i < n; i++)
    {
        printf("%s\n", s);
    }
    oL.PushInteger(n);

    return 1;
}

int l_run(lua_State* L)
{
    CMyLuaSe oL(L);
    CMyLuaSe* pMyL = new CMyLuaSe();
    pMyL->Init();
    const char* s = oL.CheckString(1);
    pMyL->AsyncRunFile(s, NULL);

    return 0;
}

int l_var(lua_State* L)
{
    CLuaSe oL(L);
    const char* s = oL.CheckString(1);
    const char* pVal = oL.GetGlobalString(s);
    oL.PushString(pVal);
    return 1;
}

int l_cancel(lua_State* L)
{
    CLuaSe oL(L);
    g_pT->Cancel();
    return 0;
}

int LuaCmdLn()
{
    CMyLuaSe oL;
    oL.Init();
    CStr oErr(128);

    if (!oL.RunString("print('asdfasdfasdf\\n')", &oErr))
    {
        fprintf(stderr, "%s\n", oErr.GetBuffer());
    }

    g_pT = oL.AsyncRunFile("../bin/test.lua", NULL);

    CStr oBuf(256);
    //lua_State* L = luaL_newstate(); /*Open Lua*/
    //luaL_openlibs(L); /*Open standard lib*/

    oL.RegisterCFunction("test", l_test);
    oL.RegisterCFunction("run", l_run);
    oL.RegisterCFunction("var", l_var);
    oL.RegisterCFunction("cancel", l_cancel);
    CReadline oRl;
    oRl.SetPrompt("> ");
    //oRl.SetOrGetIo()

    bool bRet;
    const char* pStr;

    while (oRl.Readline(oBuf) >= 0)
    {
        int t = oL.GetTopPos();
        printf("[top: %d]\n", t);
        pStr = oBuf;
        bRet = oL.RunString(pStr, &oErr);
        if (!bRet)
        {
            while (!bRet && CLuaSe::HasErrorEofMark(oErr))
            {
                oL.PushString(pStr);
                if (oRl.Readline(oBuf, ">> ") < 0)
                {
                    return 0;
                }
                oL.PushString(oBuf);
                oL.PushString("\n");
                oL.Insert(-2);
                oL.Concat(3);
                pStr = oL.ToString(CLuaSe::STACK_POS_TOP);
                oL.Pop(1);
                bRet = oL.RunString(pStr, &oErr);
            }
            if (!bRet)
            {
                fprintf(stderr, "%s\n", oErr.GetBuffer());
            }
        }
    }

    return 0;
}
#endif // TSNETFW_FEATURE_LUA

class CTest
{
public:

    int Test(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l, int m, int n)
    {
        m_n = a + b + c;
        m_m = d + e + f + g;
        return m_n + m_m;
    }

protected:
    int m_n;
    int m_m;
};

typedef struct
{
    uint32_t dwUin;
    uint16_t wFlagId;
} S_ON_INFO;

int test()
{
    int iRcvSck = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in stSa = {0};
    stSa.sin_family = AF_INET;
    stSa.sin_addr.s_addr = inet_addr("127.0.0.1");
    stSa.sin_port = htons(2888);
    bind(iRcvSck, (struct sockaddr*)&stSa, sizeof (stSa));
    for (int i = 0; i < 9; i++)
    {
        if (!fork())
        {
            break;
        }
    }

    int iRes;
    char szBuf[1024];
    for (;;)
    {
        iRes = recv(iRcvSck, szBuf, sizeof (szBuf), 0);
        if (iRes <= 0)
        {
            continue;
        }
        S_ON_INFO* pInfo = (S_ON_INFO*)szBuf;
        fprintf(stderr, "pid:%d, uin:%u, flag:%hu\n", getpid(), ntohl(pInfo->dwUin), ntohs(pInfo->wFlagId));
    }
    return 0;
}

#include <string>
using namespace std;

struct TTTT
{
    int a;
    char b;
    long c;
    short d;
};

int ccc = 0;
time_t uTm = time(NULL);

void OnExit(int)
{
    
    uTm = time(NULL) - uTm;
    fprintf(stderr, "%lu/sec", (uint64_t)ccc / uTm);
    sleep(10);
}

#include <fstream>
#include <sstream>
using namespace std;

int txt2bin()
{
    ifstream oFile("20130602.txt");
    oFile.seekg(0, ios::end);
    size_t uLen = oFile.tellg();
    oFile.seekg(0, ios::beg);
    
    FILE* pFile = fopen("20130602.bin", "wb");
    string sLine;
    
    CProgressValue oPv(uLen);
    
    while (getline(oFile, sLine))
    {
        uint32_t dwUin = 0;
        uint32_t dwCount = 0;
        stringstream ss(sLine);
        ss >> dwUin >> dwCount;
        fwrite(&dwUin, sizeof(dwUin), 1, pFile);
        fwrite(&dwCount, sizeof(dwCount), 1, pFile);
        while (!ss.eof())
        {
            uint32_t dwFo = 0;
            ss >> dwFo;
            if (!dwFo)
            {
                break;
            }
            fwrite(&dwFo, sizeof(dwFo), 1, pFile);
        }
        dwUin = 0xFFFFFFFF;
        fwrite(&dwUin, sizeof(dwUin), 1, pFile);
        if (oPv.SetPosition(oFile.tellg()))
        {
            printf("Progress: %u%%\n", oPv.GetCurPercent());
        }
    }
    stringstream ss(sLine);
    fclose(pFile);
    return 0;
}


struct MAPPED
{
    uint32_t dwCount;
};

class CTestHash : public CNormalPersistentHash<uint32_t, MAPPED>
{
public:
    typedef typename CPersistentHash<uint32_t, MAPPED>::CDataCell CDataCell;
    typedef typename CPersistentHash<uint32_t, MAPPED>::CHashNode CHashNode;
    
public:
    virtual bool OnHandleData(const CDataCell& rData)
    {
        CHashNode* pNode = (CHashNode*)FindNodeToSet(rData.tKey, NULL);
        pNode->tMapped.dwCount = rData.tData.dwCount;
        return true;
    }
    
    bool ShowAll(CHashNode& rNode, void* pParam)
    {
        printf("%u:%u\n", ((CHashNode*)&rNode)->tKey, rNode.tMapped.dwCount);
        return true;
    }
};

int main(int argc, char** argv)
{
    CCommandLine::CreateCommandLine(argc, argv);
    CServiceBase::IgnoreSignals();
    CLog::SetLogLevel(E_LL_DBG);

    CTestHash oHash;
    oHash.Init(1000, 5, 0, 0x1A000000, "./", ".blg", "./", ".dmp");
    //oHash.Attach(0x1A000000, false, "./", ".blg", "./", ".dmp");
    
    CTestHash::CDataCell oData;
    oData.tKey = 174209756;
    oData.tData.dwCount = 368;
    oHash.HandleData(oData);
    
    oData.tKey = 1115629309;
    oData.tData.dwCount = 54899;
    oHash.HandleData(oData);
    
    oHash.DumpMemory();
    
    oData.tKey = 11112222;
    oData.tData.dwCount = 1212;
    oHash.HandleData(oData);
    
    oHash.Traverse((CTestHash::TRAVERSECALLBACKFUNC)&CTestHash::ShowAll, NULL);
    
    return EXIT_SUCCESS;
}

