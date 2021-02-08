#pragma once
#include "capturer-define.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <memory>
#include <mutex>
#include "VideoFrame.h"

using namespace Microsoft::WRL;

typedef HRESULT(WINAPI *funcD3D11CreateDevice)(__in_opt IDXGIAdapter *pAdapter,
                                             D3D_DRIVER_TYPE DriverType,
                                             HMODULE Software,
                                             UINT Flags,
                                             __in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL *pFeatureLevels,
                                             UINT FeatureLevels,
                                             UINT SDKVersion,
                                             __out_opt ID3D11Device **ppDevice,
                                             __out_opt D3D_FEATURE_LEVEL *pFeatureLevel,
                                             __out_opt ID3D11DeviceContext **ppImmediateContext);


class DXGICapture : public CCapture {
  public:
    DXGICapture();
    ~DXGICapture();

  public:
    virtual bool startCaptureWindow(HWND hWnd) override;
    virtual bool startCaptureScreen(HMONITOR hMonitor) override;
    virtual bool stop() override;
    virtual bool captureImage(const DesktopRect &rect) override;
    virtual bool setCallback(funcCaptureCallback, void *) override;
    virtual bool setExcludeWindows(HWND hWnd) override;
    virtual bool setExcludeWindows(std::vector<HWND> hWnd) override;
    virtual const char *getName() override;

  public:
    bool onCaptured(DXGI_MAPPED_RECT &rect, DXGI_OUTPUT_DESC &desc);

  private:
    bool _init(HMONITOR &hm);
    void _deinit();
    bool _loadD3D11();

  private:
    ComPtr<ID3D11Device> _device = nullptr;
    ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;

    ComPtr<IDXGIOutputDuplication> _desktopDuplication = nullptr;
    DXGI_OUTPUT_DESC _outputDesc = {};

    std::unique_ptr<VideoFrame> _frames;
    DesktopRect _lastRect = {};

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;

    funcD3D11CreateDevice fnD3D11CreateDevice;
};
