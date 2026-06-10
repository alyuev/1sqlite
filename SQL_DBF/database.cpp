//database.cpp
#include "StdAfx.h"
#include <io.h>
#include "database.h"
#include "utex.h"
#include "vtab_info.h"
#include "ValueWork.hpp"
#include "resultloader.h"
#include "base64.h"
#include "../_1Common/ctxtree.h"

CString SQLiteBase::m_trace;

BL_INIT_CONTEXT(CSLDataBase);

extern "C" BOOL bNeedProfile = FALSE;
CString strProfile;
extern "C" void doProfileMsg(const char* msg) {
    //DoMsgLine("%s", mmNone, msg);
    CString tmp(msg);
    tmp.Replace("\r\n", "\n");
    strProfile += tmp;
}

int tick=0;
time_t StartTime;

NOTHROW int progress_func(void* p) {
    // /-\|
    tick+=1;
    CString stick;

    time_t t;
    time(&t);

    switch(tick) {
    case 1:
        stick = "+";
        break;
    case 2:
        stick = "++";
        break;
    case 3:
        stick = "+++";
        break;
    case 4:
        stick = "++++";
        break;
    case 5:
        stick = "+++++";
        break;
    case 6:
        tick=0;
        stick = "++++++";
        break;
    }

    //int memused = sqlite3_memory_used( );
    //DoStsLine("Выполнение запроса %i mem %i", tick,memused);
    //printf("%s",ctime(&t));
    DoStsLine("Выполнение запроса %i ",t);
    return NULL;
}

/*
void baseStr2Id(sqlite3_context* pCtx, int nParam, sqlite3_value** ppArgs) {
    CString text = (LPCSTR)sqlite3_value_text(ppArgs[0]);
    DWORD id = 0;
    const BYTE* ptr = (const BYTE*)(LPCSTR)text;
    for(;;) {
        DWORD s = (DWORD)*ptr;
        if(!s)
            break;
        id = id * 36 + (s >= 'A' ? s -'A' + 10 : s - '0');
        ptr++;
    }
    sqlite3_result_int(pCtx, id);
}
*/

/*
void baseId2Str(sqlite3_context* pCtx, int nParam, sqlite3_value** ppArgs) {
    DWORD id = sqlite3_value_int(ppArgs[0]), len = sqlite3_value_int(ppArgs[1]);
    CString text;
    BYTE* ptr = (BYTE*)text.GetBufferSetLength(len);
    memset(ptr, ' ', len);
    id2str(id, ptr, len);
    sqlite3_result_text(pCtx, text, len, SQLITE_TRANSIENT);
}
*/

static void compressFunc(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
) {
    const unsigned char *pIn;
    unsigned char *pOut;
    unsigned int nIn;
    unsigned long int nOut;
    unsigned char x[8];
    int rc;
    int32_t i, j;

    pIn = (unsigned char *)sqlite3_value_blob(argv[0]);

    nIn = sqlite3_value_bytes(argv[0]);
    nOut = 13 + nIn + (nIn+999)/1000;
    pOut = (unsigned char *)sqlite3_malloc( nOut+5 );

    for(i=4; i>=0; i--) {
        x[i] = (char)(nIn >> (7*(4-i)))&0x7f;
        if(i==0) // i>=0 не срабатывает...
            break;
    }
    for(i=0; i<4 && x[i]==0; i++) {}
    for(j=0; i<=4; i++, j++)
        pOut[j] = x[i];
    pOut[j-1] |= 0x80;
    rc = compress(&pOut[j], &nOut, pIn, nIn);
    //rc = compress2(&pOut[j], &nOut, pIn, nIn,9);
    if( rc==Z_OK ) {
        sqlite3_result_blob(context, pOut, nOut+j, sqlite3_free);
    } else {
        sqlite3_free(pOut);
    }
}

static void uncompressFunc(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
) {
    const unsigned char *pIn;
    unsigned char *pOut;
    unsigned int nIn;
    unsigned long int nOut;
    int rc;
    int i;

    pIn = (unsigned char *)sqlite3_value_blob(argv[0]);
    nIn = sqlite3_value_bytes(argv[0]);
    nOut = 0;
    for(i=0; i<nIn && i<5; i++) {
        nOut = (nOut<<7) | (pIn[i]&0x7f);
        if( (pIn[i]&0x80)!=0 ) {
            i++;
            break;
        }
    }
    pOut = (unsigned char *)sqlite3_malloc( nOut+1 );
    rc = uncompress(pOut, &nOut, &pIn[i], nIn-i);
    if( rc==Z_OK ) {
        sqlite3_result_blob(context, pOut, nOut, sqlite3_free);
    } else {
        sqlite3_free(pOut);
    }
}

//unsigned int strHash(const char *z){
void strHash(sqlite3_context* pCtx, int nParam, sqlite3_value** ppArgs) {
    const unsigned char* z = sqlite3_value_text(ppArgs[0]);
    //unsigned int len = sqlite3_value_int(ppArgs[1]);
    unsigned int h = 0;
    unsigned char c;
    char *buf;
    while( (c = (unsigned char)*z++)!=0 ) {    /*OPTIMIZATION-IF-TRUE*/
        /* Knuth multiplicative hashing.  (Sorting & Searching, p. 510).
        ** 0x9e3779b1 is 2654435761 which is the closest prime number to
        ** (2**32)*golden_ratio, where golden_ratio = (sqrt(5) - 1)/2. */
        //h += sqlite3UpperToLower[c];
        //h = (h<<3) ^ h ^ oasUpper2Lower[c];
        h += c;
        h *= 0x9e3779b1;
    }
    //printf("%04x", 4779); // gives 12ab
    buf = sqlite3_mprintf("%08X",h);
    sqlite3_result_text(pCtx, buf, 8, SQLITE_TRANSIENT);
    sqlite3_free(buf);
    //sqlite3_result_int64(pCtx,h);

}

// Функция, которая в отличие от coalesce, умеет возвращать не только
// первое не-нуль значение, но и любое заданное первым аргументом.
void coalesceEx(sqlite3_context* pCtx, int nParam, sqlite3_value** ppArgs) {
    if(nParam > 1) {
        int number = sqlite3_value_int(ppArgs[0]) - 1;
        if(number < nParam - 1) {
            for(int k = 1; k < nParam; k++) {
                if(SQLITE_NULL != sqlite3_value_type(ppArgs[k])) {
                    if(!number) {
                        sqlite3_result_value(pCtx, ppArgs[k]);
                        return;
                    }
                    number--;
                }
            }
        }
    }
    sqlite3_result_null(pCtx);
}

struct bindSqlParam {
    bindSqlParam(const CString& pn, int idxNum, sqlite3* b, sqlite3_stmt* s)
        :paramName(pn), nParamNum(idxNum), pBase(b), pStmt(s) {}

    const CString& paramName;
    int nParamNum;
    sqlite3* pBase;
    sqlite3_stmt* pStmt;

    void error(LPCSTR text) {
        CString err;
        err.Format("Ошибка установки sql-параметра %s - %s", (LPCSTR)paramName, text);
        CBLModule::RaiseExtRuntimeError(err, FALSE);
    }

    void bindText(const CString& text) {
        CString str(text);
        u8text::toUtf8(str);
        checkErr(sqlite3_bind_text(pStmt, nParamNum, str, str.GetLength(), SQLITE_TRANSIENT));
        //checkErr(sqlite3_bind_text(pStmt, nParamNum, str, -1, SQLITE_TRANSIENT));
    }

    void bindNumeric(const CValue* pValue) {
        //DoMsgLine("bindNumeric %s",mmNone,pValue->GetString());

        if(pValue->m_Number.m_nScaleLen)
            checkErr(sqlite3_bind_double(pStmt, nParamNum, pValue->m_Number.GetDouble()));
        else {
            long val = pValue->m_Number;
            if(0 != pValue->m_Number.CompareLong(val))
                checkErr(sqlite3_bind_int64(pStmt, nParamNum, _atoi64(pValue->Format())));
            else
                checkErr(sqlite3_bind_int(pStmt, nParamNum, val));
        }
    }

    void bindBlob(const CValue* pValue) {
        CBLContext *Blob_Context;
        int Blob_Method,RetVal;
        CValue* pArgs1[1];
        CValue* pArgs2[2];
        CValue vZero= 0.0;
        CValue v1= 1.0;
        CValue v0= 0.0;
        CValue vNull;
        int Blob_Size,Blob_Pointer,Blob_Method_Seek,Blob_Method_Read;
        CValue rValue;
        CValue pArgs;
        CValue vParam0;
        CValue vParam1;
        int SingleChar,RetValRead,RetValSeek;
        char *buf;

        //DoMsgLine("bindSqlParam bindBlob");

        Blob_Context = pValue->GetContext();

        Blob_Method=Blob_Context->FindMethod("Size");
        pArgs1[0]=&vNull;
        RetVal=Blob_Context->CallAsFunc(Blob_Method,rValue,pArgs1);
        Blob_Size=rValue.GetNumeric();
        //DoMsgLine("mBlobSize %i RetVal %i Size %i",mmNone,Blob_Method,RetVal,Blob_Size);

        buf = (char*)sqlite3_malloc(Blob_Size);

        Blob_Method_Seek=Blob_Context->FindMethod("Seek");
        pArgs2[0]=&v0;
        pArgs2[1]=&vNull;
        RetVal=Blob_Context->CallAsFunc(Blob_Method_Seek,rValue,pArgs2);
        Blob_Pointer=rValue.GetNumeric();
        //DoMsgLine("mBlobSeek %i RetVal %i Pointer %i",mmNone,Blob_Method_Seek,RetVal,Blob_Pointer);

        Blob_Method_Read=Blob_Context->FindMethod("ReadData");
        for(int Blob_Idx = 0; Blob_Idx < Blob_Size; Blob_Idx++) {
            CValue v0,v1;
            CValue* pArgs[2]= {&v0,&v1};
            RetValRead=Blob_Context->CallAsFunc(Blob_Method_Read,rValue,pArgs);
            SingleChar=(unsigned char)pArgs[0]->GetNumeric();
            buf[Blob_Idx]=(unsigned char)pArgs[0]->GetNumeric();
            //DoMsgLine("mBlobRead %i get %i/%i. RetVal %i Char %i",mmNone,Blob_Method_Read,Blob_Idx,Blob_Size,RetValRead,SingleChar);
        }
        checkErr(sqlite3_bind_blob(pStmt, nParamNum, buf, Blob_Size, SQLITE_TRANSIENT));
        sqlite3_free(buf);
    }

