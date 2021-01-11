#pragma once

#include <vector>
#include <string>

namespace CapUtility 
{

struct DisplaySetting
{
  public:
    DisplaySetting(nullptr_t)
    {
    }
    DisplaySetting(DISPLAY_DEVICEW dev, DEVMODEW mode)
    {
        _device = dev;
        _mode = mode;
    }

    RECT rect() const noexcept
    {
        RECT rt = { _mode.dmPosition.x, _mode.dmPosition.y, (LONG)(_mode.dmPosition.x + _mode.dmPelsWidth),
                    (LONG)(_mode.dmPosition.y + _mode.dmPelsHeight) };
        return rt;
    }

    std::wstring name() const noexcept
    {
        return _device.DeviceName;
    }

  private:
    DISPLAY_DEVICEW _device;
    DEVMODEW _mode;
};

const int kDesktopCaptureBPP = 4;
const int kDesktopCaptureDefaultDPI = USER_DEFAULT_SCREEN_DPI;
std::vector<HWND> getWindowsCovered(HWND hwnd);
BOOL getDpiForMonitor(HMONITOR hm, UINT *DPI);
BOOL getDPIForWindow(HWND hwnd, UINT *DPI);
BOOL isWndCanCap(HWND hWnd);

DisplaySetting enumDisplaySettingByName(std::wstring &name);
DisplaySetting enumDisplaySettingByMonitor(HMONITOR hMonitor);
bool getMaxResolutionInSystem(int32_t *cx, int32_t *cy);
};
