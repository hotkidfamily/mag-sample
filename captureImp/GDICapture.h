#pragma once

#include "capturer-define.h"
#include <mutex>

class GDICapture : public CCapture {
  public:
    GDICapture();
    ~GDICapture();

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
    bool onCaptured(void *srcdata, BITMAPINFOHEADER& srcheader);

  private:
    HDC _monitorDC = nullptr;
    HDC _compatibleDC = nullptr;
    HBITMAP _hDibBitmap = nullptr;
    void *_hdstPtr = nullptr;
    BITMAPINFO bmi;

    std::unique_ptr<CAPIMP::VideoFrame> _frames;
    DesktopRect _lastRect;

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;


    std::vector<HWND> _coverdWindows;
};
