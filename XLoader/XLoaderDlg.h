
// XLoaderDlg.h: 头文件
//

#pragma once


// CXLoaderDlg 对话框
class CXLoaderDlg : public CDialogEx
{
// 构造
public:
	CXLoaderDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_XLOADER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLoadsys();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnUnloadsys();
public:
	CStatusBarCtrl m_StatusBar;
};

BOOL GetDriverPath(TCHAR* lpszDriverPath, TCHAR* svrName, TCHAR* lpszSrcPath);