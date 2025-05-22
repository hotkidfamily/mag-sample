#pragma once
#include "capturer-define.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <memory>
#include <mutex>
#include "VideoFrame.h"

using namespace Microsoft::WRL;

typedef HRESULT (WINAPI *pfCreateDXGIFactory1)(REFIID riid, _COM_Outptr_ void **ppFactory);

typedef HRESULT (WINAPI *pfD3D11CreateDevice)(__in_opt IDXGIAdapter *pAdapter,
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
    virtual bool startCaptureWindow(HWND hWnd) final;
    virtual bool startCaptureScreen(HMONITOR hMonitor) final;
    virtual bool stop() final;
    virtual bool captureImage(const DesktopRect &rect) final;
    virtual bool setCallback(funcCaptureCallback, void *) final;
    virtual bool setExcludeWindows(HWND hWnd) final;
    virtual bool setExcludeWindows(std::vector<HWND> hWnd) final;
    virtual const char *getName() final;
    virtual bool usingTimer() final;

  public:
    bool onCaptured(DXGI_MAPPED_RECT &rect, DXGI_OUTPUT_DESC &desc);

  private:
    bool _init(HMONITOR &hm);
    void _deinit();
    bool _loadD3D11();

  private:
    ComPtr<ID3D11Device> _dev = nullptr;
    ComPtr<ID3D11DeviceContext> _ctx = nullptr;

    ComPtr<IDXGIOutputDuplication> _desktopDuplication = nullptr;
    DXGI_OUTPUT_DESC _outputDesc = {};

    D3D11_TEXTURE2D_DESC _sourceFormat;
    ComPtr<ID3D11Texture2D> _destFrame;

    std::unique_ptr<VideoFrame> _frames;
    DesktopRect _lastRect = {};

    std::recursive_mutex _cbMutex;
    funcCaptureCallback _callback = nullptr;
    void *_callbackargs = nullptr;

    pfD3D11CreateDevice fnD3D11CreateDevice;
    pfCreateDXGIFactory1 fnCreateDXGIFactory1;

    std::vector<HWND> _coverdWindows;
};
