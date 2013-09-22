/* 
 * File:   TSScriptEngine.cpp
 * Author: thunderliu
 * 
 * Created on 2012年5月7日, 下午4:30
 */

#include "TSPlatform.h"
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "TSString.h"
#include "TSScriptEngine.h"



// CAsyncScriptEngine::CAsyncRunFileThread

void CAsyncScriptEngine::CAsyncRunFileThread::OnExit(void* pExitCode)
{
    switch ((long)pExitCode)
    {
    case (long)true:
        m_pAse->OnRunFileOk(this, m_szFile, m_pParam);
        break;

    case (long)false:
        m_pAse->OnRunFileError(this, m_szFile, m_oErr, m_pParam);
        break;

    case -1:
        m_pAse->OnRunFileCancel(this, m_szFile, m_pParam);
        break;

    default:
        ;
    }

    //delete this;
}


// CAsyncScriptEngine::CAsyncRunStringThread

void CAsyncScriptEngine::CAsyncRunStringThread::OnExit(void* pExitCode)
{
    switch ((long)pExitCode)
    {
    case (long)true:
        m_pAse->OnRunStringOk(this, m_szStr, m_pParam);
        break;

    case (long)false:
        m_pAse->OnRunStringError(this, m_szStr, m_oErr, m_pParam);
        break;

    case -1:
        m_pAse->OnRunStringCancel(this, m_szStr, m_pParam);
        break;

    default:
        ;
    }

    //delete this;
}

#ifdef TSNETFW_FEATURE_LUA
// CLuaSe

const int CLuaSe::STACK_POS_TOP = -1;
const int CLuaSe::CALL_MULT_RET = -1;
const char* CLuaSe::EOF_MARK = "<eof>";
const size_t CLuaSe::EOF_MARK_LEN = strlen(EOF_MARK);

bool CLuaSe::Init(bool bLoadLibs)
{
    assert(m_pL);
    if (bLoadLibs)
    {
        luaL_openlibs(m_pL);
    }

    return true;
}

bool CLuaSe::Attach(lua_State* pL)
{
    if (m_pL && m_bNewHandle)
    {
        lua_close(m_pL);
    }

    assert(pL);
    m_pL = pL;
    m_bNewHandle = false;
    return true;
}

bool CLuaSe::RunFile(const char* pFile, CStr* pErr)
{
    assert(m_pL);
    assert(pFile);

    if (luaL_dofile(m_pL, pFile))
    {
        if (pErr)
        {
            pErr->Copy(lua_tostring(m_pL, -1));
        }
        lua_pop(m_pL, -1);

        return false;
    }

    return true;
}

bool CLuaSe::RunString(const char* pStr, CStr* pErr)
{
    assert(m_pL);
    assert(pStr);

    if (luaL_dostring(m_pL, pStr))
    {
        if (pErr)
        {
            pErr->Copy(lua_tostring(m_pL, -1));
        }
        lua_pop(m_pL, -1);

        return false;
    }

    return true;
}

int CLuaSe::ThrowError(bool bTellWhere, const char* pFmt, ...)
{
    assert(m_pL);
    va_list argp;
    va_start(argp, pFmt);
    if (bTellWhere)
    {
        luaL_where(m_pL, 1);
    }
    lua_pushvfstring(m_pL, pFmt, argp);
    va_end(argp);
    if (bTellWhere)
    {
        lua_concat(m_pL, 2);
    }

    return lua_error(m_pL);
}

bool CLuaSe::Call(uint32_t dwArgCount, int iRetCount, CStr* pErr)
{
    assert(m_pL);
    if (lua_pcall(m_pL, dwArgCount, iRetCount, 0))
    {
        if (pErr)
        {
            pErr->Copy(lua_tostring(m_pL, -1));
        }
        lua_pop(m_pL, -1);

        return false;
    }

    return true;
}

bool CLuaSe::HasErrorEofMark(const CStr& roErr)
{
    long lOff = roErr.GetLength() - EOF_MARK_LEN;
    if (lOff >= 0 && !strcmp(roErr.GetBuffer(lOff), EOF_MARK))
    {
        return true;
    }

    return false;
}

#endif // TSNETFW_FEATURE_LUA

