// utex.h
#pragma once
// Ётот файл содержит утилиты дл€ работы с UTF-16 и UTF-8 текстом
//#include "fixalloc.h"
#include "pstdint.h"
#include "StdAfx.h"
#include "valuework.hpp"

struct u8text {
    static void init();

    static void toUtf8(CString& strWinText) {

        char *zIn = (char *)(LPCSTR)strWinText;
        uint32_t nIn = strWinText.GetLength();

        // 613497 CString
        // 909091 sqlite3_malloc
        // 917431 malloc
        //char *zOut = (char *)sqlite3_malloc(nIn*3+3);
        char *zOut = (char *)malloc(nIn*3+3);
        //CString cOut;
        //char *zOut = (char *)(LPCSTR)cOut.GetBufferSetLength(nIn*3+3);
        uint32_t nOut=0;

        uint32_t i;
        for(i=0; i<nIn; i++) {
            const uint8_t s = zIn[i];
            //const uint8_t d = s & 0x80;
            if(s & 0x80) { //909091 до 917431
            //if(s >= 128) { // 909091
                const uint32_t us = w12512u[s & 0x7F];
                *((uint32_t*)(zOut+nOut)) = us;
                if(us > 0xFFFF) {
                    nOut+=3;
                } else {
                    nOut+=2;
                }
            } else {
                zOut[nOut++] = (uint8_t)s;
            }
        }
        if(nIn != nOut) {
            memcpy(strWinText.GetBufferSetLength(nOut), zOut, nOut);
        }
        //sqlite3_free(zOut);
        free(zOut);
    }

    static CString  fromUtf8(LPCSTR strSrc);
    static void  fromUtf8(LPCSTR strSrc, CString& res);
    static int _1Ccollate(void*, int l1, const void* str1, int l2, const void* str2);
    static int _1Ccollate2(void*, int l1, const void* str1, int l2, const void* str2);
    static int compareRtrimNoCase(LPCSTR str1, LPCSTR str2);
    static int compareRtrim(LPCSTR str1, LPCSTR str2);
    static int compareLen(LPCSTR str1, LPCSTR str2, DWORD len) {
        return memcmp(str1, str2, len);
    }
    static int compareNoCaseLen(LPCSTR str1, LPCSTR str2, DWORD len);
    static void dbUpper(CString& str);

     //__declspec(nothrow) __fastcall
    static CMapPtrToPtr uw1251;
    static const DWORD w12512u[128];
    //static const uint8_t upper[256];
  private:

//static CMapPtrToPtr uw1251;
};

void baseStr2Id(sqlite3_context* pCtx, int nParam, sqlite3_value** ppArgs);
void baseId2Str(sqlite3_context* pCtx, int nParam, sqlite3_value** ppArgs);

typedef unsigned __int64 id8;
id8 id9to8(LPCSTR pID);
id8 objIDto8(const CObjID& oid);
void id8to9(id8 id, LPSTR ptr);

struct ObjIDSet {
    ObjIDSet();
    ~ObjIDSet();
    void insert(id8 id);
    BOOL exist(id8 id);

    DWORD count();

    id8 first();
    void remove(id8 id);
    void doInsert(sqlite3_stmt* pInsert);
  protected:
    enum {hashSize = 117};

    struct assoc {
        assoc(assoc* n, id8 i) : next(n), id(i) {}
        id8 id;
        assoc* next;
    };
    assoc* ppAssoc[hashSize];
    DWORD cnt;
};


