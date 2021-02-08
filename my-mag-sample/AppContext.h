#pragma once

#include "capturer-define.h"

#include "d3drender.h"

#include <vector>
#include <thread>

const uint32_t KDefaultFPS = 10;
const uint32_t KThreadCaptureMessage = WM_USER+1;

typedef struct tagAppContext
{
    struct {
        UINT timerID;
        UINT_PTR timerInst;
        uint32_t fps;

        bool bRunning;
        std::thread capThread;
    }timer;

    struct {
        std::unique_ptr<CCapture> host;

        HWND winID;
        HMONITOR screenID;

        DesktopRect rect;
    }capturer;

    struct
    {
        std::unique_ptr<d3drender> render;
    }render;
}AppContext;