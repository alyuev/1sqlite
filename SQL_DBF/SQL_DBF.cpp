// SQL_DBF.cpp : Defines the initialization routines for the DLL.
#include "stdafx.h"
#include <afxdllx.h>
#include "utex.h"
#include "vtab_info.h"
#include "database.h"
#include "SQLite/sqlite3.h"

void logCallback(void *pArg, int iErrCode, const char *zMsg) {
    DoMsgLine("[%d] %s", mmNone, iErrCode, u8text::fromUtf8(zMsg));
}

HINSTANCE g_hInstDll = NULL;   // handle of this DLL (for its version resource)

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
    if(dwReason == DLL_PROCESS_ATTACH) {
        g_hInstDll = hInstance;
        DisableThreadLibraryCalls(hInstance);
        Init1CGlobal(hInstance);

        u8text::init();

        //SQLiteBase::initCallback(TRUE);
        //initCallback(TRUE);
        //sqlite3_config(SQLITE_CONFIG_LOG,&logCallback,NULL);

        if(pDataBase7->IsKindOf(RUNTIME_CLASS(CDBEngDB7))) {
            pDataDict = *(CDataDictionary**)((long)pDataBase7 + 0x20);
            dbMode = pDataBase7->IsOpenExclusive() ? dbDbfMono : dbDbfShare;
        }

        context_obj::CContextBase::InitAllContextClasses();
    } else if(dwReason == DLL_PROCESS_DETACH) {
        context_obj::CContextBase::DoneAllContextClasses();
        CVtabInfo::doneWork();
    }
    return 1;   // ok
}
