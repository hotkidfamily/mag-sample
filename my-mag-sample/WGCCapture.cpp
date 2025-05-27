#include "stdafx.h"
#include "WGCCapture.h"

#include <functional>

#include "CapUtility.h"
#include "CPGPU.h"

#pragma comment(lib, "windowsapp.lib")

using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics;

WGCCapture::WGCCapture()
{
}

WGCCapture::~WGCCapture()
{
}

bool WGCCapture::startCaptureWindow(HWND hWnd)
{
    _hwnd = hWnd;

    //_exit = false;
    //_thread = std::thread(std::bind(&WGCCapture::_run, this));
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    // Create Direct 3D Device
    winrt::com_ptr<ID3D11Device> d3dDevice;
    winrt::check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                           nullptr, 0, D3D11_SDK_VERSION, d3dDevice.put(), nullptr, nullptr));
    winrt::com_ptr<IDXGIDevice> dxgiDevice = d3dDevice.as<IDXGIDevice>();
    winrt::com_ptr<IInspectable> inspectable;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put()));
    auto device = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    const auto activationFactory = winrt::get_activation_factory<GraphicsCaptureItem>();
    auto interopFactory = activationFactory.as<IGraphicsCaptureItemInterop>();
    GraphicsCaptureItem captureItem = { nullptr };
    if (_hwnd != nullptr) {
        winrt::check_hresult(interopFactory->CreateForWindow(
            _hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
            reinterpret_cast<void **>(winrt::put_abi(captureItem))));
    }
    else {
        winrt::check_hresult(interopFactory->CreateForMonitor(
            _hmonitor, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
            reinterpret_cast<void **>(winrt::put_abi(captureItem))));
    }

    auto sz = captureItem.Size();
    auto framePool = Direct3D11CaptureFramePool::Create(
        device, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, sz);
    auto session = framePool.CreateCaptureSession(captureItem);
    auto revoker = framePool.FrameArrived(winrt::auto_revoke, { this, &WGCCapture::_onFrameArrived });

    session.IsCursorCaptureEnabled(false);
    session.StartCapture();

    _session = session;
    _d3d11device = d3dDevice;
    _framePool = framePool;
    _item = captureItem;
    _d3dDevice = device;
    _frame_arrived_revoker = std::move(revoker);
    _curFramePoolSz = sz;

    return true;
}

bool WGCCapture::startCaptureScreen(HMONITOR hMonitor)
{
    _hmonitor = hMonitor;
    //_exit = false;
    //_thread = std::thread(std::bind(&WGCCapture::_run, this));

    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    // Create Direct 3D Device
    winrt::com_ptr<ID3D11Device> d3dDevice;
    winrt::check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                           nullptr, 0, D3D11_SDK_VERSION, d3dDevice.put(), nullptr, nullptr));
    winrt::com_ptr<IDXGIDevice> dxgiDevice = d3dDevice.as<IDXGIDevice>();
    winrt::com_ptr<IInspectable> inspectable;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put()));
    auto device = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    const auto activationFactory = winrt::get_activation_factory<GraphicsCaptureItem>();
    auto interopFactory = activationFactory.as<IGraphicsCaptureItemInterop>();
    GraphicsCaptureItem captureItem = { nullptr };

    winrt::check_hresult(interopFactory->CreateForMonitor(
        _hmonitor, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
        reinterpret_cast<void **>(winrt::put_abi(captureItem))));

    auto sz = captureItem.Size();
    auto framePool = Direct3D11CaptureFramePool::Create(
        device, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, sz);
    auto session = framePool.CreateCaptureSession(captureItem);
    auto revoker = framePool.FrameArrived(winrt::auto_revoke, { this, &WGCCapture::_onFrameArrived });

    session.IsCursorCaptureEnabled(false);
    session.StartCapture();

    _session = session;
    _d3d11device = d3dDevice;
    _framePool = framePool;
    _item = captureItem;
    _d3dDevice = device;
    _frame_arrived_revoker = std::move(revoker);
    _curFramePoolSz = sz;

    return true;
}

