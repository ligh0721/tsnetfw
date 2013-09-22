/* 
 * File:   TSLab.cpp
 * Author: thunderliu
 * 
 * Created on 2012年6月13日, 下午3:13
 */

#include "TSPlatform.h"
#include "TSLab.h"


// CMachine

uint64_t CMachine::GetMaxHz()
{
    uint64_t ddwRet = 0;
    time_t uNow = 0;
    time_t uStart = time(NULL);
    while ((uNow = time(NULL)) <= uStart)
    {
        sleep(1);
    }

    while (uNow <= time(NULL))
    {
        OnWork();
        ++ddwRet;
    }

    return ddwRet;
}

