#pragma once
#include "capturer-define.h"

class DXGICapture : public CCapture {
  public:
    DXGICapture();
    ~DXGICapture();

  public:
    virtual bool startCaptureWindow(HWND hWnd) override;
    virtual bool startCaptureScreen(HMONITOR hMonitor) override;
    virtual bool stop() override;
    virtual bool captureImage(const DesktopRect &rect) override;
    virtual bool setCallback(funcCaptureCallback, void *) override;
    virtual bool setExcludeWindows(HWND hWnd) override;
    virtual bool setExcludeWindows(std::vector<HWND> hWnd) override;
    virtual const char *getName() override;

};
