// StrMatchMain.cpp - hybrid StrMatch: DllMain entry point (Orefkov engine).
// Loads as a 1C++ AddIn (CreateObject "StrMatch"/"СтрМатч") without registry.
#include "stdafx.h"
#include <afxdllx.h>
#include "StrMatchCtx.h"

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID) {
    if(dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInstance);
        Init1CGlobal(hInstance);
        context_obj::CContextBase::InitAllContextClasses();
    } else if(dwReason == DLL_PROCESS_DETACH) {
        context_obj::CContextBase::DoneAllContextClasses();
    }
    return 1;
}
