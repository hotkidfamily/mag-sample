#pragma once

#include <stdint.h>

constexpr int32_t kVideoFrameMaxPlane = 1;


class VideoFrame {
  public:
    enum class VideoFrameType: uint32_t
    {
        kVideoFrameTypeRGBA = 1 << 0,
        kVideoFrameTypeBGRA,
        kVideoFrameTypeRGB24,

        kVideoFrameTypeI420 = 1 << 8,
        kVideoFrameTypeNV12,

        kVideoFrameTypeTexture = 1 << 16, // texture
    }; 

    enum VideoFrameFlag
    {
        kVideoFrameFlagRGB = 1 << 0,
        kVideoFrameFlagYUV = 1 << 1,
        kVideoFrameFlagTexture = 1 << 2, // texture
    };

    static VideoFrame *MakeFrame(int32_t w, int32_t h, int32_t s, VideoFrameType type, void *texture = nullptr)
    {
        VideoFrame *frame = new VideoFrame(w, h, s, type, texture);
        return frame;
    }

    ~VideoFrame()
    {
        if (_data) {
            _aligned_free(_data);
        }
        _data = nullptr;
        _width = 0;
        _height = 0;
        _stride[0] = 0;
        _pixelType = VideoFrameType::kVideoFrameTypeRGBA;
    }

    VideoFrame()
        : _width(0)
        , _height(0)
        , _pixelType(VideoFrameType::kVideoFrameTypeRGBA)
        , _data(nullptr)
        , _flag(VideoFrameFlag::kVideoFrameFlagRGB)
    {
        memset(_stride, 0, sizeof(_stride));
    }

    int32_t width() const 
    {
        return _width;
    }

    int32_t height() const
    {
        return _height;
    }

    int32_t stride() const
    {
        return _stride[0];    
    }

    int32_t bpp() const
    {
        return 4;
    }
    
    uint8_t* data() const
    {
        return _data;
    }

    VideoFrameType type() const
    {
        return _pixelType;
    }

    VideoFrameFlag flag() const
    {
        return _flag;
    }

  private:
    VideoFrame(int32_t w, int32_t h, int32_t s, VideoFrameType t, void *texture)
    {
        _width = w;
        _height = h;
        _stride[0] = s;
        _pixelType = t;

        if (t < VideoFrameType::kVideoFrameTypeI420) {
            _flag = VideoFrameFlag::kVideoFrameFlagRGB;
            _data = (uint8_t *)_aligned_malloc(h * s, 32);
        } 
        else if (t < VideoFrameType::kVideoFrameTypeTexture) {
            _flag = VideoFrameFlag::kVideoFrameFlagYUV;
            _data = (uint8_t *)_aligned_malloc(h * s, 32);
        }
        else {
            _flag = VideoFrameFlag::kVideoFrameFlagTexture;
            _data = 0;
            _handle = texture;
        }
    }

    int32_t _width = 0;
    int32_t _height = 0;
    int32_t _stride[kVideoFrameMaxPlane];
    VideoFrameType _pixelType = VideoFrameType::kVideoFrameTypeRGBA;
    VideoFrameFlag _flag = VideoFrameFlag::kVideoFrameFlagRGB;
    uint8_t *_data = nullptr;
    void *_handle = nullptr;
};
