#pragma once

#include <vector>

namespace CapUtility 
{
const int kDesktopCaptureBPP = 4;
std::vector<HWND> getWindowsCovered(HWND hwnd);
};
