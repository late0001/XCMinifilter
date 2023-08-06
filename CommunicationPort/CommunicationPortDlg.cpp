
// CommunicationPortDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "CommunicationPort.h"
#include "CommunicationPortDlg.h"
#include "afxdialogex.h"
#include <FltUser.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "fltLib.lib")

extern HANDLE g_hPort;

#define NPMINI_NAME            L"MYminifilter"
#define NPMINI_PORT_NAME       L"\\MYMiniPort"
HANDLE g_hPort = INVALID_HANDLE_VALUE;
#ifdef _MANAGED
#pragma managed(push, off)
#endif
#ifdef _MANAGED
#pragma managed(pop)
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CCommunicationPortDlg 对话框



CCommunicationPortDlg::CCommunicationPortDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMMUNICATIONPORT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommunicationPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCommunicationPortDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SENDMSG, &CCommunicationPortDlg::OnBnClickedSendmsg)
END_MESSAGE_MAP()



int InitialCommunicationPort(void)
{
	DWORD hResult = FilterConnectCommunicationPort(
		NPMINI_PORT_NAME,
		0,
		NULL,
		0,
		NULL,
		&g_hPort);

	if (hResult != S_OK) {
		return hResult;
	}
	return 0;
}

DWORD NPSendMessage(PVOID InputBuffer)
{
	DWORD bytesReturned = 0;
	DWORD hResult = 0;


	hResult = FilterSendMessage(
		g_hPort,
		InputBuffer,
		sizeof(InputBuffer),
		NULL,
		NULL,
		&bytesReturned);

	//if (hResult != S_OK) {
	//	return hResult;
	//}
	return hResult;
}

// CCommunicationPortDlg 消息处理程序

BOOL CCommunicationPortDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CString str;
	int errorCode = InitialCommunicationPort();
	if (errorCode != 0)
	{
		str.Format(_T("ErrorCode is %x\n"), errorCode);
		MessageBox(str, _T("Error"), MB_OK);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCommunicationPortDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCommunicationPortDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCommunicationPortDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCommunicationPortDlg::OnBnClickedSendmsg()
{
	// TODO: 在此添加控件通知处理程序代码
	HWND hWnd;
	GetDlgItem(IDC_EDIT1, &hWnd);
	TCHAR strMsg[256] = { 0 };
	::GetWindowText(hWnd, strMsg, 256);
	DWORD ret = NPSendMessage(strMsg);
	if (ret != S_OK) {
		MessageBox(L"Send Message Failed!", L"Error", MB_OK);
	}
}


BOOL CCommunicationPortDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	if(g_hPort != NULL) 
		CloseHandle(g_hPort);
	return CDialogEx::DestroyWindow();
}
