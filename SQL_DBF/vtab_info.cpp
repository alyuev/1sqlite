// vtab_info.cpp
#include "StdAfx.h"
#include "vtab_info.h"
#include "utex.h"
#include "referencetabinfo.h"
#include "journal.h"
#include "docheaders.h"
#include "doctables.h"
#include "register.h"
#include "index_selector.h"
#include "longstrreader.h"
#include "systabs.h"
#include "calcjournal.h"

CNoCaseMap<CVtabInfo*> CVtabInfo::m_allTabs;
CString CVtabInfo::m_lastError;
struct tab_deleter {
    void operator()(CVtabInfo* pTab) {
        delete pTab;
    }
};

DWORD table_reader::allReaderCount = 0;
BOOL table_reader::bWeOpenTransaction = FALSE;
BOOL table_reader::bNeedTransaction = FALSE;

void CVtabInfo::doneWork() {
    m_allTabs.ForEachValue(tab_deleter());
}

NOTHROW static void TraceConstraints(sqlite3_index_info* pInfo, const CVtabInfo* pVTab) {
    if(!bDoTrace)
        return;

    //CString txt= pVTab->m_pTable->szName;

    CString text;
    text.Format("Подбор индекса для таблицы %s :\r\n\tОграничения: ", (LPCSTR)pVTab->tableName());
    //pVTab->m_pTable->szName

    //CTableEx* pTable = pVTab->table();
    //DoMsgLine("szName %s",mmNone,(LPCSTR)pTable->szName);

    sqlite3_index_info::sqlite3_index_constraint* pC = pInfo->aConstraint;
    for(int i = 0, c = pInfo->nConstraint ; i < c ; i++, pC++) {
        if(!pC->usable)
            continue;
        text += pVTab->field(pC->iColumn).name();

        switch(pC->op) {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            text += '=';
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
            text += '>';
            break;
        case SQLITE_INDEX_CONSTRAINT_LE:
            text += "<=";
            break;
        case SQLITE_INDEX_CONSTRAINT_LT:
            text += '<';
            break;
        case SQLITE_INDEX_CONSTRAINT_GE:
            text += ">=";
            break;
        case SQLITE_INDEX_CONSTRAINT_MATCH:
            text += " match";
            break;
        case SQLITE_INDEX_CONSTRAINT_LIKE:
            text += " like";
            break;
        case SQLITE_INDEX_CONSTRAINT_GLOB:
            text += " glob";
            break;
        case SQLITE_INDEX_CONSTRAINT_REGEXP:
            text += " regexp";
            break;
        case SQLITE_INDEX_CONSTRAINT_IS:
            text += " is";
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNOT:
            text += " isnot";
            break;
        case SQLITE_INDEX_CONSTRAINT_ISNOTNULL:
            text += " isnotnull";
            break;
        case SQLITE_INDEX_CONSTRAINT_NE:
            text += " ne";
            break;
        }
        text += "; ";
    }
    if(pInfo->nOrderBy) {
        text += "\r\n\tУпорядочить: ";
        orderby_ptr pOrderBy = pInfo->aOrderBy;
        for(int i = 0; i < pInfo->nOrderBy; i++, pOrderBy++) {
            text += pVTab->field(pOrderBy->iColumn).name();

            if(pOrderBy->desc)
                text += " desc";
            text += ", ";
        }
    }
    DoMsgLine("%s", mmNone, text);
}

NOTHROW static void TraceIndex(sqlite3_index_info* pIdx, BOOL bFromCash, const CVtabInfo* pVTab) {
    if(!bDoTrace)
        return;
    CString text;
    if(pIdx->idxNum >= 0) {
        CString idxunique ="";
        if (pIdx->idxFlags & SQLITE_INDEX_SCAN_UNIQUE)
            idxunique= " уникальный";

        CIndexEx* pIndex = pVTab->table()->index(pIdx->idxNum);
        text.Format("\tВыбран%s индекс %s: %s", idxunique,pIndex->p_10->szName,  pIndex->p_10->szIdxExpr);
    } else {
        text = "\tИндекс не выбран.";
    }


    //DoMsgLine(bFromCash ? "\tНайдено в кэше" : "\tВ кэше не найдено");

    DoMsgLine("%s", mmNone, text);
    if(pIdx->orderByConsumed)
        DoMsgLine("\tПопадает в сортировку");
    DoMsgLine("\tСтоимость: %i", mmNone, (int)pIdx->estimatedCost);
    //DoMsgLine("\tФлаги: %i", mmNone, pIdx->idxFlags);

}


