#pragma once

#include <vector>

namespace CapUtility 
{
const int kDesktopCaptureBPP = 4;
const int kDesktopCaptureDefaultDPI = USER_DEFAULT_SCREEN_DPI;
std::vector<HWND> getWindowsCovered(HWND hwnd);
BOOL getDpiForMonitor(HMONITOR hm, UINT *DPI);
BOOL getDPIForWindow(HWND hwnd, UINT *DPI);
};
