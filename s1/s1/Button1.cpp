// Button1.cpp : 实现文件
//

#include "stdafx.h"
#include "s1.h"
#include "Button1.h"


// Button1 对话框

IMPLEMENT_DYNAMIC(Button1, CDialog)

Button1::Button1(CWnd* pParent /*=NULL*/)
	: CDialog(Button1::IDD, pParent)
{

}

Button1::~Button1()
{
}

void Button1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Button1, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &Button1::OnBnClickedButton1)
END_MESSAGE_MAP()


// Button1 消息处理程序

void Button1::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(TEXT("gerg"),MB_OK);
}
