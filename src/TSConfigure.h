/* 
 * File:   TSConfigure.h
 * Author: thunderliu
 *
 * Created on 2011年12月25日, 上午2:58
 */

#ifndef __TSCONFIGURE_H__
#define	__TSCONFIGURE_H__


typedef struct _CFG_KEY
{

    _CFG_KEY()
    {
        memset(sz, 0, sizeof (sz));
    }
    char sz[64];
} CFG_KEY;

typedef struct _CFG_MAPPED
{

    _CFG_MAPPED()
    {
        memset(sz, 0, sizeof (sz));
    }
    char sz[1024];
} CFG_MAPPED;

typedef CHashMap<CFG_KEY, CFG_MAPPED> CCfgHashBase;

class CCfgHash : public CHashMap<CFG_KEY, CFG_MAPPED>
{
protected:
    virtual bool NodeMatch(const CFG_KEY& rKey, CCfgHashBase::CHashNode& rNode);

public:
    bool ShowCfg(CCfgHashBase::CHashNode& rNode, void* pParam);
};

class CConfigureBase
{
public:
    virtual ~CConfigureBase();
    
    virtual bool Reload();

    virtual bool ReadString(const char* pKey, CStr& roItem);
    virtual bool ReadInt(const char* pKey, int& iItem);
    virtual bool ReadLong(const char* pKey, long& lItem);
    virtual bool ReadULong(const char* pKey, size_t& uItem);
    virtual bool ReadUInt64(const char* pKey, uint64_t& ddwItem);
    virtual bool ReadUInt32(const char* pKey, uint32_t& dwItem);
    virtual bool ReadUInt16(const char* pKey, uint16_t& wItem);

    virtual bool OnLoadConfigure() = 0;
};

class CConfigureFile : public CConfigureBase
{
public:
    CConfigureFile();
    virtual ~CConfigureFile();

    bool LoadFromFile(const char* pFileName);
    virtual bool Reload();

public:
    virtual bool ReadString(const char* pKey, CStr& roItem);
    virtual bool ReadInt(const char* pKey, int& iItem);
    virtual bool ReadLong(const char* pKey, long& lItem);
    virtual bool ReadULong(const char* pKey, size_t& uItem);
    virtual bool ReadUInt64(const char* pKey, uint64_t& ddwItem);
    virtual bool ReadUInt32(const char* pKey, uint32_t& dwItem);
    virtual bool ReadUInt16(const char* pKey, uint16_t& wItem);

    virtual bool OnLoadConfigure();

protected:
    CFG_MAPPED* GetItem(const char* pKey);
    bool ItemToVal(const char* pKey, void* pVal, size_t uSize);

public:
    CCfgHash m_oCfgMap;
    CStr m_oCfgFileName;
};


#if 0
#define CFG_OBJ __oCfg_A6F39C5B__

#define CFG_REF(type, cfg) type& BH_OBJ = cfg

#define CFG_STRING(key, item, size)     __oCfg_A6F39C5B__.ReadString((key), (__oCfg_A6F39C5B__.str), (size))
#define CFG_INT(key, item)              __oCfg_A6F39C5B__.ReadInt((key), (__oCfg_A6F39C5B__.item))
#define CFG_LONG(key, item)             __oCfg_A6F39C5B__.ReadLong((key), (__oCfg_A6F39C5B__.item))
#define CFG_ULONG(key, item)            __oCfg_A6F39C5B__.ReadULong((key), (__oCfg_A6F39C5B__.item))
#define CFG_UINT64(key, item)           __oCfg_A6F39C5B__.ReadUInt64((key), (__oCfg_A6F39C5B__.item))
#define CFG_UINT32(key, item)           __oCfg_A6F39C5B__.ReadUInt32((key), (__oCfg_A6F39C5B__.item))
#define CFG_UINT16(key, item)           __oCfg_A6F39C5B__.ReadUInt16((key), (__oCfg_A6F39C5B__.item))
#endif



#include "TSConfigure.inl"

#endif	/* __TSCONFIGURE_H__ */

