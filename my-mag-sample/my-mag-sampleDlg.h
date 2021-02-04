
// my-mag-sampleDlg.h : header file
//

#pragma once

#include <memory>
#include "AppContext.h"
#include "afxwin.h"

// CmymagsampleDlg dialog
class CmymagsampleDlg : public CDialogEx
{
// Construction
public:
	CmymagsampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYMAGSAMPLE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnWndcap();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedButtonFindwind();
    afx_msg void OnBnClickedBtnScreencap();
	DECLARE_MESSAGE_MAP()

    afx_msg LRESULT OnDisplayChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDPIChanged(WPARAM wParam, LPARAM lParam);

  private:
    std::unique_ptr<AppContext> _appContext;
    CComboBox _wndListCombobox;
public:
    afx_msg void OnBnClickedBtnStop();

	void OnCaptureFrame(VideoFrame *frame);
    CStatic _previewWnd;
    CStatic _winRectInfoText;
    afx_msg void OnCbnSelchangeComboWndlist();
};
