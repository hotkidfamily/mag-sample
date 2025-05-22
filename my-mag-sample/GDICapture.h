#pragma once

#include "capturer-define.h"
#include <mutex>

class GDICapture : public CCapture {
  public:
    GDICapture();
    ~GDICapture();

  public:
    virtual bool startCaptureWindow(HWND hWnd) final;
    virtual bool startCaptureScreen(HMONITOR hMonitor) final;
    virtual bool stop() final;
    virtual bool captureImage(const DesktopRect &rect) final;
    virtual bool setCallback(funcCaptureCallback, void *) final;
    virtual bool setExcludeWindows(std::vector<HWND>& hWnd) final;
    virtual const char *getName() final;
    virtual bool usingTimer() final;

  public:
    bool onCaptured(void *srcdata, BITMAPINFOHEADER& srcheader);

  private:
    HDC _monitorDC = nullptr;
    HDC _compatibleDC = nullptr;
    HBITMAP _hDibBitmap = nullptr;
    void *_hdstPtr = nullptr;
    BITMAPINFO bmi;

    std::unique_ptr<VideoFrame> _frames;
    DesktopRect _lastRect;

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;


    std::vector<HWND> _coverdWindows;
};
