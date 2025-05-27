
// my-mag-sampleDlg.cpp : implementation file
//

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "WinVersionHelper.h"
#include "Win32WindowEnumeration.h"
#include "CapUtility.h"
#include "MagCapture.h"
#include "GDICapture.h"
#include "DXGICapture.h"
#include "WGCCapture.h"

#include "my-mag-sample.h"
#include "my-mag-sampleDlg.h"
#include "afxdialogex.h"

#include <stdexcept>
#include <set>
#include <chrono>
#include <sstream>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()

};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmymagsampleDlg dialog

std::vector<Window> _wndList;
#define TIMER_WINDOW_CAPTURE (1)
#define TIMER_SCREEN_CAPTURE (2)

CmymagsampleDlg::CmymagsampleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MYMAGSAMPLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    _appContext = std::make_unique<AppContext>();
    _appContext->timer.fps = KDefaultFPS;
}

void CmymagsampleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_WNDLIST, _wndListCombobox);
    DDX_Control(pDX, IDC_STATIC_PVWWND, _previewWnd);
    DDX_Control(pDX, IDC_STATIC_WININFO, _winRectInfoText);
    DDX_Control(pDX, IDC_COMBO_SYSTEM, _sysComboBox);
}

BEGIN_MESSAGE_MAP(CmymagsampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_WNDCAP, &CmymagsampleDlg::OnBnClickedBtnWndcap)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_FINDWIND, &CmymagsampleDlg::OnBnClickedButtonFindwind)
    ON_BN_CLICKED(IDC_BTN_SCREENCAP, &CmymagsampleDlg::OnBnClickedBtnScreencap)
    ON_BN_CLICKED(IDC_BTN_STOP, &CmymagsampleDlg::OnBnClickedBtnStop)
    ON_MESSAGE(WM_DISPLAYCHANGE, &CmymagsampleDlg::OnDisplayChanged)
    ON_MESSAGE(WM_DPICHANGED, &CmymagsampleDlg::OnDPIChanged)
    //ON_MESSAGE(WM_SESSIONCHANGE, &CmymagsampleDlg::OnDisplayChanged)
    ON_CBN_SELCHANGE(IDC_COMBO_WNDLIST, &CmymagsampleDlg::OnCbnSelchangeComboWndlist)
    ON_WM_SIZE()
    ON_MESSAGE(KThreadCaptureMessage, OnUserDefinedMessage)
    ON_WM_CLOSE()
    END_MESSAGE_MAP()


// CmymagsampleDlg message handlers

