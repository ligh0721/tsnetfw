/* 
 * File:   TSScriptEngine.inl
 * Author: thunderliu
 *
 * Created on 2012年5月7日, 下午4:31
 */

#ifndef __TSSCRIPTENGINE_INL__
#define	__TSSCRIPTENGINE_INL__

#include "TSScriptEngine.h"



// CScriptEngine

inline CScriptEngine::~CScriptEngine()
{
}


// CAsyncScriptEngine

inline CThread* CAsyncScriptEngine::AsyncRunFile(const char* pFile, const void* pParam)
{
    CAsyncRunFileThread* pThrd = new CAsyncRunFileThread(this, pFile, pParam);
    pThrd->Start();
    return pThrd;
}

inline CThread* CAsyncScriptEngine::AsyncRunString(const char* pStr, const void* pParam)
{
    CAsyncRunStringThread* pThrd = new CAsyncRunStringThread(this, pStr, pParam);
    pThrd->Start();
    return pThrd;
}

inline void CAsyncScriptEngine::OnRunFileOk(CThread* pThrd, const char* pFile, const void* pParam)
{
}

inline void CAsyncScriptEngine::OnRunFileError(CThread* pThrd, const char* pFile, const CStr& roErr, const void* pParam)
{
}

inline void CAsyncScriptEngine::OnRunFileCancel(CThread* pThrd, const char* pFile, const void* pParam)
{
}

inline void CAsyncScriptEngine::OnRunStringOk(CThread* pThrd, const char* pStr, const void* pParam)
{
}

inline void CAsyncScriptEngine::OnRunStringError(CThread* pThrd, const char* pStr, const CStr& roErr, const void* pParam)
{
}

inline void CAsyncScriptEngine::OnRunStringCancel(CThread* pThrd, const char* pStr, const void* pParam)
{
}


// CAsyncScriptEngine::CAsyncRunFileThread

inline CAsyncScriptEngine::CAsyncRunFileThread::CAsyncRunFileThread(CAsyncScriptEngine* pAse, const char* pFile, const void* pParam)
: m_pAse(pAse)
, m_szFile(pFile, strlen(pFile) + 1, true)
, m_pParam(pParam)
, m_oErr(128)
{
}

inline CAsyncScriptEngine::CAsyncRunFileThread::~CAsyncRunFileThread()
{
}

inline long CAsyncScriptEngine::CAsyncRunFileThread::ThreadProc()
{
    pthread_detach(m_uTid);
    return m_pAse->RunFile(m_szFile, &m_oErr);
}


// CAsyncScriptEngine::CAsyncRunStringThread

inline CAsyncScriptEngine::CAsyncRunStringThread::CAsyncRunStringThread(CAsyncScriptEngine* pAse, const char* pStr, const void* pParam)
: m_pAse(pAse)
, m_szStr(pStr, strlen(pStr) + 1, true)
, m_pParam(pParam)
, m_oErr(128)
{
}

inline CAsyncScriptEngine::CAsyncRunStringThread::~CAsyncRunStringThread()
{
}

inline long CAsyncScriptEngine::CAsyncRunStringThread::ThreadProc()
{
    pthread_detach(m_uTid);
    return m_pAse->RunString(m_szStr, &m_oErr);
}


#ifdef TSNETFW_FEATURE_LUA
// CLuaSe

inline CLuaSe::CLuaSe(bool bNewHandle)
: m_bNewHandle(bNewHandle)
{
    if (m_bNewHandle)
    {
        m_pL = luaL_newstate();
        assert(m_pL);
    }
    else
    {
        m_pL = NULL;
    }
}

inline CLuaSe::CLuaSe(lua_State* pL)
: m_pL(pL)
, m_bNewHandle(false)
{
    assert(m_pL);
}

inline CLuaSe::~CLuaSe()
{
    if (m_bNewHandle)
    {
        assert(m_pL);
        lua_close(m_pL);
    }
}

inline lua_State* CLuaSe::GetHandle()
{
    return m_pL;
}

inline void CLuaSe::Detach()
{
    m_pL = NULL;
}

inline void CLuaSe::RegisterCFunction(const char* pName, lua_CFunction pLuaCFunc)
{
    assert(m_pL);
    lua_register(m_pL, pName, pLuaCFunc);
}

inline const char* CLuaSe::ToString(int iStackPos, size_t* pLen)
{
    assert(m_pL);
    return lua_tolstring(m_pL, iStackPos, pLen);
}

inline lua_Number CLuaSe::ToNumber(int iStackPos, int* pIsNum)
{
    assert(m_pL);
    return lua_tonumber(m_pL, iStackPos);
}

inline lua_Integer CLuaSe::ToInteger(int iStackPos, int* pIsNum)
{
    assert(m_pL);
    return lua_tointeger(m_pL, iStackPos);
}

inline bool CLuaSe::ToBoolean(int iStackPos)
{
    assert(m_pL);
    return (bool)lua_toboolean(m_pL, iStackPos);
}

inline lua_CFunction CLuaSe::ToCFunction(int iStackPos)
{
    assert(m_pL);
    return lua_tocfunction(m_pL, iStackPos);
}

inline const char* CLuaSe::CheckString(uint32_t dwArgvIndex, size_t* pLen)
{
    assert(m_pL);
    return luaL_checklstring(m_pL, dwArgvIndex, pLen);
}

inline lua_Number CLuaSe::CheckNumber(uint32_t dwArgvIndex)
{
    assert(m_pL);
    return luaL_checknumber(m_pL, dwArgvIndex);
}

inline lua_Integer CLuaSe::CheckInteger(uint32_t dwArgvIndex)
{
    assert(m_pL);
    return luaL_checkinteger(m_pL, dwArgvIndex);
}

