/* 
 * File:   TSConfigure.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月25日, 上午2:58
 */

#include "TSPlatform.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "TSHash.h"
#include "TSString.h"
#include "TSConfigure.h"


// CConfigure

bool CConfigureFile::LoadFromFile(const char* pFileName)
{
    FILE* pFile;
    pFile = fopen(pFileName, "r");
    if (!pFile)
    {
        return false;
    }
    
    m_oCfgFileName.Copy(pFileName);

    m_oCfgMap.Release();
    m_oCfgMap.Init(100, 5, CFG_KEY());

    char szLine[1024];
    const char* pStr;
    uint32_t dwLen;
    char cSep;

    CCfgHash::CHashNode* pNode;
    bool bNew;

    while (!feof(pFile))
    {
        memset(szLine, 0, sizeof (szLine));
        if (!fgets(szLine, sizeof (szLine) - 1, pFile))
        {
            break;
        }
        pStr = szLine;

        // 提取KEY
        pStr = CStringHelper::PickOutWord(pStr, 0, 0, " \t\r\n", "= \t\r\n", &cSep, &dwLen);
        if (!pStr || !dwLen)
        {
            continue;
        }

        if (pStr[0] == '#')
        {
            continue;
        }

        CFG_KEY stKey;
        memmove(stKey.sz, pStr, dwLen);
        pStr += dwLen;

        cSep = 0;
        pStr = CStringHelper::PickOutWord(pStr, 0, 0, " \t\r\n", "=", &cSep, &dwLen);
        if (cSep != '=')
        {
            continue;
        }
        pStr += dwLen;

        // 提取MAPPED
        pStr = CStringHelper::PickOutWord(pStr, 0, 0, "= \t", " \t\r\n", NULL, &dwLen);
        if (!pStr)
        {
            continue;
        }

        CFG_MAPPED stMapped;
        memmove(stMapped.sz, pStr, dwLen);

        pNode = m_oCfgMap.FindNodeToSet(stKey, &bNew);
        if (!pNode || pNode == (CCfgHash::CHashNode*)(-1))
        {
            fclose(pFile);
            return false;
        }

        if (!bNew)
        {
            // KEY重复
            fclose(pFile);
            return false;
        }

        pNode->tMapped = stMapped;

    }

    m_oCfgMap.Traverse((CCfgHash::TRAVERSECALLBACKFUNC) & CCfgHash::ShowCfg, NULL);

    fclose(pFile);
    return true;
}

CFG_MAPPED* CConfigureFile::GetItem(const char* pKey)
{
    CFG_KEY stKey;
    strncpy(stKey.sz, pKey, sizeof (CFG_KEY));

    CCfgHash::CHashNode* pNode;
    pNode = m_oCfgMap.FindNode(stKey);
    if (!pNode || pNode == (CCfgHash::CHashNode*)(-1))
    {
        LOG_ERR("ERR | Cfg key(%s) was not found", pKey);
        return NULL;
    }

    return &pNode->tMapped;
}

bool CConfigureFile::ItemToVal(const char* pKey, void* pVal, size_t uSize)
{
    CFG_MAPPED* pMapped;
    pMapped = GetItem(pKey);

    if (!pMapped)
    {
        return false;
    }

    return CStringHelper::StrToVal(CStr(pMapped->sz, sizeof (pMapped->sz)), pVal, uSize);
}

bool CConfigureFile::ReadString(const char* pKey, CStr& roItem)
{
    CFG_MAPPED* pMapped;
    pMapped = GetItem(pKey);

    if (!pMapped)
    {
        return false;
    }

    roItem.Copy(pMapped->sz);

    return true;
}





