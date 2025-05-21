#pragma once

#include <d3d11.h>

namespace gpukits
{

HRESULT CompileVertexShader(LPCWSTR srcFile,
                            LPCSTR entryPoint,
                            ID3D11Device *device,
                            ID3DBlob **blob,
                            const D3D_SHADER_MACRO defines[] = nullptr);

HRESULT CompilePixelShader(LPCWSTR srcFile,
                           LPCSTR entryPoint,
                           ID3D11Device *device,
                           ID3DBlob **blob,
                           const D3D_SHADER_MACRO defines[] = nullptr);

HRESULT CompileComputeShader(LPCWSTR srcFile,
                           LPCSTR entryPoint,
                           ID3D11Device *device,
                           ID3DBlob **blob,
                           const D3D_SHADER_MACRO defines[] = nullptr);

HRESULT MakeTexAndSRV(ID3D11Device *device,
                      int w,
                      int h,
                      DXGI_FORMAT fmt,
                      ID3D11Texture2D **text,
                      ID3D11ShaderResourceView **res,
                      D3D11_USAGE usage,
                      UINT bindflags,
                      UINT cpuflags);
}
