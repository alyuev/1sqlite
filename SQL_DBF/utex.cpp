// utex.cpp
//#include "StdAfx.h"
#include "utex.h"

extern "C" {
    uint16_t oasUpper2Lower[0xFFFF];
}



const DWORD u8text::w12512u[128] = {
    0x000082D0, //Ђ  = 0x80 = 0x00000402
    0x000083D0, //Ѓ  = 0x81 = 0x00000403
    0x009A80E2, //‚  = 0x82 = 0x0000201A
    0x000093D1, //ѓ  = 0x83 = 0x00000453
    0x009E80E2, //„  = 0x84 = 0x0000201E
    0x00A680E2, //…  = 0x85 = 0x00002026
    0x00A080E2, //†  = 0x86 = 0x00002020
    0x00A180E2, //‡  = 0x87 = 0x00002021
    0x00AC82E2, //€  = 0x88 = 0x000020AC
    0x00B080E2, //‰  = 0x89 = 0x00002030
    0x000089D0, //Љ  = 0x8A = 0x00000409
    0x00B980E2, //‹  = 0x8B = 0x00002039
    0x00008AD0, //Њ  = 0x8C = 0x0000040A
    0x00008CD0, //Ќ  = 0x8D = 0x0000040C
    0x00008BD0, //Ћ  = 0x8E = 0x0000040B
    0x00008FD0, //Џ  = 0x8F = 0x0000040F
    0x000092D1, //ђ  = 0x90 = 0x00000452
    0x009880E2, //‘  = 0x91 = 0x00002018
    0x009980E2, //’  = 0x92 = 0x00002019
    0x009C80E2, //“  = 0x93 = 0x0000201C
    0x009D80E2, //”  = 0x94 = 0x0000201D
    0x00A280E2, //•  = 0x95 = 0x00002022
    0x009380E2, //–  = 0x96 = 0x00002013
    0x009480E2, //—  = 0x97 = 0x00002014
    0x000098C2, //  = 0x98 = 0x00000098
    0x00A284E2, //™  = 0x99 = 0x00002122
    0x000099D1, //љ  = 0x9A = 0x00000459
    0x00BA80E2, //›  = 0x9B = 0x0000203A
    0x00009AD1, //њ  = 0x9C = 0x0000045A
    0x00009CD1, //ќ  = 0x9D = 0x0000045C
    0x00009BD1, //ћ  = 0x9E = 0x0000045B
    0x00009FD1, //џ  = 0x9F = 0x0000045F
    0x0000A0C2, //   = 0xA0 = 0x000000A0
    0x00008ED0, //Ў  = 0xA1 = 0x0000040E
    0x00009ED1, //ў  = 0xA2 = 0x0000045E
    0x000088D0, //Ј  = 0xA3 = 0x00000408
    0x0000A4C2, //¤  = 0xA4 = 0x000000A4
    0x000090D2, //Ґ  = 0xA5 = 0x00000490
    0x0000A6C2, //¦  = 0xA6 = 0x000000A6
    0x0000A7C2, //§  = 0xA7 = 0x000000A7
    0x000081D0, //Ё  = 0xA8 = 0x00000401
    0x0000A9C2, //©  = 0xA9 = 0x000000A9
    0x000084D0, //Є  = 0xAA = 0x00000404
    0x0000ABC2, //«  = 0xAB = 0x000000AB
    0x0000ACC2, //¬  = 0xAC = 0x000000AC
    0x0000ADC2, //­  = 0xAD = 0x000000AD
    0x0000AEC2, //®  = 0xAE = 0x000000AE
    0x000087D0, //Ї  = 0xAF = 0x00000407
    0x0000B0C2, //°  = 0xB0 = 0x000000B0
    0x0000B1C2, //±  = 0xB1 = 0x000000B1
    0x000086D0, //І  = 0xB2 = 0x00000406
    0x000096D1, //і  = 0xB3 = 0x00000456
    0x000091D2, //ґ  = 0xB4 = 0x00000491
    0x0000B5C2, //µ  = 0xB5 = 0x000000B5
    0x0000B6C2, //¶  = 0xB6 = 0x000000B6
    0x0000B7C2, //·  = 0xB7 = 0x000000B7
    0x000091D1, //ё  = 0xB8 = 0x00000451
    0x009684E2, //№  = 0xB9 = 0x00002116
    0x000094D1, //є  = 0xBA = 0x00000454
    0x0000BBC2, //»  = 0xBB = 0x000000BB
    0x000098D1, //ј  = 0xBC = 0x00000458
    0x000085D0, //Ѕ  = 0xBD = 0x00000405
    0x000095D1, //ѕ  = 0xBE = 0x00000455
    0x000097D1, //ї  = 0xBF = 0x00000457
    0x000090D0, //А  = 0xC0 = 0x00000410
    0x000091D0, //Б  = 0xC1 = 0x00000411
    0x000092D0, //В  = 0xC2 = 0x00000412
    0x000093D0, //Г  = 0xC3 = 0x00000413
    0x000094D0, //Д  = 0xC4 = 0x00000414
    0x000095D0, //Е  = 0xC5 = 0x00000415
    0x000096D0, //Ж  = 0xC6 = 0x00000416
    0x000097D0, //З  = 0xC7 = 0x00000417
    0x000098D0, //И  = 0xC8 = 0x00000418
    0x000099D0, //Й  = 0xC9 = 0x00000419
    0x00009AD0, //К  = 0xCA = 0x0000041A
    0x00009BD0, //Л  = 0xCB = 0x0000041B
    0x00009CD0, //М  = 0xCC = 0x0000041C
    0x00009DD0, //Н  = 0xCD = 0x0000041D
    0x00009ED0, //О  = 0xCE = 0x0000041E
    0x00009FD0, //П  = 0xCF = 0x0000041F
    0x0000A0D0, //Р  = 0xD0 = 0x00000420
    0x0000A1D0, //С  = 0xD1 = 0x00000421
    0x0000A2D0, //Т  = 0xD2 = 0x00000422
    0x0000A3D0, //У  = 0xD3 = 0x00000423
    0x0000A4D0, //Ф  = 0xD4 = 0x00000424
    0x0000A5D0, //Х  = 0xD5 = 0x00000425
    0x0000A6D0, //Ц  = 0xD6 = 0x00000426
    0x0000A7D0, //Ч  = 0xD7 = 0x00000427
    0x0000A8D0, //Ш  = 0xD8 = 0x00000428
    0x0000A9D0, //Щ  = 0xD9 = 0x00000429
    0x0000AAD0, //Ъ  = 0xDA = 0x0000042A
    0x0000ABD0, //Ы  = 0xDB = 0x0000042B
    0x0000ACD0, //Ь  = 0xDC = 0x0000042C
    0x0000ADD0, //Э  = 0xDD = 0x0000042D
    0x0000AED0, //Ю  = 0xDE = 0x0000042E
    0x0000AFD0, //Я  = 0xDF = 0x0000042F
    0x0000B0D0, //а  = 0xE0 = 0x00000430
    0x0000B1D0, //б  = 0xE1 = 0x00000431
    0x0000B2D0, //в  = 0xE2 = 0x00000432
    0x0000B3D0, //г  = 0xE3 = 0x00000433
    0x0000B4D0, //д  = 0xE4 = 0x00000434
    0x0000B5D0, //е  = 0xE5 = 0x00000435
    0x0000B6D0, //ж  = 0xE6 = 0x00000436
    0x0000B7D0, //з  = 0xE7 = 0x00000437
    0x0000B8D0, //и  = 0xE8 = 0x00000438
    0x0000B9D0, //й  = 0xE9 = 0x00000439
    0x0000BAD0, //к  = 0xEA = 0x0000043A
    0x0000BBD0, //л  = 0xEB = 0x0000043B
    0x0000BCD0, //м  = 0xEC = 0x0000043C
    0x0000BDD0, //н  = 0xED = 0x0000043D
    0x0000BED0, //о  = 0xEE = 0x0000043E
    0x0000BFD0, //п  = 0xEF = 0x0000043F
    0x000080D1, //р  = 0xF0 = 0x00000440
    0x000081D1, //с  = 0xF1 = 0x00000441
    0x000082D1, //т  = 0xF2 = 0x00000442
    0x000083D1, //у  = 0xF3 = 0x00000443
    0x000084D1, //ф  = 0xF4 = 0x00000444
    0x000085D1, //х  = 0xF5 = 0x00000445
    0x000086D1, //ц  = 0xF6 = 0x00000446
    0x000087D1, //ч  = 0xF7 = 0x00000447
    0x000088D1, //ш  = 0xF8 = 0x00000448
    0x000089D1, //щ  = 0xF9 = 0x00000449
    0x00008AD1, //ъ  = 0xFA = 0x0000044A
    0x00008BD1, //ы  = 0xFB = 0x0000044B
    0x00008CD1, //ь  = 0xFC = 0x0000044C
    0x00008DD1, //э  = 0xFD = 0x0000044D
    0x00008ED1, //ю  = 0xFE = 0x0000044E
    0x00008FD1, //я  = 0xFF = 0x0000044F
};

