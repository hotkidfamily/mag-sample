// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "capturer-define.h"
#include "MagCapture.h"
#include "GDICapture.h"
#include "DXGICapture.h"

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


bool CAPIMP_CreateCapture(CCapture *&capture)
{
    CCapture *Cap = new DXGICapture();

    capture = Cap;

    return Cap != nullptr;
}
