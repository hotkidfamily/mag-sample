#pragma once

#include <Windows.h>
#include "VideoFrame.h"
#include "DesktopRect.h"

#include <vector>

typedef void (*funcCaptureCallback)(CAPIMP::VideoFrame *frame, void *args);

class CCapture 
{
  public:
    CCapture(){};
    virtual ~CCapture(){};

  public:
    virtual bool startCaptureWindow(HWND hWnd) = 0;
    virtual bool startCaptureScreen(HMONITOR hMonitor) = 0;
    virtual bool stop() = 0;
    virtual bool captureImage(const DesktopRect &rect) = 0;
    virtual bool setCallback(funcCaptureCallback, void *) = 0;
    virtual bool setExcludeWindows(HWND hWnd) = 0;
    virtual bool setExcludeWindows(std::vector<HWND> hWnd) = 0;
    virtual const char *getName() = 0;
    virtual bool usingTimer() = 0;
};


class CapOptions
{
  public:
    bool getEnableWindowFilter() const
    {
        return _usingWindowFilter;
    }
    void setEnableWindowFilter()
    {
        _usingWindowFilter = true;
    }

  private:
    bool _usingMagnificationAPI = false;
    bool _usingDXGIAPI = false;

    bool _detectPowerwPoint = false;
    bool _enableAero = false;

    bool _usingWindowFilter = false;
    bool _usingTextureCallback = false;

    bool _bDesktopCapture = false;
};

__declspec(dllexport) bool CAPIMP_CreateCapture(CCapture *&capture, CapOptions &option);
