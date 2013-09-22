/* 
 * File:   TSString.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月27日, 下午4:59
 */

#include "TSPlatform.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "TSRuntime.h"


#include "TSList.h"
#include "TSString.h"



// CStr

int CStr::Format(const char* pFmt, ...)
{
    assert(m_pBuf);
    va_list argp;
    va_start(argp, pFmt);
    int iRet = vsnprintf(m_pBuf, m_uSize, pFmt, argp);
    va_end(argp);
    return iRet;
}


// CStringHelper

char* CStringHelper::PickOutWord(const char* pStr, char cLeftPair, char cRightPair, const char* pLeftSeps, const char* pRightSeps, char* pSep, uint32_t* pWordLen)
{
    if (!pStr)
    {
        return NULL;
    }
    uint32_t i, j, k;
    bool bSepFlag = false;
    int iPairPos = -1;
    for (i = 0; pStr[i]; i++)
    {
        if (iPairPos != -1)
        {
            if (pStr[i] == cRightPair)
            {
                if (pSep) *pSep = cRightPair;
                if (pWordLen) *pWordLen = i + 1 - iPairPos;

                return (char*)&pStr[iPairPos];
            }
            else
            {
                continue;
            }
        }
        else if (pStr[i] == cLeftPair)
        {
            iPairPos = i;
            continue;
        }
        // ignore left seps
        bSepFlag = false;
        for (j = 0; pLeftSeps[j]; j++)
        {
            if (pStr[i] == pLeftSeps[j])
            {
                bSepFlag = true;
                break;
            }
        }
        if (bSepFlag)
        {
            continue;
        }

        // not a leftsep
        for (k = i; pStr[k]; k++)
        {
            for (j = 0; pRightSeps[j]; j++)
            {
                if (pStr[k] == pRightSeps[j])
                {
                    if (pSep) *pSep = pRightSeps[j];
                    if (pWordLen) *pWordLen = k - i;

                    return (char*)&pStr[i];
                }
            }
        }
        if (pSep) *pSep = 0;
        if (pWordLen) *pWordLen = k - i;
        return (char*)&pStr[i];
    }
    if (pSep) *pSep = 0;
    return NULL;
}

bool CStringHelper::StrToVal(const CStr& roStr, void* pVal, size_t uValSize)
{
    uint64_t uVal = 0;
    if (!strncmp(roStr, "dwTIME", roStr.GetSize()))
    {
        uVal = time(NULL);
        memmove(pVal, &uVal, uValSize);
        return true;
    }
    char* pTmpStr = strchr(const_cast<CStr&>(roStr), 'x');
    if (pTmpStr)
    {
        // hex
        if (!((pTmpStr[1] <= '9' && pTmpStr[1] >= '0') || (pTmpStr[1] <= 'f' && pTmpStr[1] >= 'a') || (pTmpStr[1] <= 'F' && pTmpStr[1] >= 'A')))
        {
            // error
            //LOG_POS("str(%s)", sRawInput.c_str());
            return false;
        }

        uVal = strtoull(pTmpStr + 1, NULL, 16);
        memmove(pVal, &uVal, uValSize);
        return true;
    }
    else if (strnlen(roStr, roStr.GetSize()) > 1 && roStr.GetChar(0) == '0')
    {
        // hex
        if (roStr.GetChar(1) > '7' || roStr.GetChar(1) < '0')
        {
            // error
            //LOG_POS("str(%s)", sRawInput.c_str());
            return false;
        }
        uVal = strtoull(roStr, NULL, 8);
        memmove(pVal, &uVal, uValSize);
        return true;
    }
    else if ((roStr.GetChar(0) <= '9' && roStr.GetChar(0) >= '0'))
    {
        uVal = strtoull(roStr, NULL, 10);
        memmove(pVal, &uVal, uValSize);
        return true;
    }
    else if (roStr.GetChar(0) == '%')
    {
        const char* pArgv = CCommandLine::GetArgValue(atoi(roStr.GetBuffer(1)) + 1);
        if (!pArgv)
        {
            return false;
        }

        return CStringHelper::StrToVal(CStr(pArgv, strnlen(pArgv, 1024)), pVal, uValSize);
    }
    else
    {
        // not surport
        //LOG_POS("str(%s) not suport", sRawInput.c_str());
        return false;
    }
    //LOG_POS("str(%s) not suport", sRawInput.c_str());
    return false;
}


