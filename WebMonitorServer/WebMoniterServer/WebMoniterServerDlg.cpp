
// WebMoniterServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WebMoniterServer.h"
#include "WebMoniterServerDlg.h"
#include "afxdialogex.h"

#include <string>
using namespace std;

//JSON库
#include "./json/json.h"
#include "./json/reader.h"
#include "./json/value.h"
#include "./json/writer.h"
#define WM_SOCKET (WM_USER+1234) //自定义Socket消息
SOCKET sClient; //用于通信的Socket

//性能计数器
#include "PerformanceCounter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWebMoniterServerDlg 对话框




CWebMoniterServerDlg::CWebMoniterServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWebMoniterServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWebMoniterServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWebMoniterServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKET, OnSocketHandler) //绑定Socket消息
	ON_BN_CLICKED(IDC_BUTTON_START_SERVER, &CWebMoniterServerDlg::OnBnClickedButtonStartServer)
END_MESSAGE_MAP()


// CWebMoniterServerDlg 消息处理程序

BOOL CWebMoniterServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWebMoniterServerDlg::OnPaint()
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
HCURSOR CWebMoniterServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT WINAPI SendDataThreadProc(LPVOID pParam) //发送数据的线程回调函数
{

	//创建CPU占用率性能计数器
	CPerformanceCounter CPUUseAge(TEXT("\\Processor(0)\\% Processor Time"));
	//创建内存占用率性能计数器
	CPerformanceCounter MemoryUseAge(TEXT("\\Memory\\Available MBytes"));
	//创建磁盘IO性能计数器
	CPerformanceCounter DiskUseAge(TEXT("\\PhysicalDisk(_Total)\\% Idle Time"));

	while(true)
	{
		Sleep(100);

		//得到当前计数器百分值
		double nCPUUseAge = CPUUseAge.GetCurrentValue();
		double nMemoryUseAge = 100 * (1 - MemoryUseAge.GetCurrentValue()/1976);
		double nDiskUseAge = 100 - floor(DiskUseAge.GetCurrentValue());

		//封装成JSON字符串
		Json::Value json;
		json["msg_type"] = Json::Value("data");
		json["cpu_use"] = Json::Value(nCPUUseAge);
		json["memory_use"] = Json::Value(nMemoryUseAge);
		json["disk_use"] = Json::Value(nDiskUseAge);
		string strText = json.toStyledString();

		//发送性能信息
		::send(sClient, strText.c_str(), strlen(strText.c_str()), 0);
	}
	return 1;
}
LRESULT CWebMoniterServerDlg::OnSocketHandler(WPARAM wParam, LPARAM lParam)
{
	sClient = (SOCKET)wParam;
	int nZero=0; //设置发送缓冲区为0
	setsockopt(sClient,SOL_SOCKET,SO_SNDBUF,(char *)&nZero,sizeof(int));
	setsockopt(sClient,SOL_SOCKET,SO_RCVBUF,(char *)&nZero,sizeof(int));
	if(WSAGETASYNCERROR(lParam))
	{
		//错误处理
	}
	else
	{
		switch(WSAGETSELECTEVENT(lParam))
		{
			case FD_ACCEPT:
				break;
			case FD_CONNECT:
				break;
			case FD_CLOSE:
				break;
			case FD_READ:
				char buff[1024] = {0};
				::recv(sClient, buff, 1024, 0); //接收数据
				string strText(buff);
				Json::Reader reader; //创建JSON读取对象
				Json::Value json; //创建JSON对象
				if(reader.parse(strText, json)) //解析JSON字符串
				{
					CString strMsgType(json["msg_type"].asString().c_str());
					if(strMsgType == "assign") //结点指派消息
					{
						CString strNodeNum(json["node_num"].asString().c_str());
						SetDlgItemText(IDC_STATIC_NODE_NUM, strNodeNum + "号结点");
						pDataThread = AfxBeginThread((AFX_THREADPROC)SendDataThreadProc, &sClient); //创建线程获取并发送性能信息
					}
					else if(strMsgType == "cmd") //命令消息
					{
						CString strCMD(json["str_cmd"].asString().c_str());
						ExcuteCMD(strCMD); //执行命令
					}
				}
				break;
		}
	}
	return 0;
}


/*执行命令*/
void CWebMoniterServerDlg::ExcuteCMD(CString strCMD)
{
	strCMD = _T("cmd /C ") + strCMD;
	if (!CreateProcess(NULL, strCMD.GetBuffer(), NULL,NULL, TRUE, NULL ,NULL ,NULL ,&si , &pi))
	{
		return;
	}
}

/*命令处理线程回调*/
UINT WINAPI CMDThreadProc(LPVOID pParam)
{
	HANDLE* phRead = (HANDLE*)pParam;
	char buffer[4096] = {0};
	DWORD bytesRead;
	while (true)
	{
		if(ReadFile(*phRead, buffer, 4095, &bytesRead, NULL) == NULL)break; //通过匿名管道读取命令行信息
		::send(sClient, buffer, strlen(buffer), 0); //发送命令执行结果
		memset(buffer, 0, 4095);
	}
	return 0;
}

void CWebMoniterServerDlg::OnBnClickedButtonStartServer()
{
	CString strButtonText;
	GetDlgItemText(IDC_BUTTON_START_SERVER, strButtonText);
	if(strButtonText != _T("停止监听服务器"))
	{
		//创建监听套接字
		USHORT nPort = 4567;
		sConnect = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(nPort);
		sin.sin_addr.s_addr = inet_addr("127.0.0.1");

		//连接中心服务器
		int iResult = connect(sConnect, (sockaddr*)&sin, sizeof (sin));
		if (iResult == SOCKET_ERROR)
		{  
			AfxMessageBox(TEXT("连接到服务器失败"));
		}
		else
		{
			//使用异步选择模型管理套接字
			::WSAAsyncSelect(sConnect, GetSafeHwnd(), WM_SOCKET, FD_ACCEPT|FD_CONNECT|FD_CLOSE|FD_READ);
			SetDlgItemText(IDC_BUTTON_START_SERVER, _T("停止监听服务器"));
		}

		//创建匿名管道
		SECURITY_ATTRIBUTES sa;
		
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;

		if(!CreatePipe(&hRead, &hWrite, &sa, 0))
		{
			AfxMessageBox(_T("CreatePipe Failed!"));
			return;
		}

		ZeroMemory(&si,sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);	
		GetStartupInfo(&si);
		si.hStdError = hWrite;	
		si.hStdOutput = hWrite;
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		//创建线程处理子进程
		pCMDThread = AfxBeginThread((AFX_THREADPROC)CMDThreadProc, &hRead);
	}
	else
	{
		if(pDataThread != NULL)
		{
			TerminateThread(pDataThread->m_hThread, 0); //终止性能获取线程
			CloseHandle(pDataThread->m_hThread);
		}

		//关闭匿名管道
		if(pCMDThread != NULL)
		{
			TerminateThread(pCMDThread->m_hThread, 0); //终止命令执行线程
			CloseHandle(pCMDThread->m_hThread);
		}
		CloseHandle(hWrite);
		CloseHandle(hRead);

		closesocket(sConnect); //关闭连接
		SetDlgItemText(IDC_BUTTON_START_SERVER, _T("启动监听服务器"));
	}
}