uint8_t nextGratUSymbols[256] = {0};
uint8_t noCaseCompare[256] = {0};
uint8_t upper[256] = {0};

CMapPtrToPtr u8text::uw1251;

void u8text::init() {
    uint32_t i;

    for(i = 0; i < 0xFFFF ; i++) {
        oasUpper2Lower[i] = i;
    }
    CharLowerBuffW(oasUpper2Lower + 32, 0xFFFF - 32);

    char str[65], ss = '\x80';
    for(int s = 0; s < 64; s++) {
        str[s] = ss++;
    }
    str[64] = 0;
    WCHAR wstr[65];
    MultiByteToWideChar(CP_ACP, 0, str, 64, wstr, 65);

    uw1251.InitHashTable(71);

    for(i = 0; i < 64 ; i++) {
        uw1251[(void*)wstr[i]] = (void*)str[i];
    }

    // Инициализируем таблицу нерегистрового сравнения
    CMapPtrToPtr mapWeightToUpperSymbol;
    SDBEngCodePage* pPage = CDBEngDB7::GetSelectedCodePage();

    for(i = 0; i < 256 ; i++) {
        noCaseCompare[i] = pPage->orderWeightForChars[pPage->upperSymbols[i]];
        mapWeightToUpperSymbol[(void*)noCaseCompare[i]] = (void*)pPage->upperSymbols[i];
    }
    // Фиксим порядок для пробела
    unsigned char fixSpace = noCaseCompare[' '];
    noCaseCompare[' '] = noCaseCompare[127];
    noCaseCompare[127] = fixSpace;


    // Теперь надо заполнить таблицу "следующий больший символ в верхнем регистре"
    for(i = 1; i < 256; i++) {
        unsigned char weight = noCaseCompare[i] + 1, nextSymb = 0;
        while(weight < 256) {
            if(mapWeightToUpperSymbol.Lookup((void*)weight, (void*&)nextSymb))
                break;
            weight++;
        }
        nextGratUSymbols[i] = nextSymb;
    }

    BYTE* b_upper = CDBEngDB7::GetSelectedCodePage()->upperSymbols;
    for(i = 1; i < 256; i++) {
        upper[i] = b_upper[i];
    }
    CDBEngDB7::GetSelectedCodePage()->orderWeightForChars;
}

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 12

