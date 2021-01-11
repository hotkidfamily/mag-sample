#include "stdafx.h"


#include "DesktopRect.h"
#include "MagCapture.h"

#include "CapUtility.h"
#include "logger.h"


static wchar_t kMagnifierHostClass[] = L"ScreenCapturerWinMagnifierHost";
static wchar_t kHostWindowName[] = L"MagnifierHost";
static wchar_t kMagnifierWindowClass[] = L"Magnifier";
static wchar_t kMagnifierWindowName[] = L"MagnifierWindow";

DWORD GetTlsIndex()
{
    static DWORD tlsIndex = TlsAlloc();
    return tlsIndex;
}


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
        _api.reset(new MagInterface());

        _api->Initialize = reinterpret_cast<fnMagInitialize>(
            GetProcAddress(_hMagModule, "MagInitialize"));
        _api->Uninitialize = reinterpret_cast<fnMagUninitialize>(
            GetProcAddress(_hMagModule, "MagUninitialize"));
        _api->SetWindowFilterList = reinterpret_cast<fnMagSetWindowFilterList>(
            GetProcAddress(_hMagModule, "MagSetWindowFilterList"));
        _api->GetWindowFilterList = reinterpret_cast<fnMagGetWindowFilterList>(
            GetProcAddress(_hMagModule, "MagGetWindowFilterList"));
        _api->SetWindowSource = reinterpret_cast<fnMagSetWindowSource>(
            GetProcAddress(_hMagModule, "MagSetWindowSource"));
        _api->GetWindowSource = reinterpret_cast<fnMagGetWindowSource>(
            GetProcAddress(_hMagModule, "MagGetWindowSource"));
        _api->SetImageScalingCallback = reinterpret_cast<fnMagSetImageScalingCallback>(
            GetProcAddress(_hMagModule, "MagSetImageScalingCallback"));
    }

    ret = !(!_hMagModule
        || !_api->Initialize
        || !_api->Uninitialize
        || !_api->SetWindowFilterList
        || !_api->GetWindowFilterList
        || !_api->SetWindowSource
        || !_api->GetWindowSource
        || !_api->SetImageScalingCallback);

    return ret;
}


bool MagCapture::initMagnifier()
{
    if (!loadMagnificationAPI()) {
        return false;
    }

    int32_t maxWidth = 0;
    int32_t maxHeight = 0;

    CapUtility::getMaxResolutionInSystem(&maxWidth, &maxHeight);
    _memoryRect = DesktopRect::MakeLTRB(0, 0, maxWidth, maxHeight);

    DesktopRect &rect = _memoryRect;

    BOOL result = _api->Initialize();
    if (!result) {
        return false;
    }

    HMODULE hInstance = nullptr;
    result =
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<char*>(&DefWindowProc), &hInstance);
    if (!result) {
        _api->Uninitialize();
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
    _hostWnd = CreateWindowExW(WS_EX_LAYERED, kMagnifierHostClass, kHostWindowName, 0, 0, 0, rect.width(),
                               rect.height(), nullptr, nullptr, hInstance, nullptr);
    if (!_hostWnd) {
        _api->Uninitialize();
        return false;
    }

    // Create the magnifier control.
    _magWnd = CreateWindowW(kMagnifierWindowClass, kMagnifierWindowName, WS_CHILD | WS_VISIBLE, 
                            0, 0, rect.width(), rect.height(), _hostWnd, nullptr, hInstance, nullptr);
    if (!_magWnd) {
        _api->Uninitialize();
        return false;
    }

    // Hide the host window.
    ShowWindow(_hostWnd, SW_HIDE);

    // Set the scaling callback to receive captured image.
    result = _api->SetImageScalingCallback(
        _magWnd,
        &MagCapture::OnMagImageScalingCallback);
    if (!result) {
        _api->Uninitialize();
        return false;
    }

    _bMagInit = true;
    return true;
}