    void bindNull() {
        checkErr(sqlite3_bind_null(pStmt, nParamNum));
    }

    void bindFragment(LPCSTR) {
        error("Неверный модификатор");
    }

    void checkErr(int res) {
        if(SQLITE_OK != res) {
            CString err(u8text::fromUtf8(sqlite3_errmsg(pBase)));
            error(err);
        }
    }
};

struct tran_guard {
    int ac, err;
    sqlite3* _pDB;
    tran_guard(sqlite3* pDB) : _pDB(pDB), err(TRUE), ac(sqlite3_get_autocommit(pDB)) {
        if(ac)
            sqlite3_exec(_pDB, "begin", NULL, NULL, NULL);
    }
    ~tran_guard() {
        if(ac)
            sqlite3_exec(_pDB, err ? "rollback" : "commit", NULL, NULL, NULL);
    }
};

inline BOOL isIDequal(LPCSTR id1, LPCSTR id2) {
    return *(ui64*)id1 == *(ui64*)id2 && (DWORD)(BYTE)id1[8] == (DWORD)(BYTE)id2[8];
}

typedef unsigned __int64 id8;

/*
id8 id9to8(LPCSTR pID) {
    return ((*(id8*)(pID + 6) & 0xFFFFFF) << 32) | str2id((const BYTE*)pID, 6);
}

id8 objIDto8(const CObjID& oid) {
    return ((*(id8*)oid.DBSign.Sign) << 32) | oid.ObjID;
}

void id8to9(id8 id, LPSTR ptr) {
    *(DWORD*)ptr = '    ';
    *(DWORD*)(ptr + 4) = '    ';
    id2str((DWORD)id, (BYTE*)ptr, 6);
    *(DWORD*)(ptr + 6) = (DWORD)(id >> 32);
}
*/

/*
struct ObjIDSet {
    ObjIDSet() {
        memset(ppAssoc, 0, sizeof(ppAssoc));
        cnt = 0;
    }
    ~ObjIDSet() {
        if(cnt) {
            DWORD e = cnt;
            assoc** ppAss = ppAssoc;
            for(;;) {
                while(!*ppAss)
                    ppAss++;
                assoc* pAss = *ppAss++;
                while(pAss) {
                    assoc* pDel = pAss;
                    pAss = pAss->next;
                    delete pDel;
                    if(!--e)
                        return;
                }
            }
        }
    }
    void insert(id8 id) {
        DWORD pos = id % hashSize;
        assoc* pAss = ppAssoc[pos];
        while(pAss) {
            if(pAss->id == id)
                return;
            pAss = pAss->next;
        }
        ppAssoc[pos] = new assoc(ppAssoc[pos], id);
        cnt++;
    }
    BOOL exist(id8 id) {
        for(assoc* pAss = ppAssoc[id % hashSize]; pAss ; pAss = pAss->next)
            if(pAss->id == id)
                return TRUE;
        return FALSE;
    }

    DWORD count() {
        return cnt;
    }

    id8 first() {	// Не вызывать, если нет элементов!!!!
        for(assoc** ppA = ppAssoc; !*ppA; ppA++);
        return (*ppA)->id;
    }
    void remove(id8 id) {
        assoc** ppAss = ppAssoc + id % hashSize;
        for(;;) {
            if((*ppAss)->id == id) {
                assoc* pDel = *ppAss;
                *ppAss = pDel->next;
                delete pDel;
                cnt--;
                return;
            }
            ppAss = &(*ppAss)->next;
        }
    }
    void doInsert(sqlite3_stmt* pInsert) {
        if(!cnt)
            return;

        CString text;
        DWORD e = cnt;
        assoc** ppAss = ppAssoc;

        for(;;) {
            while(!*ppAss)
                ppAss++;
            for(assoc* pAss = *ppAss++; pAss; pAss = pAss->next) {
                id8to9(pAss->id, text.GetBufferSetLength(9));
                u8text::toUtf8(text);
                sqlite3_bind_text(pInsert, 1, text, text.GetLength(), SQLITE_TRANSIENT);
                sqlite3_step(pInsert);
                sqlite3_reset(pInsert);
                if(!--e)
                    return;
            }
        }
    }
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
*/

template<int nField>
struct keyObj : CKeyObj {
    keyObj(CIndex* pI) : CKeyObj(pI, 0, 0) {}
    char id[10];
    virtual void PrepareKey() {
        m_pStoreObj->FX_String(nField, id, 9, 1);
    }
};

inline void processRefValue(CValue* pValue, CStoreObj& store, keyObj<0>& key, ObjIDSet& elements, ObjIDSet& groups, LPSTR pRecIsFolder) {
    id8 id = objIDto8(pValue->m_ObjID);
    if(0x20202000000000 == id) {
        groups.insert(id);
        return;
    }
    id8to9(id, key.id);

    if(store.Goto(&key, ccE, 0)) {
        if((DWORD)(BYTE)*pRecIsFolder == '2')
            elements.insert(id);
        else
            groups.insert(id);
    }
}

void putvlReference(CSbCntTypeDef* pRefDef, CPtrArray* pVL, CValue* pVal, sqlite3_stmt* pInsert, BOOL bWithGroups, int Flags=1) {
    ObjIDSet elements;
    ObjIDSet groups;
    ObjIDSet processedGroups;

    CTableEx* pTable = (CTableEx*)pDataDict->GetTable(pRefDef->GetTableName());
    LPSTR pRecID = pTable->recordBuffer();
    LPSTR pRecPID = pRecID + 9;
    LPSTR pRecIsFolder = pRecID + 18;
    for(DWORD i = 2; ; i++)	{
        CField* pField = pTable->field(i);
        if(*(DWORD*)pField->szName == 'OFSI')
            break;
        pRecIsFolder += pField->sizeCField;
    }

    // Ищем по идшнику
    CIndex* pI = pTable->index(0);
    CStoreObj store(pTable, pI);
    keyObj<0> key(pI);
    if(pVL)	{
        CValueItem** ppItems = (CValueItem**)pVL->GetData();
        for(DWORD c = pVL->GetSize(); c--; ppItems++)
            processRefValue(&(*ppItems)->m_value, store, key, elements, groups, pRecIsFolder);
    } else {
        processRefValue(pVal, store, key, elements, groups, pRecIsFolder);
    }


    // Теперь будем разворачивать группы
    if(groups.count()) {
        CIndex* pI = pTable->GetIndex(1);
        CStoreObj store(pTable, pI);
        keyObj<1> key(pI);
        while(groups.count()) {
            id8 id = groups.first();
            processedGroups.insert(id);
            id8to9(id, key.id);

            int cnt=0;
            if(store.Goto(&key, ccGE, 0)) {
                while(isIDequal(key.id, pRecPID)) {
                    cnt++;
                    id8 eid = id9to8(pRecID);
                    if((DWORD)(BYTE)*pRecIsFolder == '2')
                        elements.insert(eid);
                    else if(!processedGroups.exist(eid))
                        groups.insert(eid);
                    if(!store.Goto(navNext, 0))
                        break;
                }
            }
            groups.remove(id);
        }
    }
    // И теперь запихаем все в таблицу
    if (Flags==1) {
        elements.doInsert(pInsert);
        if(bWithGroups)
            processedGroups.doInsert(pInsert);
    } else if (Flags==2) {
        processedGroups.doInsert(pInsert);
    } else if (Flags==3) {
        processedGroups.doInsert(pInsert);
        elements.doInsert(pInsert);
    }
};

struct keyByPlanKode : CKeyObj {
    keyByPlanKode(CIndex* pI) : CKeyObj(pI, 0, 0), plan('    ') {}
    DWORD plan;
    CString code;
    virtual void PrepareKey() {
        m_pStoreObj->FX_String(1, (char*)&plan, 4, 1);
        m_pStoreObj->FX_String(2, (char*)(LPCSTR)code, code.GetLength(), 1);
    }
};

inline void processAccValue(CValue* pValue, CStoreObj& store, keyObj<0>& key, ObjIDSet& elements,
                            DWORD planID, DWORD codeLen, CMapStringToPtr& groups, LPSTR pRecIsFolder, LPSTR pRecCode, LPSTR pRecPlanID) {
    id8 id = objIDto8(pValue->m_ObjID);
    if(0x20202000000000 == id) {
        groups[""] = 0;
        return;
    }
    id8to9(id, key.id);

    if(store.Goto(&key, ccE, 0)) {
        if(*(DWORD*)pRecPlanID == planID) {
            if((DWORD)(BYTE)*pRecIsFolder == '0')
                elements.insert(id);
            else {
                CString code(pRecCode, codeLen);
                code.TrimRight();
                groups[code] = 0;
            }
        }
    }
}

void putvlAccounts(CPlanDef* pPlanDef, CPtrArray* pVL, CValue* pVal, sqlite3_stmt* pInsert) {
    ObjIDSet elements;
    CMapStringToPtr processedGroups;
    CMapStringToPtr groups;

    CTableEx* pTable = (CTableEx*)pDataDict->GetTable("1SACCS");
    LPSTR pRecID = pTable->recordBuffer();
    LPSTR pRecPID = pRecID + 9;
    LPSTR pRecCode = pRecPID + 4;
    int codeLen = pTable->field(2)->sizeCField;
    LPSTR pRecIsFolder = pRecCode + codeLen + pTable->field(3)->sizeCField + 3;

    CIndex* pIdxCode = pTable->GetIndex(1);
    keyByPlanKode keyCode(pIdxCode);
    id2str(pPlanDef->m_ID, (BYTE*)&keyCode.plan, 4);


    // Ищем по идшнику
    {
        CIndex* pI = pTable->index(0);
        CStoreObj store(pTable, pI);
        keyObj<0> key(pI);
        if(pVL) {
            CValueItem** ppItems = (CValueItem**)pVL->GetData();
            for(DWORD c = pVL->GetSize(); c--; ppItems++)
                processAccValue(&(*ppItems)->m_value, store, key, elements, keyCode.plan, codeLen, groups, pRecIsFolder, pRecCode, pRecPID);
        } else
            processAccValue(pVal, store, key, elements, keyCode.plan, codeLen, groups, pRecIsFolder, pRecCode, pRecPID);
    }
    // Теперь будем разворачивать группы
    CStoreObj store(pTable, pIdxCode);

    for(POSITION pos = groups.GetStartPosition(); pos;) {
        void* pv;
        groups.GetNextAssoc(pos, keyCode.code, pv);
        if(!processedGroups.Lookup(keyCode.code, pv)) {
            processedGroups[keyCode.code] = 0;
            if(store.Goto(&keyCode, ccG, 0)) {
                DWORD lenOfCode = keyCode.code.GetLength();
                for(;;) {
                    if(*(DWORD*)pRecPID != keyCode.plan)
                        break;
                    if(0 != u8text::compareLen(pRecCode, keyCode.code, lenOfCode))
                        break;
                    if((DWORD)(BYTE)*pRecIsFolder == '0')	// счет
                        elements.insert(id9to8(pRecID));
                    else { // Группа
                        CString code(pRecCode, codeLen);
                        code.TrimRight();
                        processedGroups[code] = 0;
                    }
                    if(!store.Goto(navNext, 0))
                        break;
                }
            }
        }
    }
    // И теперь запихаем все в таблицу
    elements.doInsert(pInsert);
}



/*
Установка параметра.
*/

int SQLiteBase::xCreate(sqlite3* pBase, void*, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char** pErr) {
    if(argc<4)
        return SQLITE_ERROR;
    CVtabInfo* pVtabInfo = CVtabInfo::tabInfoForName(u8text::fromUtf8(argv[3]));
    if(!pVtabInfo)
        return SQLITE_ERROR;

    sqlite3_declare_vtab(pBase, pVtabInfo->textSqlCreate());
    *ppVTab = pVtabInfo;
    return SQLITE_OK;
}

int SQLiteBase::xDestroy(sqlite3_vtab *pVTab) {
    return SQLITE_OK;
}

int SQLiteBase::xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* pIdx) {
    return static_cast<CVtabInfo*>(pVTab)->bestIndex(pIdx);
}

int SQLiteBase::xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor) {
    return static_cast<CVtabInfo*>(pVTab)->openCursor(ppCursor);
}

int SQLiteBase::xClose(sqlite3_vtab_cursor* pC) {
    static_cast<CursorImpl*>(pC)->Close();
    return SQLITE_OK;
}

int SQLiteBase::xFilter(sqlite3_vtab_cursor* pC, int idxNum, const char *idxStr, int argc, sqlite3_value **argv) {
    return static_cast<CursorImpl*>(pC)->Filter(idxNum, idxStr, argc, argv);
}

int SQLiteBase::xNext(sqlite3_vtab_cursor* pC) {
    return static_cast<CursorImpl*>(pC)->Next();
}

int SQLiteBase::xEof(sqlite3_vtab_cursor* pC) {
    return static_cast<CursorImpl*>(pC)->IsEof();
}

int SQLiteBase::xColumn(sqlite3_vtab_cursor* pC, sqlite3_context* pCtx, int nCol) {
    return static_cast<CursorImpl*>(pC)->Column(pCtx, nCol);
}

int SQLiteBase::xRowid(sqlite3_vtab_cursor* pC, sqlite3_int64 *pRowid) {
    return static_cast<CursorImpl*>(pC)->RowID(pRowid);
}

/*
void hexDump (static char *descr, static void *addr, int len) {
    int i;
    unsigned char buff[17];       // stores the ASCII data
    unsigned char *pc = (unsigned char *)addr;     // cast to make the code cleaner.
    CString buf1;
    CString buf2;



    // Output description if given.
    if (descr != NULL) {
        CString s = descr;
        DoMsgLine("dumping %s %00000000X",mmBlueTriangle,s,addr);
    }
    if(!addr){
        DoMsgLine("addr is null");
        return;
    }

    DoMsgLine("ADDR  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    DoMsgLine("-----------------------------------------------------");


    int cnt=0;
    for (i = 0; i < len; i++) {
        if(cnt==16) {
            cnt=0;
            DoMsgLine("%04X %s [%s]",mmNone,i-16,buf1,buf2);
            buf1.Empty();
            buf2.Empty();
        }
        buf1.Format("%s %02X",buf1,pc[i]);
        if(pc[i]>=32){
            char c=pc[i];
            buf2.Format(_T("%s%s"),buf2,CString(c));
        }else{
            buf2.Format(_T("%s%c"),buf2,'.');
        }
        cnt++;
    }
}
*/

int SQLiteBase::xUpdate(sqlite3_vtab* pVTab, int nArg, sqlite3_value **ppArg, sqlite_int64 *pRowid) {

    //DoMsgLine("nArg %i",mmNone,nArg);

    int RowId = sqlite3_value_int(ppArg[0]);

    if(nArg==1) { //DELETE
        if(bDeleteEnabled==FALSE){
            CBLModule::RaiseExtRuntimeError("Запрос DELETE запрещен.", FALSE);
        }
        //DoMsgLine("DELETE RowId %i",mmNone,RowId);

        CTableEx* table = static_cast<CVtabInfo*>(pVTab)->table();
        CStoreObj* obj = new CStoreObj(table,NULL);
        CRecAddr RecAddr = obj->GetRecAddr();
        RecAddr.SetLongVal(RowId);
        int res = obj->Goto(RecAddr,0);
        if (!res){
            //DoMsgLine("no res %i",mmNone,res);
            return SQLITE_OK;
        }
        obj->Locking((LockCtrl)1); // LockCtrl->wr
        obj->Delete();
        obj->Locking((LockCtrl)0); // LockCtrl->rd
        CUsersSet::IncrNetChangesCnt();
    }else{
        CBLModule::RaiseExtRuntimeError("Запросы UPDATE и INSERT не реализованы.", FALSE);
    }
    return SQLITE_OK;
}

struct finder {
    CNoCaseMap<int> names;
    finder() {
        static struct {
            LPCSTR n1, n2;
            int s;
        } data[] = {
            {"Строка", "String", ttString},
            {"Дата", "Date", ttDate},
            {"Число", "Number", ttNumber},
            {"Справочник", "Reference", ttReference},
            {"Документ", "Document", ttDocument},
            {"ВидДокумента", "DocumentKind", ttDocKind},
            {"ВидДокументаПредставление", "DocumentKindPresent", ttDocPresent},
            {"Перечисление", "Enum", ttEnum},
            {"Счет", "Account", ttAccount},
            {"Неопределенный", "Undefine", ttUndefine},
            {"ВидРасчета", "CalculationKind", ttCalcKind},
            {"Календарь", "Calendar", ttCalendar},
            {"Субконто", "Subconto", ttSubconto},
            {"Время", "Time", ttTime},
            {"ИмяВида", "KindName", ttKindName},
            {"ПредставлениеВида", "KindPresent", ttKindPresent},
            {"ВидСубконто", "SubcontoKind", ttSubcKind},
            {"ВидСубконтоПредставление", "SubcontoKindPresent", ttSubcPresent},
            {"BinaryData", "BinaryData", ttBlob},
        };
        for(DWORD i = 0; i < sizeof(data) / sizeof(data[0]) ; i++) {
            names[data[i].n1] = data[i].s;
            names[data[i].n2] = data[i].s;
        }
    }
    int find(const CString& str) {
        int res = 0;
        names.Lookup(str, res);
        return res;
    }
};