static const uint8_t utf8d[] = {
    // The first part of the table maps bytes to character classes that
    // to reduce the size of the transition table and create bitmasks.
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,

    // The second part is a transition table that maps a combination
    // of a state of the automaton and a character class to a state.
    0, 12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 0, 12, 12, 12, 12, 12, 0, 12, 0, 12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 24, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12, 12,
    12, 36, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
};

inline uint32_t utf8read(const uint8_t*& ptr, int& len) {

    uint32_t state = 0;
    uint32_t type = 0;
    uint32_t codep = 0;

    uint8_t c = *ptr++;
    len--;

    //DWORD s1 = noCaseCompare[static_cast<DWORD>(static_cast<BYTE>(*str1))];
    //DWORD s2 = noCaseCompare[static_cast<DWORD>(static_cast<BYTE>(*str2))];
    //DoMsgLine("utf8read %i %c %i",mmNone,len,c,c);

    for(;;) {
        type = utf8d[c];

        //codep = (state != UTF8_ACCEPT) ? (c & 0x3fu) | (codep << 6) : (0xff >> type) & (c);
        codep = (state == UTF8_ACCEPT) ? (0xff >> type) & (c) : (c & 0x3fu) | (codep << 6);
        state = utf8d[256 + state + type];

        //if(state == UTF8_REJECT) {
        if(state == UTF8_ACCEPT) {
            return codep;
        } else if(state == UTF8_REJECT) {
            return 0;
        } else {
            c = *ptr++;
            len--;
        }
    }
} // utf8read

