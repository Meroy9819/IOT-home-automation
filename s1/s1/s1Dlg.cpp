// s1Dlg.cpp : ʵ���ļ�
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

// Cs1Dlg �Ի���
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


// Cs1Dlg ��Ϣ�������

BOOL Cs1Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������

}
