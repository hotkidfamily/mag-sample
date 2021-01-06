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
        std::unique_ptr<MagCapture> capturer;

        HWND winID;
        HMONITOR screenID;

        DesktopRect rect;
    }capturer;

    struct
    {
        std::unique_ptr<d3drender> render;
    }render;
}AppContext;