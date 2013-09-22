/* 
 * File:   TSFile.cpp
 * Author: thunderliu
 * 
 * Created on 2013年6月4日, 下午4:42
 */

#include "TSPlatform.h"
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "TSFile.h"
#include "TSDebug.h"


bool CFileMapping::Init(const CIo* pIo, bool bReadOnly)
{
    Close();
    
    if (!pIo || !pIo->IsInUse())
    {
        return false;
    }
    struct stat stStat = {};
    //__fxstat(_STAT_VER, pIo->GetHandle(), &stStat);
    __fxstat(_STAT_VER, pIo->GetHandle(), &stStat);
    LOG_ERR("DBG | File Mode: %u", stStat.st_mode);
    m_uLen = stStat.st_size;
    m_pAddr = mmap(NULL, m_uLen, bReadOnly ? PROT_READ : PROT_READ | PROT_WRITE, bReadOnly ? MAP_PRIVATE | MAP_NORESERVE : MAP_SHARED, pIo->GetHandle(), 0);
    
    if (m_pAddr == MAP_FAILED)
    {
        return false;
    }
    
    return true;
}

bool CFileMapping::Init(const char* pFileName, bool bReadOnly)
{
    CIo oIo;
    if (!oIo.Init(pFileName, bReadOnly ? O_RDONLY : O_RDWR))
    {
        return false;
    }
    return Init(&oIo, bReadOnly);
}

bool CFileMapping::Close()
{
    if (m_pAddr == MAP_FAILED)
    {
        return false;
    }
    munmap(m_pAddr, m_uLen);
    m_pAddr = MAP_FAILED;
    m_uLen = 0;
    return true;
}

