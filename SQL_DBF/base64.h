/*
** base64.h -- SQL functions to_base64() / from_base64() for 1sqlite.
**
** Added per the 3.33.0.25 change: encoding/decoding BASE64 in pure SQL so it
** works under 1C 7.7 even on Linux+Wine where CDO is unavailable.
**
**   to_base64(X)   : text(utf-8) -> text(base64 ascii)
**   from_base64(X) : text(base64 ascii) -> text(utf-8)
**
** The functions operate on the raw byte stream that SQLite stores internally
** (UTF-8). 1sqlite converts Windows-1251 <-> UTF-8 transparently at the API
** boundary, so a string round-trips through to_base64()/from_base64() intact.
** NOTE: the bytes carried by a base64 string are UTF-8.
*/
#ifdef __cplusplus
extern "C" {
#endif

#include "SQLite/sqlite3.h"

/* base64 alphabet */
static const char b64Alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* value of a base64 character, or -1 for padding/whitespace/garbage */
static int b64Value(unsigned char c){
    if( c>='A' && c<='Z' ) return c - 'A';
    if( c>='a' && c<='z' ) return c - 'a' + 26;
    if( c>='0' && c<='9' ) return c - '0' + 52;
    if( c=='+' ) return 62;
    if( c=='/' ) return 63;
    return -1;
}

static void toBase64Func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
){
    const unsigned char *pIn;
    unsigned char *pOut;
    int nIn, nOut, i, j;
    (void)argc;
    if( sqlite3_value_type(argv[0])==SQLITE_NULL ) return;   /* NULL -> NULL */
    pIn = (const unsigned char*)sqlite3_value_text(argv[0]);
    nIn = sqlite3_value_bytes(argv[0]);
    if( pIn==0 || nIn==0 ){
        sqlite3_result_text(context, "", 0, SQLITE_STATIC);
        return;
    }
    nOut = ((nIn + 2) / 3) * 4;
    pOut = (unsigned char*)sqlite3_malloc(nOut + 1);
    if( pOut==0 ){ sqlite3_result_error_nomem(context); return; }
    for(i=0, j=0; i<nIn; i+=3){
        unsigned int n = ((unsigned int)pIn[i]) << 16;
        if( i+1<nIn ) n |= ((unsigned int)pIn[i+1]) << 8;
        if( i+2<nIn ) n |= (unsigned int)pIn[i+2];
        pOut[j++] = b64Alphabet[(n>>18) & 0x3f];
        pOut[j++] = b64Alphabet[(n>>12) & 0x3f];
        pOut[j++] = (i+1<nIn) ? b64Alphabet[(n>>6) & 0x3f] : '=';
        pOut[j++] = (i+2<nIn) ? b64Alphabet[ n     & 0x3f] : '=';
    }
    pOut[j] = 0;
    sqlite3_result_text(context, (char*)pOut, j, sqlite3_free);
}

static void fromBase64Func(
    sqlite3_context *context,
    int argc,
    sqlite3_value **argv
){
    const unsigned char *pIn;
    unsigned char *pOut;
    int nIn, nOut, i, buf, bits;
    (void)argc;
    if( sqlite3_value_type(argv[0])==SQLITE_NULL ) return;
    pIn = (const unsigned char*)sqlite3_value_text(argv[0]);
    nIn = sqlite3_value_bytes(argv[0]);
    if( pIn==0 || nIn==0 ){
        sqlite3_result_text(context, "", 0, SQLITE_STATIC);
        return;
    }
    pOut = (unsigned char*)sqlite3_malloc((nIn/4)*3 + 4);
    if( pOut==0 ){ sqlite3_result_error_nomem(context); return; }
    nOut = 0; buf = 0; bits = 0;
    for(i=0; i<nIn; i++){
        int v = b64Value(pIn[i]);
        if( v<0 ) continue;             /* skip '=', CR/LF, spaces, garbage */
        buf = (buf<<6) | v;
        bits += 6;
        if( bits>=8 ){
            bits -= 8;
            pOut[nOut++] = (unsigned char)((buf >> bits) & 0xff);
        }
    }
    sqlite3_result_text(context, (char*)pOut, nOut, sqlite3_free);
}

#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif
