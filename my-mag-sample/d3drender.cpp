#include "stdafx.h"
#include "d3drender.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")

struct CustomVertex
{
    CustomVertex(float x, float y, float z,
        float u0, float v0,
        float u1, float v1,
        float u2, float v2)
    {
        _x = x;  _y = y; _z = z;
        _u0 = u0; _v0 = v0;
        _u1 = u1; _v1 = v1;
        _u2 = u2, _v2 = v2;
    }

    float _x, _y, _z;
    float _u0, _v0;
    float _u1, _v1;
    float _u2, _v2;

    static const DWORD FVF;
};

const DWORD CustomVertex::FVF = D3DFVF_XYZ | D3DFVF_TEX3;


thread_local TCHAR d3d9_error_string_buffer[1024];
const TCHAR* formatErrMsg(HRESULT hr)
{
    _sntprintf_s(d3d9_error_string_buffer, 1024, _T("%s, %s."), DXGetErrorString(hr), DXGetErrorDescription(hr));
    return d3d9_error_string_buffer;
}

#define FONT_HEIGHT (16)

#define STR(X) #X
const char* yuv2rgb = STR(
sampler YTex;
sampler UTex;
sampler VTex;

struct PS_INPUT
{
    float2 y : TEXCOORD0;
    float2 u : TEXCOORD1;
    float2 v : TEXCOORD2;
};

float4 main(PS_INPUT input) :COLOR0
{
float y = tex2D(YTex,input.y).r;
float u = tex2D(UTex,input.u.xy).r - 0.5f;
float v = tex2D(VTex,input.v.xy).r - 0.5f;

float r = y + 1.5958f * v;
float g = y - 0.39173f * u - 0.8129f * v;
float b = y + 2.017f * u;

return float4(r,g,b, 1.0f);
}
);


const char* NV122rgb = STR(
sampler YTex;
sampler UVTex;


struct PS_INPUT
{
    float2 y : TEXCOORD0;
    float2 uv : TEXCOORD1;
};


float4 main(PS_INPUT input) :COLOR0
{
float y = tex2D(YTex,input.y).r;
//这里不需要除以2
float u = tex2D(UTex,input.uv.xy).r - 0.5f;
float v = tex2D(VTex,input.uv.xy).g - 0.5f;

float r = y + 1.14f * v;
float g = y - 0.394f * u - 0.581f * v;
float b = y + 2.03f * u;

return float4(r,g,b, 1.0f);
}

);

#undef STR

#define ERROR_MSG(X) do { MessageBox(NULL, X, _T("Error"), MB_ICONEXCLAMATION | MB_OK); } while (0)
#define HR_CHECK(X) do { HRESULT hr = (X); if (FAILED(hr)) { __debugbreak(); } } while (0)

HRESULT compile_shader(IDirect3DDevice9* device, 
    const char* strVsShader,
    IDirect3DVertexShader9** iVsShader,
    ID3DXConstantTable** vsShaderTable,
    const char* vsProfile,
    const char* strPsShader, 
    IDirect3DPixelShader9** iPsShader, 
    ID3DXConstantTable** psShaderTable,
    const char* psProfile
    )
{
    CComPtr<ID3DXBuffer> compiled_shader;
    CComPtr<ID3DXBuffer> error_msgs;
    HRESULT hRet = S_OK;

    if (strVsShader) {
        hRet = D3DXCompileShader(strVsShader, (UINT)strlen(strVsShader), NULL, NULL, "main", vsProfile, 
            D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, &compiled_shader, &error_msgs, vsShaderTable);

        if (FAILED(hRet)) {
            const TCHAR* error_str = (const TCHAR*)error_msgs->GetBufferPointer();
            ERROR_MSG(error_str);
            return hRet;
        }
        else {
            hRet = device->CreateVertexShader((const DWORD*)compiled_shader->GetBufferPointer(), iVsShader);
            if (FAILED(hRet)) {
                ERROR_MSG(_T("Failed to create shader."));
                return hRet;
            }
        }
        compiled_shader.Release();
        error_msgs.Release();
    }
   
    if (strPsShader) {
        hRet = D3DXCompileShader(strPsShader, (UINT)strlen(strPsShader), NULL, NULL, "main", psProfile, 
            D3DXSHADER_DEBUG | D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY, &compiled_shader, &error_msgs, psShaderTable);

        if (FAILED(hRet)) {
            const char* error_str = (const char*)error_msgs->GetBufferPointer();
            //ERROR_MSG(error_str);
            return hRet;
        }
        else {
            hRet = device->CreatePixelShader((const DWORD*)compiled_shader->GetBufferPointer(), iPsShader);
            if (FAILED(hRet)) {
                ERROR_MSG(_T("Failed to create shader."));
                return hRet;
            }
        }

        compiled_shader.Release();
        error_msgs.Release();
    }

    return hRet;
}

