#include "stdafx.h"
#include "DXGICapture.h"
#include "CapUtility.h"

#include "logger.h"

DXGICapture::DXGICapture()
{
    _loadD3D11();
}

DXGICapture::~DXGICapture()
{
    _deinit();
}

bool DXGICapture::_loadD3D11()
{
    bool bRet = false;

    HMODULE hm = LoadLibraryW(L"D3D11.dll");
    if (hm) {
        fnD3D11CreateDevice = reinterpret_cast<funcD3D11CreateDevice>(GetProcAddress(hm, "D3D11CreateDevice"));
    }

    bRet = !!hm && !!fnD3D11CreateDevice;

    return bRet;
}

bool DXGICapture::_init(HMONITOR &hm)
{
    bool bRet = false;
    HRESULT hr = E_FAIL;
    const char *info = nullptr;

    do {
        if (!fnD3D11CreateDevice) {
            info = "Load D3D11";
            break;
        }

        D3D_DRIVER_TYPE DriverTypes[] = {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

        D3D_FEATURE_LEVEL FeatureLevels[]
            = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_1 };
        UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

        D3D_FEATURE_LEVEL FeatureLevel;

        for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex) {
            hr = fnD3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels,
                                   D3D11_SDK_VERSION, &_device, &FeatureLevel, &_deviceContext);
            if (SUCCEEDED(hr)) {
                break;
            }
        }
        if (FAILED(hr)) {
            break;
        }

        ComPtr<IDXGIDevice> hDxgiDevice = NULL;
        hr = _device->QueryInterface(__uuidof(IDXGIDevice), &hDxgiDevice);
        if (FAILED(hr)) {
            break;
        }

        ComPtr<IDXGIAdapter> hDxgiAdapter = NULL;
        hr = hDxgiDevice->GetParent(__uuidof(IDXGIAdapter), &hDxgiAdapter);
        if (FAILED(hr)) {
            break;
        }

        INT nOutput = 0;
        int count = 0;
        IDXGIOutput *hDxgiOutput = NULL;
        do {
            hr = hDxgiAdapter->EnumOutputs(nOutput, &hDxgiOutput);
            if (hr == DXGI_ERROR_NOT_FOUND) {
                break;
            }

            DXGI_OUTPUT_DESC eDesc = {};
            hDxgiOutput->GetDesc(&eDesc);
            if (eDesc.Monitor == hm){
                break;
            }
            ++nOutput;
        } while (SUCCEEDED(hr));

        if (FAILED(hr)) {
            break;
        }

        hDxgiOutput->GetDesc(&_outputDesc);

        ComPtr<IDXGIOutput1> hDxgiOutput1 = NULL;
        hr = hDxgiOutput->QueryInterface(__uuidof(hDxgiOutput1), &hDxgiOutput1);
        if (FAILED(hr)) {
            break;
        }

        hr = hDxgiOutput1->DuplicateOutput(_device.Get(), &_desktopDuplication);
        if (FAILED(hr)) {
            break;
        }
    } while (0);

    bRet = SUCCEEDED(hr);
    if (!bRet) {
    
    }

    return bRet;
}

void DXGICapture::_deinit()
{

}

