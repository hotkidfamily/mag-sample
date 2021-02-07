#include "stdafx.h"
#include "DXGICapture.h"

DXGICapture::DXGICapture()
{
}

DXGICapture::~DXGICapture()
{
}


bool DXGICapture::startCaptureWindow(HWND hWnd)
{
    bool bRet = false;
    return bRet;
}

bool DXGICapture::startCaptureScreen(HMONITOR hMonitor)
{
    bool bRet = false;
    return bRet;
}

bool DXGICapture::stop()
{
    bool bRet = false;
    return bRet;
}

bool DXGICapture::captureImage(const DesktopRect &rect)
{
    bool bRet = false;
    return bRet;
}

bool DXGICapture::setCallback(funcCaptureCallback, void *)
{
    bool bRet = false;
    return bRet;
}

bool DXGICapture::setExcludeWindows(HWND hWnd)
{
    bool bRet = false;
    return bRet;
}

bool DXGICapture::setExcludeWindows(std::vector<HWND> hWnd)
{
    bool bRet = false;
    return bRet;
}

const char *DXGICapture::getName()
{
    return "DXGI Capture";
}
