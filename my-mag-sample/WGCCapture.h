#pragma once

#include <winrt/base.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/windows.graphics.capture.h>
#include <windows.graphics.capture.interop.h>
#include <winrt/windows.graphics.directx.h>
#include <winrt/windows.graphics.directx.direct3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <d3d11.h>

#include "capturer-define.h"
#include <memory>
#include <mutex>
#include "VideoFrame.h"


class WGCCapture : public CCapture {
  public:
    WGCCapture();
    ~WGCCapture();

  public:
    virtual bool startCaptureWindow(HWND hWnd) final;
    virtual bool startCaptureScreen(HMONITOR hMonitor) final;
    virtual bool stop() final;
    virtual bool captureImage(const DesktopRect &rect) final;
    virtual bool setCallback(funcCaptureCallback, void *) final;
    virtual bool setExcludeWindows(std::vector<HWND>& hWnd) final;
    virtual const char *getName() final;
    virtual bool usingTimer() final;

    bool onCaptured(D3D11_MAPPED_SUBRESOURCE &rect, D3D11_TEXTURE2D_DESC &header);

  private:
    void _onFrameArrived(winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const &sender,
                         winrt::Windows::Foundation::IInspectable const &);
    void _run();

  private:
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem _item{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool _framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession _session{ nullptr };
    winrt::com_ptr<ID3D11Device> _d3d11device{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker _frame_arrived_revoker;
    winrt::com_ptr<ID3D11Texture2D> _tempTexture{ nullptr };
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice _d3dDevice{ nullptr };

    std::thread _thread;
    bool _exit = true;

    HWND _hwnd = nullptr;
    HMONITOR _hmonitor = nullptr;

    std::unique_ptr<VideoFrame> _frames;
    DesktopRect _lastRect = {};

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;

};
