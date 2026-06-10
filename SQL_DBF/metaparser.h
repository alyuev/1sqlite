//metaparser.h
#pragma once

class MetaParser {
  public:
    void __fastcall processSql(CString& sql);
    void __fastcall setTextParam(const CString& name, const CValue* pValue) {
        textParams[name] = *pValue;
    }

    void __fastcall reset() {
        textParams.RemoveAll();
    }

  protected:
    BOOL __fastcall makeString(const CString& paramName, int mod, CString& paramString);
    void __fastcall processParam(struct parsingData& pd);
    void __fastcall processMetaName(struct parsingData& pd);
    CNoCaseMap<CValue> textParams;
};
