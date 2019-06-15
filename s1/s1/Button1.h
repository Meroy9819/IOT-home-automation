#pragma once
#include "Resourceppc.h"

// Button1 对话框

class Button1 : public CDialog
{
	DECLARE_DYNAMIC(Button1)

public:
	Button1(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~Button1();

// 对话框数据
	enum { IDD = IDD_S1_DIALOG_WIDE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
