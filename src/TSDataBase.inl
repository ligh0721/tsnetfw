/* 
 * File:   TSDataBase.inl
 * Author: thunderliu
 *
 * Created on 2011年12月25日, 上午3:32
 */

#ifndef __TSDATABASE_INL__
#define	__TSDATABASE_INL__

#include <assert.h>
#include <stdlib.h>

#include "TSDataBase.h"



// CDataBase

inline CDataBase::CDataBase()
{
}

inline CDataBase::~CDataBase()
{
}


#ifdef TSNETFW_FEATURE_MYSQL
// CDbMysql

inline CDbMySQL::CDbMySQL()
: m_pMysqlSock(NULL)
, m_szDbHost(64)
, m_szDbUser(64)
, m_szDbPass(64)
, m_szDbName(64)
, m_szDbCharacterSet(16)
{

}

inline CDbMySQL::~CDbMySQL()
{
    if (m_pMysqlSock)
    {
        mysql_close(m_pMysqlSock);
        m_pMysqlSock = NULL;
    }
}

inline bool CDbMySQL::IsInUse() const
{
    return m_pMysqlSock != NULL;
}

inline bool CDbMySQL::SetCharacterSet(const char* pCharacterSet)
{
    return mysql_set_character_set(m_pMysqlSock, pCharacterSet) >= 0;
}

inline uint32_t CDbMySQL::GetError() const
{
    return mysql_errno(m_pMysqlSock);
}

inline const char* CDbMySQL::GetErrorString() const
{
    return mysql_error(m_pMysqlSock);
}


// CDbMysqlResult

inline CDbMySQLResult::CDbMySQLResult(MYSQL_RES* pRes) :
m_pRes(pRes)
{
    m_dwLineCount = mysql_num_rows(m_pRes);
    m_dwRowCount = mysql_num_fields(m_pRes);
}

inline CDbMySQLResult::~CDbMySQLResult()
{
    mysql_free_result(m_pRes);
}

inline void CDbMySQLResult::Release()
{
    delete this;
}

inline uint32_t CDbMySQLResult::GetLineCount() const
{
    return m_dwLineCount;
}

inline uint32_t CDbMySQLResult::GetRowCount() const
{
    return m_dwRowCount;
}

inline bool CDbMySQLResult::GetNextLine()
{
    m_pRow = mysql_fetch_row(m_pRes);
    return m_pRow != NULL;
}

inline char* CDbMySQLResult::GetData(uint32_t dwNum)
{
    if (dwNum < m_dwRowCount)
    {
        return m_pRow[dwNum];
    }

    return NULL;
}

inline long CDbMySQLResult::GetLong(uint32_t dwNum)
{
    char* pData = GetData(dwNum);
    assert(pData);
    return atol(pData);
}


#endif // TSNETFW_FEATURE_MYSQL

#endif	/* __TSDATABASE_INL__ */

