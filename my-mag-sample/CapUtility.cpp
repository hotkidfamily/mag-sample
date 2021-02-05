#include "stdafx.h"
#include "CapUtility.h"

#include <shellscalingapi.h>
#include <set>
#include <versionhelpers.h>

namespace CapUtility
{
static HMODULE hShcoreModule = LoadLibraryW(L"Shcore.dll");
typedef HRESULT(__stdcall *funcGetDpiForMonitor)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

BOOL _getDpiForMonitor(HMONITOR hm, UINT *dpix, UINT *dpiy)
{
    HRESULT hRet = E_FAIL;
    static funcGetDpiForMonitor _ptrGetDpiForMonitor = nullptr;
    if (!_ptrGetDpiForMonitor) {
        if (hShcoreModule) {
            _ptrGetDpiForMonitor
                = reinterpret_cast<funcGetDpiForMonitor>(GetProcAddress(hShcoreModule, "GetDpiForMonitor"));
        }
    }
    else {
        hRet = _ptrGetDpiForMonitor(hm, MDT_RAW_DPI, dpix, dpiy);
    }

    return SUCCEEDED(hRet);
}


typedef HRESULT(__stdcall *funcGetScaleFactorForMonitor)(HMONITOR hmonitor, DEVICE_SCALE_FACTOR *scale);

BOOL _getScaleForMonitor(HMONITOR hm, DEVICE_SCALE_FACTOR *scale)
{
    HRESULT hRet = E_FAIL;
    static funcGetScaleFactorForMonitor _ptrGetScaleFactorForMonitor = nullptr;
    if (!_ptrGetScaleFactorForMonitor) {
        if (hShcoreModule) {
            _ptrGetScaleFactorForMonitor = reinterpret_cast<funcGetScaleFactorForMonitor>(
                GetProcAddress(hShcoreModule, "GetScaleFactorForMonitor"));
        }
    }
    else {
        hRet = _ptrGetScaleFactorForMonitor(hm, scale);
    }

    return SUCCEEDED(hRet);
}

BOOL isWndCanCap(HWND hWnd)
{
    return IsWindow(hWnd)        // 是窗口
        && IsWindowVisible(hWnd) // 可见的
        && !IsIconic(hWnd);      // 非最小化的
}

std::vector<HWND> getWindowsCovered(HWND hTartgetWnd)
{
    std::set<HWND> wndlist;
    RECT rcTarget;
    GetWindowRect(hTartgetWnd, &rcTarget);
    HWND enumWnd = hTartgetWnd;

    while (NULL != (enumWnd = GetNextWindow(enumWnd, GW_HWNDPREV))) {
        int style = GetWindowLong(enumWnd, GWL_STYLE);
        int exStyle = GetWindowLong(enumWnd, GWL_EXSTYLE);

        if (isWndCanCap(enumWnd))
        {
            HWND rootWnd = enumWnd;

            if (style & WS_POPUP) {
                if (hTartgetWnd == (rootWnd = GetAncestor(enumWnd, GA_ROOTOWNER))) { // 如果 owner 和 目标是一样的，不过滤
                    continue;
                }
                else {
                    rootWnd = enumWnd;
                }
            }

            RECT rcWnd;
            GetWindowRect(rootWnd, &rcWnd);
            if (!IsRectEmpty(&rcWnd)) {
                if (!((rcWnd.right < rcTarget.left) || (rcWnd.left > rcTarget.right) || (rcWnd.bottom < rcTarget.top)
                      || (rcWnd.top > rcTarget.bottom))) {
                    wndlist.insert(rootWnd);
                }
            }
        }
    }

    std::vector<HWND> wndList2;
    if (wndlist.size()){
        for (auto &ele : wndlist) {
            wndList2.push_back(ele);
        }
    }

    return wndList2;
}


std::vector<DisplaySetting> _enumDisplaySetting()
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


bool getMaxResolutionInSystem(int32_t *cx, int32_t *cy)
{
    std::vector<DisplaySetting> settings = _enumDisplaySetting();
    int32_t maxWidth = 0;
    int32_t maxHeight = 0;

    if (settings.size()) {
        for (auto &s : settings) {
            CRect rect(s.rect());
            if (maxWidth < rect.Width()) {
                maxWidth = rect.Width();
            }
            if (maxHeight < rect.Height()) {
                maxHeight = rect.Height();
            }
        }
    }
    else {
        maxWidth = 1920;
        maxHeight = 1080;
    }

    *cx = maxWidth;
    *cy = maxHeight;

    return true;
}

DisplaySetting enumDisplaySettingByName(std::wstring &name)
{
    std::vector<DisplaySetting> settings = _enumDisplaySetting();

    for (auto &set : settings) 
    {
        if (set.name() == name) {
            DisplaySetting m = set;
            return m;
        }
    }

    return nullptr;
}

DisplaySetting enumDisplaySettingByMonitor(HMONITOR hMonitor)
{
    MONITORINFOEXW minfo;
    minfo.cbSize = sizeof(MONITORINFOEXW);
    GetMonitorInfo(hMonitor, &minfo);
    std::wstring str(minfo.szDevice);
    CapUtility::DisplaySetting settings = CapUtility::enumDisplaySettingByName(str);
    return settings;
}

static int(WINAPI *ptrGetSystemMetricsForDpi)(int, UINT) = NULL;
static UINT(WINAPI *pfnGetDpiForSystem)() = NULL;
static UINT(WINAPI *pfnGetDpiForWindow)(HWND) = NULL;

UINT GetDPI(HWND hWnd)
{
    if (hWnd != NULL) {
        if (pfnGetDpiForWindow)
            return pfnGetDpiForWindow(hWnd);
    }
    else {
        if (pfnGetDpiForSystem)
            return pfnGetDpiForSystem();
    }
    if (HDC hDC = GetDC(hWnd)) {
        auto dpi = GetDeviceCaps(hDC, LOGPIXELSX);
        ReleaseDC(hWnd, hDC);
        return dpi;
    }
    else
        return USER_DEFAULT_SCREEN_DPI;
}

static int metrics[SM_CMETRICS] = { 0 };

template <typename P>
bool Symbol(HMODULE h, P &pointer, const char *name)
{
    if (P p = reinterpret_cast<P>(GetProcAddress(h, name))) {
        pointer = p;
        return true;
    }
    else
        return false;
}

LRESULT UpdateSysmteMetrics(HWND hWnd)
{
    HMODULE hUser32 = GetModuleHandle(L"USER32");
    Symbol(hUser32, pfnGetDpiForSystem, "GetDpiForSystem");
    Symbol(hUser32, pfnGetDpiForWindow, "GetDpiForWindow");
    Symbol(hUser32, ptrGetSystemMetricsForDpi, "GetSystemMetricsForDpi");

    long dpi = GetDPI(hWnd);
    UINT dpiSystem = GetDPI(NULL);

    if (ptrGetSystemMetricsForDpi) {
        for (auto i = 0; i != sizeof metrics / sizeof metrics[0]; ++i) {
            metrics[i] = ptrGetSystemMetricsForDpi(i, dpi);
        }
    }
    else {
        for (auto i = 0; i != sizeof metrics / sizeof metrics[0]; ++i) {
            metrics[i] = dpi * GetSystemMetrics(i) / dpiSystem;
        }
    }
    return 0;
}

LRESULT GetPrimeryWindowsRect(RECT &rect)
{
    UpdateSysmteMetrics(NULL);

    rect.left = 0;
    rect.top = 0;

    rect.right = metrics[SM_CXSCREEN];
    rect.bottom = metrics[SM_CYSCREEN];

    return 0;
}



int CaptureAnImage(HWND hWnd, std::wstring &path)
{
    HDC hdcScreen;
    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;
    DWORD dwBytesWritten = 0;
    DWORD dwSizeofDIB = 0;
    HANDLE hFile = NULL;
    char *lpbitmap = NULL;
    HANDLE hDIB = NULL;
    DWORD dwBmpSize = 0;

    // Retrieve the handle to a display device context for the client
    // area of the window.
    hdcScreen = GetDC(NULL);
    hdcWindow = GetDC(hWnd);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hdcWindow);