// CStringLine

const char* CStringLine::GetNextStringPos(uint32_t& dwLen)
{
    m_pPos = CStringHelper::PickOutWord(m_pPos, '\"', '\"', " \t\r\n", " \t\r\n", NULL, &dwLen);
    if (!m_pPos)
    {
        m_bIsEnd = true;
        return NULL;
    }
    const char* pRet = m_pPos;
    m_pPos += dwLen;
    return pRet;
}

int CStringLine::SetOrGetIo(FILE* pIn, FILE* pOut, FILE** ppOrgIn, FILE** ppOrgOut)
{
    return 0;
}

int CStringLine::ReadString(CStr& roStr)
{
    uint32_t dwLen;
    const char* pRet = GetNextStringPos(dwLen);
    if (!pRet)
    {
        return -1;
    }
    uint32_t dwStrLen = dwLen >= roStr.GetSize() ? roStr.GetSize() - 1 : dwLen;
    memmove(roStr, pRet, dwStrLen);
    roStr.GetChar(dwStrLen) = 0;
    return 0;
}

int CStringLine::ReadDword(uint32_t& dwVal)
{
    CStr oTmp(32);
    if (ReadString(oTmp) < 0)
    {
        return -1;
    }
    int iRet;
    if (oTmp.GetChar(0) == '0')
    {
        if (oTmp.GetChar(1) == 'x')
        {
            iRet = sscanf(oTmp, "%x", &dwVal);
        }
        else
        {
            iRet = sscanf(oTmp, "%o", &dwVal);
        }

    }
    else if (oTmp.GetChar(0) == 'x')
    {
        iRet = sscanf(oTmp, "x%x", &dwVal);
    }
    else
    {
        iRet = sscanf(oTmp, "%u", &dwVal);
    }
    return iRet ? 0 : -1;
}

int CStringLine::ReadQword(uint64_t& ddwVal)
{
    CStr oTmp(32);
    if (ReadString(oTmp) < 0)
    {
        return -1;
    }
    int iRet;
    if (oTmp.GetChar(0) == '0')
    {
        if (oTmp.GetChar(1) == 'x')
        {
#ifdef __x86_64__
            iRet = sscanf(oTmp, "%lx", &ddwVal);
#else
            iRet = sscanf(oTmp, "%llx", &ddwVal);
#endif
        }
        else
        {
#ifdef __x86_64__
            iRet = sscanf(oTmp, "%lo", &ddwVal);
#else
            iRet = sscanf(oTmp, "%llo", &ddwVal);
#endif
        }

    }
    else if (oTmp.GetChar(0) == 'x')
    {
#ifdef __x86_64__
        iRet = sscanf(oTmp, "x%lx", &ddwVal);
#else
        iRet = sscanf(oTmp, "x%llx", &ddwVal);
#endif
    }
    else
    {
#ifdef __x86_64__
        iRet = sscanf(oTmp, "%lu", &ddwVal);
#else
        iRet = sscanf(oTmp, "%llu", &ddwVal);
#endif
    }
    return iRet ? 0 : -1;
}

const char* CStringLine::GetPos()
{
    while (*m_pPos == ' ' || *m_pPos == '\t' || *m_pPos == '\r' || *m_pPos == '\n')
    {
        m_pPos++;
    }
    return m_pPos;
}

// CCmdStrLn