CString u8text::fromUtf8(LPCSTR strSrc) {
    CString res;
    if(!strSrc)
        return res;

    int len = strlen(strSrc);
    char *pWrite = res.GetBufferSetLength(len);
    char *pStart = pWrite;
    const unsigned char *ptr = (const unsigned char*)strSrc;

ascii:
    while(len) {
        const uint8_t c = *ptr++;
        if(c & 0x80) {
            *ptr--;
            break;
        } else {
            *pWrite++ = (uint8_t)c;
            len--;
        }
    }

    while(len) {
        const int32_t c = utf8read(ptr,len);
        if(c==0) {
            //DoMsgLine("done read");
            break;
        }

        if(c > 0x7f) {
            if(c >= 0x00000410 && c <= 0x0000044F) {	// А-я
                *pWrite++ = (uint8_t)(c - 848);
            } else if(c == 0x00000401) {
                *pWrite++ = 'Ё';
            } else if(c == 0x00000451) {
                *pWrite++ = 'ё';
            } else if(c == 0x00002116) {
                *pWrite++ = '№';
            } else {
                DWORD s;
                if(uw1251.Lookup((void*)c, (void*&)s)) {
                    *pWrite++ = (char)s;
                } else {
                    *pWrite++ = '?';
                }
            }
        } else {
            *pWrite++ = (char)c;
            goto ascii;
        }
    }
    res.GetBufferSetLength(pWrite - pStart);
    //DoMsgLine("len %i -> %i %s",mmNone,slen,(pWrite - pStart),res);
    return res;
}

void u8text::fromUtf8(LPCSTR strSrc, CString& res) {
    res = fromUtf8(strSrc);
    return;
}

inline uint32_t noCaseWeight(const unsigned char*& ptr, int& len, CMapPtrToPtr& uw1251) {
    if(!len) {
        return 0;
    }
    uint32_t c = utf8read(ptr,len);

    /*
    if(c > 1039 && c < 1104) {	// А-я
        return noCaseCompare[(c - 848)];
    } else if(c == 1025 || c == 1105) {
        return noCaseCompare[BYTE('Ё')];
    } else {
        unsigned char s;
        if(uw1251.Lookup((void*)c, (void*&)s)) {
            return noCaseCompare[s];
        }
        return 0;
    }
    */

    if(c < 0x80) { // слегка быстрее
        return noCaseCompare[c];
    } else if(c > 1039 && c < 1104) {	// А-я
        return noCaseCompare[(c - 848)];
    } else if(c == 1025 || c == 1105) {
        return noCaseCompare[BYTE('Ё')];
    } else {
        unsigned char s;
        if(uw1251.Lookup((void*)c, (void*&)s)) {
            return noCaseCompare[s];
        }
        //RuntimeError("noCaseWeight error uint32_t(%i)",c);
        //DoMsgLine("noCaseWeight error %i",mmNone,s);
        return 0;
    }

} // noCaseWeight

