#pragma once

#include "MagFuncDefine.h"

#include "DesktopRect.h"

#include <memory>
#include <mutex>

#include "VideoFrame.h"

typedef void (*funcCaptureCallback)(VideoFrame* frame, void* args);

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
    bool onCaptured(void* srcdata, MAGIMAGEHEADER srcheader);
    bool captureImage(const DesktopRect& rect);
    bool setCallback(funcCaptureCallback, void*);

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
    std::unique_ptr<MagInterface> _api = nullptr;

    HWND _hostWnd = nullptr;
    HWND _magWnd = nullptr;
    bool _bMagInit = false;
    HMODULE _hMagModule = nullptr;
    bool _bCapSuccess = false;
    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;

    std::unique_ptr<VideoFrame> _frames;
};

