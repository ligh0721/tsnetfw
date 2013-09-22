/* 
 * File:   TSProcess.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月30日, 下午3:58
 */

#include "TSPlatform.h"
#include <stdlib.h>
#include "TSProcess.h"




// CProcess

bool CProcess::Start()
{
    if (m_iPid)
    {
        return false;
    }

    m_iPid = fork();
    if (!m_iPid)
    {
        // Child
        exit(ProcessProc());
    }

    // Parent
    return true;
}

CProcess* CProcess::ForkProcess()
{
    CProcess* pProc = new CProcess(fork());
    if (pProc->IsMe())
    {
        // child
        exit(pProc->ProcessProc());
    }

    // parent
    return pProc;
}

CProcessGroup* CProcess::ForkHere(int iCount)
{
    pid_t* pPid = new pid_t[iCount];
    for (int i = 0; i < iCount; i++)
    {
        pPid[i] = fork();
        if (!pPid[i])
        {
            // Child
            iCount = 0;
            break;
        }
    }

    CProcessGroup* pPg;
    if (iCount)
    {
        pPg = new CProcessGroup(iCount, pPid);
    }
    else
    {
        pPg = NULL;
    }

    delete[] pPid;

    return pPg;
}

CProcess* CProcess::CreateProcess(const char* pPath, char* const pArgv[], char* const pEnvp[])
{
    pid_t iPid = fork();
    if (!iPid)
    {
        // child
        exit(execve(pPath, pArgv, pEnvp));
    }

    // parent
    return new CProcess(iPid);
}


// CProcessGroup

CProcessGroup::CProcessGroup(int iCount, pid_t aiPid[])
{
    m_pProcess = new CProcess[iCount];
    for (int i = 0; i < iCount; i++)
    {
        m_pProcess[i].m_iPid = aiPid[i];
    }
}

bool CProcessGroup::Wait()
{
    for (int i = 0; i < m_iCount; i++)
    {
        m_pProcess[i].Wait();
    }

    return true;
}




