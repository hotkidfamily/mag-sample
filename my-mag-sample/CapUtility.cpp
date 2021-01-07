#include "stdafx.h"
#include "CapUtility.h"

#include <shellscalingapi.h>
#include <set>

namespace CapUtility
{
static HMODULE hShcoreModule = LoadLibraryW(L"Shcore.dll");
typedef HRESULT(__stdcall *funcGetDpiForMonitor)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

BOOL _getDpiForMonitor(HMONITOR hm, UINT *dpix, UINT *dpiy)
{
    HRESULT hRet = E_FAIL;
    static funcGetDpiForMonitor _ptrGetDpiForMonitor = nullptr;
    if (!_ptrGetDpiForMonitor) {
        if (hShcoreModule) {
            _ptrGetDpiForMonitor
                = reinterpret_cast<funcGetDpiForMonitor>(GetProcAddress(hShcoreModule, "GetDpiForMonitor"));
        }
    }
    else {
        hRet = _ptrGetDpiForMonitor(hm, MDT_RAW_DPI, dpix, dpiy);
    }

    return SUCCEEDED(hRet);
}


typedef HRESULT(__stdcall *funcGetScaleFactorForMonitor)(HMONITOR hmonitor, DEVICE_SCALE_FACTOR *scale);

BOOL _getScaleForMonitor(HMONITOR hm, DEVICE_SCALE_FACTOR *scale)
{
    HRESULT hRet = E_FAIL;
    static funcGetScaleFactorForMonitor _ptrGetScaleFactorForMonitor = nullptr;
    if (!_ptrGetScaleFactorForMonitor) {
        if (hShcoreModule) {
            _ptrGetScaleFactorForMonitor = reinterpret_cast<funcGetScaleFactorForMonitor>(
                GetProcAddress(hShcoreModule, "GetScaleFactorForMonitor"));
        }
    }
    else {
        hRet = _ptrGetScaleFactorForMonitor(hm, scale);
    }

    return SUCCEEDED(hRet);
}

BOOL getDpiForMonitor(HMONITOR hMonitor, UINT *DPI)
{
    DPI_AWARENESS_CONTEXT priorCtx = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    SetThreadDpiAwarenessContext(priorCtx);

    DEVICE_SCALE_FACTOR scale;
    if (_getScaleForMonitor(hMonitor, &scale)) {
    }

    MONITORINFO minfo;
    minfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &minfo);

    UINT DPIX = 0, DPIY = 0;
    RECT &mRect = minfo.rcMonitor;
    RECT rRect = mRect;

    //DPI_AWARENESS_CONTEXT dpiAwarenessContext = GetWindowDpiAwarenessContext(hWnd);
    //DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpiAwarenessContext);
    //UINT dpi = GetDpiForWindow(hWnd);
    //BOOL bSuccess = CapUtility::getDpiForMonitor(hMonitor, &DPIX, &DPIY);
    //switch (dpiAwareness) {
    //case DPI_AWARENESS_UNAWARE:
    //    break;
    //case DPI_AWARENESS_SYSTEM_AWARE:
    //    break;
    //case DPI_AWARENESS_PER_MONITOR_AWARE:
    //    break;
    //case DPI_AWARENESS_INVALID:
    //default:
    //    break;
    //}
    return TRUE;
}

BOOL getDPIForWindow(HWND hWnd, UINT *DPI)
{
    DPI_AWARENESS_CONTEXT dpiAwarenessContext = GetWindowDpiAwarenessContext(hWnd);
    DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpiAwarenessContext);
    UINT dpi = GetDpiForWindow(hWnd);
    switch (dpiAwareness) {
    case DPI_AWARENESS_PER_MONITOR_AWARE:
        break;
    case DPI_AWARENESS_SYSTEM_AWARE:
        dpi = GetDpiForSystem();
        break;
    case DPI_AWARENESS_INVALID:
    case DPI_AWARENESS_UNAWARE:
    default:
        dpi = CapUtility::kDesktopCaptureDefaultDPI;
        break;
    }

    *DPI = dpi;
    return TRUE;
}

std::vector<HWND> getWindowsCovered(HWND hTartgetWnd)
{
    std::set<HWND> wndlist;
    RECT rcTarget;
    GetWindowRect(hTartgetWnd, &rcTarget);
    HWND enumWnd = hTartgetWnd;
    HWND shell = GetDesktopWindow();

    while (NULL != (enumWnd = GetNextWindow(enumWnd, GW_HWNDPREV))) {
        //�����˷Ǵ��ڣ�û����ʾ�ģ���Ч�ģ���windows���������ö��Ĵ��ڡ���������˷ǹ������ľ������ô��õ�һЩ��ֵĴ��ڣ�������ж��Ƿ���ס��
        int style = GetWindowLong(enumWnd, GWL_STYLE);
        int exStyle = GetWindowLong(enumWnd, GWL_EXSTYLE);

        if (IsWindow(enumWnd)                // �Ǵ���
            && IsWindowVisible(enumWnd)      // �ɼ���
            && !IsIconic(enumWnd)            // ����С����
            && !(exStyle & WS_EX_NOACTIVATE) // ��Ҫ��ȡ����Ĵ���(���뷨����)
            )
        {
            HWND rootWnd = enumWnd;

            if (style & WS_POPUP) {
                if (hTartgetWnd == (rootWnd = GetAncestor(enumWnd, GA_ROOTOWNER))) { // ��� owner �� Ŀ����һ���ģ�������
                    continue;
                }
                else {
                    rootWnd = enumWnd;
                }
            }

            RECT rcWnd;
            GetWindowRect(rootWnd, &rcWnd);
            if (!((rcWnd.right < rcTarget.left) || (rcWnd.left > rcTarget.right) || (rcWnd.bottom < rcTarget.top)
                  || (rcWnd.top > rcTarget.bottom))) {
                wndlist.insert(rootWnd);
            }
        }
    }

    std::vector<HWND> wndList2;
    if (wndlist.size()){
        for (auto &ele : wndlist) {
            wndList2.push_back(ele);
        }
    }

    return wndList2;
}
};
