#pragma once

#include <dwmapi.h>
#include <string>
#include <array>
#include <vector>

static decltype(::DwmGetWindowAttribute) *_fnDwmGetWindowAttribute = nullptr;

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

bool IsWindowExcludeFromCapture(HWND hwnd)
{
    DWORD affinity = 0;
    auto ret = GetWindowDisplayAffinity(hwnd, &affinity);
    return (ret == TRUE) && (affinity == WDA_EXCLUDEFROMCAPTURE);
}

bool IsWindowCloaked(Window wnd) 
{
    bool ret = FALSE;

    DWORD cloaked = FALSE;
    if (_fnDwmGetWindowAttribute) {
        HRESULT hrTemp = _fnDwmGetWindowAttribute(wnd.Hwnd(), DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
        ret = SUCCEEDED(hrTemp) && cloaked;
    }

    return ret;
}

bool IsInvalidWindowSize(Window wnd)
{
    RECT rect;
    GetWindowRect(wnd.Hwnd(), &rect);
    return !!IsRectEmpty(&rect);
}

bool IsWindowCapable(Window wnd)
{
    HWND hwnd = wnd.Hwnd();
    if (hwnd == GetShellWindow()) {
        return false;
    }

    if (wnd.Title().length() == 0) {
        return false;
    }

    if (!IsWindowVisible(hwnd)) {
        return false;
    }

    if (IsWindowExcludeFromCapture(hwnd)) {
        return false;
    }

    if (GetAncestor(hwnd, GA_ROOT) != hwnd){
        return false;
    }

    return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto class_name = GetClassName(hwnd);
    auto title = GetWindowText(hwnd);
    auto window = Window(hwnd, title, class_name);

    if (!IsWindowCapable(window))
    {
        return TRUE;
    }

    if (IsInvalidWindowSize(window)) {
        return TRUE;
    }

    if (IsWindowCloaked(window)) {
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

    if (!_fnDwmGetWindowAttribute) {
        hmodule = LoadLibraryW(L"Dwmapi.dll");
        if (hmodule) {
            _fnDwmGetWindowAttribute = reinterpret_cast<decltype(_fnDwmGetWindowAttribute)>(
                GetProcAddress(hmodule, "DwmGetWindowAttribute"));
        }
    }

    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

    if (hmodule)
        FreeLibrary(hmodule);

    return windows;
}