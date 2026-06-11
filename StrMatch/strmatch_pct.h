// strmatch_pct.h - similarity-as-percentage helper for CStrMatch.
// Shared by the 1C++ VK (StrMatchCtx.h) and the sqlite wrapper (strmatch_ext.cpp).
//
// The raw StrMatch score is the summed length of matching 2..5-grams of the
// SHORTER (normalized) string found in the longer one; its natural ceiling is
// that shorter string's self-score. So percent = 100 * raw / min(self_a, self_b),
// which yields 0..100 (full containment of the shorter string == 100%).
#ifndef STRMATCH_PCT_H
#define STRMATCH_PCT_H

#include "StrMatch.h"
#include <string.h>

// a, b are CP1251 buffers. Inputs are not modified by StrMatch (it copies them
// into its own LastStr1/LastStr2 before normalizing).
inline int StrMatchPercent(CStrMatch& sm, char* a, char* b) {
    if(!a || !b) return 0;
    int raw = sm.StrMatch(a, b);
    int sa  = sm.StrMatch(a, a);
    int sb  = sm.StrMatch(b, b);
    int denom = (sa < sb) ? sa : sb;        // self-score of the shorter string
    if(denom <= 0)                          // both strings shorter than 2 chars
        return (strcmp(a, b) == 0) ? 100 : 0;
    int p = (raw * 100 + denom / 2) / denom; // rounded
    if(p > 100) p = 100;
    if(p < 0)   p = 0;
    return p;
}

#endif // STRMATCH_PCT_H