SQLiteQuery::column_info::column_info(const CString& colName, DWORD col, column_info*& pNext)
    :name(colName), column(col), typeForVTColumn(0, 0), type(ttError), linkedField(NULL), next(pNext) {
    pNext = this;
    CString typeName;
    SQLiteQuery::typeField(name, &typeName);

    if(!typeName.IsEmpty()) {
        CStringArray strTypes;
        SplitStr2Array(typeName, strTypes, '.');

        int size = strTypes.GetSize();
        if(size > 0) {
            static finder fnd;
            switch(fnd.find(strTypes[0])) {
            case ttString:
                if(size < 3) {
                    type = ttString;
                    typeForVTColumn.type = typeString;
                    if(2 == size)
                        typeForVTColumn.m_length = atol(strTypes[1]);
                }
                break;
            case ttDate:
                if(1 == size) {
                    type = ttDate;
                    typeForVTColumn.type = typeDate;
                }
                break;
            case ttNumber:
                if(size < 4) {
                    type = ttNumber;
                    typeForVTColumn.type = typeNumber;
                    if(size > 1)
                        typeForVTColumn.m_length = atol(strTypes[1]);
                    if(size > 2)
                        typeForVTColumn.m_prec = atol(strTypes[2]);
                }
                break;
            case ttReference:
                if(1 == size) {
                    type = ttReference;
                    typeForVTColumn.type = typeReference;
                } else if(2 == size) {
                    CSbCntTypeDef* pDef = pMetaDataCont->GetSTypeDef(strTypes[1]);
                    if(pDef) {
                        type = ttReferenceOne;
                        typeForVTColumn.type = typeReference;
                        typeForVTColumn.m_mdid = pDef->m_ID;
                    }
                }
                break;
            case ttDocument:
                if(1 == size) {
                    type = ttDocument;
                    typeForVTColumn.type = typeDocument;
                } else if(2 == size) {
                    CDocDef* pDef = pMetaDataCont->GetDocDef(strTypes[1]);
                    if(pDef) {
                        type = ttDocumentOne;
                        typeForVTColumn.type = typeDocument;
                        typeForVTColumn.m_mdid = pDef->m_ID;
                    }
                }
                break;
            case ttDocKind:
                if(1 == size) {
                    type = ttDocKind;
                    typeForVTColumn.type = typeString;
                }
                break;
            case ttDocPresent:
                if(1 == size) {
                    type = ttDocPresent;
                    typeForVTColumn.type = typeString;
                }
                break;
            case ttEnum:
                if(1 == size) {
                    type = ttEnum;
                    typeForVTColumn.type = typeEnum;
                } else if(2 == size) {
                    CEnumDef* pDef = pMetaDataCont->GetEnumDef(strTypes[1]);
                    if(pDef) {
                        type = ttEnumOne;
                        typeForVTColumn.type = typeEnum;
                        typeForVTColumn.m_mdid = pDef->m_ID;
                    }
                }
                break;
            case ttAccount:
                if(1 == size) {
                    type = ttAccount;
                    typeForVTColumn.type = typeAccount;
                } else if(2 == size) {
                    CBuhDef* pDef = pMetaDataCont->GetBuhDef();
                    if(pDef) {
                        CPlanDef* pPlan = pDef->GetPlanDef(strTypes[1]);
                        if(pPlan) {
                            type = ttAccountOne;
                            typeForVTColumn.type = typeAccount;
                            typeForVTColumn.m_mdid = pPlan->m_ID;
                        }
                    }
                }
                break;
            case ttUndefine:
                if(1 == size)
                    type = ttUndefine;
                break;
            case ttCalcKind:
                if(1 == size) {
                    type = ttCalcKind;
                    typeForVTColumn.type = typeCalcKind;
                }
                break;
            case ttCalendar:
                if(1 == size) {
                    type = ttCalendar;
                    typeForVTColumn.type = typeCalendar;
                }
                break;
            case ttSubconto:
                if(1 == size) {
                    type = ttSubconto;
                    typeForVTColumn.type = typeUndefined;
                }
                break;
            case ttTime:
                if(1 == size) {
                    type = ttTime;
                    typeForVTColumn.type = 2;
                    typeForVTColumn.m_length = 8;
                }
                break;
            case ttKindName:
                if(1 == size) {
                    type = ttKindName;
                    typeForVTColumn.type = typeString;
                }
                break;
            case ttKindPresent:
                if(1 == size) {
                    type = ttKindPresent;
                    typeForVTColumn.type = typeString;
                }
                break;
            case ttSubcKind:
                if(1 == size) {
                    type = ttSubcKind;
                    typeForVTColumn.type = typeString;
                }
                break;
            case ttSubcPresent:
                if(1 == size) {
                    type = ttSubcPresent;
                    typeForVTColumn.type = typeString;
                }
                break;
            }
        }
    } else
        type = ttAsIs;
}

