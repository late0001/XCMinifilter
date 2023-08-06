
// XLoaderDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "XLoader.h"
#include "XLoaderDlg.h"
#include "afxdialogex.h"
#include <winsvc.h>

TCHAR szDbgMsg[256];
#define LOGD(fmt,...)                \
    do {                                        \
          _stprintf(szDbgMsg, _T("[Mango]: ")); OutputDebugString(szDbgMsg);    \
        _stprintf(szDbgMsg, fmt, __VA_ARGS__); OutputDebugString(szDbgMsg);\
          _stprintf(szDbgMsg, _T("                     [%s, line #(%d)]\n"), TEXT(__FUNCTION__), __LINE__);OutputDebugString(szDbgMsg);\
    } while(0)

#ifdef _DEBUG
#define new DEBUG_NEW
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


// CXLoaderDlg 对话框



CXLoaderDlg::CXLoaderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_XLOADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CXLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CXLoaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOADSYS, &CXLoaderDlg::OnBnClickedLoadsys)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CXLoaderDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_UNLOADSYS, &CXLoaderDlg::OnBnClickedBtnUnloadsys)
END_MESSAGE_MAP()

CXLoaderDlg *pXLoaderDlg;
// CXLoaderDlg 消息处理程序
#define DRV_NAME      "MyMinifilter"//驱动名
#define DRV_FILENAME     "MyMinifilter.sys"//驱动文件

#define STATUS_SUCCESS     ((NTSTATUS)0x00000000L)


typedef LONG NTSTATUS;

typedef struct _STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PCHAR  Buffer;
} ANSI_STRING, * PANSI_STRING;

