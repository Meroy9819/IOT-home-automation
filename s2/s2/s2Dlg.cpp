// s2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "s2.h"
#include "CECommCtrl.h"
#include "s2Dlg.h"
#include "SerialPort.h"
#include "stdafx.h"
#include <assert.h>

#include "CECommCtrl.h"

// 使用ADTL库
#include "ADTL.h"
#pragma comment (lib, "ADTL")

// 导入SmartAnim库
#include "SmartAnim.h"
#pragma comment (lib, "SmartAnim")

// 导入GeniusCtrl库
#include "GeniusCtrl.h"
#pragma comment (lib, "GeniusCtrl")

// 导入M104BPC库
#include "M104BPC.h"
#pragma comment (lib, "M104BPC")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_REFRESH 8000           // 数据刷新计时器ID
#define SEND_DATA_TIMES 3               // 发送数据的次数（无需返回值且无法查询的指令，为确保接收正常，发送的重复次数）

#define PORT_NUMBER_COORDINATOR 2       // 传感器协调器端口
#define BAUND_RATE_COORDINATOR 115200   // 传感器协调器波特率

#define PORT_NUMBER_M104BPC 3           // M104BPC端口
#define BAUND_RATE_M104BPC 19200
BYTE blockBytes[16];
CCommCtrl g_CommCtrl;
int usrTem = 26;
int usrHum = 45;
bool LED1 = false;
bool LED2 = false;
bool LED3 = false;
bool LED4 = false;
int humid;
int temp;
bool motor = false ;
int aut = 1 ;
bool bright ;

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
	ON_BN_CLICKED(IDC_BUTTON8, &Cs2Dlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &Cs2Dlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON11, &Cs2Dlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, &Cs2Dlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &Cs2Dlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON7, &Cs2Dlg::OnBnClickedButton7)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON4, &Cs2Dlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &Cs2Dlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON15, &Cs2Dlg::OnBnClickedButton15)

	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()

	//ON_BN_CLICKED(IDC_BUTTON15, &Cs2Dlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON16, &Cs2Dlg::OnBnClickedButton16)
END_MESSAGE_MAP()


// Cs2Dlg 消息处理程序
CBitmapButton m_btnLevel1;
CBitmapButton m_btnLevel2;
CBitmapButton m_btnLevel0;
CBitmapButton m_btnFood;
CBitmapButton m_btnM;
CBitmapButton m_btnMStop;

CBitmapButton m_btnDetract;
CBitmapButton m_btnShow;
CBitmapButton m_btnRecharge;

CBitmapButton m_btnStart;
CBitmapButton m_btnEnd;

CBitmapButton m_btnTemp;



BOOL Cs2Dlg::OnEraseBkgnd(CDC* pDC)
{
	CDialog::OnEraseBkgnd(pDC);
	CBitmap m_bitmap;
	m_bitmap.LoadBitmapW(IDB_Blank);
	m_bitmap1.LoadBitmapW(IDB_LightClose);
	m_bitmap2.LoadBitmapW(IDB_LightLevel1);
	m_bitmap3.LoadBitmapW(IDB_LightLevel2);
	//m_bitmap4.LoadBitmapW(IDB_MotorStop);
	//m_bitmap5.LoadBitmapW(IDB_Motor);
	if(!m_bitmap.m_hObject)
		return true;

	CRect rect;
	GetClientRect(&rect);
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap = dc.SelectObject(&m_bitmap);
	int bmw, bmh;
	BITMAP bmap;
	m_bitmap.GetBitmap(&bmap);
	bmw = bmap.bmWidth;
	bmh = bmap.bmHeight;
	int x0 = 0, y0 = 0;
	pDC->StretchBlt(x0, y0, rect.Width(), rect.Height(), &dc, 0, 0, bmw, bmh, SRCCOPY);

	dc.SelectObject(pOldBitmap);
	return true;
}

HBRUSH Cs2Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	CFont m_font;
	m_font.CreatePointFont(150, _T("微软雅黑"));
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(pWnd->GetDlgCtrlID())
	{
		case (IDC_STATIC2):
		case (IDC_STATIC1):
		case (IDC_STATIC4):
		case (IDC_STATIC5):
		case (IDC_STATIC6):
		case (IDC_STATIC7):
		case (IDC_STATIC8):
		{
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0, 0, 0));
			pDC->SelectObject(&m_font);
			return (HBRUSH)(GetStockObject(HOLLOW_BRUSH));
		}
	}
	return hbr;
}

BOOL Cs2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	
	// TODO: 在此添加额外的初始化代码
	m_btnLevel1.LoadBitmaps(IDB_LightLevel1);
	m_btnLevel1.SubclassDlgItem(IDC_BUTTON11, this);
	m_btnLevel1.SizeToContent();

	m_btnLevel2.LoadBitmaps(IDB_LightLevel2);
	m_btnLevel2.SubclassDlgItem(IDC_BUTTON4, this);
	m_btnLevel2.SizeToContent();

	m_btnLevel0.LoadBitmaps(IDB_LightClose);
	m_btnLevel0.SubclassDlgItem(IDC_BUTTON5, this);
	m_btnLevel0.SizeToContent();


	m_btnDetract.LoadBitmaps(IDB_Distract);
	m_btnDetract.SubclassDlgItem(IDC_BUTTON8, this);
	m_btnDetract.SizeToContent();

	m_btnShow.LoadBitmaps(IDB_Show);
	m_btnShow.SubclassDlgItem(IDC_BUTTON7, this);
	m_btnShow.SizeToContent();

	m_btnRecharge.LoadBitmaps(IDB_Recharge);
	m_btnRecharge.SubclassDlgItem(IDC_BUTTON9, this);
	m_btnRecharge.SizeToContent();

	m_btnStart.LoadBitmaps(IDB_Close);
	m_btnStart.SubclassDlgItem(IDC_BUTTON12, this);
	m_btnStart.SizeToContent();

	m_btnEnd.LoadBitmaps(IDB_Start1);
	m_btnEnd.SubclassDlgItem(IDC_BUTTON13, this);
	m_btnEnd.SizeToContent();

	m_btnMStop.LoadBitmaps(IDB_MotorStop);
	m_btnMStop.SubclassDlgItem(IDC_BUTTON16, this);
	m_btnMStop.SizeToContent();

	m_btnM.LoadBitmaps(IDB_Motor);
	m_btnM.SubclassDlgItem(IDC_BUTTON15, this);
	m_btnM.SizeToContent();
	
	//HBITMAP hBitmap;
	//关联图片ID
	/*	hBitmap = (HBITMAP) LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_TempHigh), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);

		CStatic *pStatic = (CStatic *) GetDlgItem(IDC_STATIC1);
		pStatic->ModifyStyle(0xF, SS_BITMAP|SS_CENTERIMAGE);
		pStatic->SetBitmap(hBitmap);
	*/

	if (!g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR))
			MessageBox(L"4",MB_OK);
	Sleep(100);
	g_CommCtrl.GetHumidity(PORT_NUMBER_COORDINATOR,&humid);
	Sleep(100);
	g_CommCtrl.GetTemperature(PORT_NUMBER_COORDINATOR,&temp);
	//temp = 12;
	//tem();
	//humid = 30;
	//hum();
	Sleep(100);
	showtem();
	showhum();
	SetTimer(1,1000,NULL);// TODO: 在此添加额外的初始化代码
	SetTimer(2,1000,NULL);
	SetTimer(3,1000,NULL);

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



