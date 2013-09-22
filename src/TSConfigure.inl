/* 
 * File:   TSConfigure.inl
 * Author: thunderliu
 *
 * Created on 2011年12月25日, 上午2:59
 */

#ifndef __TSCONFIGURE_INL__
#define	__TSCONFIGURE_INL__

#include "TSConfigure.h"
#include "TSHash.h"
#include "TSDebug.h"



// CString64Hash

inline bool CCfgHash::NodeMatch(const CFG_KEY& rKey, CCfgHashBase::CHashNode& rNode)
{
    return !strncmp(rKey.sz, rNode.tKey.sz, sizeof (CFG_KEY));
}

inline bool CCfgHash::ShowCfg(CCfgHashBase::CHashNode& rNode, void* pParam)
{
    fprintf(CLog::GetOutFile(), "%s = %s\n", static_cast<char*>(rNode.tKey.sz), static_cast<char*>(rNode.tMapped.sz));
    return true;
}


// CConfigureBase

inline CConfigureBase::~CConfigureBase()
{
}

inline bool CConfigureBase::Reload()
{
    return false;
}

inline bool CConfigureBase::ReadString(const char* pKey, CStr& roItem)
{
    return false;
}

inline bool CConfigureBase::ReadInt(const char* pKey, int& iItem)
{
    return false;
}

inline bool CConfigureBase::ReadLong(const char* pKey, long& lItem)
{
    return false;
}

inline bool CConfigureBase::ReadULong(const char* pKey, size_t& uItem)
{
    return false;
}

inline bool CConfigureBase::ReadUInt64(const char* pKey, uint64_t& ddwItem)
{
    return false;
}

inline bool CConfigureBase::ReadUInt32(const char* pKey, uint32_t& dwItem)
{
    return false;
}

inline bool CConfigureBase::ReadUInt16(const char* pKey, uint16_t& wItem)
{
    return false;
}


// CConfigureFile

inline CConfigureFile::CConfigureFile()
: m_oCfgFileName(256)
{
}

inline CConfigureFile::~CConfigureFile()
{
}

inline bool CConfigureFile::Reload()
{
    return LoadFromFile(m_oCfgFileName);
}

inline bool CConfigureFile::ReadInt(const char* pKey, int& iItem)
{
    return ItemToVal(pKey, &iItem, sizeof (iItem));
}

inline bool CConfigureFile::ReadLong(const char* pKey, long& lItem)
{
    return ItemToVal(pKey, &lItem, sizeof (lItem));
}

inline bool CConfigureFile::ReadULong(const char* pKey, size_t& uItem)
{
    return ItemToVal(pKey, &uItem, sizeof (uItem));
}

inline bool CConfigureFile::ReadUInt64(const char* pKey, uint64_t& ddwItem)
{
    return ItemToVal(pKey, &ddwItem, sizeof (ddwItem));
}

inline bool CConfigureFile::ReadUInt32(const char* pKey, uint32_t& dwItem)
{
    return ItemToVal(pKey, &dwItem, sizeof (dwItem));
}

inline bool CConfigureFile::ReadUInt16(const char* pKey, uint16_t& wItem)
{
    return ItemToVal(pKey, &wItem, sizeof (wItem));
}

inline bool CConfigureFile::OnLoadConfigure()
{
    return false;
}


#endif	/* __TSCONFIGURE_INL__ */

