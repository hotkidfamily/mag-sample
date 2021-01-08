#pragma once

#include "MagFuncDefine.h"

#include "DesktopRect.h"

#include <memory>
#include <mutex>
#include <vector>

#include "VideoFrame.h"

typedef void (*funcCaptureCallback)(VideoFrame* frame, void* args);

class MagCapture
{
public:
    MagCapture();
    ~MagCapture();

public:
    bool startCaptureWindow(HWND hWnd);
    bool startCaptureScreen(HMONITOR hMonitor);
    bool stop();
    bool onCaptured(void* srcdata, MAGIMAGEHEADER srcheader);
    bool captureImage(const DesktopRect& rect);
    bool setCallback(funcCaptureCallback, void*);
    bool setExcludeWindows(HWND hWnd);
    bool setExcludeWindows(std::vector<HWND> hWnd);

protected:
    bool initMagnifier();
    bool destoryMagnifier();
    bool loadMagnificationAPI();

    static BOOL WINAPI OnMagImageScalingCallback(HWND hwnd,
                                                 void *srcdata,
                                                 MAGIMAGEHEADER srcheader,
                                                 void *destdata,
                                                 MAGIMAGEHEADER destheader,
                                                 RECT unclipped,
                                                 RECT clipped,
                                                 HRGN dirty);

private:
    std::unique_ptr<MagInterface> _api = nullptr;
    std::unique_ptr<VideoFrame> _frames;
    DesktopRect _lastRect;
    DesktopRect _memoryRect;

    HMODULE _hMagModule = nullptr;

    HWND _hostWnd = nullptr;
    HWND _magWnd = nullptr;
    bool _bMagInit = false;

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;
};