void SQLiteQuery::column_info::toValue(sqlite3_stmt* pStmt, CValue* pValue) {
    pValue->Reset();
    int typeOfData = sqlite3_column_type(pStmt, column);
    if(SQLITE_NULL == typeOfData) {
        switch(type) {
        case ttNumber:
            *pValue = 0L;
            break;
        case ttString:
        case ttUndefine:
        case ttDocKind:
        case ttDocPresent:
        case ttTime:
        case ttKindName:
        case ttKindPresent:
        case ttSubcKind:
        case ttSubcPresent:
            *pValue = "";
            break;
        case ttDate: {
            CDate d(0, 0, 0);
            d.m_DateNum = 0;  /* 1C empty date */
            *pValue = d;
            break;
        }
        case ttReference:
            pValue->type = typeReference;
            break;
        case ttReferenceOne:
            pValue->type = typeReference;
            pValue->m_mdid = typeForVTColumn.m_mdid;
            break;
        case ttDocument:
        case ttDocumentWithLink:
            pValue->type = typeDocument;
            break;
        case ttDocumentOne:
            pValue->type = typeDocument;
            pValue->m_mdid = typeForVTColumn.m_mdid;
            break;
        case ttEnum:
            pValue->type = typeEnum;
            break;
        case ttEnumOne:
            pValue->type = typeEnum;
            pValue->m_mdid = typeForVTColumn.m_mdid;
            break;
        case ttAccount:
            pValue->type = typeAccount;
            break;
        case ttAccountOne:
            pValue->type = typeAccount;
            pValue->m_mdid = typeForVTColumn.m_mdid;
            break;
        case ttCalcKind:
            pValue->type = typeCalcKind;
            break;
        case ttCalendar:
            pValue->type = typeCalendar;
            break;
        }
        return;
    }

    static CString colText;
    CNumeric m_Number;

    // переменные для конвертации Blob->BainaryData
    CValue Blob;
    CBLContext *Blob_Context;
    int Blob_Method;
    int Blob_Length,Blob_Pointer;
    char Blob_Char;
    int Blob_idx,RetVal;
    const char * Blob_sqlite3;
    CValue* pArgs[2];
    CValue vNull = 0.0;
    CValue rValue;
    //CValue v0=

    switch(type) {
    case ttAsIs:
        switch(typeOfData) {
        case SQLITE_INTEGER:
            //DoMsgLine("SQLITE_INTEGER %s",mmNone,sqlite3_column_text(pStmt, column));
            *pValue = m_Number.FromString((LPCSTR)sqlite3_column_text(pStmt, column), NULL);
            break;
        case SQLITE_FLOAT:
            //DoMsgLine("SQLITE_FLOAT %s",mmNone,sqlite3_column_text(pStmt, column));
            *pValue = CNumeric(sqlite3_column_double(pStmt, column));
            break;
        case SQLITE_BLOB:
            Blob.CreateObject("BinaryData");
            Blob_Context = Blob.GetContext();
            if(!Blob_Context) {
                /* BinaryData unavailable (minimal config) -> return blob as text, do not crash */
                *pValue = u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column));
                break;
            }
            Blob_Method=Blob_Context->FindMethod("WriteData");
            Blob_Length = sqlite3_column_bytes(pStmt,column);
            //DoMsgLine("blob %s method %i lenght %i",mmNone,sqlite3_column_text(pStmt, column),Blob_Method,Blob_Length);

            Blob_sqlite3 = static_cast<const char*>(sqlite3_column_blob(pStmt, column));
            for(Blob_idx = 0; Blob_idx < Blob_Length; Blob_idx++) {
                Blob_Char  =   Blob_sqlite3[Blob_idx];
                //DoMsgLine("Blob_idx %i char %i ",mmNone,Blob_idx, (unsigned char)Blob_Char);
                CValue v1 = (unsigned char)Blob_Char;
                CValue v2 = 0.0;
                pArgs[0] = &v1;
                pArgs[1] = &v2;
                Blob_Context->CallAsProc(Blob_Method, pArgs);
            }

            Blob_Method=Blob_Context->FindMethod("Seek");
            pArgs[0]=&vNull;
            pArgs[1]=&vNull;
            RetVal=Blob_Context->CallAsFunc(Blob_Method,rValue,pArgs);
            Blob_Pointer=rValue.GetNumeric();
            //DoMsgLine("mBlobSeek %i RetVal %i Pointer %i",mmNone,Blob_Method,RetVal,Blob_Pointer);

            *pValue = Blob;
            break;
        case SQLITE_TEXT:
            *pValue = u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column));
            //DoMsgLine("SQLiteQuery::column_info::toValue ttAsIs type=%i m_String=%s ",mmNone,pValue->type,pValue->m_String);
            break;
        }
        break;
    case ttNumber:
        //DoMsgLine("ttNumber %s",mmNone,sqlite3_column_text(pStmt, column));

        pValue->type = typeNumber;
        pValue->m_length = typeForVTColumn.m_length;
        pValue->m_prec = typeForVTColumn.m_prec;

        switch(typeOfData) {
        case SQLITE_INTEGER:
            *pValue = m_Number.FromString((LPCSTR)sqlite3_column_text(pStmt, column), NULL);
            break;
        case SQLITE_FLOAT:
            pValue->m_Number = sqlite3_column_double(pStmt, column);
            break;
        default:
            pValue->m_Number.FromString((LPCSTR)sqlite3_column_text(pStmt, column), NULL);
        }
        if(pValue->m_length && typeOfData != SQLITE_INTEGER)
            pValue->m_Number.FromString(pValue->Format(), NULL);
        break;
    case ttString:
        *pValue = u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column));
        break;
    case ttDate:	// из utf-8 не преобразуем, тк там должны быть тока цифры
        if(SQLITE_TEXT == typeOfData) {
            colText = (LPCSTR)sqlite3_column_text(pStmt, column);
            if(colText.GetLength() == 8) {
                const unsigned char* ptr = (const unsigned char*)(LPCSTR)colText;
                DWORD y = str2dec(ptr, 4);
                CDate d(y, str2dec(ptr + 4, 2), str2dec(ptr + 6, 2));
                if(y == 0) d.m_DateNum = 0;  /* 1C empty date */
                *pValue = d;
            }
        }
        break;
    case ttReference:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 13)
                ValueStrWork<tos13>::str2val(pValue, colText, typeReference);
        }
        break;
    case ttReferenceOne:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 9)
                ValueStrWork<tos9>::str2val(pValue, colText, typeReference, typeForVTColumn.m_mdid);
        }
        break;
    case ttDocument:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 13)
                ValueStrWork<tos13>::str2val(pValue, colText, typeDocument);
        }
        break;
    case ttDocumentWithLink:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 9) {
                long mdID = 0;
                if(SQLITE_INTEGER == sqlite3_column_type(pStmt, linkedField->column))
                    mdID = sqlite3_column_int(pStmt, linkedField->column);
                else
                    mdID = str2id(sqlite3_column_text(pStmt, linkedField->column), 4);
                if(mdID)
                    ValueStrWork<tos9>::str2val(pValue, colText, typeDocument, mdID);
            }
        }
        break;
    case ttDocumentOne:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 9)
                ValueStrWork<tos9>::str2val(pValue, colText, typeDocument, typeForVTColumn.m_mdid);
        }
        break;
    case ttDocKind: {
        long id = 0;
        if(SQLITE_INTEGER == sqlite3_column_type(pStmt, column))
            id = sqlite3_column_int(pStmt, column);
        else
            id = str2id(sqlite3_column_text(pStmt, column), 4);
        if(id) {
            if(CMetaDataObjArray* pDocs = pMetaDataCont->GetDocDefs()) {
                if(CMetaDataObj* pObj = pDocs->GetItem(id))
                    *pValue = pObj->m_Code;
            }
        }
    }
    break;

    case ttDocPresent: {
        long id = 0;
        if(SQLITE_INTEGER == sqlite3_column_type(pStmt, column))
            id = sqlite3_column_int(pStmt, column);
        else
            id = str2id(sqlite3_column_text(pStmt, column), 4);
        if(id) {
            if(CMetaDataObjArray* pDocs = pMetaDataCont->GetDocDefs()) {
                if(CMetaDataObj* pObj = pDocs->GetItem(id))
                    *pValue = pObj->GetRealPresent();
            }
        }
    }
    break;

    case ttEnum:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 13)
                ValueStrWork<tos13>::str2val(pValue, colText, typeEnum);
        }
        break;
    case ttEnumOne:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 9)
                ValueStrWork<tos9>::str2val(pValue, colText, typeEnum, typeForVTColumn.m_mdid);
        }
        break;
    case ttAccount:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 13)
                ValueStrWork<tos13>::str2val(pValue, colText, typeAccount);
        }
        break;
    case ttAccountOne:
        if(SQLITE_TEXT == typeOfData) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            if(colText.GetLength() == 9)
                ValueStrWork<tos9>::str2val(pValue, colText, typeAccount, typeForVTColumn.m_mdid);
        }
        break;
    case ttUndefine:
        u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
        if(colText.GetLength() == 23) {
            ValueStrWork<tos23>::str2val(pValue, colText);
        } else if(typeOfData == SQLITE_TEXT || colText.GetLength() > 23) {
            ValueStrWork<tos23>::str2val(pValue, colText);
        }  else {
            *pValue = colText;
        }
        break;

    case ttCalcKind:
        if(SQLITE_TEXT == typeOfData) {
            colText = (LPCSTR)sqlite3_column_text(pStmt, column);
            int len = colText.GetLength();
            if(4 == len) {
                pValue->type = typeCalcKind;
                pValue->m_ObjID.ObjID = str2id((const unsigned char*)(LPCSTR)colText, 4);
                *(DWORD*)pValue->m_ObjID.DBSign.Sign = 0x00202020;
            } else if(13 == len)
                ValueStrWork<tos13>::str2val(pValue, colText, typeCalcKind);
        } else if(SQLITE_INTEGER == typeOfData) {
            pValue->type = typeCalcKind;
            pValue->m_ObjID.ObjID = sqlite3_column_int(pStmt, column);
            *(DWORD*)pValue->m_ObjID.DBSign.Sign = 0x00202020;
        }
        break;
    case ttCalendar:
        if(SQLITE_TEXT == typeOfData) {
            colText = (LPCSTR)sqlite3_column_text(pStmt, column);
            if(13 == colText.GetLength())
                ValueStrWork<tos13>::str2val(pValue, colText, typeCalendar);
        }
        break;
    case ttSubconto:
        if(SQLITE_TEXT == typeOfData && linkedField) {
            u8text::fromUtf8((LPCSTR)sqlite3_column_text(pStmt, column), colText);
            long sbKindID = 0;
            if(SQLITE_INTEGER == sqlite3_column_type(pStmt, linkedField->column))
                sbKindID = sqlite3_column_int(pStmt, linkedField->column);
            else
                sbKindID = str2id(sqlite3_column_text(pStmt, linkedField->column), 4);
            if(sbKindID) {
                CBuhDef* pDef = pMetaDataCont->GetBuhDef();
                if(pDef) {
                    CSbKindDef* pSbKindDef = pDef->GetSbKindDefs()->GetItem(sbKindID);
                    if(pSbKindDef && pSbKindDef->m_TypeCode >= typeEnum) {
                        if(0 == pSbKindDef->m_Kind) {
                            if(colText.GetLength() >= 13)
                                ValueStrWork<tos13>::str2val(pValue, colText, (Types1C)pSbKindDef->m_TypeCode);
                        } else {
                            if(colText.GetLength() >= 9)
                                ValueStrWork<tos9>::str2val(pValue, colText, (Types1C)pSbKindDef->m_TypeCode, pSbKindDef->m_Kind);
                        }
                    }
                }
            }
        }
        break;

    case ttTime: {
        DWORD time;
        if(SQLITE_TEXT == typeOfData)
            time = str2id(sqlite3_column_text(pStmt, column), sqlite3_column_bytes(pStmt, column)) / 10000;
        else if(SQLITE_INTEGER == typeOfData)
            time = sqlite3_column_int(pStmt, column) / 10000;
        else
            time = 0;
        colText.Format("%02i:%02i:%02i", time / 3600, (time % 3600) / 60, time % 60);
        *pValue = colText;
    }
    break;

    case ttKindName: {
        long id = 0;
        if(SQLITE_INTEGER == sqlite3_column_type(pStmt, column))
            id = sqlite3_column_int(pStmt, column);
        else
            id = str2id(sqlite3_column_text(pStmt, column), 4);
        if(id) {
            CMetaDataObj* pObj = pMetaDataCont->FindObject(id);
            if(pObj)
                *pValue = pObj->m_Code;
        }
    }
    break;

    case ttKindPresent: {
        long id = 0;
        if(SQLITE_INTEGER == sqlite3_column_type(pStmt, column))
            id = sqlite3_column_int(pStmt, column);
        else
            id = str2id(sqlite3_column_text(pStmt, column), 4);
        if(id) {
            CMetaDataObj* pObj = pMetaDataCont->FindObject(id);
            if(pObj)
                *pValue = pObj->GetRealPresent();
        }
    }
    break;

    case ttSubcKind: {
        long id = 0;
        if(SQLITE_INTEGER == sqlite3_column_type(pStmt, column))
            id = sqlite3_column_int(pStmt, column);
        else
            id = str2id(sqlite3_column_text(pStmt, column), 4);
        if(id) {
            if(CBuhDef* pDef = pMetaDataCont->GetBuhDef()) {
                if(CTypedFldDefsArray<CSbKindDef>* pSB = pDef->GetSbKindDefs()) {
                    if(CMetaDataObj* pObj = pSB->GetItem(id))
                        *pValue = pObj->m_Code;
                }
            }
        }
    }
    break;

    case ttSubcPresent: {
        long id = 0;
        if(SQLITE_INTEGER == sqlite3_column_type(pStmt, column))
            id = sqlite3_column_int(pStmt, column);
        else
            id = str2id(sqlite3_column_text(pStmt, column), 4);
        if(id) {
            if(CBuhDef* pDef = pMetaDataCont->GetBuhDef()) {
                if(CTypedFldDefsArray<CSbKindDef>* pSB = pDef->GetSbKindDefs()) {
                    if(CMetaDataObj* pObj = pSB->GetItem(id))
                        *pValue = pObj->GetRealPresent();
                }
            }
        }
    }
    break;
    }
    //DoMsgLine("SQLiteQuery::column_info::toValue type=%i m_String=%s ",mmNone,pValue->type,pValue->m_String);
}