bool MagCapture::destoryMagnifier()
{
    bool ret = false;

    if (_hostWnd)
        DestroyWindow(_hostWnd);

     if (_api->Uninitialize)
        _api->Uninitialize();

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
    MagCapture *owner = reinterpret_cast<MagCapture *>(TlsGetValue(GetTlsIndex()));
    
    if (owner)
        owner->onCaptured(srcdata, srcheader);

    return TRUE;
}

bool MagCapture::onCaptured(void *srcdata, MAGIMAGEHEADER header)
{
    int width = _lastRect.width();
    int height = _lastRect.height();
    int stride = width * 4;


    if ( header.stride < stride )
    {
        return false;
    }
    if (_offset != 0 && (_offset != header.offset)) {
        return false;
    }

    logger::log(LogLevel::Info, " %dx%d,%d, %d, %d", header.width, header.height, header.offset, header.stride, header.cbSize);

    int bpp = header.cbSize / header.width / header.height; // bpp should be 4
    if (!_frames.get() || header.format != GUID_WICPixelFormat32bppRGBA 
        || width != static_cast<UINT>(_frames->width()) || height != static_cast<UINT>(_frames->height())
        || stride != static_cast<UINT>(_frames->stride()) || bpp != CapUtility::kDesktopCaptureBPP) {
        _frames.reset(VideoFrame::MakeFrame(width, height, stride,
                                            VideoFrame::VideoFrameType::kVideoFrameTypeRGBA));
    }

    {
        uint8_t *pDst = reinterpret_cast<uint8_t *>(_frames->data());
        uint8_t *pSrc = reinterpret_cast<uint8_t *>(srcdata) + header.offset;

        for (int i = 0; i < height; i++) {
            memcpy(pDst, pSrc, stride);
            pDst += stride;
            pSrc += header.stride;
        }

        _offset = header.offset;
    }

    {
        std::lock_guard<decltype(_cbMutex)> guard(_cbMutex);

        if (_callback) {
            _callback(_frames.get(), _callbackargs);
        }
    }

    return true;
}

bool MagCapture::setCallback(funcCaptureCallback fcb, void* args)
{
    {
        std::lock_guard<decltype(_cbMutex)> guard(_cbMutex);
        _callback = fcb;
        _callbackargs = args;
    }
    return true;
}


bool MagCapture::captureImage(const DesktopRect& capRect) 
{
    bool bRet = false;
    __try {
        DesktopRect rect = capRect;

        if (!rect.is_empty()) {
            RECT wRect = { rect.left(), rect.top(), rect.right(), rect.bottom() };

            _lastRect = rect;
            TlsSetValue(GetTlsIndex(), this);

            // OnCaptured will be called via OnMagImageScalingCallback and fill in the
            // frame before set_window_source_func_ returns.
            bRet = _api->SetWindowSource(_magWnd, wRect);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        bRet = false;
        logger::log(LogLevel::Info, "exception");
    }

    return bRet;
}


bool MagCapture::setExcludeWindows(HWND hWnd)
{
    bool ret = false;

    if (_bMagInit) {
    }

    _api->SetWindowFilterList(_magWnd, MW_FILTERMODE_EXCLUDE, 1, &hWnd);

    return ret;
}

bool MagCapture::setExcludeWindows(std::vector<HWND> hWnd)
{
    bool ret = false;

    if (_bMagInit) {
    }

    _api->SetWindowFilterList(_magWnd, MW_FILTERMODE_EXCLUDE, hWnd.size(), hWnd.data());

    return ret;
}


bool MagCapture::startCaptureWindow(HWND hWnd)
{
    bool ret = false;

    initMagnifier();

    return ret;
}


bool MagCapture::startCaptureScreen(HMONITOR hMonitor)
{
    bool ret = false;

    initMagnifier();

    return ret;
}

bool MagCapture::stop()
{
    bool ret = false;

    return ret;
}
