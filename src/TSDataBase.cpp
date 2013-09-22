/* 
 * File:   TSDataBase.cpp
 * Author: thunderliu
 * 
 * Created on 2011年12月25日, 上午3:31
 */

#include "TSPlatform.h"
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include "TSDebug.h"
#include "TSDataBase.h"

#ifdef TSNETFW_FEATURE_MYSQL

// CDbMysql

bool CDbMySQL::Init(const char* pDbHost, int iDbPort, const char* pDbUser, const char* pDbPass, const char* pDbName, const char* pDbCharacterSet)
{
    m_szDbHost.Copy(pDbHost);
    m_iDbPort = iDbPort;
    m_szDbUser.Copy(pDbUser);
    m_szDbPass.Copy(pDbPass);
    m_szDbName.Copy(pDbName);
    m_szDbCharacterSet.Copy(pDbCharacterSet);

    return true;
}

bool CDbMySQL::Connect()
{
    if (m_pMysqlSock)
    {
        return false;
    }

    m_pMysqlSock = mysql_init(NULL);

    LOG_ERR("DBG | mysql connect %s:%d", m_szDbHost.GetBuffer(), m_iDbPort);
    if (!mysql_real_connect(m_pMysqlSock, m_szDbHost, m_szDbUser, m_szDbPass, m_szDbName, m_iDbPort, NULL, CLIENT_FOUND_ROWS))
    {
        LOG_ERR("ERR | %s, %s", mysql_error(m_pMysqlSock), strerror(errno));
        mysql_close(m_pMysqlSock);
        m_pMysqlSock = NULL;
        return false;
    }

    SetCharacterSet(m_szDbCharacterSet);

    return true;
}

bool CDbMySQL::Close()
{
    if (!m_pMysqlSock)
    {
        return false;
    }

    mysql_close(m_pMysqlSock);
    m_pMysqlSock = NULL;

    return true;
}

bool CDbMySQL::Query(const char* pQuery)
{
    if (!IsInUse())
    {
        if (!Connect())
        {
            return false;
        }
    }

    if (mysql_real_query(m_pMysqlSock, pQuery, strnlen(pQuery, 1024)) < 0)
    {
        if (mysql_errno(m_pMysqlSock) == CR_SERVER_GONE_ERROR)
        {
            Close();
            if (!Connect())
            {
                return false;
            }

            if (mysql_real_query(m_pMysqlSock, pQuery, strnlen(pQuery, 1024)) < 0)
            {

                return false;
            }

            return true;
        }

        return false;
    }

    return true;
}

CDbMySQLResult* CDbMySQL::GetResult()
{
    MYSQL_RES* pRes = mysql_store_result(m_pMysqlSock);
    if (!pRes)
    {
        return NULL;
    }

    return new CDbMySQLResult(pRes);
}

#endif // TSNETFW_FEATURE_MYSQL

