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

__declspec(dllexport) std::vector<HWND> getWindowsCovered(HWND hwnd);
__declspec(dllexport) BOOL isWndCanCap(HWND hWnd);

__declspec(dllexport) DisplaySetting enumDisplaySettingByName(std::wstring &name);
__declspec(dllexport) DisplaySetting enumDisplaySettingByMonitor(HMONITOR hMonitor);
__declspec(dllexport) bool getMaxResolutionInSystem(int32_t *cx, int32_t *cy);
__declspec(dllexport) LRESULT GetPrimeryWindowsRect(RECT &rect);

__declspec(dllexport) BOOL GetWindowRect(HWND hWnd, RECT &rect);

__declspec(dllexport) int CaptureAnImage(HWND hWnd, std::wstring &path);

__declspec(dllexport) int64_t queryWin10ReleaseID();
};
    