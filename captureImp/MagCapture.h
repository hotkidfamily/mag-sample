#pragma once

#include "capturer-define.h"

#include <memory>
#include <mutex>
#include <vector>

#include "MagFuncDefine.h"

#define USING_GDI_CAPTURE 0 // hardware render, so gdi capture nothing.

class MagCapture : public CCapture
{
public:
    MagCapture();
    ~MagCapture();

public:
    virtual bool startCaptureWindow(HWND hWnd) override;
    virtual bool startCaptureScreen(HMONITOR hMonitor) override;
    virtual bool stop() override;
    virtual bool captureImage(const DesktopRect &rect) override;
    virtual bool setCallback(funcCaptureCallback, void *) override;
    virtual bool setExcludeWindows(HWND hWnd) override;
    virtual bool setExcludeWindows(std::vector<HWND> hWnd) override;
    virtual const char *getName() override;
    virtual bool usingTimer() override;

  public:

#if USING_GDI_CAPTURE
    bool onCaptured(void *srcdata, BITMAPINFOHEADER &header);
#else
    bool onCaptured(void *srcdata, MAGIMAGEHEADER srcheader);
#endif

protected:
    bool initMagnifier(DesktopRect &rect);
    bool destoryMagnifier();
    bool loadMagnificationAPI();
#if USING_GDI_CAPTURE
#else
    static BOOL WINAPI OnMagImageScalingCallback(HWND hwnd,
                                                 void *srcdata,
                                                 MAGIMAGEHEADER srcheader,
                                                 void *destdata,
                                                 MAGIMAGEHEADER destheader,
                                                 RECT unclipped,
                                                 RECT clipped,
                                                 HRGN dirty);
#endif // USING_GDI_CAPTURE
private:
    std::unique_ptr<MagInterface> _api = nullptr;
  std::unique_ptr<CAPIMP::VideoFrame> _frames;
    DesktopRect _lastRect;
    int32_t _offset = -1;

    HMODULE _hMagModule = nullptr;
    HINSTANCE _hMagInstance = nullptr;

    HWND _hostWnd = nullptr;
    HWND _magWnd = nullptr;

#if USING_GDI_CAPTURE
    HDC _magHDC = nullptr;
    HDC _compatibleDC = nullptr;
    HBITMAP _hDibBitmap = nullptr;
    void *_hdstPtr = nullptr;
    BITMAPINFO bmi = {0};
#endif

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;
};

