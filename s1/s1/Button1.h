#pragma once
#include "Resourceppc.h"

// Button1 �Ի���

class Button1 : public CDialog
{
	DECLARE_DYNAMIC(Button1)

public:
	Button1(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~Button1();

// �Ի�������
	enum { IDD = IDD_S1_DIALOG_WIDE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
};
