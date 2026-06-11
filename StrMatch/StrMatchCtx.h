// StrMatchCtx.h - контекстный класс ВК StrMatch (1С++ AddIn).
#pragma once
#include "../_1Common/1CHEADERS/1cheaders.h"
#include "valuework.hpp"
#include "../_1Common/contextimpl.hpp"
#include "StrMatch.h"
#include "strmatch_pct.h"

class CStrMatchCtx : public CContextImpl<CStrMatchCtx> {
    CStrMatch m_sm;
public:
    BL_BEGIN_CONTEXT("StrMatch", "СтрМатч");

    // Сравнить(Строка1, Строка2 [, ВПроцентах=0])
    //   ВПроцентах=0 -> сырой счёт ("попугаи"); 1 -> похожесть 0..100 %.
    BL_FUNC_WITH_DEFVAL(StrMatch, "Сравнить", 3) {
        CString s1 = ppParams[0]->GetString();
        CString s2 = ppParams[1]->GetString();
        long asPercent = (long)ppParams[2]->GetNumeric();
        long rr;
        if(asPercent)
            rr = (long)StrMatchPercent(m_sm, s1.GetBuffer(0), s2.GetBuffer(0));
        else
            rr = (long)m_sm.StrMatch(s1.GetBuffer(0), s2.GetBuffer(0));
        s1.ReleaseBuffer();
        s2.ReleaseBuffer();
        retVal = rr;
        return TRUE;
    }
    BL_DEFVAL_FOR(StrMatch) {
        if(nParam == 2) {            // 3-й параметр (ВПроцентах) необязателен, по умолчанию 0
            if(pValue) *pValue = 0L;
            return TRUE;
        }
        return FALSE;
    }

    BL_FUNC(Version, "Версия", 0) {
        retVal.Reset();
        retVal.type = 2;
        retVal.m_String = "StrMatch 2.1.1 (1sqlite hybrid)";
        return TRUE;
    }

    BL_END_CONTEXT();
};
