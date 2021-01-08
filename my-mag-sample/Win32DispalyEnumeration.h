#pragma once

#include <Windows.h>
#include <vector>

struct Monitor
{
  public:
    Monitor(nullptr_t)
    {
    }
    Monitor(HMONITOR hm, HDC hDC, RECT &rect)
    {
        _hmonitor = hm;
        _hDC = hDC;
        _rect = rect;
    }

    HMONITOR hMonitor() const noexcept
    {
        return _hmonitor;
    }

    HDC hDC() const noexcept
    {
        return _hDC;
    }

    RECT rect() const noexcept
    {
        return _rect;
    }

  private:
    HMONITOR _hmonitor = nullptr;
    HDC _hDC = nullptr;
    RECT _rect = {0};
};

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    std::vector<Monitor> &monitors = reinterpret_cast<std::vector<Monitor> &>(dwData);

    Monitor m(hMonitor, hdcMonitor, *lprcMonitor);
    monitors.push_back(m);

    return TRUE;
}

std::vector<Monitor> enumMonitor()
{
    std::vector<Monitor> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);
    return monitors;
}


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
        RECT rt = { 0, 0, (LONG)_mode.dmPelsWidth, (LONG)_mode.dmPelsHeight};
        return rt;
    }

  private:
    DISPLAY_DEVICEW _device;
    DEVMODEW _mode;
};

std::vector<DisplaySetting> enumDisplaySetting()
{
    DISPLAY_DEVICEW device = { 0 };
    device.cb = sizeof(DISPLAY_DEVICEW);

    std::vector<DisplaySetting> settings;

    for (int index = 0;; index++) {
        if (!EnumDisplayDevicesW(NULL, index, &device, EDD_GET_DEVICE_INTERFACE_NAME))
            break;

        DEVMODEW devmode = { 0 };
        devmode.dmSize = sizeof(DEVMODEW);
        // for (int modes = 0;; modes++) {
        if (!EnumDisplaySettingsW(device.DeviceName, ENUM_CURRENT_SETTINGS, &devmode))
            continue;
        //}
        devmode = devmode;
        DisplaySetting m(device, devmode);
        settings.push_back(m);
    }

    return settings;
}
