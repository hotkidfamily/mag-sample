#pragma once

#include<d3d11.h>
#include<wrl/client.h>
#include<dxgi1_2.h>


namespace CPGPU {
auto CreateD3D11Device(D3D_DRIVER_TYPE const type, UINT flags, Microsoft::WRL::ComPtr<ID3D11Device> &device);
auto CreateD3D11Device(UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT);
auto CreateDXGISwapChain(Microsoft::WRL::ComPtr<ID3D11Device> const device,
                         uint32_t width,
                         uint32_t height,
                         DXGI_FORMAT format,
                         uint32_t bufferCount);
};
