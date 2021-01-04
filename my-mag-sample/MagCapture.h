#pragma once

#include "MagFuncDefine.h"

#include "DesktopRect.h"

class MagCapture
{
public:
    MagCapture();
    ~MagCapture();

public:
    bool loadMagnificationAPI();
    bool startCaptureWindow(HWND hWnd);
    bool startCaptureScreen(HMONITOR hMonitor);
    bool stop();
    bool OnCaptured(void* srcdata, MAGIMAGEHEADER srcheader);
    bool CaptureImage(const DesktopRect& rect);

    static BOOL WINAPI OnMagImageScalingCallback(HWND hwnd,
        void* srcdata,
        MAGIMAGEHEADER srcheader,
        void* destdata,
        MAGIMAGEHEADER destheader,
        RECT unclipped,
        RECT clipped,
        HRGN dirty);
protected:
    bool initMagnifier();
    bool destoryMagnifier();

private:
    MagInterface _apiList;
    HWND _hostWnd = NULL;
    HWND _magWnd = NULL;
    bool _bMagInit = false;
    HMODULE _hMagModule = NULL;
    bool _bCapSuccess = false;
};

