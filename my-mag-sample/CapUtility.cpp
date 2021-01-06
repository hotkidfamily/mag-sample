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
                if ((rootWnd = GetAncestor(enumWnd, GA_PARENT)) == shell) {             // ������������
                    if (hTartgetWnd == (rootWnd = GetAncestor(enumWnd, GA_ROOTOWNER))) { // ����� owner
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