inline uint32_t noCaseWeight2(const unsigned char*& ptr, int& len) {
    // нет проверки на len
    // нет проверки на *ptr<0x80
    uint32_t c = utf8read(ptr,len);
    if(c > 1039 && c < 1104) {	// А-я
        return noCaseCompare[(c - 848)];
    } else if(c == 1025 || c == 1105) {
        return noCaseCompare[BYTE('Ё')];
    } else {
        unsigned char s;
        if(u8text::uw1251.Lookup((void*)c, (void*&)s)) {
            return noCaseCompare[s];
        }
        return 0;
    }

} // noCaseWeight


int u8text::_1Ccollate(void*, int lLeft, const void* zLeft, int lRight, const void* zRight) {
    // следющее должно соблюдаться:
    // - мы верим sqlite насчет длины, поэтому критерий окончания - длина

    //CString ustr1 = fromUtf8((LPCSTR)zLeft);
    //CString ustr2 = fromUtf8((LPCSTR)zRight);
    //DoMsgLine("collate [%i][%s] and [%i][%s]",mmNone,l1,ustr1,l2,ustr2);

    //while( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
    unsigned char *pStr1 = (unsigned char *)zLeft;
    unsigned char *pStr2 = (unsigned char *)zRight;
    //uint32_t lLeft = (uint32_t)l1;
    //uint32_t lRight = (uint32_t)l2;

    uint8_t cLeft;
    uint8_t cRight;

    for(;;) {
        if (!lLeft) {
            while(lRight) {
                if((uint8_t)*pStr2 != ' ') {
                    return -1;
                }
                pStr2++;
                lRight--;
            }
            return 0;
        } else if(!lRight) {
            while(lLeft) {
                if((uint8_t)*pStr1 != ' ') {
                    return 1;
                }
                pStr1++;
                lLeft--;
            }
            return 0;
        }

        cLeft = *pStr1;
        cRight = *pStr2;

        if(cLeft < 0x80) {
            if(cLeft == cRight) {
                lLeft--;
                lRight--;
                pStr1++;
                pStr2++;
                continue;
            } else if(cRight < 0x80) {
                cLeft  = noCaseCompare[cLeft];
                cRight = noCaseCompare[cRight];
                if(cLeft != cRight) {
                    return cLeft - cRight;
                } else {
                    lLeft--;
                    lRight--;
                    pStr1++;
                    pStr2++;
                    continue;
                }
            } else { // cRight >= 0x80
                cLeft  = noCaseCompare[cLeft];
                pStr1++;
                lLeft--;
                cRight = noCaseWeight2(pStr2, lRight);
            }
        } else if(cRight < 0x80) {
            cLeft = noCaseWeight2(pStr1, lLeft);
            cRight = noCaseCompare[cRight];
            pStr2++;
            lRight--;
        } else {
            cLeft = noCaseWeight2(pStr1, lLeft);
            cRight = noCaseWeight2(pStr2, lRight);
        }
        if(cLeft != cRight) {
            return cLeft - cRight;
        }
    } // end for(;;)
    return 0;
}

int u8text::compareRtrimNoCase(LPCSTR str1, LPCSTR str2) {
    //DoMsgLine("compareRtrimNoCase [%s] [%s]",mmNone,str1,str2);

    for(;;) {
        DWORD s1 = noCaseCompare[static_cast<DWORD>(static_cast<BYTE>(*str1))];
        DWORD s2 = noCaseCompare[static_cast<DWORD>(static_cast<BYTE>(*str2))];
        if(0 == s1) {	// Первая строка кончилась. Надо проверить, а не кончается ли вторая строка пробелами
            for(;;) {
                s2 = static_cast<DWORD>(static_cast<BYTE>(*str2++));
                if(0 == s2)
                    return 0;
                if(' ' != s2)
                    return -1;
            }
        } else if(0 == s2) {	// Вторая строка кончилась. Надо проверить, а не кончается ли первая строка пробелами
            for(;;) {
                s1 = static_cast<DWORD>(static_cast<BYTE>(*str1++));
                if(0 == s1)
                    return 0;
                if(' ' != s1)
                    return 1;	// Первая строка больше
            }
        } else if(s1 != s2)
            return s1 - s2;
        str1++;
        str2++;
    }
}