int CVtabInfo::bestIndex(sqlite3_index_info* pIdx) const {
    //bDoTrace = TRUE;
    TraceConstraints(pIdx, this);

    // проверка на сырое чтение
    bool usable = false;
    //bool orderby = false;
    sqlite3_index_info::sqlite3_index_constraint* pC = pIdx->aConstraint;

    for(int i = 0, c = pIdx->nConstraint ; i < c ; i++, pC++) {
        if(pC->usable) {
            usable = true;
            break;
        }
    }

    BOOL fromCash = FALSE;

    //DoMsgLine("approxRowCount ",mmNone,approxRowCount);

    CString cashKey;
    if(!stratCash.getFromCash(pIdx, cashKey)) {
        //DoMsgLine("not from cache");

        index_selector idx_usage;
        const idx_node* pBest = idx_usage.bestIndex(pIdx, *this);
        FilterMachine::build(*this, pIdx, pBest);

        //BOOL idxUnique = FALSE;
        //pIdx->idxStr = sqlite3_mprintf("%s","my_idxStr");
        //DoMsgLine("pIdx->idxStr %s",mmNone,u8text::fromUtf8(pIdx->idxStr));

        if(pBest) {
            //DoMsgLine("num %i cnt %i flen %i ulen %i scores %i",mmNone,pBest->indexNum, approxRowCount ,pBest->fullLenInIdx ,pBest->usedLenOfIndex,pBest->scores);

            if(pBest->indexNum == -1) { // recNo
                pIdx->estimatedCost = 1;
                pIdx->estimatedRows = 1;
                pIdx->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
            } else if(usable == false) {
                if(pBest->order==nooAsc) {
                    pIdx->estimatedCost = 1;
                } else {
                    pIdx->estimatedCost = approxRowCount;
                }
                pIdx->estimatedRows = approxRowCount;
                pIdx->idxFlags = 0;
            } else {
                //logRowCount = approxRowCount;
                //logRowCount = 32;
                //DWORD r = logRowCount;
                //pBest->indexNum

                //DWORD r = approxRowCount * (pBest->fullLenInIdx - pBest->usedLenOfIndex + 1) / pBest->fullLenInIdx;
                DWORD r = approxRowCount / pBest->scores;

                DWORD r2 = 32;
                if(0 == (r & 0xFFFF0000)) {
                    r2 -= 16;
                    r <<= 16;
                }
                if(0 == (r & 0xFF000000)) {
                    r2 -= 8;
                    r <<= 8;
                }
                if(0 == (r & 0xF0000000)) {
                    r2 -= 4;
                    r <<= 4;
                }
                if(0 == (r & 0xC0000000)) {
                    r2 -= 2;
                }

                //pIdx->estimatedCost = double(logRowCount * pBest->fullLenInIdx) / (pBest->usedLenOfIndex + pBest->orderByLen);

                // проверка на уникальность индекса
                if(pIdx->idxNum == 0) {
                    BOOL idxUnique = FALSE;
                    LPCSTR tabname = (LPCSTR)this->tableName();
                    if(memcmp(m_pTable->szName, "SC", 2) == 0)
                        pIdx->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
                    else if(memcmp(m_pTable->szName, "DH", 2) == 0)
                        pIdx->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
                    else if(memcmp(m_pTable->szName, "1SJOURN", 7) == 0)
                        pIdx->idxFlags = SQLITE_INDEX_SCAN_UNIQUE;
                }

                if(pIdx->idxFlags == SQLITE_INDEX_SCAN_UNIQUE) {
                    pIdx->estimatedCost = r2/2;
                } else {
                    pIdx->estimatedCost = r2;
                }
                pIdx->estimatedRows = r2;
            }
        } else {
            pIdx->estimatedCost = approxRowCount;
            pIdx->estimatedRows = approxRowCount;
            pIdx->idxFlags = 0;
        }
        stratCash.addToCash(cashKey, pIdx);
    } else {
        //DoMsgLine("from cache");
        fromCash = TRUE;
    }

    TraceIndex(pIdx, fromCash, this);

    return SQLITE_OK;
}


