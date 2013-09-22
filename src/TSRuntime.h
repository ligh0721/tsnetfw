/* 
 * File:   TSRuntime.h
 * Author: thunderliu
 *
 * Created on 2011年12月27日, 下午9:01
 */

#ifndef __TSRUNTIME_H__
#define	__TSRUNTIME_H__

class CCommandLine
{
protected:
    CCommandLine(int iArgc, char* const* ppArgv);

public:
    static int GetArgCount();
    static const char* GetArgValue(int iIndex);
    static char* const* GetArgValues();

    static CCommandLine* CreateCommandLine(int iArgc, char* const* ppArgv);


protected:
    const int m_iArgc;
    char* const* m_ppArgv;

    static CCommandLine* m_pInstance;


};

class CRuntimeInstance
{
public:
    virtual ~CRuntimeInstance();

};





#include "TSRuntime.inl"

#endif	/* __TSRUNTIME_H__ */

