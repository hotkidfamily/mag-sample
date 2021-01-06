#include "stdafx.h"
#include "CapUtility.h"

#include <set>

namespace CapUtility
{


std::vector<HWND> getWindowsCovered(HWND hTartgetWnd)
{
    std::set<HWND> wndlist;
    RECT rcTarget;
    GetWindowRect(hTartgetWnd, &rcTarget);
    HWND enumWnd = hTartgetWnd;
    HWND shell = GetDesktopWindow();

    while (NULL != (enumWnd = GetNextWindow(enumWnd, GW_HWNDPREV))) {
        //过滤了非窗口，没有显示的，无效的，非windows工具栏，置顶的窗口。如果不过滤非工具栏的句柄，那么会得到一些奇怪的窗口，会很难判断是否被遮住。
        int style = GetWindowLong(enumWnd, GWL_STYLE);
        int exStyle = GetWindowLong(enumWnd, GWL_EXSTYLE);

        if (IsWindow(enumWnd)                // 是窗口
            && IsWindowVisible(enumWnd)      // 可见的
            && !IsIconic(enumWnd)            // 非最小化的
            && !(exStyle & WS_EX_NOACTIVATE) // 需要获取焦点的窗口(输入法窗口)
            )
        {
            HWND rootWnd = enumWnd;

            if (style & WS_POPUP) {
                if ((rootWnd = GetAncestor(enumWnd, GA_PARENT)) == shell) {             // 父窗口是桌面
                    if (hTartgetWnd == (rootWnd = GetAncestor(enumWnd, GA_ROOTOWNER))) { // 如果有 owner
                        continue;
                    }
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