extern "C" int connect1CTable(sqlite3* db, const char* zName) {
    if(!pDataDict)
        return 0;
    CString strName = u8text::fromUtf8(zName), strOrigName(strName);
    CVtabInfo* pInfo = CVtabInfo::tabInfoForName(strName);

    //DoMsgLine("strName %s",mmNone,strName);

    if(!pInfo && strName.Find('_') >= 0) {
        DWORD len = strName.GetLength();
        char *buf = new char[len + 1], *pWrite = buf + len;
        LPCSTR pStart = strName, pRead = pStart + len - 1;
        *pWrite = 0;
        while(pRead >= pStart) {
            if(*pRead == '_') {
                if(pRead[-1] == '_') {
                    *--pWrite = *pRead--;
                    pRead--;
                } else {
                    *--pWrite = '.';
                    pRead--;
                }
            } else
                *--pWrite = *pRead--;
        }
        strName = pWrite;
        delete [] buf;
        pInfo = CVtabInfo::tabInfoForName(strName);
    }
    if(pInfo) {
        CString strSqlCreate;
        strSqlCreate.Format("create virtual table temp.[%s] using dbeng(%s)", strOrigName, strName);
        u8text::toUtf8(strSqlCreate);
        return SQLITE_OK == sqlite3_exec(db, strSqlCreate, NULL, NULL, NULL);
    }
    return 0;
}

void SQLiteBase::open(const CString& fileName) {
    close(1);
    CString u8FileName = fileName;
    u8text::toUtf8(u8FileName);

    if(SQLITE_OK != sqlite3_open(u8FileName, &m_pDataBase)) {
        CString err("Недостаточно памяти");
        if(m_pDataBase) {
            u8text::fromUtf8(sqlite3_errmsg(m_pDataBase), err);
            sqlite3_close(m_pDataBase);
            m_pDataBase = NULL;
        }
        CBLModule::RaiseExtRuntimeError(err, FALSE);
        return;
    }
    if(pDataDict) {
        static sqlite3_module mod = {
            1,
            xCreate,
            xCreate,
            xBestIndex,
            xDestroy,
            xDestroy,
            xOpen,
            xClose,
            xFilter,
            xNext,
            xEof,
            xColumn,
            xRowid,
            xUpdate,//   int (*xUpdate)(sqlite3_vtab *, int, sqlite3_value **, sqlite3_int64 *);
            //   int (*xBegin)(sqlite3_vtab *pVTab);
            //   int (*xSync)(sqlite3_vtab *pVTab);
            //   int (*xCommit)(sqlite3_vtab *pVTab);
            //   int (*xRollback)(sqlite3_vtab *pVTab);
            //   int (*xFindFunction)(sqlite3_vtab *pVtab, int nArg, const char *zName, void (**pxFunc)(sqlite3_context*,int,sqlite3_value**),void **ppArg);
            //   int (*xRename)(sqlite3_vtab *pVtab, const char *zNew);
        };
        sqlite3_create_module(m_pDataBase, "dbeng", &mod, NULL);
    }
    sqlite3_create_collation(m_pDataBase, "_1C", SQLITE_UTF8, NULL, &u8text::_1Ccollate);
    sqlite3_create_function(m_pDataBase, "str2id", 1, SQLITE_UTF8 | SQLITE_DETERMINISTIC |SQLITE_INNOCUOUS, NULL, baseStr2Id, NULL, NULL);
    sqlite3_create_function(m_pDataBase, "id2str", 2, SQLITE_UTF8 | SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, NULL, baseId2Str, NULL, NULL);
    sqlite3_create_function(m_pDataBase, "coalesceex", -1, SQLITE_UTF8| SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, NULL, coalesceEx, NULL, NULL);
    sqlite3_create_function(m_pDataBase, "strHash", -1, SQLITE_UTF8 | SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, NULL, strHash, NULL, NULL);


    sqlite3_create_function(m_pDataBase, "uncompress", -1, SQLITE_UTF8| SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, NULL, uncompressFunc, NULL, NULL);
    sqlite3_create_function(m_pDataBase, "compress", -1, SQLITE_UTF8| SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, NULL, compressFunc, NULL, NULL);

    sqlite3_create_function(m_pDataBase, "to_base64", 1, SQLITE_UTF8| SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, NULL, toBase64Func, NULL, NULL);
    sqlite3_create_function(m_pDataBase, "from_base64", 1, SQLITE_UTF8| SQLITE_DETERMINISTIC|SQLITE_INNOCUOUS, NULL, fromBase64Func, NULL, NULL);

    //sqlite3_create_function(m_pDataBase, "like", 2, SQLITE_UTF8, NULL, likefunc, NULL, NULL);
    //sqlite3_progress_handler(m_pDataBase, 1000, progress_func, NULL);
    //SQLiteBase::initCallback(TRUE);
    //initCallback(TRUE);

}


void SQLiteBase::traceOn() {
    bNeedProfile = TRUE;
}

void SQLiteBase::putVT(CValueTable* pVT, const CString& strNameOfTable, BOOL bAsPersistent) {
    if(!m_pDataBase)
        CBLModule::RaiseExtRuntimeError("База данных не открыта", FALSE);

    if(strNameOfTable.IsEmpty())
        CBLModule::RaiseExtRuntimeError("Не задано имя таблицы", FALSE);


    DWORD cols = pVT->GetColumnCount();
    if(!cols)
        CBLModule::RaiseExtRuntimeError("В таблице значений нет колонок", FALSE);

    CString strSqlCreate, strSqlInsert;
    strSqlCreate.Format("drop table if exists %s; create%s table %s (", strNameOfTable, bAsPersistent? "" : " temp", strNameOfTable);
    strSqlInsert.Format("insert into %s values(", strNameOfTable);

    CDWordArray modificators;

    for(DWORD idx = 0; idx < cols ; idx++) {
        CVTColumn* pCol = pVT->GetColumn(idx);
        strSqlCreate += pCol->GetCode();
        {
            const CType& ctypeCol = pCol->GetType();
            if(ctypeCol.type == typeString && ctypeCol.m_length > 0) {
                CString vc;
                vc.Format(" varchar(%i)", (int)ctypeCol.m_length);
                strSqlCreate += vc;
            }
        }
        strSqlCreate += " not null,";
        strSqlInsert += "?,";

        //DoMsgLine("%s", " idx ");
        //DoMsgLine("\tНайдено в кэше");
        //DoMsgLine("%s", mmNone, text);

        int mod = 0;
        LPCSTR pMod = strstr(pCol->GetTitle(), "mod=");
        if(pMod)
            mod = atol(pMod + 4);
        else {
            const CType& type = pCol->GetType();

            if(type.type >= typeEnum && !type.m_mdid)
                mod = 1;
            else if(type.type == typeUndefined)
                mod = -1;
        }
        modificators.Add(mod);
    }
    strSqlCreate.SetAt(strSqlCreate.GetLength() - 1, ')');
    strSqlInsert.SetAt(strSqlInsert.GetLength() - 1, ')');

    u8text::toUtf8(strSqlCreate);
    u8text::toUtf8(strSqlInsert);

    tran_guard tg(m_pDataBase);

    if(SQLITE_OK != sqlite3_exec(m_pDataBase, strSqlCreate, NULL, NULL, NULL))
        raiseDBError();

    DWORD rows = pVT->GetRowCount();
    if(!rows) {
        tg.err = FALSE;
        return;
    }

    sqlite3_stmt* pInsert = NULL;
    const char* pTail;
    if(SQLITE_OK != sqlite3_prepare_v2(m_pDataBase, strSqlInsert, strSqlInsert.GetLength(), &pInsert, &pTail))
        raiseDBError();

    //CString text;

    for(DWORD r = 0; r < rows; r++) {
        for(DWORD c = 0; c < cols; ) {
            CVTColumn* pCol = pVT->GetColumn(c);
            int mod = modificators[c++];
            bindValue(bindSqlParam(pCol->GetCode(), c, m_pDataBase, pInsert), &pCol->Get(r), mod);
        }
        sqlite3_step(pInsert);
        sqlite3_reset(pInsert);
    }
    sqlite3_finalize(pInsert);
    tg.err = FALSE;
}

