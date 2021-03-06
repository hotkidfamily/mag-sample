#pragma once

#include <dwmapi.h>
#include <string>
#include <array>
#include <vector>

typedef HRESULT (__stdcall *funcDwmGetWindowAttribute)(
    HWND  hwnd,
    DWORD dwAttribute,
    PVOID pvAttribute,
    DWORD cbAttribute
);


static funcDwmGetWindowAttribute _ptrDwmGetWindowAttribute = nullptr;

struct Window
{
public:
    Window(nullptr_t) {}
    Window(HWND hwnd, std::wstring const& title, std::wstring& className)
    {
        m_hwnd = hwnd;
        m_title = title;
        m_className = className;
    }

    HWND Hwnd() const noexcept { return m_hwnd; }
    std::wstring Title() const noexcept { return m_title; }
    std::wstring ClassName() const noexcept { return m_className; }

private:
    HWND m_hwnd;
    std::wstring m_title;
    std::wstring m_className;
};

std::wstring GetClassName(HWND hwnd)
{
	std::array<WCHAR, 1024> className;

    ::GetClassName(hwnd, className.data(), (int)className.size());

    std::wstring title(className.data());
    return title;
}

std::wstring GetWindowText(HWND hwnd)
{
	std::array<WCHAR, 1024> windowText;

    ::GetWindowText(hwnd, windowText.data(), (int)windowText.size());

    std::wstring title(windowText.data());
    return title;
}

bool IsAltTabWindow(Window const& window)
{
    HWND hwnd = window.Hwnd();
    HWND shellWindow = GetShellWindow();

    auto title = window.Title();
    auto className = window.ClassName();

    if (hwnd == shellWindow)
    {
        return false;
    }

    if (title.length() == 0)
    {
        return false;
    }

    if (!IsWindowVisible(hwnd))
    {
        return false;
    }

    if (GetAncestor(hwnd, GA_ROOT) != hwnd)
    {
        return false;
    }

    if (IsIconic(hwnd)) {
        return false;
    }

    DWORD cloaked = FALSE;
    if (_ptrDwmGetWindowAttribute) {
        HRESULT hrTemp = _ptrDwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
        if (SUCCEEDED(hrTemp) &&
            cloaked == DWM_CLOAKED_SHELL)
        {
            return false;
        }
    }
    
    return true;
}

BOOL IsWindowApp(Window wnd) 
{
    return FALSE;
}

BOOL IsUWPFrameWorkApp(Window wnd) 
{
    BOOL ret = FALSE;
    ret = wnd.ClassName() == L"ApplicationFrameWindow";

    if (ret) {
        DWORD cloaked = FALSE;
        ret = FALSE;
        if (_ptrDwmGetWindowAttribute) {
            HRESULT hrTemp = _ptrDwmGetWindowAttribute(wnd.Hwnd(), DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
            if (SUCCEEDED(hrTemp) && cloaked) {
                ret = TRUE;
            }
        }
    }

    return ret;
}

BOOL IsUWPApp(Window wnd)
{ 
    BOOL ret = wnd.ClassName() == L"Windows.UI.Core.CoreWindow";

    return ret;
}

BOOL IsInvalidWindow(Window wnd)
{
    RECT rect;
    GetWindowRect(wnd.Hwnd(), &rect);
    return IsRectEmpty(&rect);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto class_name = GetClassName(hwnd);
    auto title = GetWindowText(hwnd);

    auto window = Window(hwnd, title, class_name);

    if (!IsAltTabWindow(window))
    {
        return TRUE;
    }

    if (IsWindowApp(window)) {
        return TRUE;
    }

    if (IsUWPFrameWorkApp(window)) {
        return TRUE;
    }

    //if (IsUWPApp(window)) {
    //    return TRUE;
    //}

    if (IsInvalidWindow(window)) {
        return TRUE;
    }
    
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (style & WS_DISABLED) {
        return TRUE;
    }

    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) {
        return TRUE;
    }

    std::vector<Window>& windows = *reinterpret_cast<std::vector<Window>*>(lParam);
    windows.push_back(window);

    return TRUE;
}

const std::vector<Window> EnumerateWindows()
{
    std::vector<Window> windows;
    HMODULE hmodule = nullptr;

    if (!_ptrDwmGetWindowAttribute) {
        hmodule = LoadLibraryW(L"Dwmapi.dll");
        if (hmodule) {
            _ptrDwmGetWindowAttribute
                = reinterpret_cast<funcDwmGetWindowAttribute>(GetProcAddress(hmodule, "DwmGetWindowAttribute"));
        }
    }

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

    if (hmodule)
        FreeLibrary(hmodule);

    return windows;
}