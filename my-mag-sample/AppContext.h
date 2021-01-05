#pragma once

#include "MagCapture.h"
#include "d3drender.h"

#include <vector>

typedef struct tagAppContext
{
    struct {
        UINT timerID;
        UINT_PTR timerInst;
        uint32_t fps;
    }timer;

    struct {
        HWND winID;
        HMONITOR screenID;
        std::unique_ptr<MagCapture> capture;
    }capture;

    struct
    {
        std::unique_ptr<d3drender> render;
        DWORD threadID;
        HANDLE threadInst;
        HANDLE threadEvent;
        BOOL bQuit;
    }render;
}AppContext;