inline const char* CLuaSe::GetGlobalString(const char* pVar, size_t* pLen)
{
    assert(m_pL);
    lua_getglobal(m_pL, pVar);
    const char* pRet = lua_tolstring(m_pL, STACK_POS_TOP, pLen);
    lua_pop(m_pL, 1);
    return pRet;
}

inline lua_Number CLuaSe::GetGlobalNumber(const char* pVar, int* pIsNum)
{
    assert(m_pL);
    lua_getglobal(m_pL, pVar);
    lua_Number dRet = lua_tonumber(m_pL, STACK_POS_TOP);
    lua_pop(m_pL, 1);
    return dRet;
}

inline lua_Integer CLuaSe::GetGlobalInteger(const char* pVar, int* pIsNum)
{
    assert(m_pL);
    lua_getglobal(m_pL, pVar);
    int lRet = lua_tointeger(m_pL, STACK_POS_TOP);
    lua_pop(m_pL, 1);
    return lRet;
}

inline bool CLuaSe::GetGlobalBoolean(const char* pVar)
{
    assert(m_pL);
    lua_getglobal(m_pL, pVar);
    bool bRet = lua_toboolean(m_pL, STACK_POS_TOP);
    lua_pop(m_pL, 1);
    return bRet;
}

inline lua_CFunction CLuaSe::GetGlobalCFunction(const char* pVar)
{
    assert(m_pL);
    lua_getglobal(m_pL, pVar);
    lua_CFunction pRet = lua_tocfunction(m_pL, STACK_POS_TOP);
    lua_pop(m_pL, 1);
    return pRet;
}

inline void CLuaSe::PushGetGlobal(const char* pVar)
{
    assert(m_pL);
    lua_getglobal(m_pL, pVar);
}

inline void CLuaSe::PushString(const char* pStr)
{
    assert(m_pL);
    lua_pushstring(m_pL, pStr);
}

inline void CLuaSe::PushString(const char* pStr, size_t uLen)
{
    assert(m_pL);
    lua_pushlstring(m_pL, pStr, uLen);
}

inline void CLuaSe::PushNumber(lua_Number dNum)
{
    assert(m_pL);
    lua_pushnumber(m_pL, dNum);
}

inline void CLuaSe::PushInteger(lua_Integer lNum)
{
    assert(m_pL);
    lua_pushinteger(m_pL, lNum);
}

inline void CLuaSe::PushBoolean(bool bBool)
{
    assert(m_pL);
    lua_pushboolean(m_pL, bBool);
}

inline void CLuaSe::PushCFunction(lua_CFunction pFun)
{
    assert(m_pL);
    lua_pushcfunction(m_pL, pFun);
}

inline void CLuaSe::PushNil()
{
    assert(m_pL);
    lua_pushnil(m_pL);
}

inline void CLuaSe::PopSetGlobal(const char* pVar)
{
    assert(m_pL);
    lua_setglobal(m_pL, pVar);
}

inline void CLuaSe::Pop(uint32_t dwCount)
{
    assert(m_pL);
    lua_pop(m_pL, dwCount);
}

inline int CLuaSe::GetTopPos() const
{
    assert(m_pL);
    return lua_gettop(m_pL);
}

inline void CLuaSe::Insert(int iStackPos)
{
    assert(m_pL);
    lua_insert(m_pL, iStackPos);
}

inline void CLuaSe::Concat(uint32_t dwCount)
{
    assert(m_pL);
    lua_concat(m_pL, dwCount);
}

inline void CLuaSe::Remove(int iStackPos)
{
    assert(m_pL);
    lua_remove(m_pL, iStackPos);
}

#if LUA_VERSION_NUM >= 502

inline lua_Unsigned CLuaSe::ToUnsigned(int iStackPos, int* pIsNum)
{
    assert(m_pL);
    return lua_tounsigned(m_pL, iStackPos);
}

inline lua_Unsigned CLuaSe::CheckUnsigned(uint32_t dwArgvIndex)
{
    assert(m_pL);
    return luaL_checkunsigned(m_pL, dwArgvIndex);
}

inline lua_Unsigned CLuaSe::GetGlobalUnsigned(const char* pVar, int* pIsNum)
{
    assert(m_pL);
    lua_getglobal(m_pL, pVar);
    lua_Unsigned dwRet = lua_tounsigned(m_pL, STACK_POS_TOP);
    lua_pop(m_pL, 1);
    return dwRet;
}

inline void CLuaSe::PushUnsigned(lua_Unsigned dwNum)
{
    assert(m_pL);
    lua_pushunsigned(m_pL, dwNum);
}

inline bool CLuaSe::Equal(int iStackPos1, int iStackPos2)
{
    assert(m_pL);
    return (bool)lua_compare(m_pL, iStackPos1, iStackPos2, LUA_OPEQ);
}

inline bool CLuaSe::LessThan(int iStackPos1, int iStackPos2)
{
    assert(m_pL);
    return (bool)lua_compare(m_pL, iStackPos1, iStackPos2, LUA_OPLT);
}

inline bool CLuaSe::LessOrEqual(int iStackPos1, int iStackPos2)
{
    assert(m_pL);
    return (bool)lua_compare(m_pL, iStackPos1, iStackPos2, LUA_OPLE);
}

inline void CLuaSe::Copy(int iStackPosSrc, int iStackPosDst)
{
    assert(m_pL);
    lua_copy(m_pL, iStackPosSrc, iStackPosDst);
}

#endif // LUA_VERSION_NUM


#endif // TSNETFW_FEATURE_LUA



#endif	/* __TSSCRIPTENGINE_INL__ */

