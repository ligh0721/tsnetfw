/* 
 * File:   TSScriptEngine.h
 * Author: thunderliu
 *
 * Created on 2012年5月7日, 下午4:30
 */

#ifndef __TSSCRIPTENGINE_H__
#define	__TSSCRIPTENGINE_H__


#include "TSThread.h"

class CScriptEngine
{
public:
    virtual ~CScriptEngine();

    virtual bool RunFile(const char* pFile, CStr* pErr = NULL) = 0;
    virtual bool RunString(const char* pStr, CStr* pErr = NULL) = 0;
};

class CAsyncScriptEngine : public CScriptEngine
{
protected:

    class CAsyncRunFileThread : public CThread
    {
    public:
        CAsyncRunFileThread(CAsyncScriptEngine* pAse, const char* pFile, const void* pParam);
        virtual ~CAsyncRunFileThread();

    protected:
        virtual long ThreadProc();
        virtual void OnExit(void* pExitCode);

    public:
        CAsyncScriptEngine* m_pAse;
        CStr m_szFile;
        const void* m_pParam;
        CStr m_oErr;
    };

    class CAsyncRunStringThread : public CThread
    {
    public:
        CAsyncRunStringThread(CAsyncScriptEngine* pAse, const char* pStr, const void* pParam);
        virtual ~CAsyncRunStringThread();

    protected:
        virtual long ThreadProc();
        virtual void OnExit(void* pExitCode);

    public:
        CAsyncScriptEngine* m_pAse;
        CStr m_szStr;
        const void* m_pParam;
        CStr m_oErr;
    };

public:
    CThread* AsyncRunFile(const char* pFile, const void* pParam = NULL);
    CThread* AsyncRunString(const char* pStr, const void* pParam = NULL);

protected:
    virtual void OnRunFileOk(CThread* pThrd, const char* pFile, const void* pParam);
    virtual void OnRunFileError(CThread* pThrd, const char* pFile, const CStr& roErr, const void* pParam);
    virtual void OnRunFileCancel(CThread* pThrd, const char* pFile, const void* pParam);

    virtual void OnRunStringOk(CThread* pThrd, const char* pStr, const void* pParam);
    virtual void OnRunStringError(CThread* pThrd, const char* pStr, const CStr& roErr, const void* pParam);
    virtual void OnRunStringCancel(CThread* pThrd, const char* pStr, const void* pParam);

};



#ifdef TSNETFW_FEATURE_LUA
#ifdef __cplusplus
extern "C"
{
#endif
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#ifdef __cplusplus
}
#endif

class CLuaSe : public CAsyncScriptEngine
{
public:
    static const int STACK_POS_TOP;
    static const int CALL_MULT_RET;
    static const char* EOF_MARK;
    static const size_t EOF_MARK_LEN;

public:
    CLuaSe(bool bNewHandle = true);
    CLuaSe(lua_State* pL);
    virtual ~CLuaSe();

    lua_State* GetHandle();
    bool Attach(lua_State* pL);
    void Detach();
    bool Init(bool bLoadLibs = true);

    virtual bool RunFile(const char* pFile, CStr* pErr = NULL);
    virtual bool RunString(const char* pStr, CStr* pErr = NULL);

    void RegisterCFunction(const char* pName, lua_CFunction pLuaCFunc);

    const char* ToString(int iStackPos, size_t* pLen = NULL);
    lua_Number ToNumber(int iStackPos, int* pIsNum = NULL);
    lua_Integer ToInteger(int iStackPos, int* pIsNum = NULL);
    bool ToBoolean(int iStackPos);
    lua_CFunction ToCFunction(int iStackPos);

    const char* CheckString(uint32_t dwArgvIndex, size_t* pLen = NULL);
    lua_Number CheckNumber(uint32_t dwArgvIndex);
    lua_Integer CheckInteger(uint32_t dwArgvIndex);

    const char* GetGlobalString(const char* pVar, size_t* pLen = NULL);
    lua_Number GetGlobalNumber(const char* pVar, int* pIsNum = NULL);
    lua_Integer GetGlobalInteger(const char* pVar, int* pIsNum = NULL);
    bool GetGlobalBoolean(const char* pVar);
    lua_CFunction GetGlobalCFunction(const char* pVar);

    void PushGetGlobal(const char* pVar);
    void PushString(const char* pStr);
    void PushString(const char* pStr, size_t uLen);
    void PushNumber(lua_Number dNum);
    void PushInteger(lua_Integer lNum);
    void PushBoolean(bool bBool);
    void PushCFunction(lua_CFunction pFun);
    void PushNil();

    void PopSetGlobal(const char* pVar);
    void Pop(uint32_t dwCount);
    int ThrowError(bool bTellWhere, const char* pFmt, ...);
    int GetTopPos() const;
    void Insert(int iStackPos);
    void Concat(uint32_t dwCount);
    void Remove(int iStackPos);
    bool Call(uint32_t dwArgCount, int iRetCount, CStr* pErr = NULL);
    static bool HasErrorEofMark(const CStr& roErr);
    
#if LUA_VERSION_NUM >= 502
    lua_Unsigned ToUnsigned(int iStackPos, int* pIsNum = NULL);
    lua_Unsigned CheckUnsigned(uint32_t dwArgvIndex);
    lua_Unsigned GetGlobalUnsigned(const char* pVar, int* pIsNum = NULL);
    void PushUnsigned(lua_Unsigned dwNum);
    bool Equal(int iStackPos1, int iStackPos2);
    bool LessThan(int iStackPos1, int iStackPos2);
    bool LessOrEqual(int iStackPos1, int iStackPos2);
    void Copy(int iStackPosSrc, int iStackPosDst);
#endif // LUA_VERSION_NUM


protected:
    lua_State* m_pL;
    bool m_bNewHandle;
};

#define DEF_LUACFUNC(func)      int func(lua_State* L)
#define DEF_LUAOBJ(type, obj)   type obj(L)
#endif // TSNETFW_FEATURE_LUA





#include "TSScriptEngine.inl"

#endif	/* __TSSCRIPTENGINE_H__ */

