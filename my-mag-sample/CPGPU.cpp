#include "stdafx.h"
#include "CPGPU.h"

namespace CPGPU
{



auto CreateD3D11Device(D3D_DRIVER_TYPE const type, UINT flags, Microsoft::WRL::ComPtr<ID3D11Device> &device)
{
    return D3D11CreateDevice(nullptr, type, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, device.GetAddressOf(),
                             nullptr, nullptr);
}

auto CreateD3D11Device(UINT flags)
{
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    HRESULT hr = CreateD3D11Device(D3D_DRIVER_TYPE_HARDWARE, flags, device);
    if (DXGI_ERROR_UNSUPPORTED == hr) {
        hr = CreateD3D11Device(D3D_DRIVER_TYPE_WARP, flags, device);
    }

    return device;
}

auto CreateDXGISwapChain(Microsoft::WRL::ComPtr<ID3D11Device> const device,
                         uint32_t width,
                         uint32_t height,
                         DXGI_FORMAT format,
                         uint32_t bufferCount)
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = format;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferCount = bufferCount;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    Microsoft::WRL::ComPtr<IDXGIDevice2> dxgi;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain;

    auto hr = device.As(&dxgi);
    if (FAILED(hr)) {
    }
    hr = dxgi->GetParent(_uuidof(IDXGIAdapter), (void **)&adapter);
    if (FAILED(hr)) {
    }
    hr = adapter->GetParent(_uuidof(IDXGIFactory2), (void **)&factory);
    if (FAILED(hr)) {
    }
    hr = factory->CreateSwapChainForComposition(device.Get(), &desc, nullptr, swapchain.GetAddressOf());
    if (FAILED(hr)) {
    }

    return swapchain;
}

} // namespace CPGPU