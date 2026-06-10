// strategycash.h
#pragma once

class StrategyCash {

  public:
    ~StrategyCash();
    BOOL __fastcall getFromCash(sqlite3_index_info* pIdx, CString& cashKey);
    void __fastcall addToCash(const CString& cashKey, sqlite3_index_info* pIdx);

  protected:
    struct cash_entry {
        CString cashKey;
        CString idxStr;
        int idxNum;
        int estimatedCost;
        BOOL bOrderBy;
        CDWordArray args;
        /* Fields below are only available in SQLite 3.8.2 and later */
        sqlite3_int64 estimatedRows;    /* Estimated number of rows returned */
        /* Fields below are only available in SQLite 3.9.0 and later */
        int idxFlags;              /* Mask of SQLITE_INDEX_SCAN_* flags */
    };
    CPtrList cash;
};
