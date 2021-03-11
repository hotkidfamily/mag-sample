// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "capturer-define.h"
#include "MagCapture.h"
#include "GDICapture.h"
#include "DXGICapture.h"

#include <VersionHelpers.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

int64_t queryWin10ReleaseID()
{
    static int64_t releaseID = 0;
    if (releaseID == 0) {
        HKEY hKey = NULL;
        DWORD dwType;
        ULONG nBytes;
        LONG lRes = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ,
                                   &hKey);
        if (lRes == ERROR_SUCCESS) {
            lRes = ::RegQueryValueEx(hKey, L"ReleaseId", NULL, &dwType, NULL, &nBytes);
            if (lRes != ERROR_SUCCESS) {
                return false;
            }

            if (dwType != REG_SZ && dwType != REG_EXPAND_SZ) {
                return false;
            }

            uint8_t *tmp = new uint8_t[nBytes];
            lRes = ::RegQueryValueEx(hKey, L"ReleaseId", NULL, &dwType, (LPBYTE)tmp, &nBytes);
            std::wstring value;
            value.assign((wchar_t*)tmp);
            delete[] tmp;

            releaseID = std::stol(value);

            ::RegCloseKey(hKey);
        }
    }

    return releaseID;
}

bool CAPIMP_CreateCapture(CCapture *&capture, CapOptions &option)
{
    CCapture *Cap = nullptr; 
    if (option.enableWindowFilter()) {
        if (IsWindows7OrGreater()) {
            Cap = new MagCapture();
        }
        else {
            Cap = new GDICapture();
        }
    }
    else {
        if (IsWindows8OrGreater()) {
            if (IsWindows10OrGreater() && (queryWin10ReleaseID() > 1904)) {
                Cap = new DXGICapture();
            }
            else {
                Cap = new DXGICapture();
            }
        }
        else if(IsWindows7OrGreater()){
            Cap = new MagCapture();
        }
        else {
            Cap = new GDICapture();
        }
    }


    capture = Cap;

    return Cap != nullptr;
}
