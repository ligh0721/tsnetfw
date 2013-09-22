/* 
 * File:   TSProcess.h
 * Author: thunderliu
 *
 * Created on 2011年12月30日, 下午3:58
 */

#ifndef __TSPROCESS_H__
#define	__TSPROCESS_H__


#include <unistd.h>
#include <wait.h>
#include "TSRuntime.h"




class CProcessGroup;


// CProcess

class CProcess : public CRuntimeInstance
{
    friend class CProcessGroup;

public:
    CProcess();
    virtual ~CProcess();

protected:
    CProcess(pid_t iPid);

public:
    static CProcess* Attach(pid_t iPid);
    static CProcess* ForkProcess();
    static CProcess* CreateProcess(const char* pPath, char* const pArgv[], char* const pEnvp[]);
    static CProcessGroup* ForkHere(int iCount);

    virtual bool Start();
    virtual bool Wait();
    bool IsMe() const;
    pid_t GetPid() const;
    bool Release();


protected:
    virtual int ProcessProc();
    //long ParentProc();

protected:
    pid_t m_iPid;

};


// CProcessGroup

class CProcessGroup
{
    friend class CProcess;

protected:
    CProcessGroup(int iCount, pid_t aiPid[]);
    virtual ~CProcessGroup();

public:
    bool Wait();
    bool Release();

protected:
    int m_iCount;
    CProcess* m_pProcess;
};


#include "TSProcess.inl"

#endif	/* __TSPROCESS_H__ */