//十秒定时
void Cs2Dlg::showtem()
{
	CRect rc;
	GetDlgItem(IDC_STATIC6)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	InvalidateRect(rc);
	CString t;
	t.Format(_T("%d"),temp);
	GetDlgItem(IDC_STATIC4)->SetWindowText(t);

}

void Cs2Dlg::showhum()
{
	CRect rc;
	GetDlgItem(IDC_STATIC6)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	InvalidateRect(rc);
	CString t;
	t.Format(_T("%d"),humid);
	GetDlgItem(IDC_STATIC6)->SetWindowText(t);
}
void Cs2Dlg::hum()
{
	MOTOR_STATUS status = STOP;
	//showhum();
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
void Cs2Dlg::lig()
{
	if (g_CommCtrl.GetLight(PORT_NUMBER_COORDINATOR,&bright))
	{
		LED1 = true ;
		LED2 = false;
		g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
		Sleep(200);
		g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
		Sleep(200);
	}
	else
	{
		LED1 = LED2 = false;
		g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
		Sleep(200);
		g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
		Sleep(200);
	}
}

void Cs2Dlg::tem()
{
	MOTOR_STATUS status = STOP;
	//showtem();
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




#define MAX_LOADSTRING 100              // 主窗体标题及类名最大长度
#define MAX_LINE_LENGTH 128             // 支持的配置文件每行最长字符数

#define TIMER_ID_REFRESH 8000           // 数据刷新计时器ID
#define SEND_DATA_TIMES 3               // 发送数据的次数（无需返回值且无法查询的指令，为确保接收正常，发送的重复次数）

#define PORT_NUMBER_COORDINATOR 2       // 传感器协调器端口
#define BAUND_RATE_COORDINATOR 115200   // 传感器协调器波特率

#define PORT_NUMBER_M104BPC 3           // M104BPC端口
#define BAUND_RATE_M104BPC 19200        // M104BPC波特率

#define BASE_DIR TEXT("\\PocketMory1\\")                            // 程序基础文件夹
#define MAKE_FULL_PATH(dayNumber) (BASE_DIR TEXT(#dayNumber))       // 展开文件完整路径到程序基础文件夹

#define ANIM_RES_FILENAME MAKE_FULL_PATH(AnimRes.grc)               // 动画资源文件完整路径
#define PIC_RES_FILENAME MAKE_FULL_PATH(PicRes.grc)                 // 图片资源文件完整路径
#define CTRL_RES_FILENAME MAKE_FULL_PATH(CtrlRes.grc)               // 控制资源文件完整路径
#define PIC_DIR_PATH MAKE_FULL_PATH(Pics\\)                         // 图片资源文件夹完整路径

#define SOUND_FAMILY_SUCCEED MAKE_FULL_PATH(Family_Succeed.wav)     // 智慧家庭读卡成功声音文件路径
#define SOUND_FAMILY_FAILED MAKE_FULL_PATH(Family_Failed.wav)       // 智慧家庭读卡失败声音文件路径
#define SOUND_SUBWAY_SUCCEED MAKE_FULL_PATH(Subway_Succeed.wav)     // 一卡通读卡成功声音文件路径
#define SOUND_SUBWAY_FAILED MAKE_FULL_PATH(Subway_Failed.wav)       // 一卡通读卡失败声音文件路径
#define SOUND_FOOD_SUCCEED MAKE_FULL_PATH(Food_Succeed.wav)         // 食品溯源读卡成功声音文件路径
#define SOUND_FOOD_FAILED MAKE_FULL_PATH(Food_Failed.wav)           // 食品溯源读卡失败声音文件路径

#define SERVER_PATH MAKE_FULL_PATH(Gemini2\\DeviceServer.exe)       // 设备服务器路径
#define RFID_COF_FILE MAKE_FULL_PATH(RFID.txt)                      // RFID卡ID路径
#define SERVER_ADDR_FILE MAKE_FULL_PATH(ServerAddr.txt)             // 网络通信地址路径

#define LOG_FILE MAKE_FULL_PATH(log.txt)                            // 日志文件

// 传感器状态结构体
typedef struct _SENSOR_STATUS {
    bool bAirConditioner;               // 空调状态
    bool bLight[4];                     // 灯状态
    bool bSpray;                        // 喷水头状态
    bool bFan;                          // 排气扇状态
    bool bHumidityAuto;                 // 湿度自动控制状态
    int nHumiditySettingValue;          // 湿度设置值
} SENSOR_STATUS, *PSENSOR_STATUS;
int balance;
enum DateSettingItem { YEAR, MONTH, DAY, HOUR, MINUTE };     // 日期设置项目枚举

// 全局变量:
HINSTANCE g_hInst;					    // 当前应用程序实例
HWND g_hWnd;						    // 主窗体句柄
HWND g_hDeviceServer = NULL;            // 设备服务器消息窗体句柄

UINT WM_SEND_WINDOW_HANDLE = 0;         // 设备服务器发送其消息窗体句柄
UINT WM_SET_WEBSITE_ADDR = 0;           // 设定设备服务器网站连接地址
UINT WM_SET_LED = 0;                    // 设置LED灯状态
UINT WM_SET_MOTOR = 0;                  // 设置电机状态
UINT WM_UPDATE_TEMPERATURE = 0;         // 更新温度
UINT WM_UPDATE_HUMIDITY = 0;            // 更新湿度
UINT WM_UPDATE_DAY_NIGHT = 0;           // 更新光照状态
UINT WM_CLOSE_SERVER = 0;               // 关闭设备服务器

int g_InterfaceID = -1;                 // 当前界面ID
int g_GoalInterfaceID = -1;             // 目标界面ID，该ID指示将要进入的界面

CSmartAnim g_SmartAnim;                 // SmartAnim实例
CGeniusCtrl g_GeniusCtrl;               // GeniusCtrl实例


HANDLE g_hReadCardThread = NULL;        // 读卡线程句柄
bool g_bStopReadCardThread = false;     // 是否终止读卡线程

bool g_bRechargeCard = false;           // 是否充值卡片

SYSTEMTIME g_DateSettingValue;          // 日期设置值
DateSettingItem g_DateSettingItem;      // 日期设置项

BYTE g_RFID[4];                         // 保存通过的RFID
DWORD g_LocalAddr = 0;                  // 本地网络地址
DWORD g_RemoteAddr = 0;                 // 远程网络地址

// 传感器状态
SENSOR_STATUS g_sensorStatus = { false, 
                                 { false, false, false, false }, 
                                 false, 
                                 false, 
                                 false, 
                                 50 };

// 此代码模块中包含的函数的前向声明:
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void AnimEndProcess (int animID, bool obverse);
bool RegisterGeniusCtrlProcess ();
void DestroyWindow ();
void EnterMainInterface ();
void EnterReadCardInterface ();
void ReadCardSucceed ();
void ReadCardFailed ();
void CALLBACK TimerProcess_Refresh (HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
void ReadCardStop ();
bool LoadRFID (PCTSTR lpCofFileName);
bool LoadServerAddr (PCTSTR lpCofFileName);
DWORD ThreadReadCard (LPVOID lpvoid);
void SetFan (bool open);
bool AddLog (PSTR lpszLogStr, PTSTR lpszLogFileName);
bool AddLog (PSTR lpszLogStr, PTSTR lpszLogFileName)
{
    HANDLE hFile = NULL;
    CHAR szLogStr[MAX_PATH];
    DWORD dwErr = GetLastError ();
    bool bRet = false;

    // 打开文件
	hFile = CreateFile (lpszLogFileName,
		                GENERIC_WRITE,
		                FILE_SHARE_WRITE,
		                NULL,
		                OPEN_ALWAYS,
		                FILE_ATTRIBUTE_NORMAL,
		                NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto End;

    // 获取系统时间
    SYSTEMTIME systemTime;
    GetLocalTime (&systemTime);

    // 组织Log
    if (sprintf (szLogStr, "%4d-%02d-%02d %02d:%02d:%02d %s %d\r\n", systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
                 systemTime.wHour, systemTime.wMinute, systemTime.wSecond, lpszLogStr, dwErr) == -1)
        goto End;

    // 写入Log
    DWORD numberOfBytesWritten;
    if (SetFilePointer (hFile, 0, NULL, FILE_END) == 0xFFFFFFFF)
        goto End;
    if (!WriteFile (hFile, szLogStr, strlen (szLogStr), &numberOfBytesWritten, NULL))
        goto End;

    bRet = true;
End:
    if (hFile != INVALID_HANDLE_VALUE && hFile != NULL) {
        CloseHandle (hFile);
        hFile = NULL;
    }

    return bRet;
}
//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
void Cs2Dlg::showbalance()
{
	CString t;
	int tem=(int)(blockBytes[0]);
	t.Format(_T("%d"),tem);
	GetDlgItem(IDC_STATIC2)->SetWindowText(t);

}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
    switch (message) 
    {
	case WM_ERASEBKGND:
		break;
    case WM_LBUTTONDOWN:
		g_GeniusCtrl.DoLButtonDown (g_InterfaceID, lParam);
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        if (message == WM_SEND_WINDOW_HANDLE) {
            // 保存设备服务器消息窗体句柄
            g_hDeviceServer = (HWND)wParam;

            // 设置设备服务器网站连接地址
            SendMessage (g_hDeviceServer, WM_SET_WEBSITE_ADDR, g_LocalAddr, g_RemoteAddr);
        }
        else if (message == WM_SET_LED) {
            for (int i = 0; i < SEND_DATA_TIMES; i++)
                g_CommCtrl.SetLED (PORT_NUMBER_COORDINATOR, (int)wParam, lParam ? true : false);
        }
        else if (message == WM_SET_MOTOR) {
            for (int i = 0; i < SEND_DATA_TIMES; i++)
                g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR, (MOTOR_STATUS)wParam);
        }
        else
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



bool GetByte (CHAR *szLine, int index, BYTE *pByte)
{
    BYTE retByte = 0;

    for (int i = index; i <= index + 1; i++) {
        BYTE mul = i == index ? 0x10 : 1;
        switch (szLine[i]) {
            case '0':
                break;
            case '1':
                retByte += 1 * mul;
                break;
            case '2':
                retByte += 2 * mul;
                break;
            case '3':
                retByte += 3 * mul;
                break;
            case '4':
                retByte += 4 * mul;
                break;
            case '5':
                retByte += 5 * mul;
                break;
            case '6':
                retByte += 6 * mul;
                break;
            case '7':
                retByte += 7 * mul;
                break;
            case '8':
                retByte += 8 * mul;
                break;
            case '9':
                retByte += 9 * mul;
                break;
            case 'A':
            case 'a':
                retByte += 10 * mul;
                break;
            case 'B':
            case 'b':
                retByte += 11 * mul;
                break;
            case 'C':
            case 'c':
                retByte += 12 * mul;
                break;
            case 'D':
            case 'd':
                retByte += 13 * mul;
                break;
            case 'E':
            case 'e':
                retByte += 14 * mul;
                break;
            case 'F':
            case 'f':
                retByte += 15 * mul;
                break;
            default:
                return false;
        }
    }

    *pByte = retByte;
    return true;
}

bool GetNumber (CHAR *szLine, int nMaxLen, BYTE *pCount, int *pNumber)
{
    int retNumber = 0;
    BYTE i = 0;
    bool bStop = false;

    while (!bStop && i < nMaxLen) {
        switch (szLine[i]) {
            case '0':
                retNumber *= 10;
                i++;
                break;
            case '1':
                retNumber = retNumber * 10 + 1;
                i++;
                break;
            case '2':
                retNumber = retNumber * 10 + 2;
                i++;
                break;
            case '3':
                retNumber = retNumber * 10 + 3;
                i++;
                break;
            case '4':
                retNumber = retNumber * 10 + 4;
                i++;
                break;
            case '5':
                retNumber = retNumber * 10 + 5;
                i++;
                break;
            case '6':
                retNumber = retNumber * 10 + 6;
                i++;
                break;
            case '7':
                retNumber = retNumber * 10 + 7;
                i++;
                break;
            case '8':
                retNumber = retNumber * 10 + 8;
                i++;
                break;
            case '9':
                retNumber = retNumber * 10 + 9;
                i++;
                break;
            default:
                bStop = true;
        }
    }

    *pNumber = retNumber;
    *pCount = i;
    return true;
}
    
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    读取RFID卡文件
// Parameters:	lpCofFileName   LPTSTR  [IN]    配置文件名称（完整路径）
// Return:      成功返回true，失败返回false。
//
bool LoadRFID (PCTSTR lpCofFileName)
{
    bool bRet = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BYTE* pFileContent = NULL;

	// 参数范围检查
	if (!lpCofFileName)
		goto End;

	// 打开文件
	hFile = CreateFile (lpCofFileName,
						GENERIC_READ,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto End;

	// 获取文件长度
	DWORD dwLen = GetFileSize (hFile, NULL);
	if (dwLen == 0xFFFFFFFF) 
		goto End;

	// 读取文件
	pFileContent = new BYTE[dwLen];
	DWORD dwNumberOfBytesRead;
	if (!ReadFile (hFile, pFileContent, dwLen, &dwNumberOfBytesRead, NULL))
		goto End;
	if (dwNumberOfBytesRead != dwLen) 
		goto End;

	// 获取文件的每一行
	CHAR szLine[MAX_LINE_LENGTH];
	BYTE* pNow = pFileContent;
	BYTE* pEnd = pNow + dwLen;
    PINTERFACE_STATUS pInterfaceStatus = NULL;
    PHOT_DOT pHotDot = NULL;
	while (pNow = GetOneLine (pNow, pEnd, szLine)) {
        BYTE byte;
        for (int i = 0; i < 4; i++) {
            if (!GetByte (szLine, i * 2, &byte))
                goto End;

            g_RFID[i] = byte;
        }
        break;
	}

	bRet = true;

End:
	DWORD dwErr = GetLastError ();

	if (pFileContent)
		delete[] pFileContent;

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle (hFile);

	SetLastError (dwErr);

	return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    读取服务器地址
// Parameters:	lpCofFileName   LPTSTR  [IN]    配置文件名称（完整路径）
// Return:      成功返回true，失败返回false。
//
bool LoadServerAddr (PCTSTR lpCofFileName)
{
    bool bRet = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BYTE* pFileContent = NULL;

	// 参数范围检查
	if (!lpCofFileName)
		goto End;

	// 打开文件
	hFile = CreateFile (lpCofFileName,
						GENERIC_READ,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto End;

	// 获取文件长度
	DWORD dwLen = GetFileSize (hFile, NULL);
	if (dwLen == 0xFFFFFFFF) 
		goto End;

	// 读取文件
	pFileContent = new BYTE[dwLen];
	DWORD dwNumberOfBytesRead;
	if (!ReadFile (hFile, pFileContent, dwLen, &dwNumberOfBytesRead, NULL))
		goto End;
	if (dwNumberOfBytesRead != dwLen) 
		goto End;

	// 获取文件的每一行
	CHAR szLine[MAX_LINE_LENGTH];
	BYTE* pNow = pFileContent;
	BYTE* pEnd = pNow + dwLen;
    PINTERFACE_STATUS pInterfaceStatus = NULL;
    PHOT_DOT pHotDot = NULL;
    int nLine = 0;
	while (pNow = GetOneLine (pNow, pEnd, szLine)) {
        DWORD addr = 0;
        int num;
        int count = 0;
        for (int i = 0; i < 4; i++) {
            BYTE cnt;
            if (!GetNumber (szLine + count, strlen (szLine) - count, &cnt, &num))
                goto End;
            count += cnt + 1;

            addr = addr * 0x100 + num;
        }
        switch (nLine) {
            case 0:
                g_LocalAddr = addr;
                break;
            case 1:
                g_RemoteAddr = addr;
                break;
            default:
                bRet = true;
                goto End;
        }
        nLine++;
	}

	bRet = true;

End:
	DWORD dwErr = GetLastError ();

	if (pFileContent)
		delete[] pFileContent;

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle (hFile);

	SetLastError (dwErr);

	return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    计算某月的天数
// Parameters:  year    int     [IN]    年份
//              month   int     [IN]    月份
// Return:      返回特定年特定月的天数
//
int DayNumberInMonth(int year,int month)
{
    int dayNumber;
    if (month == 2) {
        if (year % 4 == 0 && year % 100 != 0 || year % 400 == 0)
            dayNumber = 29;
        else 
            dayNumber = 28;
    }
    else if (month == 4 || month == 6 || month == 9 || month == 11)
        dayNumber = 30;
    else dayNumber = 31;

    return dayNumber;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    更新时间
// Parameters:  NULL
// Return:      void
// Remark:		该函数更新需要显示时间的相应界面资源，但并不刷新界面
//
void UpdateTime ()
{
	// 获取系统时间
	SYSTEMTIME systemTime;
	GetLocalTime (&systemTime);

	// 更新日期-年
	CHAR szDate[11] = "date_0.bmp";
	int year[4] = { systemTime.wYear / 1000, (systemTime.wYear % 1000) / 100, (systemTime.wYear % 100) / 10, systemTime.wYear % 10 };
	for (int i = 0; i < 4; i++) {
		szDate[5] = year[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 1 + i, szDate);
		g_SmartAnim.ReplacePic (1, 1 + i, szDate);
	}

	// 更新日期-月
	int month[2] = { systemTime.wMonth / 10, systemTime.wMonth % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = month[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 6 + i, szDate);
		g_SmartAnim.ReplacePic (1, 6 + i, szDate);
	}

	// 更新日期-日
	int day[2] = { systemTime.wDay / 10, systemTime.wDay % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = day[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 9 + i, szDate);
		g_SmartAnim.ReplacePic (1, 9 + i, szDate);
	}

	// 更新日期-星期
	CHAR szDayOfWeek[14];
	switch (systemTime.wDayOfWeek) {
		case 0:
			strcpy (szDayOfWeek, "sunday.bmp");
			break;
		case 1:
			strcpy (szDayOfWeek, "monday.bmp");
			break;
		case 2:
			strcpy (szDayOfWeek, "tuesday.bmp");
			break;
		case 3:
			strcpy (szDayOfWeek, "wednesday.bmp");
			break;
		case 4:
			strcpy (szDayOfWeek, "thursday.bmp");
			break;
		case 5:
			strcpy (szDayOfWeek, "friday.bmp");
			break;
		case 6:
			strcpy (szDayOfWeek, "saturday.bmp");
			break;
	}
	g_SmartAnim.ReplacePic (0, 0, szDayOfWeek);
	g_SmartAnim.ReplacePic (1, 0, szDayOfWeek);

	// 更新时间-时
	CHAR szTime[11] = "time_0.bmp";
	int hour[2] = { systemTime.wHour / 10, systemTime.wHour % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = hour[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 11 + i, szTime);
		g_SmartAnim.ReplacePic (1, 11 + i, szTime);
	}

	// 更新时间-分
	int minute[2] = { systemTime.wMinute / 10, systemTime.wMinute % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = minute[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 14 + i, szTime);
		g_SmartAnim.ReplacePic (1, 14 + i, szTime);
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    更新日期设置项目和值
// Parameters:  NULL
// Return:      更新成功返回true，否则返回false。
// Remark:		该函数更新需要显示日期设置项目和值的相应界面资源，但并不刷新界面
//
void UpdateDateSettingItemAndValue ()
{
    // 更新日期设置项
    for (DateSettingItem i = YEAR; i <= MINUTE; i = (DateSettingItem)(i + 1)) {
        if (i == g_DateSettingItem) {
            switch (i) {
                case YEAR:
                case MONTH:
                case DAY:
                    g_SmartAnim.ReplacePic (16, 19 + i, "arrow_bottom.bmp");
                    break;
                case HOUR:
                case MINUTE:
                    g_SmartAnim.ReplacePic (16, 19 + i, "arrow_top.bmp");
                    break;
            }
        }
        else
            g_SmartAnim.ReplacePic (16, 19 + i, "null");
    }

    // 更新日期-年
	CHAR szDate[11] = "date_0.bmp";
	int year[4] = { g_DateSettingValue.wYear / 1000, 
                    (g_DateSettingValue.wYear % 1000) / 100, 
                    (g_DateSettingValue.wYear % 100) / 10, 
                    g_DateSettingValue.wYear % 10 };
	for (int i = 0; i < 4; i++) {
		szDate[5] = year[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 1 + i, szDate);
		g_SmartAnim.ReplacePic (16, 1 + i, szDate);
	}

	// 更新日期-月
	int month[2] = { g_DateSettingValue.wMonth / 10, g_DateSettingValue.wMonth % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = month[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 6 + i, szDate);
		g_SmartAnim.ReplacePic (16, 6 + i, szDate);
	}

	// 更新日期-日
	int day[2] = { g_DateSettingValue.wDay / 10, g_DateSettingValue.wDay % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = day[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 9 + i, szDate);
		g_SmartAnim.ReplacePic (16, 9 + i, szDate);
	}

	// 更新日期-星期
	CHAR szDayOfWeek[14];
	switch (g_DateSettingValue.wDayOfWeek) {
		case 0:
			strcpy (szDayOfWeek, "sunday.bmp");
			break;
		case 1:
			strcpy (szDayOfWeek, "monday.bmp");
			break;
		case 2:
			strcpy (szDayOfWeek, "tuesday.bmp");
			break;
		case 3:
			strcpy (szDayOfWeek, "wednesday.bmp");
			break;
		case 4:
			strcpy (szDayOfWeek, "thursday.bmp");
			break;
		case 5:
			strcpy (szDayOfWeek, "friday.bmp");
			break;
		case 6:
			strcpy (szDayOfWeek, "saturday.bmp");
			break;
	}
	g_SmartAnim.ReplacePic (15, 0, szDayOfWeek);
	g_SmartAnim.ReplacePic (16, 0, szDayOfWeek);

	// 更新时间-时
	CHAR szTime[11] = "time_0.bmp";
	int hour[2] = { g_DateSettingValue.wHour / 10, g_DateSettingValue.wHour % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = hour[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 11 + i, szTime);
		g_SmartAnim.ReplacePic (16, 11 + i, szTime);
	}

	// 更新时间-分
	int minute[2] = { g_DateSettingValue.wMinute / 10, g_DateSettingValue.wMinute % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = minute[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 14 + i, szTime);
		g_SmartAnim.ReplacePic (16, 14 + i, szTime);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    退出程序
// Parameters:  NULL
// Return:      void
// Remark:		该函数为界面点击处理函数
//
GCPROCESS void DestroyWindow ()
{
	DestroyWindow (g_hWnd);
}

GCPROCESS void CardRecharge ()
{
    // 设置充值标志
    g_bRechargeCard = true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变日期设置
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变日期设置状态，并刷新界面显示
//
GCPROCESS void ChangeDateSettings ()
{
    // 改变设置项
    if (g_DateSettingItem == MINUTE)
        g_DateSettingItem = YEAR;
    else
        g_DateSettingItem = (DateSettingItem)(g_DateSettingItem + 1);

    // 更新设置项
    UpdateDateSettingItemAndValue ();

    // 刷新界面显示
    g_SmartAnim.ShowAnim (16, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    读卡成功
// Parameters:  NULL
// Return:      void
// Remark:      该函数模拟读卡成功，仅在无硬件条件下使用
//              该函数为界面点击处理函数
//
GCPROCESS void ReadCardSucceed ()
{
    // 终止读卡线程
    ReadCardStop ();
    // 播放声音
    PlaySound (SOUND_FAMILY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    读卡失败
// Parameters:  NULL
// Return:      void
// Remark:      该函数模拟读卡失败，仅在无硬件条件下使用
//              该函数为界面点击处理函数
//
GCPROCESS void ReadCardFailed ()
{
    // 终止读卡线程
    ReadCardStop ();

            // 播放声音
    PlaySound (SOUND_FAMILY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    点击日期设置加按钮
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void DateSettingAdd ()
{
    // 更新日期设置值
    int dayNumberInMonth;
    switch (g_DateSettingItem) {
        case YEAR:
            if (g_DateSettingValue.wYear == 9999)
                g_DateSettingValue.wYear = 0;
            else
                g_DateSettingValue.wYear++;

            dayNumberInMonth = DayNumberInMonth (g_DateSettingValue.wYear, g_DateSettingValue.wMonth);
            if (g_DateSettingValue.wDay > dayNumberInMonth)
                g_DateSettingValue.wDay = dayNumberInMonth;
            break;
        case MONTH:
            if (g_DateSettingValue.wMonth == 12)
                g_DateSettingValue.wMonth = 1;
            else
                g_DateSettingValue.wMonth++;

            dayNumberInMonth = DayNumberInMonth (g_DateSettingValue.wYear, g_DateSettingValue.wMonth);
            if (g_DateSettingValue.wDay > dayNumberInMonth)
                g_DateSettingValue.wDay = dayNumberInMonth;
            break;
        case DAY:
            if (g_DateSettingValue.wDay == DayNumberInMonth (g_DateSettingValue.wYear, g_DateSettingValue.wMonth))
                g_DateSettingValue.wDay = 1;
            else
                g_DateSettingValue.wDay++;
            break;
        case HOUR:
            if (g_DateSettingValue.wHour == 23)
                g_DateSettingValue.wHour = 0;
            else
                g_DateSettingValue.wHour++;
            break;
        case MINUTE:
            if (g_DateSettingValue.wMinute == 59)
                g_DateSettingValue.wMinute = 0;
            else
                g_DateSettingValue.wMinute++;
            break;
    }

    // 设置时间
    SetLocalTime (&g_DateSettingValue);
    GetLocalTime (&g_DateSettingValue);

    // 更新设置项
    UpdateDateSettingItemAndValue ();

    // 刷新界面显示
    g_SmartAnim.ShowAnim (16, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    点击日期设置减按钮
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void DateSettingMinus ()
{
    // 更新日期设置值
    int dayNumberInMonth;
    switch (g_DateSettingItem) {
        case YEAR:
            if (g_DateSettingValue.wYear == 0)
                g_DateSettingValue.wYear = 9999;
            else
                g_DateSettingValue.wYear--;

            dayNumberInMonth = DayNumberInMonth (g_DateSettingValue.wYear, g_DateSettingValue.wMonth);
            if (g_DateSettingValue.wDay > dayNumberInMonth)
                g_DateSettingValue.wDay = dayNumberInMonth;
            break;
        case MONTH:
            if (g_DateSettingValue.wMonth == 1)
                g_DateSettingValue.wMonth = 12;
            else
                g_DateSettingValue.wMonth--;

            dayNumberInMonth = DayNumberInMonth (g_DateSettingValue.wYear, g_DateSettingValue.wMonth);
            if (g_DateSettingValue.wDay > dayNumberInMonth)
                g_DateSettingValue.wDay = dayNumberInMonth;
            break;
        case DAY:
            if (g_DateSettingValue.wDay == 1)
                g_DateSettingValue.wDay = DayNumberInMonth (g_DateSettingValue.wYear, g_DateSettingValue.wMonth);
            else
                g_DateSettingValue.wDay--;
            break;
        case HOUR:
            if (g_DateSettingValue.wHour == 0)
                g_DateSettingValue.wHour = 23;
            else
                g_DateSettingValue.wHour--;
            break;
        case MINUTE:
            if (g_DateSettingValue.wMinute == 0)
                g_DateSettingValue.wMinute = 59;
            else
                g_DateSettingValue.wMinute--;
            break;
    }

    // 设置时间
    SetLocalTime (&g_DateSettingValue);
    GetLocalTime (&g_DateSettingValue);

    // 更新设置项
    UpdateDateSettingItemAndValue ();

    // 刷新界面显示
    g_SmartAnim.ShowAnim (16, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    进入主界面
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void EnterMainInterface ()
{
    // 区分进入该界面前的界面
	switch (g_InterfaceID) {
		case -1:					            // 程序启动
            // 刷新主界面信息
            UpdateTime ();


			// 显示主界面进入动画
			g_SmartAnim.ShowAnim (0, true);
			break;
		case 1:						            // 读卡界面
            // 终止读卡线程
            ReadCardStop ();

			// 退出读卡界面，准备进入主界面
			g_InterfaceID = -1;
            g_GoalInterfaceID = 0;
			g_SmartAnim.ShowAnim (2, false);
            break;
        case 2:                                 // 家庭控制界面
            // 关闭家庭控制界面控制项

            g_sensorStatus.bAirConditioner = false;
            for (int i = 1; i <= 4; i++) {
    
                g_sensorStatus.bLight[i - 1] = false;
            }

            // 显示家庭控制界面退出动画，准备进入主界面
            g_InterfaceID = -1;
            g_GoalInterfaceID = 0;
            g_SmartAnim.ShowAnim (5, false);
            break;
        case 3:                                 // 智慧农业界面
            // 关闭智慧农业界面控制项
        

            g_sensorStatus.bSpray = false;
            g_sensorStatus.bFan = false;

            // 退出智慧农业界面，准备进入主界面
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (7, false);
            break;
        case 4:                                 // 一卡通界面
            // 终止读卡线程
            ReadCardStop ();

            // 退出一卡通界面，准备进入主界面
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (9, false);
            break;
        case 5:                                 // 食品溯源界面
            // 终止读卡线程
            ReadCardStop ();

            // 退出食品溯源界面，准备进入主界面
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (12, false);
            break;
        case 6:                                 // 设置界面
            // 退出设置界面，准备进入主界面
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (15, false);   
            break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    进入读卡界面
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void EnterReadCardInterface ()
{
    // 区分进入该界面前的界面
    switch (g_InterfaceID) {
        case 0:                                 // 主界面
			// 退出主界面，准备进入读卡界面
			g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (0, false);
            break;
        case 2:                                 // 家庭控制界面
            // 退出家庭控制界面，准备进入读卡界面
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (5, false);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    进入智慧农业界面
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void EnterFarmingInterface ()
{
    // 退出主界面，准备进入智慧农业界面
	g_InterfaceID = -1;
    g_GoalInterfaceID = 3;
    g_SmartAnim.ShowAnim (0, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    进入一卡通界面
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void EnterSubwayInterface ()
{
    // 退出主界面，准备进入一卡通界面
	g_InterfaceID = -1;
    g_GoalInterfaceID = 4;
    g_SmartAnim.ShowAnim (0, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    进入食品溯源界面
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void EnterFoodInterface ()
{
    // 退出主界面，准备进入食品溯源界面
	g_InterfaceID = -1;
    g_GoalInterfaceID = 5;
    g_SmartAnim.ShowAnim (0, false); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    进入设置界面
// Parameters:  NULL
// Return:      void
// Remark:      该函数为界面点击处理函数
//
GCPROCESS void EnterSettingsInterface ()
{
    // 退出主界面，准备进入设置界面
	g_InterfaceID = -1;
    g_GoalInterfaceID = 6;
    g_SmartAnim.ShowAnim (0, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    注册鼠标按下处理函数
// Parameters:  NULL
// Return:      成功返回true，失败返回false。
//
bool RegisterGeniusCtrlProcess ()
{
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("DestroyWindow", DestroyWindow)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("CardRecharge", CardRecharge)) return false;

    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeDateSettings", ChangeDateSettings)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ReadCardSucceed", ReadCardSucceed)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ReadCardFailed", ReadCardFailed)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("DateSettingAdd", DateSettingAdd)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("DateSettingMinus", DateSettingMinus)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("EnterMainInterface", EnterMainInterface)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("EnterReadCardInterface", EnterReadCardInterface)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("EnterFarmingInterface", EnterFarmingInterface)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("EnterSubwayInterface", EnterSubwayInterface)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("EnterFoodInterface", EnterFoodInterface)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("EnterSettingsInterface", EnterSettingsInterface)) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    开始RFID读卡
// Parameters:  NULL
// Return:      成功启动读卡线程返回true，否则返回false。
// Remark:		该函数启动RFID读卡线程
//
bool ReadCardStart ()
{
    // 停止已启动的读卡线程
    ReadCardStop ();
    // 启动读卡线程
    g_bStopReadCardThread = false;
    if (!(g_hReadCardThread = CreateThread (NULL, 0, ThreadReadCard, NULL, 0, NULL)))
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    终止RFID读卡
// Parameters:  NULL
// Return:      NULL
// Remark:		该函数终止RFID读卡线程，并等待线程结束
//
void ReadCardStop ()
{
    // 终止读卡线程
    g_bStopReadCardThread = true;

    // 等待读卡线程结束
    WaitForSingleObject (g_hReadCardThread, INFINITE);

    // 释放线程句柄
    if (g_hReadCardThread) {
        CloseHandle (g_hReadCardThread);
        g_hReadCardThread = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    RFID读卡线程
// Parameters:  lpvoid  LPVOID  线程参数指针
// Return:      返回线程退出码
// Remark:		该线程将在调用ReadCardStop函数或处理完一张卡片后退出
//              函数在读取到一张卡片的序列号后，会按照当前界面的要求进行相应的操作。
//
bool bSuccess;
DWORD ThreadReadCard (LPVOID lpvoid)
{
    BYTE cardSize;
    BYTE pKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    // RFID开射频
    bool rfOpen = false;
    for (int i = 0; i < 3; i++)
    {
        if (m104bpc_RFCtrl (PORT_NUMBER_M104BPC, true)) {
            rfOpen = true;
            continue;
        }
        else
            AddLog ("Open RF Error!", LOG_FILE);
    }
    if (!rfOpen) {
        AddLog ("Can not Open RF!", LOG_FILE);
        ASSERTMSG (TEXT ("射频模块启动失败！请重新启动设备。"), FALSE);
    }

    while (!g_bStopReadCardThread) {
        // 激活卡片
        CARD_MODEL cardModel;
        if (!m104bpc_ActiveCard (PORT_NUMBER_M104BPC, true, &cardModel))
            continue;

        // 防冲突（获取卡片序列号）
        BYTE cardNo[4];
        if (!m104bpc_Anticollision (PORT_NUMBER_M104BPC, cardNo))
            continue;
        switch (g_InterfaceID) {
            case 1:                         // 智慧家庭刷卡界面
                // 判断是否为已登记卡号
                if (cardNo[0] == g_RFID[0] &&
                    cardNo[1] == g_RFID[1] &&
                    cardNo[2] == g_RFID[2] &&
                    cardNo[3] == g_RFID[3]) {
                    // 播放声音
                    PlaySound (SOUND_FAMILY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

	                // 显示读卡成功动画
				
	                g_InterfaceID = -1;
                }
                else {
                    // 播放声音
                    PlaySound (SOUND_FAMILY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

                    // 显示读卡失败动画
                    g_InterfaceID = -1;
                    g_SmartAnim.ShowAnim (4, true);
                }
                break;
            case 4:                         // 一卡通界面
                bSuccess = false;

                // 非正确的卡
                if (cardNo[0] != g_RFID[0] ||
                    cardNo[1] != g_RFID[1] ||
                    cardNo[2] != g_RFID[2] ||
                    cardNo[3] != g_RFID[3])
                        goto SubWayEnd;

                // 选卡
                if (!m104bpc_SelectCard (PORT_NUMBER_M104BPC, cardNo, &cardSize))
                    goto SubWayEnd;

                // 验证密码
                if (!m104bpc_CheckKey (PORT_NUMBER_M104BPC, KEY_A, 2, pKey))
                    goto SubWayEnd;
                    
                // 读卡
                if (!m104bpc_ReadBlock (PORT_NUMBER_M104BPC, 2, blockBytes)) 
                    goto SubWayEnd;
                
                if (g_bRechargeCard) {  // 充值
                    // 写卡
					printf("g recharge %d\n",g_bRechargeCard);
                    blockBytes[0] = 100;
					if (!m104bpc_WriteBlock (PORT_NUMBER_M104BPC, 2, blockBytes))
                        goto SubWayEnd;

   
                    
        
                    // 充值完成
                    g_bRechargeCard = false;
                    bSuccess = true;


                    // 播放声音
                    PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);
					 g_InterfaceID = -1;
                }
                else {                  // 读卡
                    if (blockBytes[0] >= 1) {   // 余额足够
                        // 扣除金额
                        blockBytes[0] -= 1;
				
						//showbalance(blockBytes[0]);
                        if (!m104bpc_WriteBlock (PORT_NUMBER_M104BPC, 2, blockBytes))
                            goto SubWayEnd;
                        
                        // 更新界面
                        g_SmartAnim.ReplacePic (9, 2, "date_2.bmp");
                        g_SmartAnim.ReplacePic (10, 2, "date_2.bmp");
                        g_SmartAnim.ReplacePic (11, 2, "date_2.bmp");

                        int money[3];
                        money[0] = blockBytes[0] % 10;
                        money[1] = (blockBytes[0] / 10) % 10;
                        money[2] = (blockBytes[0] / 100) % 10;
                        CHAR szTemp[11] = "date_0.bmp";
                        if (blockBytes[0] < 10) {
                            // 个位
                            szTemp[5] = 0x30 + money[0];
                            g_SmartAnim.ReplacePic (9, 4, szTemp);
                            g_SmartAnim.ReplacePic (10, 4, szTemp);
                            g_SmartAnim.ReplacePic (11, 4, szTemp);

                            // 留空
                            g_SmartAnim.ReplacePic (9, 5, "null");
                            g_SmartAnim.ReplacePic (10, 5, "null");
                            g_SmartAnim.ReplacePic (11, 5, "null");
                            g_SmartAnim.ReplacePic (9, 6, "null");
                            g_SmartAnim.ReplacePic (10, 6, "null");
                            g_SmartAnim.ReplacePic (11, 6, "null");
                        }
                        else if (blockBytes[0] < 100) {
                            // 十位
                            szTemp[5] = 0x30 + money[1];
                            g_SmartAnim.ReplacePic (9, 4, szTemp);
                            g_SmartAnim.ReplacePic (10, 4, szTemp);
                            g_SmartAnim.ReplacePic (11, 4, szTemp);

                            // 个位
                            szTemp[5] = 0x30 + money[0];
                            g_SmartAnim.ReplacePic (9, 5, szTemp);
                            g_SmartAnim.ReplacePic (10, 5, szTemp);
                            g_SmartAnim.ReplacePic (11, 5, szTemp);

                            // 留空
                            g_SmartAnim.ReplacePic (9, 6, "null");
                            g_SmartAnim.ReplacePic (10, 6, "null");
                            g_SmartAnim.ReplacePic (11, 6, "null");
                        }
                        else {
                            // 百位
                            szTemp[5] = 0x30 + money[2];
                            g_SmartAnim.ReplacePic (9, 4, szTemp);
                            g_SmartAnim.ReplacePic (10, 4, szTemp);
                            g_SmartAnim.ReplacePic (11, 4, szTemp);

                            // 十位
                            szTemp[5] = 0x30 + money[1];
                            g_SmartAnim.ReplacePic (9, 5, szTemp);
                            g_SmartAnim.ReplacePic (10, 5, szTemp);
                            g_SmartAnim.ReplacePic (11, 5, szTemp);

                            // 个位
                            szTemp[5] = 0x30 + money[0];
                            g_SmartAnim.ReplacePic (9, 6, szTemp);
                            g_SmartAnim.ReplacePic (10, 6, szTemp);
                            g_SmartAnim.ReplacePic (11, 6, szTemp);
                        }

                        // 读卡完成
                        bSuccess = true;

                        // 播放声音
                        PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

                        // 显示读卡成功动画
                        g_InterfaceID = -1;
                        g_SmartAnim.ShowAnim (10, true);
                    }
                    else {                      // 余额不足
                        // 更新界面
                        g_SmartAnim.ReplacePic (9, 2, "date_0.bmp");
                        g_SmartAnim.ReplacePic (11, 2, "date_0.bmp");

                        CHAR szTemp[11] = "date_0.bmp";

                        // 个位
                        szTemp[5] = 0x30 + blockBytes[0];
                        g_SmartAnim.ReplacePic (9, 4, szTemp);
                        g_SmartAnim.ReplacePic (11, 4, szTemp);

                        // 留空
                        g_SmartAnim.ReplacePic (9, 5, "null");
                        g_SmartAnim.ReplacePic (11, 5, "null");
                        g_SmartAnim.ReplacePic (9, 6, "null");
                        g_SmartAnim.ReplacePic (11, 6, "null");
                    }
                }
SubWayEnd:
                if (!bSuccess) {
                    // 播放声音
                    PlaySound (SOUND_SUBWAY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

                    // 显示读卡失败动画
                    g_InterfaceID = -1;
	                g_SmartAnim.ShowAnim (11, true);
                }

                break;
        }

        // 处理完一张卡片，退出线程
        break;
    }

    // RFID关射频
    m104bpc_RFCtrl (PORT_NUMBER_M104BPC, false);

    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    已进入主界面
// Parameters:  NULL
// Return:      void
//
void MainInterfaceEntered ()
{
	// 设置界面ID
    g_InterfaceID = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    已进入读卡界面
// Parameters:  NULL
// Return:      void
//
void ReadCardInterfaceEntered ()
{
	// 设置界面ID
	g_InterfaceID = 1;

    // 开始循环读卡
    ReadCardStart ();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    已进入家庭控制界面
// Parameters:  NULL
// Return:      void
//
void HomeControlInterfaceEntered ()
{
    // 设置界面ID
    g_InterfaceID = 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    已进入智慧农业界面
// Parameters:  NULL
// Return:      void
//
void FarmingInterfaceEntered ()
{
    // 设置界面ID
    g_InterfaceID = 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    已进入一卡通界面
// Parameters:  NULL
// Return:      void
//
void SubwayInterfaceEntered ()
{
    // 设置界面ID
    g_InterfaceID = 4;

    // 开始循环读卡
    ReadCardStart ();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    已进入食品溯源界面
// Parameters:  NULL
// Return:      void
//
void FoodInterfaceEntered ()
{
    // 设置界面ID
    g_InterfaceID = 5;

    // 开始循环读卡
    ReadCardStart ();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    已进入食品溯源界面
// Parameters:  NULL
// Return:      void
//
void SettingsInterfaceEntered ()
{
    // 设置界面ID
    g_InterfaceID = 6;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    数据刷新定时器处理函数
// Parameters:  hWnd    HWND    与定时器链接的窗体句柄
//              uMsg    UINT    WM_TIMER消息
//              idEvent UINT    定时器事件序号
//              dwTimer DOWRD   操作系统启动至今的毫秒数
// Return:      void
//
void CALLBACK TimerProcess_Refresh (HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    // 上传传感器数据到设备服务器
 

    switch (g_InterfaceID) {
        case 0:         // 主界面
  
            g_SmartAnim.ShowAnim (1, true);
            break;
        case 2:         // 家庭控制界面
  
            g_SmartAnim.ShowAnim (6, true);
            break;
        case 3:         // 智慧农业界面
   
            g_SmartAnim.ShowAnim (8, true);
            break;
    }
}



void Cs2Dlg::OnBnClickedButton8()
{
	// TODO: 在此添加控件通知处理程序代码
	g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR);
	g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR);
	LoadServerAddr (SERVER_ADDR_FILE);
	m104bpc_OpenPort (PORT_NUMBER_M104BPC, BAUND_RATE_M104BPC);
	    // 设置RFID模块TypeA模式
    bool rfOpen = false;
    for (int i = 0; i < 3; i++)
    {
        if (m104bpc_SetTypeA (PORT_NUMBER_M104BPC)) {
            rfOpen = true;
            break;
        }
        else
            AddLog ("Set TypeA Error!", LOG_FILE);
    }
    if (!rfOpen) {
        AddLog ("Can not Set TypeA!", LOG_FILE);
        ASSERTMSG (TEXT ("射频模块设置模式失败！请重新启动设备。"), FALSE);

    }
	 // 注册与设备服务器程序通信的消息
    WM_SEND_WINDOW_HANDLE = RegisterWindowMessage (TEXT ("WM_SEND_WINDOW_HANDLE"));
    WM_SET_WEBSITE_ADDR = RegisterWindowMessage (TEXT ("WM_SET_WEBSITE_ADDR"));
    WM_SET_LED = RegisterWindowMessage (TEXT ("WM_SET_LED"));
    WM_SET_MOTOR = RegisterWindowMessage (TEXT ("WM_SET_MOTOR"));
    WM_UPDATE_TEMPERATURE = RegisterWindowMessage (TEXT ("WM_UPDATE_TEMPERATURE"));
    WM_UPDATE_HUMIDITY = RegisterWindowMessage (TEXT ("WM_UPDATE_HUMIDITY"));
    WM_UPDATE_DAY_NIGHT = RegisterWindowMessage (TEXT ("WM_UPDATE_DAY_NIGHT"));
    WM_CLOSE_SERVER = RegisterWindowMessage (TEXT ("WM_CLOSE_SERVER"));

    // 启动设备服务器
	PROCESS_INFORMATION procInfo;
	procInfo.hProcess = NULL;
	TCHAR commLine[16];
    swprintf (commLine, TEXT ("%d"), g_hWnd);
	CreateProcess (SERVER_PATH, commLine, NULL, NULL, FALSE, 0, NULL, NULL, NULL, &procInfo);
	g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR);
	RegisterGeniusCtrlProcess(); 
	LoadRFID (RFID_COF_FILE);
	g_InterfaceID=4;
	ReadCardStart ();

}

void Cs2Dlg::OnBnClickedButton9()
{
	// TODO: 在此添加控件通知处理程序代码
	CardRecharge ();
	g_InterfaceID=4;
	ReadCardStart ();
	showbalance();
	printf("%d\n",blockBytes[0]);
	PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);
}


void Cs2Dlg::OnBnClickedButton11()
{
	// TODO: 在此添加控件通知处理程序代码
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
	CBitmap bitmap;
	HBITMAP hbmp;
	bitmap.LoadBitmapW(IDB_LightLevel1);
	hbmp = (HBITMAP)bitmap.GetSafeHandle();
	/*m_ctrlpicture.SetBitmap(hbmp);
	static int testval = 0;
	if(testval == 0)
	{
		testval = 1;
		m_ctrlpicture.SetBitmap((HBITMAP)m_bitmap1);
	}
	else 
	{
		testval = 0;
		m_ctrlpicture.SetBitmap((HBITMAP)m_bitmap2);
	}
	m_ctrlpicture.InvalidateRect(NULL);*/
	LED1 = true;
	LED2 = false;
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
	Sleep(200);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
	Sleep(200);
}


void Cs2Dlg::OnBnClickedButton12()
{
	aut = 0;	// TODO: 在此添加控件通知处理程序代码
	MOTOR_STATUS status = STOP;
	KillTimer(1);
	KillTimer(2);
	KillTimer(3);
	Sleep(200);
	g_CommCtrl.SetMotor(PORT_NUMBER_COORDINATOR,status);
	Sleep(200);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,3,false);
	Sleep(200);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,4,false);
	Sleep(200);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,false);
	Sleep(200);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,false);
	Sleep(200);
	// TODO: 在此添加控件通知处理程序代码
}


void Cs2Dlg::OnBnClickedButton13()
{
	aut = 1;
	Sleep(200);
	g_CommCtrl.GetHumidity(PORT_NUMBER_COORDINATOR,&humid);
	Sleep(200);
	g_CommCtrl.GetTemperature(PORT_NUMBER_COORDINATOR,&temp);
	Sleep(200);
	showtem();
	showhum();
	// TODO: 在此添加控件通知处理程序代码
	SetTimer(1,1000,NULL);// TODO: 在此添加额外的初始化代码
	SetTimer(2,1000,NULL);
	SetTimer(3,1000,NULL);
}

void Cs2Dlg::OnBnClickedButton7()
{
	g_InterfaceID=1;
	showbalance();
	// TODO: 在此添加控件通知处理程序代码
}

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
		case 3:
			lig();
			break;

	}// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnTimer(nIDEvent);
}

void Cs2Dlg::OnBnClickedButton4()
{
	LED1=LED2=true;
	
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
	Sleep(500);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
	Sleep(500);
	// TODO: 在此添加控件通知处理程序代码
}

void Cs2Dlg::OnBnClickedButton5()
{
	LED1=LED2=false;
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
	Sleep(500);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
	Sleep(500);
	// TODO: 在此添加控件通知处理程序代码
}



void Cs2Dlg::OnBnClickedButton15()
{
	if (aut==0)
	{
		MOTOR_STATUS status ;
		status = FORWARD;
		g_CommCtrl.SetMotor(PORT_NUMBER_COORDINATOR,status);
	}
	else
	{
		CString s ;
		s = "错误！请先关闭自动控制！";
		GetDlgItem(IDC_STATIC30)->SetWindowText(s);
		//GetDlgItem(IDC_STATIC30)->s
		//MessageBox(_T("自动控制冲突！系统自动关闭自动控制！"));
		//Dlg2
	}
	// TODO: 在此添加控件通知处理程序代码
}


void Cs2Dlg::OnBnClickedButton16()
{
	if (aut==0)
	{
		motor = !motor;
		MOTOR_STATUS status ;
		status = STOP;
		g_CommCtrl.SetMotor(PORT_NUMBER_COORDINATOR,status);
	}
	else
	{
		//CString s ;
		//s = "错误！请先关闭自动控制！";
		//GetDlgItem(IDC_STATIC30)->SetWindowText(s);
		//MessageBox(_T("自动控制冲突！系统自动关闭自动控制！"));
		//Dlg2
	}
	// TODO: 在此添加控件通知处理程序代码
}