typedef struct _UNICODE_STRING {
	USHORT  Length;
	USHORT  MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;
//*********************************************************************************************
// Assign loaddriver priviledge to our process, so we can load our support driver.
//
//*********************************************************************************************

BOOL getLoadDriverPriv()
{
	HANDLE hToken;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		LUID huid;
		if (LookupPrivilegeValue(NULL, L"SeLoadDriverPrivilege", &huid))
		{
			LUID_AND_ATTRIBUTES priv;
			priv.Attributes = SE_PRIVILEGE_ENABLED;
			priv.Luid = huid;

			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			tp.Privileges[0] = priv;

			if (AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CXLoaderDlg::OnInitDialog()
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
	m_StatusBar.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0); 
	//int strPartDim[2] = {  380 , -1}; //分割数量
	int strPartDim[1] = {  -1 }; //分割数量
	m_StatusBar.SetParts(1, strPartDim);
	//设置状态栏文本
	m_StatusBar.SetText(L"分栏一", 0, 0);
	//m_StatusBar.SetText(L"分栏二", 1, 0);
	//m_StatusBar.SetText(L"分栏三", 1, 0);
	//下面是在状态栏中加入图标
	//m_StatusBar.SetIcon(1,m_hIcon);//为第二个分栏中加的图标
	pXLoaderDlg = this;

	if (!getLoadDriverPriv())
	{
		//printf("Error getting load driver privilege! ");
		AfxMessageBox(_T("Error getting load driver privilege! "));
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

int StatusPrintf(_In_z_ _Printf_format_string_    wchar_t const* const _Format,
	...)
{
	int _Result;
	va_list _ArgList;
	wchar_t _Buffer[256] = {0};
	//_stprintf();
	__crt_va_start(_ArgList, _Format);
	_Result = __vswprintf_l(_Buffer, _Format, NULL, _ArgList);
	__crt_va_end(_ArgList);
	
	if (pXLoaderDlg != NULL)
	{
		pXLoaderDlg->m_StatusBar.SetText(_Buffer, 0, 0);
	}
	return _Result;
}

void CXLoaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CXLoaderDlg::OnPaint()
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
HCURSOR CXLoaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



//*********************************************************************************************
// Sets up the necessary registry settings to load the support driver
//
//*********************************************************************************************

BOOL setupRegistry(TCHAR *lpszSvrName)
{
	HKEY hKey;
	/*if (RegCreateKey(HKEY_LOCAL_MACHINE, L"System\CurrentControlSet\Services\"DRV_NAME, &hkey) != ERROR_SUCCESS)
		return FALSE;

	DWORD val;
	val = 1;
	if (RegSetValueEx(hkey, "Type", 0, REG_DWORD, (PBYTE)&val, sizeof(val)) != ERROR_SUCCESS)
		return FALSE;

	if (RegSetValueEx(hkey, "ErrorControl", 0, REG_DWORD, (PBYTE)&val, sizeof(val)) != ERROR_SUCCESS)
		return FALSE;

	val = 3;
	if (RegSetValueEx(hkey, "Start", 0, REG_DWORD, (PBYTE)&val, sizeof(val)) != ERROR_SUCCESS)
		return FALSE;

	char* imgName = "System32\DRIVERS\"DRV_FILENAME;

		if (RegSetValueEx(hkey, "ImagePath", 0, REG_EXPAND_SZ, (PBYTE)imgName, strlen(imgName)) != ERROR_SUCCESS)
			return FALSE;*/
	//TCHAR lpszDriverName[] = _T(DRV_NAME);
	wchar_t lpszAltitude[] = L"370030";
	TCHAR szTempStr[256];
	char szTempBuf[256];
	ZeroMemory(szTempStr, sizeof(szTempStr));
	ZeroMemory(szTempBuf, sizeof(szTempBuf));
	_tcscpy(szTempStr, _T("SYSTEM\\CurrentControlSet\\Services\\"));
	_tcscat(szTempStr, lpszSvrName);
	_tcscat(szTempStr, _T("\\Instances"));
	DWORD dwData;
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szTempStr, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	// 注册表驱动程序的DefaultInstance 值 
	_tcscpy(szTempStr, lpszSvrName);
	_tcscat(szTempStr, _T(" Instance"));
	
	if (RegSetValueEx(hKey, _T("DefaultInstance"), 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)_tcslen(szTempStr) *sizeof(wchar_t)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);


	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances\\DriverName Instance子键下的键值项 
	//-------------------------------------------------------------------------------------------------------
	_tcscpy(szTempStr, _T("SYSTEM\\CurrentControlSet\\Services\\"));
	_tcscat(szTempStr, lpszSvrName);
	_tcscat(szTempStr, _T("\\Instances\\"));
	_tcscat(szTempStr, lpszSvrName);
	_tcscat(szTempStr, _T(" Instance"));
	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szTempStr, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, (LPDWORD)&dwData) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	// 注册表驱动程序的Altitude 值
	wcscpy(szTempStr, lpszAltitude);
	if (RegSetValueEx(hKey, _T("Altitude"), 0, REG_SZ, (CONST BYTE*)szTempStr, (DWORD)10*sizeof(wchar_t)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	// 注册表驱动程序的Flags 值
	dwData = 0x0;
	if (RegSetValueEx(hKey, _T("Flags"), 0, REG_DWORD, (CONST BYTE*) & dwData, sizeof(DWORD)) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegFlushKey(hKey);//刷新注册表
	RegCloseKey(hKey);
	return TRUE;
}

BOOL svrRegistryKeyIfExist(TCHAR *lpszSvrName)
{
	TCHAR szTempStr[256];
	HKEY hKey;
	ZeroMemory(szTempStr, sizeof(szTempStr));
	_tcscpy(szTempStr, _T("SYSTEM\\CurrentControlSet\\Services\\"));
	_tcscat(szTempStr, lpszSvrName);
	//_tcscat(szTempStr, _T("\\Instances"));
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		szTempStr,
		0,// reserved
		KEY_QUERY_VALUE, // KEY_ALL_ACCESS,   security access mask
		&hKey ) != ERROR_SUCCESS)   // handle to open key) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	RegCloseKey(hKey);
	return TRUE;
}
//*********************************************************************************************
// Actual code to load our driver into memory
//
//*********************************************************************************************

BOOL loadDriver(TCHAR* lpszSvrName)
{
	// call ntdll APIs
	HMODULE hntdll;

	NTSTATUS(WINAPI * _RtlAnsiStringToUnicodeString)(PUNICODE_STRING  DestinationString, IN PANSI_STRING  SourceString, IN

		BOOLEAN);

	VOID(WINAPI * _RtlInitAnsiString)
		(IN OUT PANSI_STRING  DestinationString,
			IN PCHAR  SourceString);

	VOID(WINAPI * _RtlInitUnicodeString)
		(IN OUT PUNICODE_STRING  DestinationString,
			IN PWCHAR  SourceString);
	
	NTSTATUS(WINAPI * _ZwLoadDriver)
		(IN PUNICODE_STRING DriverServiceName);

	NTSTATUS(WINAPI * _ZwUnloadDriver)
		(IN PUNICODE_STRING DriverServiceName);

	VOID(WINAPI * _RtlFreeUnicodeString)
		(IN PUNICODE_STRING  UnicodeString);


	hntdll = GetModuleHandle(L"ntdll.dll");
	if (hntdll == NULL) 
		MessageBox(NULL, L"GetModuleHandle ntdll.dll failed", L"ERROR", MB_OK);

	*(FARPROC*)&_ZwLoadDriver = GetProcAddress(hntdll, "NtLoadDriver");

	*(FARPROC*)&_ZwUnloadDriver = GetProcAddress(hntdll, "NtUnloadDriver");

	*(FARPROC*)&_RtlAnsiStringToUnicodeString =
		GetProcAddress(hntdll, "RtlAnsiStringToUnicodeString");

	*(FARPROC*)&_RtlInitAnsiString =
		GetProcAddress(hntdll, "RtlInitAnsiString");

	*(FARPROC*)&_RtlInitUnicodeString =
		GetProcAddress(hntdll, "RtlInitUnicodeString");

	*(FARPROC*)&_RtlFreeUnicodeString =
		GetProcAddress(hntdll, "RtlFreeUnicodeString");

	if (_ZwLoadDriver && _ZwUnloadDriver  &&
		_RtlInitUnicodeString && _RtlFreeUnicodeString)
	{
		UNICODE_STRING uStr;
		TCHAR szBuf[MAX_PATH + 1] = {0};
		_tcscpy(szBuf, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");
		_tcsncat(szBuf, lpszSvrName, MAX_PATH);

		_RtlInitUnicodeString(&uStr, szBuf);


		//if (_RtlAnsiStringToUnicodeString(&uStr, &aStr, TRUE) != STATUS_SUCCESS)
		//	return FALSE;
		//else
		//{
			if (_ZwLoadDriver(&uStr) == STATUS_SUCCESS)
			{
				//_RtlFreeUnicodeString(&uStr);
				return TRUE;
			}
			//_RtlFreeUnicodeString(&uStr);
		//}
	}

	return FALSE;
}

//*********************************************************************************************
// Actual code to remove our driver from memory
//
//*********************************************************************************************

BOOL unloadDriver(TCHAR* lpszSvrName)
{
	// call ntdll APIs
	HMODULE hntdll;
	NTSTATUS(WINAPI * _RtlAnsiStringToUnicodeString)
		(PUNICODE_STRING  DestinationString,
			IN PANSI_STRING  SourceString,
			IN BOOLEAN);

	VOID(WINAPI * _RtlInitAnsiString)
		(IN OUT PANSI_STRING  DestinationString,
			IN PCHAR  SourceString);

	VOID(WINAPI * _RtlInitUnicodeString)
		(IN OUT PUNICODE_STRING  DestinationString,
			IN PWCHAR  SourceString);

	NTSTATUS(WINAPI * _ZwLoadDriver)
		(IN PUNICODE_STRING DriverServiceName);

	NTSTATUS(WINAPI * _ZwUnloadDriver)
		(IN PUNICODE_STRING DriverServiceName);

	VOID(WINAPI * _RtlFreeUnicodeString)
		(IN PUNICODE_STRING  UnicodeString);


	hntdll = GetModuleHandle(L"ntdll.dll");

	*(FARPROC*)&_ZwLoadDriver = GetProcAddress(hntdll, "NtLoadDriver");

	*(FARPROC*)&_ZwUnloadDriver = GetProcAddress(hntdll, "NtUnloadDriver");

	*(FARPROC*)&_RtlAnsiStringToUnicodeString =
		GetProcAddress(hntdll, "RtlAnsiStringToUnicodeString");

	*(FARPROC*)&_RtlInitAnsiString =
		GetProcAddress(hntdll, "RtlInitAnsiString");

	*(FARPROC*)&_RtlInitUnicodeString =
		GetProcAddress(hntdll, "RtlInitUnicodeString");

	*(FARPROC*)&_RtlFreeUnicodeString =
		GetProcAddress(hntdll, "RtlFreeUnicodeString");

	if (_ZwLoadDriver && _ZwUnloadDriver &&
		_RtlInitUnicodeString && _RtlFreeUnicodeString)
	{
		UNICODE_STRING uStr;
		TCHAR szBuf[MAX_PATH + 1] = { 0 };
		_tcscpy(szBuf, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");
		_tcsncat(szBuf, lpszSvrName, MAX_PATH);
		_RtlInitUnicodeString(&uStr, szBuf);

		if (_ZwUnloadDriver(&uStr) == STATUS_SUCCESS)
		{
			//_RtlFreeUnicodeString(&uStr);
			return TRUE;
		}
		//_RtlFreeUnicodeString(&uStr);
	}

	return FALSE;
}
//using namespace std;
LSTATUS DeleteValueKey(HKEY hKeyRoot, TCHAR *Subkey, TCHAR *ValueKey)
{
	HKEY hKey = NULL;
	LSTATUS bReturn = FALSE;

	if (RegOpenKeyEx(hKeyRoot, Subkey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
#ifndef _WIN64
		bReturn = RegDeleteKey(hKey, ValueKey);
#else
		//bReturn = RegDeleteKeyEx(hKey, ValueKey, KEY_WRITE, 0);
		SHDeleteKey(hKey, ValueKey);
#endif
	}

	if (hKey != NULL) { RegCloseKey(hKey); }

	return bReturn;
}
//*********************************************************************************************
// Removes our driver file and registry settings
//
//*********************************************************************************************

void cleanupDriver(TCHAR *lpszDriverPath, TCHAR *szlpSvrName)
{
	LSTATUS bRet;
	TCHAR szBuf[256];
	//TCHAR sysDir[MAX_PATH + 1];
	//GetSystemDirectory(sysDir, MAX_PATH);
	//_tcsncat(sysDir, L"\\drivers\\" DRV_FILENAME, MAX_PATH);
	bRet = DeleteFile(lpszDriverPath);
	if (!bRet) {
		_stprintf(szBuf, L"Delete %s Failed! %d", lpszDriverPath, GetLastError());
		AfxMessageBox(szBuf);
	}

	TCHAR SubKey[MAX_PATH+1] = { 0 };
	TCHAR szKey[MAX_PATH + 1] = { 0 };
	_tcscpy(SubKey, L"System\\CurrentControlSet\\Services\\"); //L"System\\CurrentControlSet\\Services\\");
	_tcsncat(SubKey, szlpSvrName, MAX_PATH);
	_tcscpy(szKey, L"Instances");
	bRet = DeleteValueKey(HKEY_LOCAL_MACHINE, SubKey, szKey);
	if (bRet != ERROR_SUCCESS ){
		StatusPrintf(_T("Error NO: %d"), bRet);
		return;
	}
	_tcscpy(SubKey, L"System\\CurrentControlSet\\Services");//L"System\\CurrentControlSet\\Services\\");
	_tcscpy(szKey, szlpSvrName);
	bRet = DeleteValueKey(HKEY_LOCAL_MACHINE, SubKey, szKey);

	if (bRet != ERROR_SUCCESS) {
		StatusPrintf(_T("Error NO: %d"), bRet);
		return;
	}
	StatusPrintf(_T("Clean successful!"));
}

BOOL loadNTDriver(TCHAR *lpszDriverPath)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hService = NULL;// NT驱动程序的服务句柄
	TCHAR szTempBuf[512];
	TCHAR lpszDriverName[] = _T(DRV_NAME);
	TCHAR szDriverImagePath[256];

	ZeroMemory(szTempBuf, sizeof(szTempBuf));
	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		// OpenSCManager失败
		StatusPrintf(L"OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		// OpenSCManager成功
		StatusPrintf(L"OpenSCManager() ok ! \n");
	}

	TCHAR sysDir[MAX_PATH + 1];
	GetSystemDirectory(sysDir, MAX_PATH);
	_tcscpy(szDriverImagePath, sysDir);
	_tcsncat(szDriverImagePath, _T("\\drivers\\" DRV_FILENAME), MAX_PATH);
	CopyFile(lpszDriverPath, szDriverImagePath, TRUE);
	//创建驱动所对应的服务
	hService = CreateService(hServiceMgr,
		lpszDriverName,             // 驱动程序的在注册表中的名字
		lpszDriverName,             // 注册表驱动程序的DisplayName 值
		SERVICE_ALL_ACCESS,         // 加载驱动程序的访问权限
		SERVICE_FILE_SYSTEM_DRIVER, // 表示加载的服务是文件系统驱动程序
		SERVICE_DEMAND_START,       // 注册表驱动程序的Start 值       01234 五个选项 0 由系统核心进行加载 1 io子系统加载 2自动启动 3 手动启动 4禁止启动
		SERVICE_ERROR_IGNORE,       // 注册表驱动程序的ErrorControl 值
		szDriverImagePath,          // 注册表驱动程序的ImagePath 值
		L"FSFilter Activity Monitor",// 注册表驱动程序的Group 值       如果是文件过滤驱动动态加载则需要指定这个分组
		NULL,
		L"FltMgr",                   // 注册表驱动程序的DependOnService 值  文件过滤驱动需要依赖FltMgr 需要在这里执行
		NULL,
		NULL);
	DWORD dwRtn;
	// 判断服务是否失败
	if (hService == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//由于其他原因创建服务失败
			StatusPrintf(L"CrateService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//服务创建失败，是由于服务已经创立过
			StatusPrintf(L"CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
		}

		// 驱动程序已经加载，只需要打开 
		hService = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hService == NULL)
		{
			// 如果打开服务也失败，则意味错误
			dwRtn = GetLastError();
			StatusPrintf(L"OpenService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			StatusPrintf(L"OpenService() ok ! \n");
		}
	}
	else
	{
		StatusPrintf(L"CrateService() ok ! \n");
	}

	// 开启此项服务
	bRet = StartService(hService, NULL, NULL);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			StatusPrintf(L"StartService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				// 设备被挂住
				StatusPrintf(L"StartService() Faild ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				// 服务已经开启
				StatusPrintf(L"StartService() Faild ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;
	// 离开前关闭句柄
BeforeLeave:
	if (hService)
	{
		CloseServiceHandle(hService); // 服务句柄
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr); // SCM句柄
	}
	return bRet;

}

// 卸载驱动程序 
BOOL unloadNTDriver(TCHAR* szSvrName)
{
	/************************* 卸载NT驱动的代码******************************
	   ① 调用OpenSCManager,打开SCM管理器,如果返回NULL,则返回失败,否则继续.
	   ② 调用OpenService.如果返回NULL,则返回失败,否则继续
	   ③ 调用DeleteService卸载此项服务.
	   ④ 成功返回.
	************************************************************************/

	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hService = NULL;// NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	// 打开SCM管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		// 打开SCM管理器失败
		StatusPrintf(L"OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		// 打开SCM管理器失败成功
		StatusPrintf(L"OpenSCManager() ok ! \n");
	}

	// 打开驱动所对应的服务
	hService = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);

	if (hService == NULL)
	{
		// 打开驱动所对应的服务失败
		StatusPrintf(L"OpenService() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		StatusPrintf(L"OpenService() ok ! \n");
	}

	// 停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &SvrSta))
	{
		StatusPrintf(L"ControlService() Faild %d !\n", GetLastError());
	}
	else
	{
		// 打开驱动所对应的失败
		StatusPrintf(L"ControlService() ok !\n");
	}
	// 动态卸载驱动程序。 
	if (!DeleteService(hService))
	{
		// 卸载失败
		StatusPrintf(L"DeleteSrevice() Faild %d !\n", GetLastError());
	}
	else
	{
		// 卸载成功
		StatusPrintf(L"DelServer:eleteSrevice() ok !\n");
	}
	bRet = TRUE;
BeforeLeave:
	// 离开前关闭打开的句柄
	if (hService)
	{
		CloseServiceHandle(hService); // 服务句柄
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr); // SCM 句柄
	}
	return bRet;
}

void GetErrMsg(DWORD eCode, TCHAR **ssInfo)
{
	TCHAR *sInfo;
	switch (eCode) {
	case ERROR_ACCESS_DENIED: sInfo = L"ERROR_ACCESS_DENIED"; break;
	case ERROR_CIRCULAR_DEPENDENCY: sInfo = L"ERROR_CIRCULAR_DEPENDENCY"; break;
	case ERROR_DUPLICATE_SERVICE_NAME: sInfo = L"ERROR_DUPLICATE_SERVICE_NAME"; break;
	case ERROR_INVALID_HANDLE: sInfo = L"ERROR_INVALID_HANDLE"; break;
	case ERROR_INVALID_NAME: sInfo = L"ERROR_INVALID_NAME"; break;
	case ERROR_INVALID_PARAMETER: sInfo = L"ERROR_INVALID_PARAMETER"; break;
	case ERROR_INVALID_SERVICE_ACCOUNT: sInfo = L"ERROR_INVALID_SERVICE_ACCOUNT"; break;
	case ERROR_SERVICE_EXISTS: sInfo = L"ERROR_SERVICE_EXISTS"; break;
	case ERROR_SERVICE_MARKED_FOR_DELETE: sInfo = L"ERROR_SERVICE_MARKED_FOR_DELETE"; break;
	default:
		sInfo = L"Unknow "; break;
	}

	*ssInfo = sInfo;

}

BOOL CreateDriverSvc(TCHAR* lpszDriverPath, TCHAR *lpszDriverName)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;// SCM管理器的句柄
	SC_HANDLE hService = NULL;// NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	//TCHAR szTempBuf[512];
	//TCHAR lpszDriverName[] = _T(DRV_NAME);
	//TCHAR szDriverImagePath[256];

	//ZeroMemory(szTempBuf, sizeof(szTempBuf));
	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		// OpenSCManager失败
		StatusPrintf(L"OpenSCManager() Faild %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		// OpenSCManager成功
		StatusPrintf(L"OpenSCManager() ok ! \n");
	}

	//TCHAR sysDir[MAX_PATH + 1];
	//GetSystemDirectory(sysDir, MAX_PATH);
	//_tcscpy(szDriverImagePath, sysDir);
	//_tcsncat(szDriverImagePath, _T("\\drivers\\" DRV_FILENAME), MAX_PATH);
	//CopyFile(lpszDriverPath, szDriverImagePath, TRUE);
	//创建驱动所对应的服务
	hService = CreateService(hServiceMgr,
		lpszDriverName,             // 驱动程序的在注册表中的名字
		lpszDriverName,             // 注册表驱动程序的DisplayName 值
		SERVICE_ALL_ACCESS,         // 加载驱动程序的访问权限
		SERVICE_FILE_SYSTEM_DRIVER, // 表示加载的服务是文件系统驱动程序
		SERVICE_DEMAND_START,       // 注册表驱动程序的Start 值       01234 五个选项 0 由系统核心进行加载 1 io子系统加载 2自动启动 3 手动启动 4禁止启动
		SERVICE_ERROR_IGNORE,       // 注册表驱动程序的ErrorControl 值
		lpszDriverPath,          // 注册表驱动程序的ImagePath 值
		L"FSFilter Activity Monitor",// 注册表驱动程序的Group 值       如果是文件过滤驱动动态加载则需要指定这个分组
		NULL,
		L"FltMgr",                   // 注册表驱动程序的DependOnService 值  文件过滤驱动需要依赖FltMgr 需要在这里执行
		NULL,
		NULL);
	DWORD dwRtn;
	// 判断服务是否失败
	if (hService == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//由于其他原因创建服务失败
			StatusPrintf(L"CrateService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			//goto BeforeLeave;
		}
		else
		{
			//服务创建失败，是由于服务已经创立过
			StatusPrintf(L"CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
			
		}

		// 驱动程序已经加载，只需要打开 
		hService = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hService == NULL)
		{
			// 如果打开服务也失败，则意味错误
			dwRtn = GetLastError();
			StatusPrintf(L"OpenService() Faild %d ! \n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			StatusPrintf(L"OpenService() ok ! \n");

		}
		// 停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
		if (!ControlService(hService, SERVICE_CONTROL_STOP, &SvrSta))
		{
			StatusPrintf(L"ControlService() Faild %d !\n", GetLastError());
		}
		else
		{
			// 打开驱动所对应的失败
			StatusPrintf(L"ControlService() ok !\n");
		}
		// 动态卸载驱动程序。 
		if (!DeleteService(hService))
		{
			// 卸载失败
			TCHAR* ErrMsg;
			DWORD errCode = GetLastError();
			GetErrMsg(errCode, &ErrMsg);
			StatusPrintf(L"DeleteSrevice() Faild ErrCode:%d %s!\n", errCode, ErrMsg);
			bRet = TRUE;
			goto BeforeLeave;
		}
		else
		{
			// 卸载成功
			StatusPrintf(L"DelServer: DeleteService() ok !\n");
		}
	}
	else
	{
		StatusPrintf(L"CreateService() ok ! \n");
	}

	
	bRet = TRUE;
	// 离开前关闭句柄
BeforeLeave:
	if (hService)
	{
		CloseServiceHandle(hService); // 服务句柄
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr); // SCM句柄
	}
	return bRet;

}


//*********************************************************************************************
// Attempts to get a handle to our kernel driver.  If fails, try to install the driver.
//
//*********************************************************************************************

HANDLE installDriver()
{
	HANDLE hDevice;
	
	TCHAR symLinkPath[MAX_PATH];
	
	TCHAR lpszDriverPath[MAX_PATH + 1];
	TCHAR lpszSrcPath[MAX_PATH + 1] = {0};
	TCHAR svrName[MAX_PATH + 1];
	GetDriverPath(lpszDriverPath, svrName, lpszSrcPath);
	//CreateFile打开驱动建立的符号链接，得根据驱动中建立的名字更改
	_tcscpy(symLinkPath, _T("\\\\.\\"));
	_tcsncat(symLinkPath, svrName, MAX_PATH);
	hDevice = CreateFile(symLinkPath,
		GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		//TCHAR lpszDriverPath[MAX_PATH + 1];
		//TCHAR* filePart;

		//ZeroMemory(lpszDriverPath, MAX_PATH);
		//GetFullPathName(_T(DRV_FILENAME), MAX_PATH, lpszDriverPath, &filePart);

		StatusPrintf(L"%s ", lpszDriverPath);
		HANDLE hFile = CreateFile(lpszDriverPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			//printf("Cannot find required driver file %s ", lpszDriverPath);
			StatusPrintf(_T("Cannot find required driver file"));
			//CopyFile(lpszSrcPath, lpszDriverPath, TRUE);
			//return INVALID_HANDLE_VALUE;
		}
		else
		{
			CloseHandle(hFile);
			//Copy .sys to system\drivers 
			/*TCHAR sysDir[MAX_PATH + 1];
			GetSystemDirectory(sysDir, MAX_PATH);
			_tcsncat(sysDir, _T("\\drivers\\" DRV_FILENAME), MAX_PATH);*/
			
		}
		
		CopyFile(lpszSrcPath, lpszDriverPath, TRUE);
		if (!CreateDriverSvc(lpszDriverPath, svrName))
			return INVALID_HANDLE_VALUE;

		//if (!getLoadDriverPriv())
		//{
		//	//printf("Error getting load driver privilege! ");
		//	AfxMessageBox(_T("Error getting load driver privilege! "));
		//}
		//else
		{
			if (!setupRegistry(svrName))
			{
				//printf("Error setting driver registry keys! Make sure you are running this as
				//Administrator. ");
				AfxMessageBox(_T("Error setting driver registry keys! Make sure you are running this as\
					Administrator. "));
			}
			else
			{
				loadDriver(svrName);

				/*hDevice = CreateFile(_T("\\\\.\\" DRV_NAME),
					GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ

					| FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hDevice == INVALID_HANDLE_VALUE)
				{
					//printf("Error loading kernel support driver! Make sure you are running
					//this as Administrator. ");
						AfxMessageBox(_T("Error loading kernel support driver! Make sure you are\
running this as Administrator. "));
					}
					else
					{
						AfxMessageBox(_T("loading kernel support driver success"));
					}*/
			}
		}
		//cleanupDriver();
	}

	return hDevice;
}

BOOL GetDriverPath(TCHAR *lpszDriverPath, TCHAR *svrName, TCHAR *lpszSrcPath)
{
	//TCHAR lpszDriverPath[MAX_PATH + 1];
	CString strBuf;
	TCHAR szStrName[MAX_PATH] = {0};
	ASSERT(pXLoaderDlg != NULL);
	HWND hEdit = pXLoaderDlg->GetDlgItem(IDC_EDT_IMAGEPATH)->GetSafeHwnd();
	GetWindowTextW(hEdit, strBuf.GetBuffer(MAX_PATH), MAX_PATH);
	_tcsncpy(lpszSrcPath, strBuf, _tcslen(strBuf.GetBuffer()));

	CString strName = strBuf.GetBufferSetLength(MAX_PATH+1);
	int pos = strName.ReverseFind(_T('\\'));
	//int nCount = _tcslen(strName) - pos - 1 - 4;
	strName = strName.Mid(pos + 1, strName.GetLength() - pos -1);
	GetSystemDirectory(lpszDriverPath, MAX_PATH);
	_tcsncat(lpszDriverPath, _T("\\drivers\\"), MAX_PATH);
	_tcsncat(lpszDriverPath, strName, MAX_PATH);
	hEdit = pXLoaderDlg->GetDlgItem(IDC_EDT_SVRNAME)->GetSafeHwnd();
	GetWindowTextW(hEdit, strBuf.GetBuffer(MAX_PATH), MAX_PATH);
	ZeroMemory(svrName, MAX_PATH);
	_tcsncat(svrName, strBuf, MAX_PATH);

	return TRUE;
}

//*********************************************************************************************
// Remove our kernel driver from memory
//
//*********************************************************************************************

void uninstallDriver(void)
{
	TCHAR lpszDriverPath[MAX_PATH + 1];
	TCHAR svrName[MAX_PATH + 1];
	TCHAR* filePart;
	TCHAR lpszSrcPath[MAX_PATH + 1] = { 0 };
	ZeroMemory(lpszDriverPath, sizeof(lpszDriverPath));
	ZeroMemory(svrName, sizeof(svrName));

	GetDriverPath(lpszDriverPath, svrName, lpszSrcPath);

	
	//ZeroMemory(lpszDriverPath, MAX_PATH);
	//GetFullPathName(_T(DRV_FILENAME), MAX_PATH, lpszDriverPath, &filePart);

	HANDLE hFile = CreateFile(lpszDriverPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		//printf("Cannot find required driver file %s ", lpszDriverPath);
		AfxMessageBox(_T("Cannot find required driver file "));
		return;
	}
	else
	{
		CloseHandle(hFile);

		//TCHAR sysDir[MAX_PATH + 1];
		//GetSystemDirectory(sysDir, MAX_PATH);
		//_tcsncat(sysDir, _T("\\drivers\\" DRV_FILENAME), MAX_PATH);
		//	CopyFile(lpszDriverPath, sysDir, TRUE);

		//if (!getLoadDriverPriv())
		//{
		//	//printf("Error getting load driver privilege! ");
		//	AfxMessageBox(_T("Error getting load driver privilege! "));
		//}
		//else
		{
			if (svrRegistryKeyIfExist(svrName))
			{
				
				unloadNTDriver(svrName);

				if (unloadDriver(svrName))
					//printf("Support driver successfully unloaded. ");
					StatusPrintf(_T("Support driver successfully unloaded. "));
				else
					//printf("Unload support driver failed.  It is probably not loaded. ");
					StatusPrintf(_T("Unload support driver failed.  It is probably not loaded or already unloaded. "));
			}
		}
		cleanupDriver(lpszDriverPath, svrName);
	}
}

void CXLoaderDlg::OnBnClickedBtnBrowse()
{
	// TODO: 在此添加控件通知处理程序代码
    BOOL isOpen = TRUE; //是否打开(否则为保存)
	CString defaultExt = L"txt"; //默认打开的文件路径
	CString fileName = L""; //默认打开的文件名
	CString filter = L"文件 (*.sys; *.*)|*.sys;*.*||"; //文件过虑的类型
	CFileDialog openFileDlg(isOpen, defaultExt, fileName, OFN_HIDEREADONLY | OFN_READONLY, filter, NULL);
	//openFileDlg.GetOFN().lpstrInitialDir = L"C:\\Windows\\system32\\drivers";
	openFileDlg.GetOFN().lpstrInitialDir = L"D:\\jd_projects\\minifilter\\MyMinifilter\\x64\\Debug\\MyMinifilter.sys";
	INT_PTR result = openFileDlg.DoModal();
	CString filePath = fileName;//+ "\\test.doc";
	if (result == IDOK) {
		filePath = openFileDlg.GetPathName();
	}
	SetDlgItemTextW(IDC_EDT_IMAGEPATH, filePath);
	int pos = filePath.ReverseFind(_T('\\'));
	if (pos < 0) {
		AfxMessageBox(L"Cannot find file seperator ");
		return;
	}
	CString svrName = filePath.Mid(pos+1, filePath.GetLength()- pos -1 -4);
	SetDlgItemTextW(IDC_EDT_SVRNAME, svrName);
}

void CXLoaderDlg::OnBnClickedLoadsys()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR lpszDriverPath[MAX_PATH] = { 0 };
	GetDlgItemTextW(IDC_EDT_IMAGEPATH, lpszDriverPath, MAX_PATH);
	CString strPath = lpszDriverPath;
	if (strPath.IsEmpty()){
		AfxMessageBox(L"Please select sys file path for install drivers");
		return;
	}
		
	installDriver();
}

void CXLoaderDlg::OnBnClickedBtnUnloadsys()
{
	// TODO: 在此添加控件通知处理程序代码
	TCHAR lpszDriverPath[MAX_PATH] = { 0 };
	GetDlgItemTextW(IDC_EDT_IMAGEPATH, lpszDriverPath, MAX_PATH);
	CString strPath = lpszDriverPath;
	if (strPath.IsEmpty()) {
		AfxMessageBox(L"Please select sys file path for unstall drivers");
		return;
	}
		
	uninstallDriver();
}