BOOL CmymagsampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    SetWindowDisplayAffinity(GetSafeHwnd(), WDA_EXCLUDEFROMCAPTURE);

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    DWORD pid = GetCurrentProcessId();

    std::wstring title = L"my-mag-sample - " + std::to_wstring(pid);
    SetWindowTextW(title.c_str());

    std::vector<std::wstring> systemList = { L"Win11",      // 0
                                             L"Win10-1903", // 1
                                             L"Win10",      // 2
                                             L"Win8.1",     // 3
                                             L"Win8",       // 4
                                             L"Win7SP1",    // 5
                                             L"Win7",       // 6
                                             L"WInVista",   // 7
                                             L"WInXP" };    // 8

    for (auto &it : systemList) {
        _sysComboBox.AddString(it.c_str());
    }
    _sysComboBox.SetCurSel(0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CmymagsampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CmymagsampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CmymagsampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CmymagsampleDlg::OnCaptureFrame(VideoFrame *frame)
{
    auto &render = _appContext->render;
    render.render->display(*frame);
}

void CaptureCallback(VideoFrame *frame, void *args)
{
    CmymagsampleDlg *pDlg = reinterpret_cast<CmymagsampleDlg *>(args);
    pDlg->OnCaptureFrame(frame);
}

LRESULT CmymagsampleDlg::OnDPIChanged(WPARAM wParam, LPARAM lParam)
{
    auto &capturer = _appContext->capturer;

    long dpi = LOWORD(wParam);
    RECT newRect = *(reinterpret_cast<const RECT *>(lParam));

    return 0;
}

LRESULT CmymagsampleDlg::OnDisplayChanged(WPARAM wParam, LPARAM lParam)
{
    lParam = lParam;
    auto &capturer = _appContext->capturer;

    capturer.rect.set_width(LOWORD(lParam));
    capturer.rect.set_width(HIWORD(lParam));

    if (capturer.host.get()) {
        if (capturer.winID) {
            OnBnClickedBtnWndcap();
        }
        else if (capturer.screenID) {
            OnBnClickedBtnScreencap();
        }
    }

    return 0;
}

LRESULT CmymagsampleDlg::OnUserDefinedMessage(WPARAM wParam, LPARAM lParam)
{
    auto &timer = _appContext->timer;
    auto &capturer = _appContext->capturer;

    if (timer.timerID == TIMER_SCREEN_CAPTURE) {
        DesktopRect rect = capturer.rect;
        std::vector<HWND> es = { GetSafeHwnd() };
        capturer.host->setExcludeWindows(es);
        if (capturer.host.get())
            capturer.host->captureImage(rect);
    }

    return 1;
}

void CmymagsampleDlg::OnTimer(UINT_PTR nIDEvent)
{
    auto &capturer = _appContext->capturer;

    if (TIMER_WINDOW_CAPTURE == nIDEvent) {
        RECT wRect;
        HWND &hWnd = capturer.winID;
        CapUtility::GetWindowRect(hWnd, wRect);

        HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        CapUtility::DisplaySetting settings = CapUtility::enumDisplaySettingByMonitor(hMonitor);
        DesktopRect monitorRect = DesktopRect::MakeRECT(settings.rect());

        capturer.rect = DesktopRect::MakeRECT(wRect);
        if (capturer.rect.width() > monitorRect.width()) { // out of screen
            capturer.rect.IntersectWith(monitorRect);
        }

        std::vector<HWND> wndList = CapUtility::getWindowsCovered(hWnd);

        if (CapUtility::isWndCanCap(hWnd)) {
            DesktopRect &rect = capturer.rect;
            if (capturer.host.get()) {
                capturer.host->setExcludeWindows(wndList);
                capturer.host->captureImage(rect);
            }
        }
    }
    else if (TIMER_SCREEN_CAPTURE == nIDEvent) {
        DesktopRect rect = capturer.rect;
        std::vector<HWND> es = { GetSafeHwnd() };
        capturer.host->setExcludeWindows(es);
        if (capturer.host.get())
            capturer.host->captureImage(rect);
    }
}


void CmymagsampleDlg::OnBnClickedButtonFindwind()
{
    int curSel = 0;
    auto &capturer = _appContext->capturer;

    _wndListCombobox.ResetContent();
    _wndList.clear();
    _wndList = EnumerateWindows();

    for (auto &wnd : _wndList) {
        std::wstringstream ss;
        ss << wnd.Hwnd() << "|" << wnd.Title() << wnd.ClassName();

        int index = _wndListCombobox.AddString(ss.str().c_str());
        if (wnd.Hwnd() == capturer.winID) {
            curSel = index;
        }
    }

    _wndListCombobox.SetCurSel(curSel);
}

void captureThread(CmymagsampleDlg *args)
{
    CmymagsampleDlg *pthis = (CmymagsampleDlg *)args;
    pthis->CaptureThread();
}

void CmymagsampleDlg::CaptureThread()
{
    auto &timer = _appContext->timer;
    while (timer.bRunning) {
        std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
        OnTimer(timer.timerID);
        std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
        auto interval = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (interval.count() >= 1000 / timer.fps)
            continue;
        Sleep((DWORD)(1000LL / timer.fps - interval.count()));
    }
}

void CmymagsampleDlg::OnBnClickedBtnWndcap()
{
    auto &timer = _appContext->timer;
    auto &capturer = _appContext->capturer;
    auto &render = _appContext->render;
    auto idx = _sysComboBox.GetCurSel();

    OnBnClickedBtnStop();

    try {
        capturer.winID = _wndList.at(_wndListCombobox.GetCurSel()).Hwnd();
    }
    catch (std::out_of_range &e) {
        return;
    }
    capturer.screenID = nullptr;

    if (Platform::IsWin10_1903OrGreater() && idx < 2) {
        capturer.host = std::make_unique<WGCCapture>();
    }
    else if (Platform::IsWindowsVistaOrGreater() && idx < 8) {
        //_PreviousHwnd = ::GetForegroundWindow();
        //::SetForegroundWindow(capturer.winID);
        //::BringWindowToTop(capturer.winID);
        capturer.host = std::make_unique<MagCapture>();
    }
    else {
        capturer.host = std::make_unique<GDICapture>();
    }
    capturer.host->setCallback(CaptureCallback, this);
    if (!capturer.host->startCaptureWindow(capturer.winID)) {
        capturer.host = std::make_unique<GDICapture>();
        capturer.host->setCallback(CaptureCallback, this);
        if (!capturer.host->startCaptureWindow(capturer.winID)) {
            FlashWindowEx(FLASHW_ALL, 3, 300);
            return;
        }
    }
    std::vector<HWND> es = { GetSafeHwnd() };
    capturer.host->setExcludeWindows(es);
    
    if (render.render) {
        render.render.reset(nullptr);
    }

    render.render = std::make_unique<d3d11render>();

    render.render->init(_previewWnd.GetSafeHwnd());
    render.render->setMode(2);

    timer.fps = KDefaultFPS;
    timer.timerID = TIMER_WINDOW_CAPTURE;

    if (capturer.host->usingTimer()) {
        timer.timerInst = SetTimer(timer.timerID, 1000 / timer.fps, NULL);
    }
    else
    {
        timer.bRunning = true;
        timer.capThread = std::thread(captureThread, this);
    }
}

void CmymagsampleDlg::OnBnClickedBtnScreencap()
{
    auto &timer = _appContext->timer;
    auto &capturer = _appContext->capturer;
    auto &render = _appContext->render;

    auto idx = _sysComboBox.GetCurSel();

    OnBnClickedBtnStop();

    capturer.screenID = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
    CapUtility::DisplaySetting settings = CapUtility::enumDisplaySettingByMonitor(capturer.screenID);
    capturer.rect = DesktopRect::MakeRECT(settings.rect());

    capturer.winID = 0;
    if (Platform::IsWin10_1903OrGreater() && idx<2) {
        capturer.host = std::make_unique<WGCCapture>();
    }
    else if (Platform::IsWindows8OrGreater() && idx < 5) {
        capturer.host = std::make_unique<DXGICapture>();
    }
    else if (Platform::IsWindowsVistaOrGreater() && idx < 8) {
        capturer.host = std::make_unique<MagCapture>();
    }
    else {
        capturer.host = std::make_unique<GDICapture>();
    }
    capturer.host->setCallback(CaptureCallback, this);
    if (!capturer.host->startCaptureScreen(capturer.screenID)) {
        capturer.host = std::make_unique<GDICapture>();
        capturer.host->setCallback(CaptureCallback, this);
        if (!capturer.host->startCaptureScreen(capturer.screenID)) {
            FlashWindowEx(FLASHW_ALL, 3, 300);
            return;
        }
    }

    if (render.render) {
        render.render.reset(nullptr);
    }
    render.render = std::make_unique<d3d11render>();
    render.render->init(_previewWnd.GetSafeHwnd());
    render.render->setMode(2);

    timer.timerID = TIMER_SCREEN_CAPTURE;

    if (capturer.host->usingTimer()) {
        timer.timerInst = SetTimer(timer.timerID, 1000 / timer.fps, NULL);
    }
    else {
        timer.bRunning = true;
        timer.capThread = std::thread(captureThread, this);
    }
}


void CmymagsampleDlg::OnBnClickedBtnStop()
{
    auto &timer = _appContext->timer;
    auto &capture = _appContext->capturer;
    auto &render = _appContext->render;

    timer.bRunning = false;

    if (timer.timerInst) {
        KillTimer(timer.timerID);
        timer.timerInst = 0;
    }

    if (timer.capThread.joinable()) {
        timer.capThread.join();
    }

    if (capture.host.get()) {
        capture.host->stop();
        capture.host.reset(nullptr);
    }
        

    if (render.render) {
        render.render.reset(nullptr);
    }

    _previewWnd.InvalidateRect(NULL);
    if (!Platform::IsWin10_1903OrGreater()) {
        //::SetForegroundWindow(_PreviousHwnd);
    }
}


void CmymagsampleDlg::OnCbnSelchangeComboWndlist()
{
    HWND hWnd = nullptr;

    try {
        hWnd = _wndList.at(_wndListCombobox.GetCurSel()).Hwnd();
    }
    catch (std::out_of_range &e) {
        return;
    }

    if (hWnd) 
    {
        CRect tOrigRect;
        WINDOWINFO info;
        info.cbSize = sizeof(WINDOWINFO);
        ::GetWindowRect(hWnd, &tOrigRect);
        ::GetWindowInfo(hWnd, &info);

        BOOL bEnable;
        if (S_OK == DwmIsCompositionEnabled(&bEnable)) {
        }

        CRect tDwmRect;
        if (bEnable && (S_OK != DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &tDwmRect, sizeof(RECT)))) {
        }

        CRect tWithoutBorder;
        CapUtility::GetWindowRect(hWnd, tWithoutBorder);

        WCHAR strInfo[MAX_PATH];
        swprintf_s(strInfo, MAX_PATH,
                   L"Info: GetWindowRect = %d,%d,%d,%d(%dx%d), "
                   L"WinInfo: (%dx%d)%d,%d,%d,%d\n"
                   L"DwmGetWindowAttribute = %d,%d,%d,%d(%dx%d), "
                   L"my = %d,%d,%d,%d(%dx%d)",
                   tOrigRect.left, tOrigRect.top, tOrigRect.right, tOrigRect.bottom, tOrigRect.Width(),
                   tOrigRect.Height(), info.cxWindowBorders, info.cyWindowBorders, info.rcWindow.left,
                   info.rcWindow.top, info.rcWindow.right, info.rcWindow.bottom, tDwmRect.left, tDwmRect.top,
                   tDwmRect.right, tDwmRect.bottom, tDwmRect.Width(), tDwmRect.Height(), tWithoutBorder.left,
                   tWithoutBorder.top, tWithoutBorder.right, tWithoutBorder.bottom, tWithoutBorder.Width(),
                   tWithoutBorder.Height()   
        );

        _winRectInfoText.SetWindowTextW(strInfo);
    }
}

void CmymagsampleDlg::OnSize(UINT nType, int cx, int cy)
{
    if (_appContext->render.render) {
        _appContext->render.render->onResize(cx, cy);
    }
    CDialogEx::OnSize(nType, cx, cy);
}

void CmymagsampleDlg::OnClose()
{
    OnBnClickedBtnStop();
    CDialogEx::OnClose();
}