bool DXGICapture::onCaptured(DXGI_MAPPED_RECT &rect, DXGI_OUTPUT_DESC &header)
{
    bool bRet = false;
    int x = abs(_lastRect.left());
    int y = abs(_lastRect.top());
    int width = _lastRect.width();
    int height = _lastRect.height();
    int stride = width * 4;
    auto inStride = rect.Pitch;

    uint8_t *pBits = (uint8_t *)rect.pBits;
    DesktopRect capRect = DesktopRect::MakeRECT(_outputDesc.DesktopCoordinates);

    int bpp = CapUtility::kDesktopCaptureBPP;
    if (!_frames.get() || width != static_cast<UINT>(_frames->width()) || height != static_cast<UINT>(_frames->height())
        || stride != static_cast<UINT>(_frames->stride()) || bpp != CapUtility::kDesktopCaptureBPP) {
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

bool DXGICapture::startCaptureWindow(HWND hWnd)
{
    bool bRet = false;
    return bRet;
}

bool DXGICapture::startCaptureScreen(HMONITOR hMonitor)
{
    bool bRet = false;

    bRet = _init(hMonitor);

    return bRet;
}

bool DXGICapture::stop()
{
    bool bRet = true;
    return bRet;
}

bool DXGICapture::captureImage(const DesktopRect &rect)
{
    bool bRet = false;

    _lastRect = rect;

    ComPtr<IDXGIResource> hDesktopResource = NULL;
    DXGI_OUTDUPL_FRAME_INFO FrameInfo;
    HRESULT hr = _desktopDuplication->AcquireNextFrame(0, &FrameInfo, &hDesktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return TRUE;
    }
    else if (FAILED(hr)) {
        return TRUE;
    }

    ComPtr<ID3D11Texture2D> hAcquiredDesktopImage = NULL;
    hr = hDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), &hAcquiredDesktopImage);
    if (FAILED(hr)) {
        return FALSE;
    }

    D3D11_TEXTURE2D_DESC fDesc;
    hAcquiredDesktopImage->GetDesc(&fDesc);
    ComPtr<ID3D11Texture2D> hNewDesktopImage2 = NULL;
    fDesc.Usage = D3D11_USAGE_STAGING;
    fDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    fDesc.BindFlags = 0;
    fDesc.MiscFlags = 0;
    fDesc.MipLevels = 1;
    fDesc.ArraySize = 1;
    fDesc.SampleDesc.Count = 1;
    hr = _device->CreateTexture2D(&fDesc, NULL, &hNewDesktopImage2);
    if (FAILED(hr)) {
        _desktopDuplication->ReleaseFrame();
        return FALSE;
    }
#if 0 // draw cursor info
    hAcquiredDesktopImage->GetDesc(&fDesc);
    ComPtr<ID3D11Texture2D> hNewDesktopImage = NULL;
    fDesc.Usage = D3D11_USAGE_DEFAULT;
    fDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    fDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
    fDesc.MipLevels = 1;
    fDesc.ArraySize = 1;
    fDesc.SampleDesc.Count = 1;
    fDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    hr = _device->CreateTexture2D(&fDesc, NULL, &hNewDesktopImage);
    if (FAILED(hr)) {
        _desktopDuplication->ReleaseFrame();
        return FALSE;
    }

    _deviceContext->CopyResource(hNewDesktopImage.Get(), hAcquiredDesktopImage.Get());
    _desktopDuplication->ReleaseFrame();

    {
        ComPtr<IDXGISurface1> hStagingSurf = NULL;
        hr = hNewDesktopImage->QueryInterface(__uuidof(IDXGISurface1), &hStagingSurf);
        if (SUCCEEDED(hr)) {
            CURSORINFO lCursorInfo = { 0 };

            lCursorInfo.cbSize = sizeof(lCursorInfo);
            auto lBoolres = GetCursorInfo(&lCursorInfo);

            if (lBoolres == TRUE) {
                if (lCursorInfo.flags == CURSOR_SHOWING) {
                    auto lCursorPosition = lCursorInfo.ptScreenPos;
                    auto lCursorSize = lCursorInfo.cbSize;

                    HDC lHDC;
                    HRESULT tHr = hStagingSurf->GetDC(FALSE, &lHDC);
                    if (SUCCEEDED(tHr))
                        DrawIconEx(lHDC, lCursorPosition.x, lCursorPosition.y, lCursorInfo.hCursor, 0, 0, 0, 0,
                                   DI_NORMAL | DI_DEFAULTSIZE);
                    hStagingSurf->ReleaseDC(nullptr);
                }
            }
        }
    }

    _deviceContext->CopyResource(hNewDesktopImage2.Get(), hNewDesktopImage.Get());
#else
    _deviceContext->CopyResource(hNewDesktopImage2.Get(), hAcquiredDesktopImage.Get());
    _desktopDuplication->ReleaseFrame();
#endif

    ComPtr<IDXGISurface1> hStagingSurf = NULL;
    hr = hNewDesktopImage2->QueryInterface(__uuidof(IDXGISurface1), &hStagingSurf);
    if (FAILED(hr)) {
        return FALSE;
    }

    DXGI_MAPPED_RECT mappedRect;
    hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
    if (SUCCEEDED(hr)) {
        onCaptured(mappedRect, _outputDesc);
        hStagingSurf->Unmap();
    }

    bRet = SUCCEEDED(hr);

    return bRet;
}

bool DXGICapture::setCallback(funcCaptureCallback fcb, void *args)
{
    bool bRet = true;

    std::lock_guard<decltype(_cbMutex)> guard(_cbMutex);
    _callback = fcb;
    _callbackargs = args;

    return bRet;
}

bool DXGICapture::setExcludeWindows(HWND hWnd)
{
    bool bRet = true;

    return bRet;
}

bool DXGICapture::setExcludeWindows(std::vector<HWND> hWnd)
{
    bool bRet = false;
    return bRet;
}

const char *DXGICapture::getName()
{
    return "DXGI Capture";
}

bool DXGICapture::usingTimer()
{
    return false;
}