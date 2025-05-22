#include "stdafx.h"
#include "gpukits.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace gpukits
{

HRESULT CompileVertexShader(LPCWSTR srcFile,
                            LPCSTR entryPoint,
                            ID3D11Device *device,
                            ID3DBlob **blob,
                            const D3D_SHADER_MACRO defines[])
{
    if (!srcFile || !entryPoint || !device || !blob)
        return E_INVALIDARG;

    *blob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // We generally prefer to use the higher vs shader profile when possible
    // as vs 5.0 is better performance on 11-class hardware
    LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "vs_5_0" : "vs_4_0";

    ID3DBlob *shaderBlob = nullptr;
    ID3DBlob *errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0,
                                    &shaderBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char *)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return hr;
    }

    *blob = shaderBlob;

    return hr;
}

HRESULT CompilePixelShader(LPCWSTR srcFile,
                           LPCSTR entryPoint,
                           ID3D11Device *device,
                           ID3DBlob **blob,
                           const D3D_SHADER_MACRO defines[])
{
    if (!srcFile || !entryPoint || !device || !blob)
        return E_INVALIDARG;

    *blob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // We generally prefer to use the higher CS shader profile when possible
    // as ps 5.0 is better performance on 11-class hardware
    LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "ps_5_0" : "ps_4_0";

    ID3DBlob *shaderBlob = nullptr;
    ID3DBlob *errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0,
                                    &shaderBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char *)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return hr;
    }

    *blob = shaderBlob;

    return hr;
}

HRESULT CompileComputeShader(LPCWSTR srcFile,
                             LPCSTR entryPoint,
                             ID3D11Device *device,
                             ID3DBlob **blob,
                             const D3D_SHADER_MACRO defines[])
{
    if (!srcFile || !entryPoint || !device || !blob)
        return E_INVALIDARG;

    *blob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // We generally prefer to use the higher CS shader profile when possible
    // as CS 5.0 is better performance on 11-class hardware
    LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";

    ID3DBlob *shaderBlob = nullptr;
    ID3DBlob *errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0,
                                    &shaderBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char *)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return hr;
    }

    *blob = shaderBlob;

    return hr;
}


HRESULT MakeTex(ID3D11Device *device,
                       int w,
                       int h,
                       DXGI_FORMAT fmt,
                       ID3D11Texture2D **text,
                       D3D11_USAGE Usage,
                       UINT BindFlags,
                       UINT CPUAccessFlags)
{
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = fmt;
    desc.SampleDesc = { 1, 0 };
    desc.Usage = Usage;
    desc.BindFlags = BindFlags;
    desc.CPUAccessFlags = CPUAccessFlags;

    HRESULT hr = device->CreateTexture2D(&desc, NULL, text);
    return hr;
}



HRESULT MakeSRV(ID3D11Device *device, ID3D11Texture2D *text, ID3D11ShaderResourceView **res)
{
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
    text->GetDesc(&desc);

    D3D11_SHADER_RESOURCE_VIEW_DESC resourceDesc;
    ZeroMemory(&resourceDesc, sizeof(resourceDesc));
    resourceDesc.Texture2D.MostDetailedMip = 0;
    resourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    resourceDesc.Texture2D.MipLevels = desc.MipLevels;
    resourceDesc.Format = desc.Format;
    hr = device->CreateShaderResourceView(text, &resourceDesc, res);

    return hr;
}

HRESULT MakeTexAndSRV(ID3D11Device *device,
                      int w,
                      int h,
                      DXGI_FORMAT fmt,
                      ID3D11Texture2D **text,
                      ID3D11ShaderResourceView **res,
                      D3D11_USAGE usage,
                      UINT bindflags,
                      UINT cpuflags)
{
    HRESULT hr = MakeTex(device, w, h, fmt, text, usage, bindflags, cpuflags);
    if (SUCCEEDED(hr)) {
        hr = MakeSRV(device, *text, res);
    }
    return hr;
}


} // namespace gpukits