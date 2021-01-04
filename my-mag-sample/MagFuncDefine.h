#pragma once
#ifndef __MAGFUNCDEFINE_H__
#define __MAGFUNCDEFINE_H__

#include <magnification.h>

// Public Functions
typedef BOOL(WINAPI *fnMagInitialize)();
typedef BOOL(WINAPI *fnMagUninitialize)();

typedef BOOL(WINAPI *fnMagSetWindowSource)(HWND hwnd, RECT rect);
typedef BOOL(WINAPI *fnMagGetWindowSource)(HWND hwnd, RECT *pRect);
typedef BOOL(WINAPI *fnMagSetWindowTransform)(HWND hwnd, PMAGTRANSFORM pTransform);
typedef BOOL(WINAPI *fnMagGetWindowTransform)(HWND hwnd, PMAGTRANSFORM pTransform);
typedef BOOL(WINAPI *fnMagSetWindowFilterList)(HWND hwnd, DWORD dwFilterMode, int count, HWND *pHWND);
typedef int  (WINAPI *fnMagGetWindowFilterList)(HWND hwnd, DWORD *pdwFilterMode, int count, HWND *pHWND);
typedef BOOL(WINAPI *fnMagSetImageScalingCallback)(HWND hwnd, MagImageScalingCallback callback);
typedef MagImageScalingCallback(WINAPI *fnMagGetImageScalingCallback)(HWND hwnd);
typedef BOOL(WINAPI *fnMagSetColorEffect)(HWND hwnd, PMAGCOLOREFFECT pEffect);
typedef BOOL(WINAPI *fnMagGetColorEffect)(HWND hwnd, PMAGCOLOREFFECT pEffect);

struct MagInterface
{
    fnMagInitialize Initialize;
    fnMagUninitialize Uninitialize;
    fnMagSetWindowSource SetWindowSource;
    fnMagGetWindowSource GetWindowSource;
    fnMagSetWindowTransform SetWindowTransform;
    fnMagGetWindowTransform GetWindowTransform;
    fnMagSetWindowFilterList SetWindowFilterList;
    fnMagGetWindowFilterList GetWindowFilterList;
    fnMagSetImageScalingCallback SetImageScalingCallback;
    fnMagGetImageScalingCallback GetImageScalingCallback;
    fnMagSetColorEffect SetColorEffect;
    fnMagGetColorEffect GetColorEffect;
};

#endif //__MAGFUNCDEFINE_H__
