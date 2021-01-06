#pragma once

#include <vector>

namespace CapUtility 
{
const int kDesktopCaptureBPP = 4;
const int kDesktopCaptureDefaultDPI = 96;
std::vector<HWND> getWindowsCovered(HWND hwnd);
};