// Сравнение идет в соответствии с порядком сортировки строк в таблицах 1С!!!
int u8text::compareRtrim(LPCSTR str1, LPCSTR str2) {

    //DoMsgLine("compareRtrim [%s] [%s]",mmNone,str1,str2);

    BYTE* _1C_dborder = CDBEngDB7::GetSelectedCodePage()->orderWeightForChars;
    for(;;) {
        uint8_t s1 = _1C_dborder[*str1];
        uint8_t s2 = _1C_dborder[*str2];
        if(0 == s1) {	// Первая строка кончилась. Надо проверить, а не кончается ли вторая строка пробелами
            for(;;) {
                s2 = *str2;
                if(0 == s2)
                    return 0;
                if(' ' != s2)
                    return -1;	// Первая строка меньше
                str2++;
            }
        } else if(0 == s2) {	// Вторая строка кончилась. Надо проверить, а не кончается ли первая строка пробелами
            for(;;) {
                s1 = *str1;
                if(0 == s1)
                    return 0;
                if(' ' != s1)
                    return 1;	// Первая строка больше
                str1++;
            }
        } else if(s1 == s2) {
            str1++;
            str2++;
        } else {
            return s1 - s2;
        }
    }
}

int u8text::compareNoCaseLen(LPCSTR str1, LPCSTR str2, DWORD len) {
    //729927
    while(len) {
        const uint8_t s1 = noCaseCompare[*str1];
        const uint8_t s2 = noCaseCompare[*str2];
        if(s1 == s2) {
            str1++;
            str2++;
            len--;
        } else {
            return s1 - s2;
        }
    }
    return 0;
}

void u8text::dbUpper(CString& str) {
    //BYTE* upper = CDBEngDB7::GetSelectedCodePage()->upperSymbols;
    DWORD len = str.GetLength();
    BYTE* symb = (BYTE*)str.GetBufferSetLength(len);
    while(len--) {
        //DWORD idx = (DWORD)*symb;
        *symb++ = upper[(DWORD)*symb];
    }
}

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
    //sqlite3_result_int64(pCtx, id);
}
void baseId2Str(sqlite3_context* pCtx, int nParam, sqlite3_value** ppArgs) {
    DWORD id = sqlite3_value_int(ppArgs[0]), len = sqlite3_value_int(ppArgs[1]);
    CString text;
    BYTE* ptr = (BYTE*)text.GetBufferSetLength(len);
    memset(ptr, ' ', len);
    id2str(id, ptr, len);
    sqlite3_result_text(pCtx, text, len, SQLITE_TRANSIENT);
}

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

ObjIDSet::ObjIDSet() {
        memset(ppAssoc, 0, sizeof(ppAssoc));
        cnt = 0;
    }

ObjIDSet::~ObjIDSet() {
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

    void ObjIDSet::insert(id8 id) {
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
    BOOL ObjIDSet::exist(id8 id) {
        for(assoc* pAss = ppAssoc[id % hashSize]; pAss ; pAss = pAss->next)
            if(pAss->id == id)
                return TRUE;
        return FALSE;
    }

    DWORD ObjIDSet::count() {
        return cnt;
    }

    id8 ObjIDSet::first() {	// Не вызывать, если нет элементов!!!!
        for(assoc** ppA = ppAssoc; !*ppA; ppA++);
        return (*ppA)->id;
    }
    void ObjIDSet::remove(id8 id) {
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
    void ObjIDSet::doInsert(sqlite3_stmt* pInsert) {
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
    /*
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



