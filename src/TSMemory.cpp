/* 
 * File:   TSMemory.cpp
 * Author: thunderliu
 * 
 * Created on 2012年4月16日, 下午4:29
 */

#include "TSPlatform.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include "TSMemory.h"



bool CShareMemory::Init(key_t dwKey, size_t uSize)
{
    unlikely(!uSize)
    {
        return false;
    }
    
    m_iId = shmget(dwKey, uSize, IPC_CREAT | S_IRUSR | S_IWUSR);
    unlikely(m_iId < 0)
    {
        return false;
    }
        
    m_pAddr = shmat(m_iId, NULL, 0);
    unlikely((long)m_pAddr == -1)
    {
        return false;
    }
    
    shmid_ds stDs = {};
    unlikely(shmctl(m_iId, IPC_STAT, &stDs) < 0)
    {
        return false;
    }
    m_dwKey = dwKey;
    m_uLen = stDs.shm_segsz;
    
    return true;
}

bool CShareMemory::Attach(key_t dwKey, bool bReadOnly)
{
    m_iId = shmget(dwKey, 0, S_IRUSR | S_IWUSR);
    shmid_ds stDs = {};
    unlikely(shmctl(m_iId, IPC_STAT, &stDs) < 0)
    {
        return false;
    }
    m_dwKey = dwKey;
    m_uLen = stDs.shm_segsz;
    
    m_pAddr = shmat(m_iId, NULL, bReadOnly ? SHM_RDONLY : 0);
    unlikely((long)m_pAddr == -1)
    {
        return false;
    }
    
    return true;
}

