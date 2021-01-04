#pragma once

#include "MagCapture.h"

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
}AppContext;