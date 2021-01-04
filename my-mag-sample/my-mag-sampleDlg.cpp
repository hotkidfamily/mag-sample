
// my-mag-sampleDlg.cpp : implementation file
//

#include "stdafx.h"

#include "Win32WindowEnumeration.h"

#include "my-mag-sample.h"
#include "my-mag-sampleDlg.h"
#include "afxdialogex.h"

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
#define TIMER_CAPTURE (1)

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


void CmymagsampleDlg::OnBnClickedBtnWndcap()
{
    auto &timer = _appContext->timer;
    auto &capture = _appContext->capture;
    if (timer.timerInst) {
        KillTimer(timer.timerID);
        timer.timerInst = 0;
    }
    timer.fps = 10;
    timer.timerID = TIMER_CAPTURE;
    timer.timerInst = SetTimer(timer.timerID, 1000/timer.fps, NULL);

    try {
        capture.winID = _wndList.at(_wndListCombobox.GetCurSel()).Hwnd();
    }catch(std::out_of_range &e) {
        capture.winID = 0;
    }

    capture.capture.reset(new MagCapture());
    capture.capture->startCaptureWindow(capture.winID);
    
}


void CmymagsampleDlg::OnTimer(UINT_PTR nIDEvent)
{
    auto &capture = _appContext->capture;

    DesktopRect rect = DesktopRect::MakeXYWH(0, 0, 1280, 720);

    if (capture.capture.get())
        capture.capture->CaptureImage(rect);

    //CDialogEx::OnTimer(nIDEvent);
}


void CmymagsampleDlg::OnBnClickedButtonFindwind()
{
    int curSel = 0;
    auto & capture = _appContext->capture;

    _wndListCombobox.ResetContent();
    _wndList.clear();
    _wndList = EnumerateWindows();

    for (auto & wnd : _wndList) {
        std::wstring title;
        title = wnd.Title() + L"|" + wnd.ClassName();

        int index = _wndListCombobox.AddString(title.c_str());
        if (wnd.Hwnd() == capture.winID) {
            curSel = index;
        }
    }

    _wndListCombobox.SetCurSel(curSel);
}


void CmymagsampleDlg::OnBnClickedBtnScreencap()
{
    auto &timer = _appContext->timer;
    auto &capture = _appContext->capture;
    if (timer.timerInst) {
        KillTimer(timer.timerID);
        timer.timerInst = 0;
    }
    timer.fps = 10;
    timer.timerID = TIMER_CAPTURE;
    timer.timerInst = SetTimer(timer.timerID, 1000 / timer.fps, NULL);

    capture.screenID = MonitorFromWindow(GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
}


void CmymagsampleDlg::OnBnClickedBtnStop()
{
    auto &capture = _appContext->capture;
    if(capture.capture.get())
        capture.capture.reset(nullptr);
}