void CVtabInfo::fillNamesFromObjs(CMetaDataObjArray* arr, CNoCaseMap<CString>& aliaces, CDWordArray* longStr) {
    for(int i = 0, c = arr->GetNItems(); i<c ; i++) {
        CMetaDataObj* pObj = arr->GetAt(i);
        if(pObj->IsTypedObj()) {
            if(longStr) {
                const CType& t = ((CMetaDataTypedObj*)pObj)->GetType();
                if(t.type == 2 && 0 == t.m_length)
                    longStr->Add(pObj->m_ID);
            }
            aliaces[((CMetaDataTypedObj*)pObj)->GetFieldName()] = pObj->m_Code;
        }
    }
}

void CVtabInfo::fillTabInfo(CNoCaseMap<CString>& aliaces, CDWordArray* longStr) {
    // дает сбой!!! m_pTable->EnableIndexing(FALSE);

    m_phisInfo = phisical_info::buildInfo(m_pTable);

    m_pFields = new one_field[m_phisInfo->fieldsCount() + m_phisInfo->indexesCount() + 1 + (longStr ? longStr->GetSize() : 0)];
    m_pFields->type = one_field::fRecNo;
    m_pFields->m_name = "recNo[rowid]";

    // Заполним физические поля
    one_field* pOneField = ++m_pFields;
    CNoCaseMap<int> allFields;

    CString create, name, fmt, fldName;
    for(int i = 0; i < m_phisInfo->fieldsCount(); i++) {
        CField* pField = m_pTable->field(i);
        fldName = pField->szName;
        if(!aliaces.Lookup(pField->szName, name))
            name = fldName;
        else
            fldName = fldName + '[' + name + ']';
        one_field::fType type = one_field::fField;

        while(allFields.IsExist(name))
            name += '_';
        allFields[name] = 0;

        create = create + ",[" + name + "]";
        switch(pField->TypeCField) {
        case 1: // char
            fmt.Format(" char(%i) collate _1C", pField->sizeCField);
            create += fmt;
            break;
        case 2:	// numeric
            fmt.Format(" numeric(%i, %i)", pField->sizeCField, pField->precCField);
            create += fmt;
            {
                LPCSTR pRead = pField->szName;
                while(*pRead && *pRead > '9')
                    pRead++;
                if(*pRead) {
                    CMetaDataObj* pObj = pMetaDataCont->FindObject(atol(pRead));
                    if(pObj && pObj->IsTypedObj() && !((CMetaDataTypedObj*)pObj)->GetType().IsPositiveOnly())
                        type = one_field::fNumNegateField;
                }
            }
            break;
        case 3:	// date
            create += " char(8)";
            break;
        default:
            create += " text collate _1C";
        }
        create += " not null\r\n";
        pOneField->type = type;
        pOneField->position = i;
        pOneField->m_name = fldName;
        pOneField++;
    }
    // Заполним поля длинных строк
    if(longStr) {
        for(i = 0; i<longStr->GetSize(); i++) {
            int id = longStr->GetAt(i);
            CMetaDataObj* pObj = pMetaDataCont->FindObject(id);
            int buf = 0x20202020;
            char* ptr = ((char*)&buf) + 3;
            do {
                int mod = id % 36;
                *ptr-- = mod < 10 ? mod + '0' : mod - 10 + 'A';
                id /= 36;
            } while(id);

            pOneField->type = one_field::fLongStr;
            pOneField->m_name = pObj->m_Code;
            pOneField->position = buf;
            create = create + "," + pOneField->m_name + " text collate _1C default null\r\n";
            pOneField++;
        }
    }

    CString idxName;

    for(i = 0; i < m_phisInfo->indexesCount() ; i++) {
        CIndexEx* pIndex = m_pTable->index(i);


        DWORD len = 0, nCount = pIndex->fieldsCount();
        idxName = "idx";
        for(DWORD j = 0; j < nCount ; j++) {
            CField* pField = pIndex->field(j);

            if(!aliaces.Lookup(pField->szName, name))
                name = pField->szName;
            idxName = idxName + '_' + name;
            len += pField->sizeCField;
        }
        fmt.Format(", %s char(%i) collate _1C default null\r\n", idxName, len);
        create += fmt;
        pOneField->type = one_field::fVirtIdx;
        pOneField->position = i;
        pOneField->m_name = CString(pIndex->szName) + '[' + ((LPCSTR)idxName + 1) + ']';
        pOneField++;
    }

    create.SetAt(0, ' ');
    m_strSqlCreate.Format("create table x(\r\n%s)", create);
    if(bDoTrace)
        DoMsgLine("%s", mmNone, m_strSqlCreate);
    u8text::toUtf8(m_strSqlCreate);

    // Получим приблизительное количество строк.
    CStoreObj store(m_pTable, NULL);
    store.Goto(navLast, 0);

    approxRowCount = store.m_pos;

    //DoMsgLine("%s", mmNone, m_strSqlCreate);
    //DoMsgLine("approxRowCount %i", mmNone, approxRowCount);
    // Вычислим приблизительное значение log2 от количества строк,
    // как 32 - количество ведущих нулей в битах числа
    //logRowCount = approxRowCount;
    /*logRowCount = 32;
    DWORD r = approxRowCount;
    if(0 == (r & 0xFFFF0000)){logRowCount -= 16; r <<= 16;}
    if(0 == (r & 0xFF000000)){logRowCount -= 8; r <<= 8;}
    if(0 == (r & 0xF0000000)){logRowCount -= 4; r <<= 4;}
    if(0 == (r & 0xC0000000)){logRowCount -= 2;}
    */
}

