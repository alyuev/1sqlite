// SQLite loadable extension wrapper for CStrMatch (fuzzy string rating).
// Provides SQL functions:
//   strmatch(s1, s2)     -> raw score ("в попугаях")
//   strmatch_pct(s1, s2) -> similarity 0..100 %
// Algorithm (c) 2005-2015 Rakunov Aleksandr (Sk0rp), MIT; https://github.com/5k0rp/StrMatch
// Loadable extension wrapper for use with 1sqlite (load_extension('strmatch.dll')).
#include <windows.h>
#include "StrMatch.h"
#include "strmatch_pct.h"
#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

static CStrMatch g_sm;

// 1sqlite passes UTF-8 to sqlite; CStrMatch works in CP1251 -> convert.
static void utf8to1251(const char* in, char* out, int outsz) {
    if(!in) { if(outsz) out[0] = 0; return; }
    int wl = MultiByteToWideChar(CP_UTF8, 0, in, -1, NULL, 0);
    if(wl <= 0) { out[0] = 0; return; }
    WCHAR* w = new WCHAR[wl];
    MultiByteToWideChar(CP_UTF8, 0, in, -1, w, wl);
    WideCharToMultiByte(1251, 0, w, -1, out, outsz, NULL, NULL);
    delete[] w;
}

static void strmatchFunc(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    char b1[4096], b2[4096];
    if(argc < 2) { sqlite3_result_int(ctx, 0); return; }
    utf8to1251((const char*)sqlite3_value_text(argv[0]), b1, sizeof(b1));
    utf8to1251((const char*)sqlite3_value_text(argv[1]), b2, sizeof(b2));
    sqlite3_result_int(ctx, g_sm.StrMatch(b1, b2));
}

static void strmatchPctFunc(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    char b1[4096], b2[4096];
    if(argc < 2) { sqlite3_result_int(ctx, 0); return; }
    utf8to1251((const char*)sqlite3_value_text(argv[0]), b1, sizeof(b1));
    utf8to1251((const char*)sqlite3_value_text(argv[1]), b2, sizeof(b2));
    sqlite3_result_int(ctx, StrMatchPercent(g_sm, b1, b2));
}

extern "C" __declspec(dllexport)
int sqlite3_strmatch_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi) {
    SQLITE_EXTENSION_INIT2(pApi);
    sqlite3_create_function(db, "strmatch",     2, SQLITE_UTF8, 0, strmatchFunc,    0, 0);
    sqlite3_create_function(db, "strmatch_pct", 2, SQLITE_UTF8, 0, strmatchPctFunc, 0, 0);
    return SQLITE_OK;
}
