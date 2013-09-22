/* 
 * File:   TSDebug.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月29日, 上午11:32
 */

#include "TSPlatform.h"
#include <stdio.h>
#include <stdint.h>
#include "TSDebug.h"


// CDebug

void CDebug::SimpleDump(const void* pBuf, size_t uSize)
{
    for (size_t i = 0; i < uSize; i++)
    {
        if (i % 0x10 == 0 && i != 0)
        {
            fprintf(CLog::m_pFile, "\n");
        }
        fprintf(CLog::m_pFile, "%02X ", uint32_t(*((uint8_t*)pBuf + i)));
    }
    fprintf(CLog::m_pFile, "\n");
}

// CLog

E_LOG_LVL CLog::m_eLogLv = E_LL_ERR;
FILE* CLog::m_pFile = stderr;
//const char* CLog::m_pFuncName = __FUNCTION__;

