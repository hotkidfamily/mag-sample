#pragma once

#include "capturer-define.h"

#include "d3drender.h"

#include <vector>

const uint32_t KDefaultFPS = 10;

typedef struct tagAppContext
{
    struct {
        UINT timerID;
        UINT_PTR timerInst;
        uint32_t fps;
    }timer;

    struct {
        std::unique_ptr<CCapture> host;

        HWND winID;
        HMONITOR screenID;

        DesktopRect rect;
    }host;

    struct
    {
        std::unique_ptr<d3drender> render;
    }render;
}AppContext;