bool WGCCapture::stop()
{
    //_exit = true;
    //if (_thread.joinable()) {
    //    _thread.join();
    //}

    _frame_arrived_revoker.revoke();
    _framePool.Close();
    _session.Close();

    _framePool = nullptr;
    _session = nullptr;
    _item = nullptr;
    _hwnd = nullptr;

    //winrt::uninit_apartment();

    return true;
}

bool WGCCapture::captureImage(const DesktopRect &rect)
{
    _lastRect = rect;
    return true;
}

bool WGCCapture::setCallback(funcCaptureCallback fcb, void *args)
{
    std::lock_guard<decltype(_cbMutex)> guard(_cbMutex);
    _callback = fcb;
    _callbackargs = args;

    return false;
}

bool WGCCapture::setExcludeWindows(std::vector<HWND> &hWnd)
{
    return false;
}

const char *WGCCapture::getName()
{
    return "Windows.Graphics.Capture";
}

bool WGCCapture::usingTimer()
{
    return false;
}

bool WGCCapture::onCaptured(D3D11_MAPPED_SUBRESOURCE &rect, D3D11_TEXTURE2D_DESC &header)
{
    bool bRet = false;
    int x = abs(_lastRect.left());
    int y = abs(_lastRect.top());
    int width = _lastRect.width();
    int height = _lastRect.height();
    int stride = width * 4;
    auto inStride = rect.RowPitch;

    if (_lastRect.is_empty()) {
        width = header.Width;
        height = header.Height;
        stride = width * 4;
    }

    uint8_t *pBits = (uint8_t *)rect.pData;
    DesktopRect capRect = DesktopRect::MakeXYWH(0, 0, header.Width, header.Height);

    int bpp = CapUtility::kDesktopCaptureBPP;
    if (!_frames.get() || width != _frames->width() || height != _frames->height() || stride != _frames->stride()
        || bpp != CapUtility::kDesktopCaptureBPP) {
        _frames.reset(VideoFrame::MakeFrame(width, height, stride, VideoFrameType::kVideoFrameTypeRGBA));
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


void WGCCapture::_onFrameArrived(winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const &sender,
                                 winrt::Windows::Foundation::IInspectable const &handler)
{
    winrt::com_ptr<ID3D11Texture2D> texture;
    auto frame = sender.TryGetNextFrame();
    auto contentSz = frame.ContentSize();
    auto access = frame.Surface().as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
    access->GetInterface(winrt::guid_of<ID3D11Texture2D>(), texture.put_void());

    winrt::com_ptr<ID3D11DeviceContext> ctx;
    _d3d11device->GetImmediateContext(ctx.put());
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    // desc.Usage = D3D11_USAGE_STAGING;
    // desc.BindFlags = 0;
    // desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    // desc.MiscFlags = 0;

    // winrt::com_ptr<ID3D11Texture2D> userTexture = nullptr;
    // winrt::check_hresult(_dev->CreateTexture2D(&desc, NULL, userTexture.put()));

    if (!_tempTexture) {
        winrt::check_hresult(CPGPU::MakeTex(_d3d11device.get(), desc.Width, desc.Height, desc.Format,
                                            _tempTexture.put(), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ));
    }

    D3D11_TEXTURE2D_DESC desc2;
    _tempTexture->GetDesc(&desc2);
    if (desc2.Width != desc.Width || desc2.Height != desc.Height) {
        winrt::check_hresult(CPGPU::MakeTex(_d3d11device.get(), desc.Width, desc.Height, desc.Format,
                                            _tempTexture.put(), D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ));
    }

    ctx->CopyResource(_tempTexture.get(), texture.get());
    D3D11_MAPPED_SUBRESOURCE resource;
    winrt::check_hresult(ctx->Map(_tempTexture.get(), NULL, D3D11_MAP_READ, 0, &resource));
    {
        D3D11_TEXTURE2D_DESC desc{ 0 };
        _tempTexture->GetDesc(&desc);
        onCaptured(resource, desc);
    }
    ctx->Unmap(_tempTexture.get(), 0);
    return;
}
