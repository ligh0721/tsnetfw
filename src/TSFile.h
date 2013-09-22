/* 
 * File:   TSFile.h
 * Author: thunderliu
 *
 * Created on 2013年6月4日, 下午4:42
 */

#ifndef __TSFILE_H__
#define	__TSFILE_H__

#include "TSIo.h"
#include <sys/mman.h>
#include <stdint.h>


class CFileMapping
{  
public:  
    CFileMapping();
    
    bool Init(const CIo* pIo, bool bReadOnly);
    bool Init(const char* pFileName, bool bReadOnly);
    bool Close();
    
    operator const void*() const;
    operator void*();
    size_t GetSize() const;
    void* GetAddress();
    
protected:
    void* m_pAddr;
    size_t m_uLen;
};

#include "TSFile.inl"

#endif	/* __TSFILE_H__ */

