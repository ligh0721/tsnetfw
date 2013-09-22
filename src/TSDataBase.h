/* 
 * File:   TSDataBase.h
 * Author: thunderliu
 *
 * Created on 2011年12月25日, 上午3:31
 */

#ifndef __TSDATABASE_H__
#define	__TSDATABASE_H__

#include "TSString.h"

class CDataBase
{
public:
    CDataBase();
    virtual ~CDataBase();

    virtual bool Connect() = 0;
    virtual bool IsInUse() const = 0;
    virtual bool Close() = 0;

};

#ifdef TSNETFW_FEATURE_MYSQL
#include <mysql/mysql.h>
#include <mysql/errmsg.h>

class CDbMySQLResult;

class CDbMySQL : public CDataBase
{
public:
    CDbMySQL();
    virtual ~CDbMySQL();

    bool Init(const char* pDbHost, int iDbPort, const char* pDbUser, const char* pDbPass, const char* pDbName, const char* pDbCharacterSet);

    virtual bool Connect();
    virtual bool IsInUse() const;
    virtual bool Close();

    bool Query(const char* pQuery);
    bool SetCharacterSet(const char* pCharacterSet);

    CDbMySQLResult* GetResult();
    uint32_t GetError() const;
    const char* GetErrorString() const;

private:
    MYSQL *m_pMysqlSock;
    CStr m_szDbHost;
    int m_iDbPort;
    CStr m_szDbUser;
    CStr m_szDbPass;
    CStr m_szDbName;
    CStr m_szDbCharacterSet;

};

class CDbMySQLResult
{
    friend class CDbMySQL;

protected:
    CDbMySQLResult(MYSQL_RES* pRes);
    virtual ~CDbMySQLResult();

public:
    void Release();
    uint32_t GetLineCount() const;
    uint32_t GetRowCount() const;

    bool GetNextLine();
    char* GetData(uint32_t dwNum);
    long GetLong(uint32_t dwNum);

protected:
    MYSQL_RES* m_pRes;
    uint32_t m_dwLineCount;
    uint32_t m_dwRowCount;
    MYSQL_ROW m_pRow;
};

#endif // TSNETFW_FEATURE_MYSQL

#include "TSDataBase.inl"

#endif	/* __TSDATABASE_H__ */

