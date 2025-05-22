#pragma once

#include "capturer-define.h"

#include <memory>
#include <mutex>
#include <vector>

#include "MagFuncDefine.h"

class MagCapture : public CCapture
{
public:
    MagCapture();
    ~MagCapture();

public:
    virtual bool startCaptureWindow(HWND hWnd) final;
    virtual bool startCaptureScreen(HMONITOR hMonitor) final;
    virtual bool stop() final;
    virtual bool captureImage(const DesktopRect &rect) final;
    virtual bool setCallback(funcCaptureCallback, void *) final;
    virtual bool setExcludeWindows(HWND hWnd) final;
    virtual bool setExcludeWindows(std::vector<HWND> hWnd) final;
    virtual const char *getName() final;
    virtual bool usingTimer() final;

  public:
    bool onCaptured(void *srcdata, MAGIMAGEHEADER srcheader);

protected:
    bool initMagnifier(DesktopRect &rect);
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
    int32_t _offset = -1;

    HMODULE _hMagModule = nullptr;
    HINSTANCE _hMagInstance = nullptr;

    HWND _hostWnd = nullptr;
    HWND _magWnd = nullptr;

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;
};