d3drender::d3drender()
{
}


d3drender::~d3drender()
{
}

HRESULT d3drender::init(HWND hwnd)
{
    HRESULT hRet = S_OK;
    TCHAR* info = _T("OK");
    _hwnd = hwnd;

    do {
        // Setup D3D9.
        _d3d9Api = Direct3DCreate9(D3D_SDK_VERSION);
        if (!_d3d9Api) {
            info = _T("Failed to initialize Direct3D 9 - the application was built against the correct header files.");
            break;
        }
        break;

        CRect wndRect;
        GetWindowRect(hwnd, &wndRect);

        _lastRect = wndRect;

        HR_CHECK(_resetDevice(wndRect));

        HR_CHECK(_reallocTexture(wndRect.Size(), _curPixType));
        HR_CHECK(_compile_sharder(_curPixType));
        HR_CHECK(_setupVertex(_curPixType));


    } while (0);

    return hRet;
}

HRESULT d3drender::_resetDevice(CRect &wndRect)
{
    HRESULT hRet = S_OK;
    TCHAR* info = nullptr;

    release();

    do {
        int32_t width = wndRect.Width();
        int32_t height = wndRect.Height();

        D3DDISPLAYMODE mode;
        hRet = _d3d9Api->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
        if (FAILED(hRet)) {
            info = _T("Direct3D 9 was unable to get adapter display mode.");
            break;
        }

        hRet = _d3d9Api->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, D3DFMT_A8R8G8B8, true);
        if (FAILED(hRet)) {
            info = _T("HAL was detected as not supported by DirectD 9 for the D3DFMT_A8R8G8B8 adapter/backbuffer format.");
            break;
        }

        D3DPRESENT_PARAMETERS params;
        D3DCAPS9 caps;

        hRet = _d3d9Api->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
        if (FAILED(hRet)) {
            info = _T("Failed to gather Direct3D 9 _deviceice caps.");
            break;
        }

        int flags = 0;
        if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
            flags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
            assert(caps.VertexProcessingCaps);
        }
        else {
            flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
            assert(caps.VertexProcessingCaps);
        }

        // 默认不使用多采样
        D3DMULTISAMPLE_TYPE multiType = D3DMULTISAMPLE_NONE;
        if (_d3d9Api->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, TRUE, D3DMULTISAMPLE_4_SAMPLES, NULL) == D3D_OK)
        {
            // 保存多采样类型
            multiType = D3DMULTISAMPLE_4_SAMPLES;
        }

        ZeroMemory(&params, sizeof(params));
        params.BackBufferWidth = width;
        params.BackBufferHeight = height;
        params.BackBufferFormat = D3DFMT_A8R8G8B8;
        params.BackBufferCount = 1;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.hDeviceWindow = _hwnd;
        params.Windowed = true;
        params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
        params.MultiSampleType = multiType;
        params.MultiSampleQuality = 0;

        hRet = _d3d9Api->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _hwnd, flags, &params, &_device);
        if (FAILED(hRet)) {
            info = _T("Failed to create Direct3D 9 device.");
            break;
        }


        HR_CHECK(_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
        HR_CHECK(_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
        HR_CHECK(_device->SetRenderState(D3DRS_LIGHTING, FALSE));

        // Setup global render state.
//         HR_CHECK(_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR));
//         HR_CHECK(_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR));
        HR_CHECK(_device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
        HR_CHECK(_device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC));
        
        HR_CHECK(hRet = D3DXCreateFont(_device, FONT_HEIGHT, 0, FW_NORMAL, 0, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Calibri"), &_printer));

        _lastRect = wndRect;

    } while (0);

    return hRet;
}

HRESULT d3drender::_compile_sharder(VideoFrame::VideoFrameType type)
{
    HRESULT hRet = S_OK;

    const char* vs_profile = D3DXGetVertexShaderProfile(_device);
    const char* ps_profile = D3DXGetPixelShaderProfile(_device);

    ps_profile = "ps_2_0";

    _Ps.Release();
    _psCT.Release();

    switch (type) {
    case VideoFrame::VideoFrameType::kVideoFrameTypeI420:
        {
            hRet = compile_shader(_device, NULL, NULL, NULL, NULL, yuv2rgb, &_Ps, &_psCT, ps_profile);
        }
        break;
    case VideoFrame::VideoFrameType::kVideoFrameTypeNV12:
        {
            hRet = compile_shader(_device, NULL, NULL, NULL, NULL, NV122rgb, &_Ps, &_psCT, ps_profile);
        }
        break;
    }

    return hRet;
}

HRESULT d3drender::_setupVertex(VideoFrame::VideoFrameType type)
{
    HRESULT hRet = S_OK;

    _vertexBuffer.Release();

    hRet = _device->CreateVertexBuffer(6 * sizeof(CustomVertex),
        D3DUSAGE_WRITEONLY, CustomVertex::FVF, D3DPOOL_MANAGED, &_vertexBuffer, NULL);

    CustomVertex* v = 0;
    HR_CHECK(_vertexBuffer->Lock(0, 0, (void**)&v, 0));

    v[0] = CustomVertex(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    v[1] = CustomVertex(-1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    v[2] = CustomVertex( 1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);

    v[3] = CustomVertex(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    v[4] = CustomVertex( 1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    v[5] = CustomVertex( 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    _vertexBuffer->Unlock();


    return S_OK;
}

HRESULT d3drender::_reallocTexture(CSize destBufSize, VideoFrame::VideoFrameType destBufFormat)
{
    HRESULT hRet = S_OK;
    auto &width = destBufSize.cx;
    auto &height = destBufSize.cy;

    _renderTex.Release();
    _ytexture.Release();
    _utexture.Release();
    _vtexture.Release();

    // Create off-screen texture.
    hRet = _device->CreateTexture(width, height, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &_renderTex, NULL);

    switch (destBufFormat)
    {
    case VideoFrame::VideoFrameType::kVideoFrameTypeI420:
        {
            if (SUCCEEDED(hRet)) {
                hRet = _device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &_ytexture, NULL);
            }

            if (SUCCEEDED(hRet)) {
                hRet = _device->CreateTexture(width / 2, height / 2, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &_utexture, NULL);
            }

            if (SUCCEEDED(hRet)) {
                hRet = _device->CreateTexture(width / 2, height / 2, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &_vtexture, NULL);
            }

            if (SUCCEEDED(hRet)) {
                _YTexHandle = _psCT->GetConstantByName(0, "YTex");
                _UTexHandle = _psCT->GetConstantByName(0, "UTex");
                _VTexHandle = _psCT->GetConstantByName(0, "VTex");

                UINT count;

                _psCT->GetConstantDesc(_YTexHandle, &_YTexDesc, &count);
                _psCT->GetConstantDesc(_UTexHandle, &_UTexDesc, &count);
                _psCT->GetConstantDesc(_VTexHandle, &_VTexDesc, &count);
                _psCT->SetDefaults(_device);

                _device->SetTexture(_YTexDesc.RegisterIndex, _ytexture);

                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);

                _device->SetTexture(_UTexDesc.RegisterIndex, _utexture);

                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);

                _device->SetTexture(_VTexDesc.RegisterIndex, _vtexture);

                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
            }
        }

        break;
        case VideoFrame::VideoFrameType::kVideoFrameTypeNV12:
        {
            hRet = _device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &_ytexture, NULL);

            if (SUCCEEDED(hRet)) {
                hRet = _device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT, &_utexture, NULL);
            }

            if (SUCCEEDED(hRet)) {
                _YTexHandle = _psCT->GetConstantByName(0, "YTex");
                _UTexHandle = _psCT->GetConstantByName(0, "UTex");

                UINT count;

                _psCT->GetConstantDesc(_YTexHandle, &_YTexDesc, &count);
                _psCT->GetConstantDesc(_UTexHandle, &_UTexDesc, &count);
                _psCT->SetDefaults(_device);

                _device->SetTexture(_YTexDesc.RegisterIndex, _ytexture);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);

                _device->SetTexture(_UTexDesc.RegisterIndex, _utexture);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
                _device->SetSamplerState(_YTexDesc.RegisterIndex, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
            }
        }
        break;
    case VideoFrame::VideoFrameType::kVideoFrameTypeRGB24:
    case VideoFrame::VideoFrameType::kVideoFrameTypeBGRA:
    case VideoFrame::VideoFrameType::kVideoFrameTypeRGBA:
        {
            hRet = _device->CreateTexture(width, height, 0, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &_ytexture, NULL);
        }
        break;
    }

    if (SUCCEEDED(hRet)) {
        _curBuffSize = destBufSize;
        _curPixType = destBufFormat;
    }

    return hRet;
}

void fast_unpack(char* rgba, const char* rgb, const int count)
{
    if (count == 0)
        return;
    for (int i = count; --i; rgba += 4, rgb += 3) {
        *(uint32_t*)(void*)rgba = *(const uint32_t*)(const void*)rgb;
        std::swap(rgba[0], rgba[2]);
    }
    for (int j = 0; j < 3; ++j) {
        rgba[j] = rgb[j];
    }
}


HRESULT d3drender::_copyToTexture(const VideoFrame &frame)
{
    HRESULT hRet = S_OK;

    auto fmt = frame.type();
    switch (fmt) {
    case VideoFrame::VideoFrameType::kVideoFrameTypeI420:
        {
            D3DLOCKED_RECT rect;
            hRet = _ytexture->LockRect(0, &rect, 0, 0);
            if (SUCCEEDED(hRet)) {
                uint8_t* pDest = (uint8_t*)rect.pBits;
                uint8_t* pSrc = (uint8_t*)frame.data();
                int stride = frame.stride();
                auto height = frame.height();
                for (auto i = 0; i < height; i++) {
                    memcpy(pDest, pSrc, stride);
                    pDest += rect.Pitch;
                    pSrc += stride;
                }
            }
            _ytexture->UnlockRect(0);

            hRet = _utexture->LockRect(0, &rect, 0, 0);
            if (SUCCEEDED(hRet)) {
                uint8_t* pDest = (uint8_t*)rect.pBits;
                uint8_t* pSrc = (uint8_t*)frame.data();
                int stride = frame.stride();
                auto height = frame.height() / 2;
                for (auto i = 0; i < height; i++) {
                    memcpy(pDest, pSrc, stride);
                    pDest += rect.Pitch;
                    pSrc += stride;
                }
            }
            _utexture->UnlockRect(0);

            hRet = _vtexture->LockRect(0, &rect, 0, 0);
            if (SUCCEEDED(hRet)) {
                uint8_t* pDest = (uint8_t*)rect.pBits;
                uint8_t* pSrc = (uint8_t*)frame.data();
                int stride = frame.stride();
                auto height = frame.height() / 2;
                for (auto i = 0; i < height; i++) {
                    memcpy(pDest, pSrc, stride);
                    pDest += rect.Pitch;
                    pSrc += stride;
                }
            }
            _vtexture->UnlockRect(0);
        }
        break;
        case VideoFrame::VideoFrameType::kVideoFrameTypeNV12:
        {
            D3DLOCKED_RECT rect;
            hRet = _ytexture->LockRect(0, &rect, 0, 0);
            if (SUCCEEDED(hRet)) {
                uint8_t* pDest = (uint8_t*)rect.pBits;
                uint8_t* pSrc = (uint8_t*)frame.data();
                int stride = frame.stride();
                auto height = frame.height();
                for (auto i = 0; i < height; i++) {
                    memcpy(pDest, pSrc, stride);
                    pDest += rect.Pitch;
                    pSrc += stride;
                }
            }
            _ytexture->UnlockRect(0);

            hRet = _utexture->LockRect(0, &rect, 0, 0);
            if (SUCCEEDED(hRet)) {
                uint8_t* pDest = (uint8_t*)rect.pBits;
                uint8_t* pSrc = (uint8_t*)frame.data();
                int stride = frame.stride();
                auto height = frame.height();
                for (auto i = 0; i < height; i++) {
                    memcpy(pDest, pSrc, stride);
                    pDest += rect.Pitch;
                    pSrc += stride;
                }
            }
            _utexture->UnlockRect(0);
        }
        break;
        case VideoFrame::VideoFrameType::kVideoFrameTypeRGB24:
        {
            D3DLOCKED_RECT rect;
            hRet = _ytexture->LockRect(0, &rect, 0, 0);
            if (SUCCEEDED(hRet)) {
                uint8_t* pDest = (uint8_t*)rect.pBits;
                int stride = frame.stride();

                uint8_t* pSrc = (uint8_t*)frame.data();
                auto height = frame.height();
                for (auto i = 0; i < height; i++) {
                    fast_unpack((char*)pDest, (char*)pSrc, frame.width());
                    pDest += rect.Pitch;
                    pSrc += stride;
                }
            }
            _ytexture->UnlockRect(0);
        }
        break;
    case VideoFrame::VideoFrameType::kVideoFrameTypeBGRA:
    case VideoFrame::VideoFrameType::kVideoFrameTypeRGBA:
        {
            D3DLOCKED_RECT rect;
            hRet = _ytexture->LockRect(0, &rect, 0, 0);
            if (SUCCEEDED(hRet)) {
                uint8_t* pDest = (uint8_t*)rect.pBits;
                int stride = frame.stride();

                uint8_t* pSrc = (uint8_t*)frame.data();
                auto height = frame.height();
                for (auto i = 0; i < height; i++) {
                    memcpy(pDest, pSrc, stride);
                    pDest += rect.Pitch;
                    pSrc += stride;
                }
            }
            _ytexture->UnlockRect(0);
        }
        break;
    }

    return hRet;
}

HRESULT  d3drender::display(const VideoFrame &frame)
{
    HRESULT hRet = S_OK;

    auto fmtType = frame.type();
    auto width = frame.width();
    auto height = frame.height();

    CRect wndRect;
    GetWindowRect(_hwnd, &wndRect);
    CSize frameSize(width, height);
    
    auto curNono = std::chrono::high_resolution_clock::now();

    {
        if (!(wndRect.Size() == _lastRect.Size())) {
            HR_CHECK(_resetDevice(wndRect));
        }

        if (_curPixType != fmtType
            || (frameSize != _curBuffSize) )
        {
            HR_CHECK(_setupVertex(fmtType));

            HR_CHECK(_compile_sharder(fmtType));

            HR_CHECK(_reallocTexture(frameSize, fmtType));
        }
    }

    HR_CHECK(_copyToTexture(frame));

    {
        // 适应窗口方式
        float rTop = 1.0f;
        float rLeft = 1.0f;
        switch (_methodRender) {
        case 1:
            {
                float nH = width *  wndRect.Height() * 1.0f / wndRect.Width();
                rTop = nH / height;
            }
            break;
        case 2:
            {
                float nWidth = height * wndRect.Width() * 1.0f / wndRect.Height();
                rLeft = nWidth / width;
            }
            break;
        }

        D3DXMATRIX P;
        D3DXMatrixPerspectiveOffCenterLH(&P, -rLeft, rLeft, -rTop, rTop, 1.0f, 1000.0f);
        _device->SetTransform(D3DTS_PROJECTION, &P);
    }

    {
        // copy display buffer
        HR_CHECK(_device->BeginScene());
        HR_CHECK(_device->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0));

        switch (fmtType) {
        case VideoFrame::VideoFrameType::kVideoFrameTypeI420:
        case VideoFrame::VideoFrameType::kVideoFrameTypeNV12:
            {
                //_device->SetViewport(&_viewport);

                HR_CHECK(_device->SetPixelShader(_Ps));
                HR_CHECK(_device->SetFVF(CustomVertex::FVF));
                HR_CHECK(_device->SetStreamSource(0, _vertexBuffer, 0, sizeof(CustomVertex)));

                HR_CHECK(_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2));
            }
            break;
        case VideoFrame::VideoFrameType::kVideoFrameTypeRGB24:
        //case VideoFrame::VideoFrameType::kVideoFrameType0BGR32:
        case VideoFrame::VideoFrameType::kVideoFrameTypeBGRA:
        case VideoFrame::VideoFrameType::kVideoFrameTypeRGBA:
            {
               // _device->SetViewport(&_viewport);

                HR_CHECK(_device->UpdateTexture(_ytexture, _renderTex));

                HR_CHECK(_device->SetTexture(0, _renderTex));

                //Binds a vertex buffer to a device data stream.
                HR_CHECK(_device->SetStreamSource(0, _vertexBuffer, 0, sizeof(CustomVertex)));

                //Sets the current vertex stream declaration.
                HR_CHECK(_device->SetFVF(CustomVertex::FVF));
                //Renders a sequence of non indexed, geometric primitives of the 
                //specified type from the current set of data input streams.
                HR_CHECK(_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2));
            }
            break;
        }

        {
            double frameInterval = 1.0f;
            double frameCost = 1.0f;
            if (_lastRenderTimeMs.time_since_epoch().count() != 0) {

                auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(curNono - _lastRenderTimeMs);
                _renderIntervalList.push_back({ std::chrono::high_resolution_clock::now(), interval.count() });

                {
                    auto sampleCount = std::chrono::duration_cast<std::chrono::milliseconds>(
                        _renderIntervalList.back().timeStamp - _renderIntervalList.front().timeStamp);

                    do {
                        if (sampleCount.count() <= 5000) {
                            break;
                        }

                        _renderIntervalList.pop_front();

                        sampleCount = std::chrono::duration_cast<std::chrono::milliseconds>(
                            _renderIntervalList.front().timeStamp - _renderIntervalList.back().timeStamp);
                    } while (1);

                    auto size = _renderIntervalList.size();
                    frameInterval = size * 1000.0f / sampleCount.count();

                    size = 0;
                    for (auto e : _renderIntervalList) {
                        size += e.value;
                    }
                    frameCost = size * 1.0f/ _renderIntervalList.size();
                }
            }
            
            CRect cltRect;
            GetClientRect(_hwnd, &cltRect);
            HR_CHECK(_drawInfo(NULL, cltRect, _T("F: %dx%d W:%dx%d Fmt:%d P:%8.4f, C:%f"), 
                frameSize.cx, frameSize.cy,
                               wndRect.Width(), wndRect.Height(), fmtType, frameInterval, frameCost));
        }

        HR_CHECK(_device->EndScene());
        hRet = _device->Present(NULL, NULL, NULL, NULL);
        if (FAILED(hRet))
        {
            hRet = _device->TestCooperativeLevel();
            if (hRet == D3DERR_DEVICELOST)
            {
            }
            else if (hRet == D3DERR_DEVICENOTRESET)
            {
                HR_CHECK(_resetDevice(wndRect));
            }
        }
        _lastRenderTimeMs = curNono;
    }

    return S_OK;
}

HRESULT d3drender::release()
{
    _device.Release();
    _ytexture.Release();
    _utexture.Release();
    _vtexture.Release();
    _renderTex.Release();
    _vertexBuffer.Release();
    _Ps.Release();
    _psCT.Release();
    _printer.Release();

    _lastRect.SetRectEmpty();
    _curPixType = VideoFrame::VideoFrameType::kVideoFrameTypeRGBA;
    _curBuffSize.SetSize(0,0);

    return S_OK;
}

HRESULT d3drender::_drawInfo(HDC hdc, RECT *rc, TCHAR *format, ...)
{
    HRESULT hRet = S_OK;

    TCHAR buf[1024] = { TEXT('\0') };
    va_list va_alist;

    va_start(va_alist, format);
    _vsntprintf_s(buf, 1024, format, va_alist);
    va_end(va_alist);

    ::OffsetRect(rc, 0, 2);

    hRet = _printer->DrawTextW(nullptr, buf, -1, rc, DT_LEFT | DT_TOP, 0xff000000);

    rc->top += FONT_HEIGHT;

    return hRet;
}