void SQLiteBase::putObjects(CValue* pObjects, const CString& strNameOfTable, BOOL bAsPersistent, const CString& kindOfObjects,const int Flags) {

    // Flags
    // 0 без преобразования
    // 1 as is при развороте без групп (по-умолчанию)
    // 2 при развороте только группы
    // 3 при развороте группы и элементы
    //DoMsgLine("Flags %i",mmNone,Flags);

    if(!m_pDataBase)
        CBLModule::RaiseExtRuntimeError("База данных не открыта", FALSE);
    CPtrArray* pVL = NULL;

    if(pObjects->type == 100 && pObjects->m_Context && !strcmp(pObjects->m_Context->GetRuntimeClass()->m_lpszClassName, "CValueListContext"))
        pVL=*(CPtrArray**)(((char*)pObjects->m_Context) + 0x30);
    else if(Flags==0) {}
    else if(!(pObjects->type == typeReference || pObjects->type == typeAccount))
        CBLModule::RaiseExtRuntimeError("Недопустимый тип первого параметра1", FALSE);

    if(strNameOfTable.IsEmpty())
        CBLModule::RaiseExtRuntimeError("Не задано имя таблицы", FALSE);

    CSbCntTypeDef* pReference = NULL;
    CPlanDef* pChartOfAccount = NULL;
    BOOL bRefWithGroups = FALSE;
    if(!kindOfObjects.IsEmpty()) {
        LPCSTR pKindOfObjects = kindOfObjects;
        if(pKindOfObjects[0] == '+') {
            bRefWithGroups = TRUE;
            pKindOfObjects++;
        }
        pReference = pMetaDataCont->GetSTypeDef(pKindOfObjects);
        if(!pReference) {
            CBuhDef* pBuh = pMetaDataCont->GetBuhDef();
            if(pBuh)
                pChartOfAccount = pBuh->GetPlanDef(kindOfObjects);
            if(!pChartOfAccount)
                CBLModule::RaiseExtRuntimeError("Не найден справочник или план счетов заданного вида", FALSE);
        }
    }

    //pReference->GetParentID()

    if(pReference && pReference->GetLevelsLimit() <= 1)
        pReference = NULL; // разворачивать по одноуровнему справочнику глупо.
    if((pReference || pChartOfAccount) && !pDataDict)
        CBLModule::RaiseExtRuntimeError("Обработка иерархии возможна только в DBF базах", FALSE);

    CString strSqlCreate, strSqlInsert;
    strSqlCreate.Format(
        "drop table if exists %s; create %s table %s (val char(9))",
        strNameOfTable, bAsPersistent? "" : "temp", strNameOfTable);
    strSqlInsert.Format("insert into %s values(?)", strNameOfTable);

    u8text::toUtf8(strSqlCreate);
    u8text::toUtf8(strSqlInsert);

    tran_guard tg(m_pDataBase);

    if(SQLITE_OK != sqlite3_exec(m_pDataBase, strSqlCreate, NULL, NULL, NULL))
        raiseDBError();

    sqlite3_stmt* pInsert = NULL;
    const char* pTail;
    if(SQLITE_OK != sqlite3_prepare_v2(m_pDataBase, strSqlInsert, strSqlInsert.GetLength(), &pInsert, &pTail))
        raiseDBError();

    CString val;
    if(Flags==0) { // Укладка объектов как есть
        if(pVL) {
            CValue** ppVals = (CValue**)pVL->GetData();
            for(int i = pVL->GetSize(); i--; ppVals++) {
                CString val;
                CValue pValue=**ppVals;
                if (pValue.type==2) {
                    if(pValue.m_String.GetLength() <= 32) {
                        ValueStrWork<tos23>::val2str(**ppVals, val);
                    } else {
                        val="S"+pValue.m_String;
                    }
                } else {
                    ValueStrWork<tos23>::val2str(**ppVals, val);
                }
                u8text::toUtf8(val);
                sqlite3_bind_text(pInsert, 1, val, val.GetLength(), SQLITE_TRANSIENT);
                sqlite3_step(pInsert);
                sqlite3_reset(pInsert);
            }
        } else {
            if (pObjects->type==2) {
                if(pObjects->m_String.GetLength() <= 32) {
                    ValueStrWork<tos23>::val2str(*pObjects, val);
                } else {
                    val="S"+pObjects->m_String;
                }
            } else {
                ValueStrWork<tos23>::val2str(*pObjects, val);
            }
            u8text::toUtf8(val);
            sqlite3_bind_text(pInsert, 1, val, val.GetLength(), SQLITE_TRANSIENT);
            sqlite3_step(pInsert);
            sqlite3_reset(pInsert);
        }
    } else if(pChartOfAccount) {
        putvlAccounts(pChartOfAccount, pVL, pObjects, pInsert);
    } else if(pReference) {

        if(pReference->GetParentID() ==0 ) {
            putvlReference(pReference, pVL, pObjects, pInsert, bRefWithGroups,Flags);
        } else {
            //    DoMsgLine("new algo");
            // Djelf
            //putvlReference(pReference, pVL, pObjects, pInsert, bRefWithGroups);

            CString val;
            CString totalval;

            u8text::toUtf8(totalval);

            if(pVL) {
                CValue** ppVals = (CValue**)pVL->GetData();
                for(int i = pVL->GetSize(); i--; ppVals++) {
                    ValueStrWork<tos9>::val2str(**ppVals, val);
                    if (totalval.IsEmpty())
                        totalval="'"+val+"'";
                    else
                        totalval=totalval+",'"+val+"'";
                }
            } else {
                ValueStrWork<tos9>::val2str(*pObjects, val);
                totalval="'"+val+"'";
            }

            //DoMsgLine("pReference totalval %s",mmNone,totalval);

            strSqlCreate.Format("drop table if exists %s; create%s table %s (val char(9))",strNameOfTable, bAsPersistent? "" : " temp", strNameOfTable);
            u8text::toUtf8(strSqlCreate);
            if(SQLITE_OK != sqlite3_exec(m_pDataBase, strSqlCreate, NULL, NULL, NULL))
                raiseDBError();

            CString strWhere="";
            if (Flags==1) // только эллементы
                strWhere="WHERE Cte.ISFOLDER=2;";
            else if (Flags==2) // только группы
                strWhere="WHERE Cte.ISFOLDER=1;";

            strSqlInsert.Format("\
        WITH RECURSIVE Cte(ID,PARENTEXT,ISFOLDER) AS ( \
        SELECT \
            ID,PARENTEXT,ISFOLDER \
        FROM [Справочник.%s] \
        WHERE ID IN ("+totalval+") \
        UNION ALL SELECT \
		Спр.ID,Спр.PARENTEXT,Спр.ISFOLDER \
        FROM [Справочник.%s] AS Спр \
        INNER JOIN Cte ON Cte.ID = Спр.PARENTID AND Cte.PARENTEXT = Спр.PARENTEXT \
        WHERE Cte.ISFOLDER=1 \
        ) \
        INSERT INTO %s SELECT \
            Cte.ID \
        FROM Cte "+strWhere,kindOfObjects,kindOfObjects,strNameOfTable);


            //DoMsgLine("strSqlInsert %s",mmNone,strSqlInsert);

            u8text::toUtf8(strSqlInsert);

            if(SQLITE_OK != sqlite3_prepare_v2(m_pDataBase, strSqlInsert, strSqlInsert.GetLength(), &pInsert, &pTail))
                raiseDBError();

            sqlite3_bind_text(pInsert, 1, val, val.GetLength(), SQLITE_TRANSIENT);
            sqlite3_step(pInsert);
            sqlite3_reset(pInsert);
        }
    } else {	// Не надо разворачивать по группам
        if(pVL) {
            CValue** ppVals = (CValue**)pVL->GetData();
            for(int i = pVL->GetSize(); i--; ppVals++) {
                ValueStrWork<tos9>::val2str(**ppVals, val);
                u8text::toUtf8(val);
                sqlite3_bind_text(pInsert, 1, val, val.GetLength(), SQLITE_TRANSIENT);
                sqlite3_step(pInsert);
                sqlite3_reset(pInsert);
            }
        } else {
            ValueStrWork<tos9>::val2str(*pObjects, val);
            u8text::toUtf8(val);
            sqlite3_bind_text(pInsert, 1, val, val.GetLength(), SQLITE_TRANSIENT);
            sqlite3_step(pInsert);
            sqlite3_reset(pInsert);
        }
    }

    sqlite3_finalize(pInsert);
    tg.err = FALSE;

}

void SQLiteQuery::prepare(const CString& query) {
    close();
    if(!m_pParent->base())
        CBLModule::RaiseExtRuntimeError("База данных не открыта", FALSE);

    CString u8strQuery(query);

    m_parser.processSql(u8strQuery);
    m_parser.reset();

    //DoMsgLine("u8strQuery %s", mmNone, u8strQuery);

    if(m_bIsDebug) {
        DoMsgLine("%s", mmNone, u8strQuery);
        bDoTrace = TRUE;
    }

    u8text::toUtf8(u8strQuery);

    //DoMsgLine("u8strQuery out %s", mmNone, u8text::fromUtf8(u8strQuery));

    const char* pTail;
    CString prep_error;
    int res = sqlite3_prepare_v2(m_pParent->base(), u8strQuery, u8strQuery.GetLength(), &m_pStmt, &pTail);

    m_bIsDebug = bDoTrace = FALSE;

    if(SQLITE_OK == res) {
        int cols = sqlite3_column_count(m_pStmt);
        if(cols) {
            CNoCaseMap<column_info*> allCols;
            CString colName;
            while(cols) {
                u8text::fromUtf8(sqlite3_column_name(m_pStmt, --cols), colName);
                new column_info(colName, cols, m_columns);
                if(m_columns->type == ttError)
                    prep_error = prep_error + "Неправильная типизация в колонке: " + colName + "\r\n";
                if(allCols.InsertExist(m_columns->name, m_columns))
                    prep_error = prep_error + "Колонка с именем " + m_columns->name + " уже существует\r\n";
            }
            if(prep_error.IsEmpty()) {
                for(column_info* ptr1 = m_columns; ptr1; ptr1 = ptr1->next) {
                    if(ptr1->type == ttDocument || ptr1->type == ttSubconto) {
                        if(!allCols.Lookup(ptr1->name + "_вид", ptr1->linkedField))
                            allCols.Lookup(ptr1->name + "_kind", ptr1->linkedField);
                        if(!ptr1->linkedField && ptr1->type == ttSubconto)
                            prep_error = prep_error + "Для колонки " + ptr1->name + " не найдена типизирующая колонка.\r\n";
                        if(ptr1->linkedField && ptr1->type == ttDocument)
                            ptr1->type = ttDocumentWithLink;
                    }
                }
            }
        }
    } else
        u8text::fromUtf8(sqlite3_errmsg(m_pParent->base()), prep_error);

    if(!prep_error.IsEmpty()) {
        close();
        CBLModule::RaiseExtRuntimeError(prep_error, FALSE);
    }
}

struct reset_stmt_guard {
    reset_stmt_guard(sqlite3_stmt* pStmt, CString& p) :
        m_pStmt(pStmt), m_profileDest(p) {}
    ~reset_stmt_guard() {
        sqlite3_reset(m_pStmt);
        if(bNeedProfile) {
            strProfile.Replace("\n", "\r\n");
            u8text::fromUtf8(strProfile, m_profileDest);
            strProfile.Empty();
            bNeedProfile = FALSE;
        }
    }
  private:
    sqlite3_stmt* m_pStmt;
    CString& m_profileDest;
};

