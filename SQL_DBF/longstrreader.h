// longstrreader.h
#pragma once

struct LongStrReader {
    static LongStrReader& __fastcall get() {
        static LongStrReader reader;
        return reader;
    }
    void __fastcall ReadStr(LPCSTR pRecordBufer, DWORD mdCharID, CString& val);

  private:
    LongStrReader() : store(pDataDict->GetTable("1SBLOB"), NULL) {
        store.SetOrderIndex(store.pTable->GetIndex(0));
        pReadBuf = ((CTableEx*)store.pTable)->recordBuffer();
    }
    CStoreObj store;
    LPCSTR pReadBuf;
};
