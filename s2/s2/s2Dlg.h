// s2Dlg.h : ͷ�ļ�
//

#pragma once

// Cs2Dlg �Ի���
class Cs2Dlg : public CDialog
{
// ����

	
public:
	Cs2Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_S2_DIALOG };

	CBitmap m_bitmap1, m_bitmap2, m_bitmap3,m_bitmap4,m_bitmap5;
	CStatic m_ctrlpicture;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	 void showbalance();
	friend DWORD ThreadReadCard (LPVOID);
	void showtem();
	void showhum();
	void hum();
	void tem();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnStnClickedStatic6();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedButton15();
	void lig();
	afx_msg void OnBnClickedButton16();
};