// Инфраструктура для построения цепочки классов-обработчиков
struct null_object {
    enum {val = -1};
};
template<int I> struct prev_type {
    typedef prev_type<I-1>::type type;
};
template<> struct prev_type<__LINE__> {
    typedef null_object type;
};

#define FIND_CLASS(className)\
struct className##_fd\
{\
	typedef prev_type<__LINE__ - 1>::type prev;\
	typedef className classType;\
	enum{val = prev::val + 1};\
};\
template<> struct prev_type<__LINE__> {typedef className##_fd type;}

struct first_string_hash {
    CNoCaseMap<int> TypeNums;
    template<typename T>
    __forceinline first_string_hash(T* p) {
        Fill(p);
    }
    template<typename T>
    __forceinline void Fill(T*) {
        LPCSTR p1, p2;
        T::classType::GetFindInfo(p1, p2);
        TypeNums[p1] = T::val;
        TypeNums[p2] = T::val;
        Fill((T::prev*)NULL);
    }
    template<> void Fill<null_object>(null_object*) {}
};

template<typename T>
__forceinline CVtabInfo* _GetObject(CStringArray& arrOfNames, CNoCaseMap<CVtabInfo*>& allObjects, const int iTypeNum, T* p=NULL) {
    if(T::val == iTypeNum) {
        CString strName;
        typedef T::classType ct;
        if(!ct::GetNameFrom(arrOfNames, strName)) {
            return NULL;
        }

        strName.Insert(0, ct::GetPrefix());
        CVtabInfo* pInfo;
        if(allObjects.Lookup(strName, pInfo)) {
            return pInfo;
        }

        pInfo = new ct(arrOfNames);
        if(!pInfo->table()) {
            static_cast<ct*>(pInfo)->Free();
            pInfo = NULL;
        } else {
            allObjects[strName] = pInfo;
        }

        return pInfo;
    } else {
        return _GetObject(arrOfNames, allObjects, iTypeNum, (T::prev*)NULL);
    }

}

template<>
__forceinline CVtabInfo* _GetObject<null_object>(CStringArray& arrOfNames, CNoCaseMap<CVtabInfo*>& allObjects, const int iTypeNum, null_object* p) {
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
// НАЧАЛО ОПИСАНИЯ КЛАССОВ ОБЪЕКТОВ РАЗЛИЧНЫХ ТАБЛИЦ
//////////////////////////////////////////////////////////////////////////
FIND_CLASS(CReferenceTabInfo);
FIND_CLASS(CJournInfo);
FIND_CLASS(CDHTabInfo);
FIND_CLASS(CDTTabInfo);
FIND_CLASS(CRegTabInfo);
FIND_CLASS(CRegTotalsTabInfo);
FIND_CLASS(CCoreTabs);
FIND_CLASS(CCJTabInfo);
// КОНЕЦ ОПИСАНИЯ КЛАССОВ ОБЪЕКТОВ РАЗЛИЧНЫХ ТАБЛИЦ

typedef prev_type<__LINE__>::type lastFindType;

CVtabInfo* CVtabInfo::tabInfoForName(const CString& strUserName) {
    static first_string_hash fsh((lastFindType*)NULL);
    CStringArray arrOfNames;
    SplitStr2Array(strUserName, arrOfNames);
    if(!arrOfNames.GetSize())
        return NULL;
    int iTypeNum;
    if(!fsh.TypeNums.Lookup(arrOfNames[0], iTypeNum))
        return NULL;

    return _GetObject<lastFindType>(arrOfNames, m_allTabs, iTypeNum);
}

void one_field::column(sqlite3_context* pCtx, const cursor_data& cursor, const CVtabInfo& table) const {
    static CString val;
    sqlite3_int64 bigint;

    if(isField()) {
        //DoMsgLine("isField",mmNone);
        const field_info& fi = table.phisInfo().field(position);
        memcpy(val.GetBufferSetLength(fi.length()), cursor.record.bufer() + fi.offsetInRecord(), fi.length());

        if(fi.type() == field_info::ftNumeric) {
            //DoMsgLine("one_field_column %s",mmNone,val);
            if(fi.precession())
                sqlite3_result_double(pCtx, atof(val));
            else
                sqlite3_result_int64(pCtx, _atoi64(val));
        } else {
            //DoMsgLine("%s",mmNone,val);
            u8text::toUtf8(val);
            //sqlite3_result_text(pCtx, val, val.GetLength(), SQLITE_TRANSIENT);
            sqlite3_result_text(pCtx, val, -1, SQLITE_TRANSIENT);
        }
    } else if(isIndex()) {
        const index_info* pIndexInfo = table.phisInfo().index(position);
        LPSTR pWrite = val.GetBufferSetLength(pIndexInfo->keySize() + 10);
        const idx_field_info* pFieldInfo = pIndexInfo->fields();
        for(DWORD c = pIndexInfo->fieldsCount(); c--;) {
            int len = pFieldInfo->info().length();
            memcpy(pWrite, cursor.record.bufer() + pFieldInfo->info().offsetInRecord(), len);
            pWrite += len;
            pFieldInfo++;
        }
        pWrite += 9;
        long v = cursor.store.m_pos;
        for(c = 10; c--; pWrite--) {
            if(v) {
                *pWrite = (v % 10) + '0';
                v /= 10;
            } else
                *pWrite = ' ';
        }
        u8text::toUtf8(val);
        sqlite3_result_text(pCtx, val, val.GetLength(), SQLITE_TRANSIENT);
    } else if(isLnStr()) {
        LongStrReader::get().ReadStr(cursor.record.bufer(), position, val);
        u8text::toUtf8(val);
        sqlite3_result_text(pCtx, val, val.GetLength(), SQLITE_TRANSIENT);
    } else
        sqlite3_result_null(pCtx);
}


