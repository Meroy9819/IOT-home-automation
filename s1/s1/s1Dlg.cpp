// s1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "s1.h"
#include "s1Dlg.h"
#include "CECommCtrl.h"
#include "Button1.h"
#include "Resourceppc.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Cs1Dlg 对话框
CCommCtrl g_CommCtrl;  
Cs1Dlg::Cs1Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cs1Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cs1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cs1Dlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_STN_CLICKED(IDC_STATIC_1, &Cs1Dlg::OnStnClickedStatic1)
END_MESSAGE_MAP()


// Cs1Dlg 消息处理程序

BOOL Cs1Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void Cs1Dlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_S1_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_S1_DIALOG));
	}
}
#endif


void Cs1Dlg::OnStnClickedStatic1()
{
	// TODO: 在此添加控件通知处理程序代码

}
