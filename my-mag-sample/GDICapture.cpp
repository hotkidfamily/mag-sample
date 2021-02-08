#include "stdafx.h"
#include "GDICapture.h"

#include "CapUtility.h"
#include "logger.h"

GDICapture::GDICapture()
{
}

GDICapture::~GDICapture()
{

}

bool GDICapture::startCaptureWindow(HWND hWnd)
{
    bool bRet = false;

    HMONITOR hm = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX mInfo;
    ZeroMemory(&mInfo, sizeof(MONITORINFOEX));
    mInfo.cbSize = sizeof(MONITORINFOEX);

    GetMonitorInfoW(hm, &mInfo);

    _monitorDC = CreateDCW(L"myDisplay", mInfo.szDevice, NULL, NULL);
    _compatibleDC = ::CreateCompatibleDC(_monitorDC);

    bRet = _monitorDC && _compatibleDC;
    if (!bRet)
        logger::log(LogLevel::Info, "monitor DC %x, compatible DC %x, bRet %d", _monitorDC, _compatibleDC, bRet);

    return bRet;
}

bool GDICapture::startCaptureScreen(HMONITOR hMonitor)
{
    bool bRet = false;

    MONITORINFOEX mInfo ;
    ZeroMemory(&mInfo, sizeof(MONITORINFOEX));
    mInfo.cbSize = sizeof(MONITORINFOEX);

    GetMonitorInfoW(hMonitor, &mInfo);

    _monitorDC = CreateDCW(L"myDisplay", mInfo.szDevice, NULL, NULL);
    _compatibleDC = ::CreateCompatibleDC(_monitorDC);

    bRet = _monitorDC && _compatibleDC;
    if (!bRet)
        logger::log(LogLevel::Info, "monitor DC %x, compatible DC %x, bRet %d", _monitorDC, _compatibleDC, bRet);

    return bRet;
}

bool GDICapture::stop()
{
    bool bRet = false;

    if (_compatibleDC)
        ReleaseDC(NULL, _compatibleDC);

    if (_monitorDC)
        ReleaseDC(NULL, _monitorDC);

    if (_hDibBitmap)
        ::DeleteObject(_hDibBitmap);

    _compatibleDC = nullptr;
    _monitorDC = nullptr;
    _hDibBitmap = nullptr;

    return bRet;
}

static int ComputePitch(_In_ int nWidth, _In_ int nBPP)
{
    return ((((nWidth * nBPP) + 31) / 32) * 4);
}

bool GDICapture::onCaptured(void *srcdata, BITMAPINFOHEADER &header)
{
    bool bRet = false;
    int x = abs(_lastRect.left());
    int y = abs(_lastRect.top());
    int width = _lastRect.width();
    int height = _lastRect.height();
    int stride = ComputePitch(width, 32);
    auto inStride = ComputePitch( header.biWidth, 32);

    uint8_t *pBits = (uint8_t*)srcdata;

    int bpp = header.biBitCount >> 3; // bpp should be 4
    if (!_frames.get() || width != static_cast<UINT>(_frames->width())
        || height != static_cast<UINT>(_frames->height()) || stride != static_cast<UINT>(_frames->stride())
        || bpp != CapUtility::kDesktopCaptureBPP) {
        _frames.reset(VideoFrame::MakeFrame(width, height, stride, VideoFrame::VideoFrameType::kVideoFrameTypeRGBA));
    }

    {
        uint8_t *pDst = reinterpret_cast<uint8_t *>(_frames->data());
        uint8_t *pSrc = reinterpret_cast<uint8_t *>(pBits);//+y *inStride + x *bpp;

        for (int i = 0; i < height; i++) {
            memcpy(pDst, pSrc, stride);
            pDst += stride;
            pSrc += inStride;
        }
        {
            std::vector<DesktopRect> maskRects;
            for (auto &h : _coverdWindows) {
                CRect rect;
                CapUtility::GetWindowRect(h, rect);
                CRect iRect;
                RECT sRect = { _lastRect.left(), _lastRect.top(), _lastRect.right(), _lastRect.bottom() };
                if (IntersectRect(&iRect, &rect, &sRect)) {
                    int left = iRect.left - sRect.left;
                    int top = iRect.top - sRect.top;

                    uint8_t *pDst
                        = reinterpret_cast<uint8_t *>(_frames->data()) + top * _frames->stride() + left * bpp;
                    int32_t mStride = iRect.Width() * bpp;

                    for (int i = 0; i < iRect.Height(); i++) {
                        memset(pDst, 50, mStride);
                        pDst += _frames->stride();
                    }
                }
            }
        }
    }

    {
        std::lock_guard<decltype(_cbMutex)> guard(_cbMutex);

        if (_callback) {
            _callback(_frames.get(), _callbackargs);
        }
    }

    return bRet;
}

bool GDICapture::captureImage(const DesktopRect &rect)
{
    bool bRet = false;
    BITMAPINFOHEADER &bmiHeader = bmi.bmiHeader;

    if (!_lastRect.equals(rect)) {
        int nBitPerPixel = GetDeviceCaps(_monitorDC, BITSPIXEL);

        memset(&bmi, 0, sizeof(bmi));
        bmiHeader.biSize = sizeof(bmiHeader);
        bmiHeader.biWidth = rect.width();
        bmiHeader.biHeight = -rect.height();
        bmiHeader.biPlanes = 1;
        bmiHeader.biBitCount = USHORT(nBitPerPixel);
        bmiHeader.biCompression = BI_RGB;

        _hDibBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &_hdstPtr, NULL, 0);
        if (_hDibBitmap == NULL) {
            return (FALSE);
        }
    }
    _lastRect = rect;


    HBITMAP hOldBitmap = HBITMAP(::SelectObject(_compatibleDC, _hDibBitmap));

    DIBSECTION dibsection;
    int nBytes;
    nBytes = ::GetObject(_hDibBitmap, sizeof(DIBSECTION), &dibsection);

    bRet = StretchBlt(_compatibleDC,               // 保存到的目标 图片对象 上下文
           0, 0,    // 起始 x, y 坐标
           rect.width(), rect.height(), // 截图宽高
           _monitorDC,     // 截取对象的 上下文句柄
           rect.left(), rect.top(),     // 指定源矩形区域左上角的 X, y 逻辑坐标
           rect.width(), rect.height(), // 截图宽高
           SRCCOPY) == TRUE;

    _hDibBitmap = HBITMAP(::SelectObject(_compatibleDC, hOldBitmap));

    onCaptured(_hdstPtr, bmiHeader);


    return bRet;
}

bool GDICapture::setCallback(funcCaptureCallback fcb, void *args)
{
    bool bRet = false;

    std::lock_guard<decltype(_cbMutex)> guard(_cbMutex);
    _callback = fcb;
    _callbackargs = args;

    return bRet;
}

bool GDICapture::setExcludeWindows(HWND hWnd)
{
    bool bRet = false;
    _coverdWindows.clear();
    _coverdWindows.push_back(hWnd);
    return bRet;
}

bool GDICapture::setExcludeWindows(std::vector<HWND> hWnd)
{
    bool bRet = false;

    _coverdWindows = std::move(hWnd);

    return bRet;
}

const char* GDICapture::getName()
{
    return "GDI capture";
}

bool GDICapture::usingTimer()
{
    return true;
}