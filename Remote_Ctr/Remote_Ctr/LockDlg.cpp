// LockDlg.cpp: 实现文件
//

#include "pch.h"
#include "Remote_Ctr.h"
#include "LockDlg.h"
#include "afxdialogex.h"


// CLockDlg 对话框

IMPLEMENT_DYNAMIC(CLockDlg, CDialog)

CLockDlg::CLockDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_INFO, pParent)
{

}

CLockDlg::~CLockDlg()
{
}

void CLockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLockDlg, CDialog)
END_MESSAGE_MAP()


// CLockDlg 消息处理程序
