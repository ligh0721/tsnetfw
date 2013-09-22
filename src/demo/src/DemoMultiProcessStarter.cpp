/* 
 * File:   DemoMultiProcessStarter.cpp
 * Author: thunderliu
 *
 * Created on 2011年12月30日, 下午3:57
 */

/*
 * 
 */

#include "TSNetFw.h"

int main(int argc, char* argv[])
{
    CCommandLine::CreateCommandLine(argc, argv);

    if (CCommandLine::GetArgCount() < 3)
    {
        fprintf(stderr, "usage: %s PROCESSCOUNT PATH PARAM ...\n", CCommandLine::GetArgValue(0));
        return 0;
    }

    int iCount;
    char szPath[1024];

    CStringHelper::StrToVal(CStr(CCommandLine::GetArgValue(1), 16), &iCount, sizeof (iCount));
    strncpy(szPath, CCommandLine::GetArgValue(2), sizeof (szPath));

    CProcess** ppProc = new CProcess*[iCount];

    char** ppArgv = new char*[CCommandLine::GetArgCount() - 1];
    memmove(ppArgv, CCommandLine::GetArgValues() + 2, sizeof (char*)* CCommandLine::GetArgCount() - 1);
    ppArgv[CCommandLine::GetArgCount()] = NULL;

    for (int i = 0; i < iCount; i++)
    {
        ppProc[i] = CProcess::CreateProcess(szPath, ppArgv, NULL);
    }

    //delete[] ppArgv;

    for (int i = 0; i < iCount; i++)
    {
        ppProc[i]->Wait();
    }



    return 0;
}

