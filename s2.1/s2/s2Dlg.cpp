// s2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "s2.h"
#include "CECommCtrl.h"
#include "s2Dlg.h"
#include "SerialPort.h"
#include "stdafx.h"

#include "CECommCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_REFRESH 8000           // 数据刷新计时器ID
#define SEND_DATA_TIMES 3               // 发送数据的次数（无需返回值且无法查询的指令，为确保接收正常，发送的重复次数）

#define PORT_NUMBER_COORDINATOR 2       // 传感器协调器端口
#define BAUND_RATE_COORDINATOR 115200   // 传感器协调器波特率

#define PORT_NUMBER_M104BPC 3           // M104BPC端口
#define BAUND_RATE_M104BPC 19200

CCommCtrl g_CommCtrl;
int usrTem = 26;
int usrHum = 45;
bool LED1 = false;
bool LED2 = false;
bool LED3 = false;
bool LED4 = false;
int humid;
int temp;

struct Time{
	int hour;
	int minute;
	int second;
};
Time *t;

long lstt;

// Cs2Dlg 对话框

Cs2Dlg::Cs2Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cs2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cs2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cs2Dlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &Cs2Dlg::OnBnClickedButton1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON2, &Cs2Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &Cs2Dlg::OnBnClickedButton3)
	ON_STN_CLICKED(IDC_STATIC2, &Cs2Dlg::OnStnClickedStatic2)
	ON_STN_CLICKED(IDC_STATIC3, &Cs2Dlg::OnStnClickedStatic3)
END_MESSAGE_MAP()


// Cs2Dlg 消息处理程序

BOOL Cs2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	
	if (!g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR))
        MessageBox(L"4",MB_OK);
	Sleep(100);
	//g_CommCtrl.GetHumidity(PORT_NUMBER_COORDINATOR,&humid);
	Sleep(100);
	//g_CommCtrl.GetTemperature(PORT_NUMBER_COORDINATOR,&temp);
	temp = 12;
	tem();
	Sleep(100);
	showtem();
	showhum();
	SetTimer(1,5000,NULL);// TODO: 在此添加额外的初始化代码
	SetTimer(2,5000,NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void Cs2Dlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_S2_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_S2_DIALOG));
	}
}
#endif




void Cs2Dlg::OnBnClickedButton1()
{
	//打开串口通信

	//自动控温
	/*MOTOR_STATUS status = STOP;
	for (;;)
	{
		Timer();
		if (g_CommCtrl.GetTemperature(PORT_NUMBER_COORDINATOR,&temp))
		{
			if(temp>usrTem)
			{
				status = FORWARD;
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
			}
			else if(temp==usrTem)
			{
				status = STOP;
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
				LED3 = false;
				LED4 = false;
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,3,LED3);
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,4,LED4);
				
			}
			else if (temp<usrTem && temp-usrTem<=5)
			{
				LED3 = true;
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,3,LED3);
			}
			else if (temp<usrTem && temp-usrTem>5)
			{
				LED4 = true;
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,4,LED4);
			}
		}
	}*/

	//自动控湿
	/*MOTOR_STATUS status = STOP;
	for (;;)
	{
		Timer();
		if (g_CommCtrl.GetHumidity(PORT_NUMBER_COORDINATOR,&humid))
		{
			if(humid>usrHum)
			{
				status = FORWARD;
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
			}
			else if(humid==usrHum)
			{
				status = STOP;
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
			}
			else if (humid<usrHum)
			{
				status = BACKWARD;
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
			}
		}
	}*/
	//if (!m104bpc_OpenPort (PORT_NUMBER_M104BPC, BAUND_RATE_M104BPC))
    //    MessageBox(L"5",MB_OK);
	//开关灯
	LED1 = !LED1;
	LED2 = !LED2;
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
	Sleep(500);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
	Sleep(500);

}

//十秒定时
void Cs2Dlg::showtem()
{
	CString t;
	t.Format(_T("%d"),temp);
	GetDlgItem(IDC_STATIC3)->SetWindowText(t);

}

void Cs2Dlg::showhum()
{
	CString t;
	t.Format(_T("%d"),humid);
	GetDlgItem(IDC_STATIC5)->SetWindowText(t);
}
void Cs2Dlg::hum()
{
	MOTOR_STATUS status = STOP;
		if (g_CommCtrl.GetHumidity(PORT_NUMBER_COORDINATOR,&humid))
		{
			if(humid>usrHum)
			{
				status = FORWARD;
				Sleep(100);
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
				Sleep(100);
			}
			else if(humid==usrHum)
			{
				status = STOP;
				Sleep(100);
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
				Sleep(100);
			}
			else if (humid<usrHum)
			{
				status = BACKWARD;
				Sleep(100);
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
				Sleep(100);
			}
		}
}
void Cs2Dlg::tem()
{
	MOTOR_STATUS status = STOP;
		if (g_CommCtrl.GetTemperature(PORT_NUMBER_COORDINATOR,&temp))
		{
			if(temp>usrTem)
			{
				status = FORWARD;
				Sleep(100);
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
				Sleep(100);
			}
			else if(temp==usrTem)
			{
				status = STOP;
				Sleep(100);
				g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR,status);
				Sleep(100);
				LED3 = false;
				LED4 = false;
				Sleep(100);
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,3,LED3);
				Sleep(100);
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,4,LED4);
				Sleep(100);
				
			}
			else if (temp<usrTem && usrTem-temp<=5)
			{
				LED3 = true;
				Sleep(100);
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,3,LED3);
				Sleep(100);
			}
			else if (temp<usrTem && usrTem-temp>5)
			{
				LED4 = true;
				Sleep(100);
				g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,4,LED4);
				Sleep(100);
			}
		}
}

//时钟


void Cs2Dlg::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
		case 1:
			tem();
			showtem();
			break;
		case 2:
			hum();
			showhum();
			break;

	}// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnTimer(nIDEvent);
}

void Cs2Dlg::OnBnClickedButton2()
{
	MOTOR_STATUS status = STOP;
	KillTimer(1);
	KillTimer(2);
	Sleep(100);
	g_CommCtrl.SetMotor(PORT_NUMBER_COORDINATOR,status);
	Sleep(100);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,3,false);
	Sleep(100);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,4,false);
	Sleep(100);
	// TODO: 在此添加控件通知处理程序代码
}

void Cs2Dlg::OnBnClickedButton3()
{
	SetTimer(1,5000,NULL);// TODO: 在此添加额外的初始化代码
	SetTimer(2,5000,NULL);
}

void Cs2Dlg::OnStnClickedStatic2()
{
	// TODO: 在此添加控件通知处理程序代码
}

void Cs2Dlg::OnStnClickedStatic3()
{
	// TODO: 在此添加控件通知处理程序代码
}
