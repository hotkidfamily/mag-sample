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
        uint8_t *pSrc = reinterpret_cast<uint8_t *>(pBits);

        for (int i = 0; i < height; i++) {
            memcpy(pDst, pSrc, stride);
            pDst += stride;
            pSrc += inStride;
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

    bRet = StretchBlt(_compatibleDC,               // ���浽��Ŀ�� ͼƬ���� ������
           0, 0,    // ��ʼ x, y ����
           rect.width(), rect.height(), // ��ͼ����
           _monitorDC,     // ��ȡ����� �����ľ��
           0,0,     // ָ��Դ�����������Ͻǵ� X, y �߼�����
           rect.width(), rect.height(), // ��ͼ����
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
    return bRet;
}

bool GDICapture::setExcludeWindows(std::vector<HWND> hWnd)
{
    bool bRet = false;
    return bRet;
}