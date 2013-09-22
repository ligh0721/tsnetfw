/* 
 * File:   TSString.h
 * Author: thunderliu
 *
 * Created on 2011年12月27日, 下午4:59
 */

#ifndef __TSSTRING_H__
#define	__TSSTRING_H__

#include <stdio.h>
#include "TSList.h"

class CStr
{
public:
    CStr(size_t uSize);
    CStr(const char* pBuf, size_t uSize, bool bAllocMem = false);
    virtual ~CStr();

    char* GetBuffer(size_t uIndex = 0);
    const char* GetBuffer(size_t uIndex = 0) const;
    char& GetChar(size_t uIndex);
    const char& GetChar(size_t uIndex) const;
    size_t GetSize() const;

    void SetChars(char cToSet);
    size_t GetLength() const;
    char* Copy(const char* pSrcStr);
    int Compare(const char* pStr);
    char* Concat(const char* pStr);
    int Format(const char* pFmt, ...);

    operator char*();
    operator const char*() const;


protected:
    char* m_pBuf;
    size_t m_uSize;
    bool m_bAllocMem;
};

class CStringHelper
{
public:
    static char* PickOutWord(const char* pStr, char cLeftPair, char cRightPair, const char* pLeftSeps, const char* pRightSeps, char* pSep, uint32_t* pWordLen);
    static bool StrToVal(const CStr& roStr, void* pVal, size_t uValSize);
};

class CIoInterface
{
public:
    virtual ~CIoInterface();
    virtual int SetOrGetIo(FILE* pIn, FILE* pOut, FILE** ppOrgIn = NULL, FILE** ppOrgOut = NULL) = 0;
    virtual int ReadString(CStr& roStr) = 0;
    virtual int ReadDword(uint32_t& dwVal) = 0;
    virtual int ReadQword(uint64_t& ddwVal) = 0;
};

class CStringLine : public CIoInterface
{
public:
    CStringLine(const char* pLine = NULL);

    void Attach(const char* pLine);
    virtual const char* GetNextStringPos(uint32_t& dwLen);

    virtual int SetOrGetIo(FILE* pIn, FILE* pOut, FILE** ppOrgIn = NULL, FILE** ppOrgOut = NULL);
    virtual int ReadString(CStr& roStr);
    virtual int ReadDword(uint32_t& dwVal);
    virtual int ReadQword(uint64_t& ddwVal);

    const char* GetPos();
    const char* GetStringAddr() const;
    bool IsEnd();

protected:
    const char* m_pLine;
    const char* m_pPos;
    bool m_bIsEnd;

};

class CCmdStrLn : public CStringLine
{
public:
    CCmdStrLn(int iArgc = 0, const char** ppArgv = NULL);

    virtual int ReadString(CStr& roStr);
    void SetArg(int iArgc, const char** ppArgv);
    int GetArgc() const;
    const char** GetArgv() const;


protected:
    int m_iArgc;
    const char** m_ppArgv;
};

#ifdef TSNETFW_FEATURE_READLINE

class CReadline : public CCmdStrLn
{
public:
    CReadline();
    static CReadline* Instance();
    static void Instance(CReadline* pInstance);

    virtual int SetOrGetIo(FILE* pIn, FILE* pOut, FILE** ppOrgIn = NULL, FILE** ppOrgOut = NULL);

    virtual int ReadString(CStr& roStr);
    int ReadStringEnd(CStr& roStr);

    void AddToReadline(const char* pName);
    void SetPrompt(const char* pPrompt);
    int Readline(CStr& roLine, const char* pPrompt = NULL);


protected:
    static char* CommandGenerator(const char* pText, int iState);
    static char** CommandCompletion(const char* pText, int iStart, int iEnd);
    static void InitializeReadline();

protected:
    typedef char STR_CMD[64];
    typedef CList<STR_CMD> CCmdList;
    static CReadline* m_pInstance;
    CCmdList m_lstCmd;
    CStr m_szPrompt;


};
#endif // TSNETFW_FEATURE_READLINE


#include "TSString.inl"

#endif	/* __TSSTRING_H__ */

