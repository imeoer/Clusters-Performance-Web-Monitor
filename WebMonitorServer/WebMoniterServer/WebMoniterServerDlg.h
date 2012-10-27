
// WebMoniterServerDlg.h : 头文件
//

#pragma once
#include "initsock.h" //初始化Winsock库


// CWebMoniterServerDlg 对话框
class CWebMoniterServerDlg : public CDialogEx
{
// 构造
public:
	CWebMoniterServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WEBMONITERSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CInitSock InitSock; //初始化Winsock库
	SOCKET sConnect; //用于连接的Socket
	CWinThread* pDataThread; //用于发送数据的线程
	CWinThread* pCMDThread; //用于处理命令的线程

	afx_msg LRESULT OnSocketHandler(WPARAM wParam, LPARAM lParam);
	void ExcuteCMD(CString strCMD);

	//匿名管道相关
	STARTUPINFO si;	
	PROCESS_INFORMATION pi;
	HANDLE hRead,hWrite;
public:
	afx_msg void OnBnClickedButtonStartServer();
};
