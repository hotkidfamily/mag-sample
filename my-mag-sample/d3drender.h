#pragma once

#include <d3dx9.h>
#include <DxErr.h>

#include <list>
#include <chrono>

#include "VideoFrame.h"

typedef struct tagSample
{
    std::chrono::steady_clock::time_point timeStamp;
    int64_t value;
}Sample;


class d3drender
{
public:
    d3drender();
    ~d3drender();

    HRESULT init(HWND hwnd);

    HRESULT display(const VideoFrame &frame);

    HRESULT release();
    
    HRESULT setMode(int mode) { _methodRender = mode; return S_OK; }

protected:
    HRESULT _compile_sharder(VideoFrame::VideoFrameType type);
    HRESULT _setupVertex(VideoFrame::VideoFrameType type);
    HRESULT _reallocTexture(CSize destBufSize, VideoFrame::VideoFrameType destBufFormat);
    HRESULT _copyToTexture(const VideoFrame &frame);
    HRESULT _resetDevice(CRect &wndRect);

    HRESULT _drawInfo(HDC hdc, RECT *rc, TCHAR *format, ...);

private:
    HWND _hwnd = nullptr;
    CRect _lastRect;
    VideoFrame::VideoFrameType _curPixType = VideoFrame::VideoFrameType::kVideoFrameTypeRGBA;
    CSize _curBuffSize;
    int _methodRender = 1;
    
    CComPtr<IDirect3DDevice9> _device = nullptr;
    CComPtr<IDirect3D9> _d3d9Api = nullptr;
   
    CComPtr<IDirect3DTexture9> _ytexture = nullptr;
    CComPtr<IDirect3DTexture9> _utexture = nullptr;
    CComPtr<IDirect3DTexture9> _vtexture = nullptr;
    CComPtr<IDirect3DTexture9> _renderTex = nullptr;

    CComPtr<IDirect3DVertexBuffer9> _vertexBuffer = nullptr;

    D3DXHANDLE _YTexHandle = 0;
    D3DXHANDLE _UTexHandle = 0;
    D3DXHANDLE _VTexHandle = 0;
    D3DXCONSTANT_DESC _YTexDesc;
    D3DXCONSTANT_DESC _UTexDesc;
    D3DXCONSTANT_DESC _VTexDesc;

    CComPtr<IDirect3DPixelShader9> _Ps = nullptr;
    CComPtr<ID3DXConstantTable> _psCT = nullptr;
    
    CComPtr<ID3DXFont> _printer = nullptr;

    std::chrono::steady_clock::time_point _lastRenderTimeMs = {};

     std::list<Sample> _renderIntervalList;
};
