/* 
 * File:   TSBitmap.cpp
 * Author: thunderliu
 * 
 * Created on 2011年11月17日, 下午4:49
 */

#include "TSPlatform.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "TSBitmap.h"

CBitmap* CBitmap::CreateBitmap(size_t uBitCount)
{
    if (!uBitCount)
    {
        return NULL;
    }

    void* pBuf = malloc((uBitCount + 7) / 8);
    if (!pBuf)
    {
        return NULL;
    }

    CBitmap* pBitmap = new CBitmap(uBitCount, pBuf);
    if (!pBitmap)
    {
        free(pBuf);
        return NULL;
    }
    return pBitmap;
}

int CBitmap::GetBit(size_t uPos) const
{
    return uPos < m_uBitCount ? (m_pBuf[uPos / 8] & (0x1 << (uPos % 8))) != 0 : -1;
}

int CBitmap::SetBit(size_t uPos, bool bIsOn)
{
    if (uPos >= m_uBitCount)
    {
        return -1;
    }

    if (bIsOn)
    {
        m_pBuf[uPos / 8] |= (0x1 << (uPos % 8));
    }
    else
    {
        m_pBuf[uPos / 8] &= ~(0x1 << (uPos % 8));
    }

    return 0;
}

