// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "capturer-define.h"
#include "MagCapture.h"
#include "GDICapture.h"
#include "DXGICapture.h"
#include "CapUtility.h"

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


bool CAPIMP_CreateCapture(CCapture *&capture, CapOptions &option)
{
    CCapture *Cap = nullptr; 
    if (option.getEnableWindowFilter()) {
        if (IsWindows7OrGreater()) {
            Cap = new MagCapture();
        }
    }

    if (!Cap)
    {
        if (IsWindows8OrGreater()) {
            if (IsWindows10OrGreater() && (CapUtility::queryWin10ReleaseID() > 1803)) {
                Cap = new DXGICapture(); // WinRT capture 
            }
            else {
                Cap = new DXGICapture(); // DXGI capture
            }
        }
        else if(IsWindows7OrGreater()){
            Cap = new MagCapture(); // using Mag Capture or dx11 
        }
        else {
            Cap = new GDICapture();
        }
    }


    capture = Cap;

    return Cap != nullptr;
}
