// journal.cpp
#include "StdAfx.h"
#include "journal.h"
#include "utex.h"

CJournInfo::CJournInfo(CStringArray& /*arrOfNames*/)
{
	m_strTableName = "1SJOURN";
	m_pTable = static_cast<CTableEx*>(pDataDict->GetTable(m_strTableName));

	CNoCaseMap<CString> aliaces;
	CDWordArray longStr;
	fillNamesFromObjs(pMetaDataCont->GetGenJrnlFlds(), aliaces);

	for(int i = 0, c = pMetaDataCont->GetNRegDefs(); i < c; i++)
	{
		CRegDef* pReg = pMetaDataCont->GetRegDefAt(i);
		CString name;
		//name.Format(_T("%sФр"), pReg->m_Code);
		name=pReg->m_Code;
		name+=u8text::fromUtf8("Фр");

		aliaces[pReg->GetFieldName()] = name;
	}

	CMetaDataObjArrayTemplate<class CDocStreamDef>* pStreams = pMetaDataCont->GetDocStreamDefs();
	for(i = 0, c = pStreams->GetNItems(); i < c ;i++)
	{
		CDocStreamDef* pDef = pStreams->GetAt(i);
		CString name;
		//name.Format("%sПс", pDef->m_Code);
		name=pDef->m_Code;
		name+=u8text::fromUtf8("Пс");

		aliaces[pDef->GetFieldName()] = name;
	}
	fillTabInfo(aliaces, &longStr);
}