    if (!hdcMemDC) {
        MessageBoxW(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the client area for size calculation.
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    // This is the best stretch mode.
    SetStretchBltMode(hdcWindow, HALFTONE);

    // The source DC is the entire screen, and the destination DC is the current window (HWND).
    if (!StretchBlt(hdcWindow, 0, 0, rcClient.right, rcClient.bottom, hdcScreen, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                    GetSystemMetrics(SM_CYSCREEN), SRCCOPY)) {
        MessageBoxW(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Create a compatible bitmap from the Window DC.
    hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

    if (!hbmScreen) {
        MessageBoxW(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(hdcMemDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdcWindow, 0, 0,
                SRCCOPY)) {
        MessageBoxW(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }


    // Get the BITMAP from the HBITMAP.
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer
    // that's pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbmScreen, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // A file is created, this is where we will save the screen capture.
    hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Add the size of the headers to the size of the bitmap to get the total file size.
    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.

    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    CloseHandle(hFile);

    // Clean up.
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    ReleaseDC(hWnd, hdcWindow);

    return 0;
}

BOOL GetWindowRect(HWND hWnd, RECT &iRect)
{
    BOOL bRet = FALSE;
    if (IsWindows8OrGreater()) {
        bRet = S_OK == DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &iRect, sizeof(RECT));
    }
    else if (IsWindows7OrGreater()){
        WINDOWINFO info;
        info.cbSize = sizeof(WINDOWINFO);
        CRect rect;
        ::GetWindowRect(hWnd, rect);
        ::GetWindowInfo(hWnd, &info);

        if (IsZoomed(hWnd)) {
            rect.top += info.cyWindowBorders;
            rect.left += info.cxWindowBorders;
            rect.bottom -= info.cyWindowBorders;
            rect.right -= info.cxWindowBorders;
        }
        else if (!(info.dwStyle & WS_THICKFRAME)) {
            rect.top += info.cyWindowBorders / 4;
            rect.left += info.cxWindowBorders / 4;
            rect.bottom -= info.cyWindowBorders / 4;
            rect.right -= info.cxWindowBorders / 4;
        }
        else if (info.dwStyle & WS_OVERLAPPEDWINDOW) {
            rect.left += info.cxWindowBorders;
            rect.bottom -= info.cyWindowBorders;
            rect.right -= info.cxWindowBorders;
        }
        iRect = rect;
        bRet = TRUE;
    }

    if( !bRet ){
        GetWindowRect(hWnd, &iRect);
        bRet = TRUE;
    }

    return bRet;
}

};