void SQLiteQuery::execute(CValue* pDst, CValue* pDstParam, CValue& retVal) {
    if(!m_pStmt)
        CBLModule::RaiseExtRuntimeError("Запрос не подготовлен", FALSE);

    read_tran_guard qg(m_bNeedTransaction);
    reset_stmt_guard rg(m_pStmt, SQLiteBase::m_trace);

    int cols = sqlite3_column_count(m_pStmt);
    if(cols) {
        ISQLiteResultLoader* pLoader = NULL;

        IVTResultLoader vtLoader;
        IVLResultLoader vlLoader;
        IScalarResultLoader scLoader;

        if(pDst->type == typeNumber) {
            int colInScalar = (int)pDst->m_Number;
            if(colInScalar < cols) {
                scLoader.setCol(colInScalar);
                pLoader = &scLoader;
            } else
                CBLModule::RaiseExtRuntimeError("Неверный номер колонки скалярного результата", FALSE);
        } else if(pDst->type == 100) {
            //LPCSTR  str = pCont->GetRuntimeClass()->m_lpszClassName;
            //DoMsgLine("pDst->type %i name %s",mmNone,pDst->type,pCont->GetRuntimeClass()->m_lpszClassName);

            if(CBLContext* pCont = pDst->m_Context) {
                //DoMsgLine("pDst->type %i name %s",mmNone,pDst->type,pCont->GetRuntimeClass()->m_lpszClassName);

                if(0 == strcmp(pCont->GetRuntimeClass()->m_lpszClassName, "CValueTableContext")) {
                    vtLoader.setContext(pCont);
                    pLoader = &vtLoader;
                } else if(0 == strcmp(pCont->GetRuntimeClass()->m_lpszClassName, "CValueListContext")) {
                    vlLoader.setContext(pCont);
                    pLoader = &vlLoader;
                } else if(0 == strcmp(pCont->GetRuntimeClass()->m_lpszClassName, "CStdOleBLContext")) {
                    CBLModule::RaiseExtRuntimeError("OLE объект не может быть в качестве объекта-приемника", FALSE);
                } else {
                    CastContext::Dynamic(pCont, pLoader);
                }
            }
        } else if(pDst->IsEmpty()) {
            CValue val;
            //LoadValueFromString(&val, "{\"VT\",\"1\",{\"0\",{{\"\",\"0\",\"0\",\"0\",\"\",\"2\"}}}}");
            // LoadValueFromString каким то образом сломалось
            val.CreateObject("ТаблицаЗначений");
            vtLoader.setContext(val.m_Context);
            pLoader = &vtLoader;
        }

        if(!pLoader)
            CBLModule::RaiseExtRuntimeError("Не удалось создать объект-приемник", FALSE);

        cols = pLoader->init(cols, m_pParent->base(), pDstParam);

        struct val_array {
            CValue *pVals, **ppVals;
            val_array(int cols) {
                pVals = new CValue[cols];
                ppVals = new CValue*[cols];
            }
            ~val_array() {
                delete [] ppVals;
                delete [] pVals;
            }
        };

        val_array va(cols);
        CValue* pVals = va.pVals;
        CValue** ppVals = va.ppVals;

        column_info* pColInfo = m_columns;

        for(int i = 0; i < cols; i++) {
            pLoader->setColumn(i, pColInfo->name, pColInfo->typeForVTColumn);
            ppVals[i] = pVals + i;
            pColInfo = pColInfo->next;
        }
        for(;;) {
            int ret = sqlite3_step(m_pStmt);
            if(ret == SQLITE_ROW) {
                pColInfo = m_columns;
                for(int i = 0; i<cols ; i++) {
                    pColInfo->toValue(m_pStmt, pVals + i);
                    //DoMsgLine("%i %s",mmNone,pVals->type,pVals->m_String);

                    pColInfo = pColInfo->next;
                }
                pLoader->addValues(ppVals);
            } else if(SQLITE_DONE == ret)
                break;
            else
                m_pParent->raiseDBError();
        }
        pLoader->assignRetValue(retVal);
    } else {
        int res = sqlite3_step(m_pStmt);
        if(res == SQLITE_DONE)
            retVal = sqlite3_changes(m_pParent->base());
        else
            m_pParent->raiseDBError();
    }
}

void SQLiteQuery::setSqlParam(const CValue& param, const CValue& value, int mod, BOOL bThrow /*= TRUE*/) {
    if(!m_pStmt)
        CBLModule::RaiseExtRuntimeError("Запрос не подготовлен", FALSE);

    CString paramName;
    int idx;

    //DoMsgLine("setSqlParam param.type %i ",mmNone,param.type);

    if(param.type == 2) {	// Строка
        paramName = param.m_String;
        CString name = paramName;
        u8text::toUtf8(name);
        idx = sqlite3_bind_parameter_index(m_pStmt, name);
    } else {
        idx = (long)param.GetNumeric();
        paramName.Format("%i", idx);
    }

    if(0 == idx || idx > sqlite3_bind_parameter_count(m_pStmt)) {
        if(bThrow)
            CBLModule::RaiseExtRuntimeError("Неверный номер параметра", FALSE);
        return;
    }
    bindValue(bindSqlParam(paramName, idx, m_pParent->base(), m_pStmt), &value, mod);
}

void SQLiteQuery::close() {
    if(m_pStmt) {
        sqlite3_finalize(m_pStmt);
        /*if (sqlite3_finalize(m_pStmt)==SQLITE_OK){
            //DoMsgLine("sqlite3_finalize SQLITE_OK");
            //DoMsgLine("sqlite3_finalize SQLITE_OK %s",mmNone, u8text::fromUtf8(sqlite3_sql(m_pStmt)));

        }else{
            DoMsgLine("sqlite3_finalize SQLITE_ERROR");
        }*/
        m_pStmt = NULL;
    }
    if(m_columns) {
        column_info* pInfo = m_columns;
        while(pInfo) {
            column_info* pDel = pInfo;
            pInfo = pInfo->next;
            delete pDel;
        }
        m_columns = NULL;
    }
}

void SQLiteQuery::getFields(CStringArray& fields, TFArray& types) {
    if(!m_columns)
        CBLModule::RaiseExtRuntimeError("Запрос не подготовлен", FALSE);
    column_info* pInfo = m_columns;
    DWORD s = sqlite3_column_count(m_pStmt);
    fields.SetSize(s);
    types.SetSize(s);
    typesOfFields* pTypes = types.GetData();
    CString* pString = fields.GetData();
    while(pInfo) {
        *pString++ = pInfo->name;
        *pTypes++ = pInfo->type;
        pInfo = pInfo->next;
    }
}

void SQLiteQuery::typeField(CString& name, CString* pType) {
    name.TrimLeft('[');
    name.TrimRight(']');

    /* type marker (':' preferred, else '$') is taken only OUTSIDE string
       literals '...', so JSON paths ('$.a'), JSON objects, and time masks
       ('%H:%M') in an auto-named column are not mistaken for a 1C type. */
    int isTyped = -1;
    {
        int len = name.GetLength();
        int isColon = -1, isDollar = -1;
        BOOL inQuote = FALSE;
        for(int i = 0; i < len; i++) {
            char c = (char)name[i];
            if(c == 39) { inQuote = !inQuote; } /* 39 = ' (single quote) */
            else if(!inQuote) {
                if(c == ':') { if(isColon < 0) isColon = i; }
                else if(c == '$') { if(isDollar < 0) isDollar = i; }
            }
        }
        isTyped = (isColon >= 0) ? isColon : isDollar;
    }

    if(isTyped > 0) {
        if(pType) {
            *pType = name.Mid(isTyped + 1);
            pType->TrimLeft();
            pType->TrimRight();
        }
        name.GetBufferSetLength(isTyped);
    }
    name.TrimLeft();
    name.TrimRight();
    name.Replace(' ', '_');
    name.Replace('.', '_');
}

BOOL CSLDataBase::PutVT(CValue** ppParams) {
    CValueTable* pVT = NULL;//, const CString& strNameOfTable, BOOL bAsPersistent
    if(ppParams[0]->type == 100 && ppParams[0]->m_Context) {
        CBLContext* pCont = CastContext::ByRTCName(ppParams[0]->m_Context, "CValueTableContext");
        if(pCont)
            pVT = ((CValueTableContextData*)pCont->GetInternalData())->GetValueTable();
    }
    if(!pVT)
        CBLModule::RaiseExtRuntimeError("Не удалось получить таблицу значений из первого параметра", FALSE);
    putVT(pVT, ppParams[1]->GetString(), 0 != (long)ppParams[2]->GetNumeric());

    return TRUE;
}





/* //* Применяется для поиска утечек памяти.
struct mem_test
{
	CMapPtrToPtr allocks;
	~mem_test()
	{
		DoMsgLine("Count deallocated %i", mmNone, allocks.GetCount());
		for(POSITION pos = allocks.GetStartPosition(); pos;)
		{
			DWORD s;
			void* p;
			allocks.GetNextAssoc(pos, p, (void*&)s);
		}
	}
} _mem_;

void* operator new (size_t s)
{
	if(s == 0)
	{
		DoMsgLine("aa");
	}
	void* p = malloc(s);
	_mem_.allocks[p] = (void*)s;
	return p;
}

void operator delete(void* p)
{
	if(p)
	{
		_mem_.allocks.RemoveKey(p);
		free(p);
	}
}
*/