int CCmdStrLn::ReadString(CStr& roStr)
{
    CStr oBuf(1024);
    if (CStringLine::ReadString(oBuf) < 0)
    {
        return -1;
    }
    const char* pRet;
    uint32_t dwLen;
    if (oBuf.GetChar(0) == '%')
    {
        pRet = m_ppArgv[atoi(oBuf.GetBuffer(1)) + 1];
    }
    else
    {
        pRet = oBuf;
    }
    dwLen = strlen(pRet);
    uint32_t dwStrLen = dwLen >= roStr.GetSize() ? roStr.GetSize() - 1 : dwLen;
    memmove(roStr, pRet, dwStrLen);
    roStr.GetChar(dwStrLen) = 0;
    return 0;
}

#ifdef TSNETFW_FEATURE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
// CIoReadline

CReadline* CReadline::m_pInstance = NULL;

CReadline::CReadline()
: m_szPrompt(64)
{
    m_pInstance = this;
    InitializeReadline();
}

int CReadline::Readline(CStr& roLine, const char* pPrompt)
{
    char* pCmdLn;
    static CStr oCmdLn(10240);

    if (!pPrompt)
    {
        pPrompt = m_szPrompt;
    }
    pCmdLn = readline(pPrompt);
    if (!pCmdLn)
    {

        return -1;
    }
    //    if (!(*pCmdLn))
    //    {
    //        return;
    //    }
    if (oCmdLn.Compare(pCmdLn))
    {
        oCmdLn.Copy(pCmdLn);
        add_history(pCmdLn);
    }

    roLine.Copy(pCmdLn);
    return 0;
}

int CReadline::ReadString(CStr& roStr)
{
    int iRet = 0;
    while (CCmdStrLn::ReadString(roStr) < 0)
    {
        iRet = 1;
        if (Readline(roStr) < 0)
        {
            iRet = -1;
            break;
        }
        else
        {
            Attach(roStr);
        }
    }

    return iRet;
}

int CReadline::SetOrGetIo(FILE* pIn, FILE* pOut, FILE** ppOrgIn, FILE** ppOrgOut)
{
    if (ppOrgIn)
    {
        *ppOrgIn = rl_instream;
    }

    if (ppOrgOut)
    {
        *ppOrgOut = rl_outstream;
    }

    if (pIn)
    {
        rl_instream = pIn;
    }

    if (pOut)
    {
        rl_outstream = pOut;
    }

    return 0;
}

char* CReadline::CommandGenerator(const char* pText, int iState)
{
    const char* pName;
    static int iLen = 0;
    static CCmdList::CListNode* pPos;
    if (!iState)
    {
        pPos = m_pInstance->m_lstCmd.GetHeadNode();
        iLen = strlen(pText);
        //iCount = m_pInstance->m_lstCmd;
    }

    //printf("<%d:%s> ", iState, pText);
    while (pPos)
    {
        pName = pPos->tData;
        pPos = pPos->pNext;

        if (!strncmp(pName, pText, iLen))
        {
            return strdup(pName);
        }
    }

    return NULL;

}

char** CReadline::CommandCompletion(const char* pTest, int iStart, int iEnd)
{
    char** pRet = NULL;
    if (!iStart)
    {
        pRet = rl_completion_matches(pTest, CReadline::CommandGenerator);
    }
    return pRet;
}

void CReadline::InitializeReadline()
{
    rl_readline_name = "CIoReadline";
    rl_attempted_completion_function = CReadline::CommandCompletion;
}

void CReadline::AddToReadline(const char* pName)
{
    if (pName && strcmp(pName, ""))
    {
        CCmdList::CListNode* pNode = new CCmdList::CListNode;
        assert(pNode);
        strncpy(pNode->tData, pName, sizeof (STR_CMD));
        if (!m_lstCmd.InsertNodeAfter(pNode, m_lstCmd.GetTailNode()))
        {
            delete pNode;
        }
    }
}

#endif // TSNETFW_FEATURE_READLINE

