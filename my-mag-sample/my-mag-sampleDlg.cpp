
// my-mag-sampleDlg.cpp : implementation file
//

#include "stdafx.h"

#include "Win32WindowEnumeration.h"
#include "CapUtility.h"

#include "my-mag-sample.h"
#include "my-mag-sampleDlg.h"
#include "afxdialogex.h"

#include <stdexcept>
#include <set>

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
    _appContext.reset(new AppContext());
}

void CmymagsampleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_WNDLIST, _wndListCombobox);
    DDX_Control(pDX, IDC_STATIC_PVWWND, _previewWnd);
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
    END_MESSAGE_MAP()


// CmymagsampleDlg message handlers

BOOL CmymagsampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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
    //this->OnVisualEnvironmentChange();

    return 0;
}

LRESULT CmymagsampleDlg::OnDisplayChanged(WPARAM wParam, LPARAM lParam)
{
    lParam = lParam;
    auto &capturer = _appContext->capturer;

    capturer.rect.set_width(LOWORD(lParam));
    capturer.rect.set_width(HIWORD(lParam));

    if (capturer.capturer.get()) {
        if (capturer.winID) {
            OnBnClickedBtnWndcap();
        }
        else if (capturer.screenID) {
            OnBnClickedBtnScreencap();
        }
    }

    return 0;
}

void CmymagsampleDlg::OnTimer(UINT_PTR nIDEvent)
{
    auto &capturer = _appContext->capturer;

    if (TIMER_WINDOW_CAPTURE == nIDEvent) {
        RECT wRect;
        HWND &hWnd = capturer.winID;
        if (S_OK != DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &wRect, sizeof(RECT))) {
            ::GetWindowRect(hWnd, &wRect);
        }

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
            if (capturer.capturer.get()) {
                capturer.capturer->setExcludeWindows(wndList);
                capturer.capturer->captureImage(rect);
            }
        }
    }
    else if (TIMER_SCREEN_CAPTURE == nIDEvent) {
        DesktopRect rect = capturer.rect;

        if (capturer.capturer.get())
            capturer.capturer->captureImage(rect);
    }
}


void CmymagsampleDlg::OnBnClickedButtonFindwind()
{
    int curSel = 0;
    auto &capturer = _appContext->capturer;

    _wndListCombobox.ResetContent();
    _wndList.clear();
    _wndList = EnumerateWindows();

    for (auto & wnd : _wndList) {
        std::wstring title;
        title = std::to_wstring(reinterpret_cast<int>(wnd.Hwnd())) + L"|" + wnd.Title() + L"|" + wnd.ClassName();

        int index = _wndListCombobox.AddString(title.c_str());
        if (wnd.Hwnd() == capturer.winID) {
            curSel = index;
        }
    }

    _wndListCombobox.SetCurSel(curSel);
}


void CmymagsampleDlg::OnBnClickedBtnWndcap()
{
    auto &timer = _appContext->timer;
    auto &capturer = _appContext->capturer;
    auto &render = _appContext->render;

    OnBnClickedBtnStop();

    try {
        capturer.winID = _wndList.at(_wndListCombobox.GetCurSel()).Hwnd();
    }
    catch (std::out_of_range &e) {
        return;
    }
    capturer.screenID = nullptr;
    capturer.capturer.reset(new MagCapture());
    capturer.capturer->setCallback(CaptureCallback, this);
    capturer.capturer->startCaptureWindow(capturer.winID);
    capturer.capturer->setExcludeWindows(GetSafeHwnd());

    if (render.render) {
        render.render.reset(nullptr);
    }
    render.render.reset(new d3drender);
    render.render->init(_previewWnd.GetSafeHwnd());
    render.render->setMode(2);

    timer.fps = 10;
    timer.timerID = TIMER_WINDOW_CAPTURE;
    timer.timerInst = SetTimer(timer.timerID, 1000 / timer.fps, NULL);
}

void CmymagsampleDlg::OnBnClickedBtnScreencap()
{
    auto &timer = _appContext->timer;
    auto &capture = _appContext->capturer;
    auto &render = _appContext->render;

    OnBnClickedBtnStop();

    capture.screenID = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
    CapUtility::DisplaySetting settings = CapUtility::enumDisplaySettingByMonitor(capture.screenID);
    capture.rect = DesktopRect::MakeRECT(settings.rect());

    capture.winID = 0;
    capture.capturer.reset(new MagCapture());
    capture.capturer->setCallback(CaptureCallback, this);
    capture.capturer->startCaptureScreen(capture.screenID);

    if (render.render) {
        render.render.reset(nullptr);
    }
    render.render.reset(new d3drender);
    render.render->init(_previewWnd.GetSafeHwnd());
    render.render->setMode(2);

    timer.fps = 10;
    timer.timerID = TIMER_SCREEN_CAPTURE;
    timer.timerInst = SetTimer(timer.timerID, 1000 / timer.fps, NULL);
}


void CmymagsampleDlg::OnBnClickedBtnStop()
{
    auto &timer = _appContext->timer;
    auto &capture = _appContext->capturer;
    auto &render = _appContext->render;

    if (timer.timerInst) {
        KillTimer(timer.timerID);
        timer.timerInst = 0;
    }

    if (capture.capturer.get())
        capture.capturer.reset(nullptr);

    if (render.render) {
        render.render.reset(nullptr);
    }

    _previewWnd.InvalidateRect(NULL);
}
