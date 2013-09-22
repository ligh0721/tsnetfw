/* 
 * File:   TSBitmap.h
 * Author: thunderliu
 *
 * Created on 2011年11月17日, 下午4:49
 */

#ifndef __TSBITMAP_H__
#define	__TSBITMAP_H__

class CBitmap
{
public:
    CBitmap();
    bool Init(size_t uBitCount, void* pBuf);
    virtual ~CBitmap();
    
protected:
    CBitmap(size_t uBitCount, void* pBuf);

public:
    static CBitmap* CreateBitmap(size_t uBitCount);
    void Release();
    
    // returns bit value by uPos: 1/0/-1(err)
    int GetBit(size_t uPos) const;

    // returns: 0/-1(err)
    int SetBit(size_t uPos, bool bIsOn);

    void SetAllBit(bool bIsOn);

    void Clear();

protected:
    uint8_t* m_pBuf;
    size_t m_uBitCount;

};

#include "TSBitmap.inl"

#endif	/* __TSBITMAP_H__ */

