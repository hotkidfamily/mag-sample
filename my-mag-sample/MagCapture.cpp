#include "stdafx.h"
#include "DesktopRect.h"
#include "MagCapture.h"

static wchar_t kMagnifierHostClass[] = L"ScreenCapturerWinMagnifierHost";
static wchar_t kHostWindowName[] = L"MagnifierHost";
static wchar_t kMagnifierWindowClass[] = L"Magnifier";
static wchar_t kMagnifierWindowName[] = L"MagnifierWindow";


MagCapture::MagCapture()
{
}


MagCapture::~MagCapture()
{
}

bool MagCapture::loadMagnificationAPI()
{
    bool ret = false;

    _hMagModule = LoadLibraryW(L"magnification.dll");


    if (_hMagModule) {

        memset(&_apiList, 0, sizeof(_apiList));

        _apiList.Initialize = reinterpret_cast<fnMagInitialize>(
            GetProcAddress(_hMagModule, "MagInitialize"));
        _apiList.Uninitialize = reinterpret_cast<fnMagUninitialize>(
            GetProcAddress(_hMagModule, "MagUninitialize"));
        _apiList.SetWindowFilterList = reinterpret_cast<fnMagSetWindowFilterList>(
            GetProcAddress(_hMagModule, "MagSetWindowFilterList"));
        _apiList.GetWindowFilterList = reinterpret_cast<fnMagGetWindowFilterList>(
            GetProcAddress(_hMagModule, "MagGetWindowFilterList"));
        _apiList.SetWindowSource = reinterpret_cast<fnMagSetWindowSource>(
            GetProcAddress(_hMagModule, "MagSetWindowSource"));
        _apiList.GetWindowSource = reinterpret_cast<fnMagGetWindowSource>(
            GetProcAddress(_hMagModule, "MagGetWindowSource"));
        _apiList.SetImageScalingCallback = reinterpret_cast<fnMagSetImageScalingCallback>(
            GetProcAddress(_hMagModule, "MagSetImageScalingCallback"));
    }

    ret = !(!_hMagModule
        || !_apiList.Initialize
        || !_apiList.Uninitialize
        || !_apiList.SetWindowFilterList
        || !_apiList.GetWindowFilterList
        || !_apiList.SetWindowSource
        || !_apiList.GetWindowSource
        || !_apiList.SetImageScalingCallback);

    return ret;
}


bool MagCapture::initMagnifier() {

    //if (GetSystemMetrics(SM_CMONITORS) != 1) {
    //  // Do not try to use the magnifier in multi-screen setup (where the API
    //  // crashes sometimes).
    //  //RTC_LOG_F(LS_WARNING) << "Magnifier capturer cannot work on multi-screen "
    //  //                         "system.";
    //  return false;
    //}
    if (!loadMagnificationAPI()) {
        return false;
    }

    BOOL result = _apiList.Initialize();
    if (!result) {
        return false;
    }

    HMODULE hInstance = nullptr;
    result =
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<char*>(&DefWindowProc), &hInstance);
    if (!result) {
        _apiList.Uninitialize();
        return false;
    }

    // Register the host window class. See the MSDN documentation of the
    // Magnification API for more infomation.
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = &DefWindowProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = kMagnifierHostClass;

    // Ignore the error which may happen when the class is already registered.
    RegisterClassExW(&wcex);

    // Create the host window.
    _hostWnd =
        CreateWindowExW(WS_EX_LAYERED, kMagnifierHostClass, kHostWindowName, 0, 0,
            0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!_hostWnd) {
        _apiList.Uninitialize();
        return false;
    }

    SetWindowLongPtr(_hostWnd, 0, (LONG_PTR)this);

    // Create the magnifier control.
    _magWnd = CreateWindowW(kMagnifierWindowClass, kMagnifierWindowName,
        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
        _hostWnd, nullptr, hInstance, nullptr);
    if (!_magWnd) {
        _apiList.Uninitialize();
        return false;
    }

    // Hide the host window.
    ShowWindow(_hostWnd, SW_HIDE);

    // Set the scaling callback to receive captured image.
    result = _apiList.SetImageScalingCallback(
        _magWnd,
        &MagCapture::OnMagImageScalingCallback);
    if (!result) {
        _apiList.Uninitialize();
        return false;
    }

    //SetWindowLongPtr(_magWnd, 0, (LONG_PTR)this);

#if 0
    if (excluded_window_) {
        result = set_window_filter_list_func_(
            magnifier_window_, MW_FILTERMODE_EXCLUDE, 1, &excluded_window_);
        if (!result) {
            mag_uninitialize_func_();
            //RTC_LOG_F(LS_WARNING)
            //    << "Failed to initialize ScreenCapturerWinMagnifier: "
            //       "error from MagSetWindowFilterList "
            //    << GetLastError();
            return false;
        }
    }
#endif

    _bMagInit = true;
    return true;
}

bool MagCapture::destoryMagnifier()
{
    bool ret = false;

    if (_hostWnd)
        DestroyWindow(_hostWnd);

     if (_apiList.Uninitialize)
        _apiList.Uninitialize();

    if (_hMagModule)
        FreeLibrary(_hMagModule);

    return ret;
}


BOOL WINAPI MagCapture::OnMagImageScalingCallback(HWND hwnd,
    void* srcdata,
    MAGIMAGEHEADER srcheader,
    void* destdata,
    MAGIMAGEHEADER destheader,
    RECT unclipped,
    RECT clipped,
    HRGN dirty)
{
    MagCapture* owner =
        reinterpret_cast<MagCapture*>(GetWindowLongPtr(hwnd, NULL));
    if (owner)
        owner->OnCaptured(srcdata, srcheader);

    return TRUE;
}

bool MagCapture::OnCaptured(void* srcdata, MAGIMAGEHEADER srcheader)
{
    _bCapSuccess = true;
    return false;
}


bool MagCapture::CaptureImage(const DesktopRect& rect) 
{
    // Set the magnifier control to cover the captured rect. The content of the
    // magnifier control will be the captured image.
    BOOL result = SetWindowPos(_magWnd, NULL, rect.left(), rect.top(),
        rect.width(), rect.height(), 0);
    if (!result) {
        return false;
    }

    RECT native_rect;
    native_rect.left = rect.left();
    native_rect.top = rect.top();
    native_rect.right = rect.right();
    native_rect.bottom = rect.bottom();

    _bCapSuccess = false;

    // OnCaptured will be called via OnMagImageScalingCallback and fill in the
    // frame before set_window_source_func_ returns.
    result = _apiList.SetWindowSource(_magWnd, native_rect);

    if (!result) {
        return false;
    }
    return _bCapSuccess;
}

bool MagCapture::startCaptureWindow(HWND hWnd)
{
    bool ret = false;

    if (_bMagInit) {

    }

    initMagnifier();


    return ret;
}
bool MagCapture::startCaptureScreen(HMONITOR hMonitor)
{
    bool ret = false;

    if (_bMagInit) {

    }
    initMagnifier();
    return ret;
}

bool MagCapture::stop()
{
    bool ret = false;

    return ret;
}
