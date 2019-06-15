#include "stdafx.h"
#include "Gemini2.h"
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
CCommCtrl g_CommCtrl;                   // CommCtrl实例

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

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;
    
	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}
        
    // 隐藏任务栏
	HWND hwndTask = FindWindow (L"HHTaskBar", NULL);
	ShowWindow (hwndTask, SW_HIDE);

	// 隐藏光标
	HCURSOR hPrevCursor = SetCursor (NULL);

	// 初始化SmartAnim
	if (!g_SmartAnim.Initialize (g_hWnd, ANIM_RES_FILENAME, PIC_RES_FILENAME, PIC_DIR_PATH))
		return 1;

    // 初始化GeniusCtrl
    if (!g_GeniusCtrl.Initialize (CTRL_RES_FILENAME))
        return 2;

	// 注册界面处理回调函数
	if (!RegisterGeniusCtrlProcess ())
        return 3;

    // 注册动画结束回调函数
    g_SmartAnim.RegisterAnimEndProcess (AnimEndProcess);

    // 打开传感器协调器端口
    if (!g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR))
        return 4;

    // 打开M104BPC端口
    if (!m104bpc_OpenPort (PORT_NUMBER_M104BPC, BAUND_RATE_M104BPC))
        return 5;

    // 读取RFID卡号
    if (!LoadRFID (RFID_COF_FILE))
        return 6;

    // 读取网络通信地址
    if (!LoadServerAddr (SERVER_ADDR_FILE))
        return 7;

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
        return 8;
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

    // 启动数据刷新定时器
    SetTimer (g_hWnd, TIMER_ID_REFRESH, 1000, (TIMERPROC)TimerProcess_Refresh);

    // 显示主界面
	EnterMainInterface ();

    //AddLog ("Application Started!", LOG_FILE);
    
	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    // 停止数据刷新定时器
    KillTimer (g_hWnd, TIMER_ID_REFRESH);

	// 停止设备服务器
    if (procInfo.hProcess) {
        SendMessage (g_hDeviceServer, WM_CLOSE_SERVER, 0, 0);
        TerminateProcess (procInfo.hProcess, 0);
    }

    // 关闭传感器协调器端口
    g_CommCtrl.CloseSerialPort (PORT_NUMBER_COORDINATOR);

    // 关闭M104BPC端口
    m104bpc_ClosePort (PORT_NUMBER_M104BPC);

    // 释放GeniusCtrl
    g_GeniusCtrl.Uninitialize ();

	// 释放SmartAnim
	g_SmartAnim.Uninitialize ();

    // 启动任务管理器
    if (!FindWindow (L"DesktopExplorerWindow", NULL))
        CreateProcess (TEXT ("explorer.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);

	// 恢复光标
	SetCursor (hPrevCursor);

    // 显示任务栏
	ShowWindow (hwndTask, SW_SHOW);

    //AddLog ("Application Stopped!", LOG_FILE);

	return (int) msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GEMINI2));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    TCHAR szTitle[MAX_LOADSTRING];		    // 标题栏文本
    TCHAR szWindowClass[MAX_LOADSTRING];	// 主窗口类名

    g_hInst = hInstance;                    // 将实例句柄存储在全局变量中

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_GEMINI2, szWindowClass, MAX_LOADSTRING);

    //如果它已经在运行，则将焦点置于窗口上，然后退出
    g_hWnd = FindWindow(szWindowClass, szTitle);	
    if (g_hWnd) 
    {
        // 将焦点置于最前面的子窗口
        // “| 0x00000001”用于将所有附属窗口置于前台并
        // 激活这些窗口。
        SetForegroundWindow((HWND)((ULONG) g_hWnd | 0x00000001));
        return 0;
    }

    if (!MyRegisterClass(hInstance, szWindowClass))
    	return FALSE;

    // 创建全屏窗口
    g_hWnd = CreateWindowEx(WS_EX_TOPMOST, 
                            szWindowClass, 
                            szTitle, 
                            WS_VISIBLE | WS_POPUP,
                            0, 0, GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN), 
                            NULL, 
                            NULL, 
                            hInstance, 
                            NULL);

    if (!g_hWnd)
    {
        return FALSE;
    }

    SetFocus (g_hWnd);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return TRUE;
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
// Function:    更新温度
// Parameters:  NULL
// Return:      更新成功返回true，否则返回false。
// Remark:		该函数更新需要显示温度的相应界面资源，但并不刷新界面
//
bool UpdateTemperature ()
{
    bool bRet;

    // 获取温度数值
    int temperature;
    bRet = g_CommCtrl.GetTemperature (PORT_NUMBER_COORDINATOR, &temperature);

    // 更新温度（屏蔽0摄氏度）
    if (bRet && temperature != 0) {
        CHAR szTemp[11] = "temp_0.bmp";
        bool isMinus = temperature < 0 ? true : false;
        int temp[2] = { (temperature % 100) / 10, temperature % 10 };
        if (abs (temperature) <= 9) {
            // 负号
            g_SmartAnim.ReplacePic (0, 16, "null");
		    g_SmartAnim.ReplacePic (1, 16, "null");
            g_SmartAnim.ReplacePic (5, 7, "null");
            g_SmartAnim.ReplacePic (6, 7, "null");
            g_SmartAnim.ReplacePic (7, 13, "null");
            g_SmartAnim.ReplacePic (8, 13, "null");
		    if (!isMinus) {
                // 十位
                g_SmartAnim.ReplacePic (0, 17, "null");
			    g_SmartAnim.ReplacePic (1, 17, "null");
                g_SmartAnim.ReplacePic (5, 8, "null");
                g_SmartAnim.ReplacePic (6, 8, "null");
                g_SmartAnim.ReplacePic (7, 14, "null");
                g_SmartAnim.ReplacePic (8, 14, "null");
		    }
		    else {
                // 十位
                g_SmartAnim.ReplacePic (0, 17, "temp_minus.bmp");
			    g_SmartAnim.ReplacePic (1, 17, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (5, 8, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (6, 8, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (7, 14, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (8, 14, "temp_minus.bmp");
		    }
            szTemp[5] = temp[1] + 0x30;
            // 个位
            g_SmartAnim.ReplacePic (0, 18, szTemp);
		    g_SmartAnim.ReplacePic (1, 18, szTemp);
            g_SmartAnim.ReplacePic (5, 9, szTemp);
            g_SmartAnim.ReplacePic (6, 9, szTemp);
            g_SmartAnim.ReplacePic (7, 15, szTemp);
            g_SmartAnim.ReplacePic (8, 15, szTemp);
        }
        else {
		    if (!isMinus) {
                // 负号
                g_SmartAnim.ReplacePic (0, 16, "null");
			    g_SmartAnim.ReplacePic (1, 16, "null");
                g_SmartAnim.ReplacePic (5, 7, "null");
                g_SmartAnim.ReplacePic (6, 7, "null");
                g_SmartAnim.ReplacePic (7, 13, "null");
                g_SmartAnim.ReplacePic (8, 13, "null");
		    }
		    else {
                // 负号
                g_SmartAnim.ReplacePic (0, 16, "temp_minus.bmp");
			    g_SmartAnim.ReplacePic (1, 16, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (5, 7, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (6, 7, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (7, 13, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (8, 13, "temp_minus.bmp");
		    }
            // 十位
            szTemp[5] = temp[0] + 0x30;
            g_SmartAnim.ReplacePic (0, 17, szTemp);
		    g_SmartAnim.ReplacePic (1, 17, szTemp);
            g_SmartAnim.ReplacePic (5, 8, szTemp);
            g_SmartAnim.ReplacePic (6, 8, szTemp);
            g_SmartAnim.ReplacePic (7, 14, szTemp);
            g_SmartAnim.ReplacePic (8, 14, szTemp);


            // 个位
            szTemp[5] = temp[1] + 0x30;
            g_SmartAnim.ReplacePic (0, 18, szTemp);
		    g_SmartAnim.ReplacePic (1, 18, szTemp);
            g_SmartAnim.ReplacePic (5, 9, szTemp);
            g_SmartAnim.ReplacePic (6, 9, szTemp);
            g_SmartAnim.ReplacePic (7, 15, szTemp);
            g_SmartAnim.ReplacePic (8, 15, szTemp);
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    上传温度数值到设备服务器
// Parameters:  NULL
// Return:      VOID
//
void UploadTemperature ()
{
    int temperature;
    if (g_CommCtrl.GetTemperature (PORT_NUMBER_COORDINATOR, &temperature))
        SendMessage (g_hDeviceServer, WM_UPDATE_TEMPERATURE, temperature, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    更新湿度
// Parameters:  NULL
// Return:      更新成功返回true，否则返回false。
// Remark:		该函数更新需要显示湿度的相应界面资源，但并不刷新界面
//
bool UpdateHumidity ()
{
    bool bRet;

    // 获取湿度数值
    int humidity;
    bRet = g_CommCtrl.GetHumidity (PORT_NUMBER_COORDINATOR, &humidity);

    // 更新湿度
    if (bRet) {
        CHAR szTemp[11] = "temp_0.bmp";
        int temp[2] = { (humidity % 100) / 10, humidity % 10 };
        if (humidity < 10) {
            // 十位
            g_SmartAnim.ReplacePic (7, 2, "null");
            g_SmartAnim.ReplacePic (8, 2, "null");

            // 个位
            szTemp[5] = temp[1] + 0x30;
            g_SmartAnim.ReplacePic (7, 3, szTemp);
            g_SmartAnim.ReplacePic (8, 3, szTemp);
        }
        else {
            // 十位
            szTemp[5] = temp[0] + 0x30;
            g_SmartAnim.ReplacePic (7, 2, szTemp);
            g_SmartAnim.ReplacePic (8, 2, szTemp);

            // 个位
            szTemp[5] = temp[1] + 0x30;
            g_SmartAnim.ReplacePic (7, 3, szTemp);
            g_SmartAnim.ReplacePic (8, 3, szTemp);
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    上传湿度数值到设备服务器
// Parameters:  NULL
// Return:      VOID
//
void UploadHumidity ()
{
    int humidity;
    if (g_CommCtrl.GetHumidity (PORT_NUMBER_COORDINATOR, &humidity))
        SendMessage (g_hDeviceServer, WM_UPDATE_HUMIDITY, humidity, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    更新湿度设置值
// Parameters:  NULL
// Return:      更新成功返回true，否则返回false。
// Remark:		该函数更新需要显示湿度设置值的相应界面资源，但并不刷新界面
//
void UpdateHumiditySettingValue ()
{
    return;

    CHAR szTemp[11] = "time_0.bmp";
    int temp[2] = { (g_sensorStatus.nHumiditySettingValue % 100) / 10, g_sensorStatus.nHumiditySettingValue % 10 };
    if (g_sensorStatus.nHumiditySettingValue < 10) {
        // 十位
        g_SmartAnim.ReplacePic (7, 6, "null");
        g_SmartAnim.ReplacePic (8, 6, "null");

        // 个位
        szTemp[5] = temp[1] + 0x30;
        g_SmartAnim.ReplacePic (7, 7, szTemp);
        g_SmartAnim.ReplacePic (8, 7, szTemp);
    }
    else {
        // 十位
        szTemp[5] = temp[0] + 0x30;
        g_SmartAnim.ReplacePic (7, 6, szTemp);
        g_SmartAnim.ReplacePic (8, 6, szTemp);

        // 个位
        szTemp[5] = temp[1] + 0x30;
        g_SmartAnim.ReplacePic (7, 7, szTemp);
        g_SmartAnim.ReplacePic (8, 7, szTemp);
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
// Function:    上传光照信息到设备服务器
// Parameters:  NULL
// Return:      VOID
//
void UploadDayAndNight ()
{
    bool bright;
    if (g_CommCtrl.GetLight (PORT_NUMBER_COORDINATOR, &bright))
        SendMessage (g_hDeviceServer, WM_UPDATE_DAY_NIGHT, bright ? 1 : 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    更新光照信息
// Parameters:  NULL
// Return:      更新成功返回true，否则返回false。
// Remark:		该函数更新需要显示光照的相应界面资源，但并不刷新界面
//
bool UpdateDayAndNight ()
{
    bool bRet;

    // 获取光照数值
    bool bright;
    bRet = g_CommCtrl.GetLight (PORT_NUMBER_COORDINATOR, &bright);

    // 更新光照信息
    if (bRet) {
        if (bright) {
            g_SmartAnim.ReplacePic (5, 4, "home_control_day.bmp");
            g_SmartAnim.ReplacePic (6, 4, "home_control_day.bmp");
        }
        else {
            g_SmartAnim.ReplacePic (5, 4, "home_control_night.bmp");
            g_SmartAnim.ReplacePic (6, 4, "home_control_night.bmp");
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    设置空调状态
// Parameters:  open    bool    [IN]    指示是否打开空调
// Return:      void
// Remark:      该函数控制空调状态，并刷新界面显示
//
void SetAirConditioner (bool open)
{
    if (open) {
        g_SmartAnim.ReplacePic (5, 6, "home_control_wind.bmp");
        g_SmartAnim.ReplacePic (6, 6, "home_control_wind.bmp");
        for (int i = 0; i < SEND_DATA_TIMES; i++)
            g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR, FORWARD);
    }
    else {
        g_SmartAnim.ReplacePic (5, 6, "null");
        g_SmartAnim.ReplacePic (6, 6, "null");
        for (int i = 0; i < SEND_DATA_TIMES; i++)
            g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR, STOP);
    }

    // 刷新家庭控制界面

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    设置灯状态
// Parameters:  lightID     int     [IN]    要控制的灯序号（从1开始）
//              open        bool    [IN]    指示是否打开灯
// Return:      void
// Remark:      该函数控制灯状态，并刷新界面显示
//
void SetLight (int ligthID, bool open)
{
    if (open) {
        g_SmartAnim.ReplacePic (5, ligthID - 1, "home_control_bright.bmp");
        g_SmartAnim.ReplacePic (6, ligthID - 1, "home_control_bright.bmp");
    }
    else {
        g_SmartAnim.ReplacePic (5, ligthID - 1, "home_control_dark.bmp");
        g_SmartAnim.ReplacePic (6, ligthID - 1, "home_control_dark.bmp");
    }

    for (int i = 0; i < SEND_DATA_TIMES; i++)
        g_CommCtrl.SetLED (PORT_NUMBER_COORDINATOR, ligthID, open);

    // 刷新家庭控制界面

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    设置喷水头状态
// Parameters:  open        bool    [IN]    指示是否打开喷水头
// Return:      void
// Remark:      该函数控制喷水头状态，并刷新界面显示
//
void SetSpray (bool open)
{
    if (open) {
        g_SmartAnim.ReplacePic (7, 0, "farm_spray_on.bmp");
        g_SmartAnim.ReplacePic (8, 0, "farm_spray_on.bmp");
        if (g_sensorStatus.bFan) {
            g_sensorStatus.bFan = false;
            SetFan (false);
        }
        for (int i = 0; i < SEND_DATA_TIMES; i++)
            g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR, FORWARD);
    }
    else {
        g_SmartAnim.ReplacePic (7, 0, "farm_spray_off.bmp");
        g_SmartAnim.ReplacePic (8, 0, "farm_spray_off.bmp");
        for (int i = 0; i < SEND_DATA_TIMES; i++)
            g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR, STOP);
    }

    // 刷新智慧农业界面

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    设置排气扇状态
// Parameters:  open        bool    [IN]    指示是否打开排气扇
// Return:      void
// Remark:      该函数控制排气扇状态，并刷新界面显示
//
void SetFan (bool open)
{
    if (open) {
        g_SmartAnim.ReplacePic (7, 1, "farm_fan_on.bmp");
        g_SmartAnim.ReplacePic (8, 1, "farm_fan_on.bmp");
        if (g_sensorStatus.bSpray) {
            g_sensorStatus.bSpray = false;
            SetSpray (false);
        }
        for (int i = 0; i < SEND_DATA_TIMES; i++)
            g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR, BACKWARD);
    }
    else {
        g_SmartAnim.ReplacePic (7, 1, "farm_fan_off.bmp");
        g_SmartAnim.ReplacePic (8, 1, "farm_fan_off.bmp");
        for (int i = 0; i < SEND_DATA_TIMES; i++)
            g_CommCtrl.SetMotor (PORT_NUMBER_COORDINATOR, STOP);
    }

    // 刷新智慧农业界面

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变灯状态
// Parameters:  lightID int [IN]    要控制的灯序号（从1开始）
// Return:      void
// Remark:      该函数改变相应灯的状态，并刷新界面显示
//
void ChangeLight (int lightID)
{
    g_sensorStatus.bLight[lightID - 1] = !g_sensorStatus.bLight[lightID - 1];
    SetLight (lightID, g_sensorStatus.bLight[lightID - 1]);
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
// Function:    改变空调状态
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变当前空调的状态，并刷新界面显示
//
GCPROCESS void ChangeAirConditioner ()
{
    g_sensorStatus.bAirConditioner = !g_sensorStatus.bAirConditioner;
    SetAirConditioner (g_sensorStatus.bAirConditioner);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变灯1状态
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变灯1的状态，并刷新界面显示
//              该函数为界面点击处理函数
//
GCPROCESS void ChangeLight1 ()
{
    ChangeLight (1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变灯2状态
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变灯2的状态，并刷新界面显示
//              该函数为界面点击处理函数
//
GCPROCESS void ChangeLight2 ()
{
    ChangeLight (2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变灯3状态
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变灯3的状态，并刷新界面显示
//              该函数为界面点击处理函数
//
GCPROCESS void ChangeLight3 ()
{
    ChangeLight (3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变灯4状态
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变灯4的状态，并刷新界面显示
//              该函数为界面点击处理函数
//
GCPROCESS void ChangeLight4 ()
{
    ChangeLight (4);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变喷水头状态
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变当前喷水头的状态，并刷新界面显示
//
GCPROCESS void ChangeSpray ()
{
    g_sensorStatus.bSpray = !g_sensorStatus.bSpray;
    SetSpray (g_sensorStatus.bSpray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    改变排气扇状态
// Parameters:  NULL
// Return:      void
// Remark:      该函数改变当前排气扇的状态，并刷新界面显示
//
GCPROCESS void ChangeFan ()
{
    g_sensorStatus.bFan = !g_sensorStatus.bFan;
    SetFan (g_sensorStatus.bFan);
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
    
    // 显示读卡成功动画
    switch (g_InterfaceID) {
        case 1:         // 读卡界面
            // 播放声音
            PlaySound (SOUND_FAMILY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (3, true);
            break;
        case 4:         // 一卡通界面
            // 播放声音
            PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (10, true);
            break;
        case 5:         // 食品溯源界面
            // 播放声音
            PlaySound (SOUND_FOOD_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (13, true);
            break;
    }
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

	// 显示读卡失败动画
    switch (g_InterfaceID) {
        case 1:         // 读卡界面
            // 播放声音
            PlaySound (SOUND_FAMILY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

	        g_InterfaceID = -1;
	        g_SmartAnim.ShowAnim (4, true);
            break;
        case 4:         // 一卡通界面
            // 播放声音
            PlaySound (SOUND_SUBWAY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (11, true);
            break;
        case 5:         // 食品溯源界面
            // 播放声音
            PlaySound (SOUND_FOOD_FAILED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (14, true);
            break;
    }
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
            UpdateTemperature ();

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
            SetAirConditioner (false);
            g_sensorStatus.bAirConditioner = false;
            for (int i = 1; i <= 4; i++) {
                SetLight (i, false);
                g_sensorStatus.bLight[i - 1] = false;
            }

            // 显示家庭控制界面退出动画，准备进入主界面
            g_InterfaceID = -1;
            g_GoalInterfaceID = 0;
            g_SmartAnim.ShowAnim (5, false);
            break;
        case 3:                                 // 智慧农业界面
            // 关闭智慧农业界面控制项
            SetSpray (false);
            SetFan (false);
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
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeAirConditioner", ChangeAirConditioner)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeLight1", ChangeLight1)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeLight2", ChangeLight2)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeLight3", ChangeLight3)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeLight4", ChangeLight4)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeSpray", ChangeSpray)) return false;
    if (!g_GeniusCtrl.RegisterMouseDownProcess ("ChangeFan", ChangeFan)) return false;
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
DWORD ThreadReadCard (LPVOID lpvoid)
{
    bool bSuccess;
    BYTE cardSize;
    BYTE pKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    BYTE blockBytes[16];

    // RFID开射频
    bool rfOpen = false;
    for (int i = 0; i < 3; i++)
    {
        if (m104bpc_RFCtrl (PORT_NUMBER_M104BPC, true)) {
            rfOpen = true;
            break;
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
	                g_SmartAnim.ShowAnim (3, true);
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
                    blockBytes[0] = 100;
                    if (!m104bpc_WriteBlock (PORT_NUMBER_M104BPC, 2, blockBytes))
                        goto SubWayEnd;
                    
                    // 更新界面
                    g_SmartAnim.ReplacePic (9, 2, "date_0.bmp");
                    g_SmartAnim.ReplacePic (10, 2, "date_0.bmp");
                    g_SmartAnim.ReplacePic (11, 2, "date_0.bmp");

                    CHAR szTemp[11] = "date_0.bmp";

                    // 百位
                    g_SmartAnim.ReplacePic (9, 4, "date_1.bmp");
                    g_SmartAnim.ReplacePic (10, 4, "date_1.bmp");
                    g_SmartAnim.ReplacePic (11, 4, "date_1.bmp");

                    // 十位
                    g_SmartAnim.ReplacePic (9, 5, "date_0.bmp");
                    g_SmartAnim.ReplacePic (10, 5, "date_0.bmp");
                    g_SmartAnim.ReplacePic (11, 5, "date_0.bmp");

                    // 个位
                    g_SmartAnim.ReplacePic (9, 6, "date_0.bmp");
                    g_SmartAnim.ReplacePic (10, 6, "date_0.bmp");
                    g_SmartAnim.ReplacePic (11, 6, "date_0.bmp");

                    // 充值完成
                    g_bRechargeCard = false;
                    bSuccess = true;

                    // 播放声音
                    PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

                    // 显示读卡成功动画
                    g_InterfaceID = -1;
                    g_SmartAnim.ShowAnim (10, true);
                }
                else {                  // 读卡
                    if (blockBytes[0] >= 2) {   // 余额足够
                        // 扣除金额
                        blockBytes[0] -= 2;
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
            case 5:                         // 食品溯源界面
                bSuccess = false;

                // 判断是否为已登记卡号
                if (cardNo[0] == g_RFID[0] &&
                    cardNo[1] == g_RFID[1] &&
                    cardNo[2] == g_RFID[2] &&
                    cardNo[3] == g_RFID[3]) {
                    // 播放声音
                    PlaySound (SOUND_FOOD_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

	                // 显示读卡成功动画
	                g_InterfaceID = -1;
	                g_SmartAnim.ShowAnim (13, true);
                }
                else {
                    // 播放声音
                    PlaySound (SOUND_FOOD_FAILED, NULL, SND_FILENAME | SND_ASYNC);

                    // 显示读卡失败动画
                    g_InterfaceID = -1;
                    g_SmartAnim.ShowAnim (14, true);
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
// Function:    进入家庭控制界面
// Parameters:  NULL
// Return:      void
//
void EnterHomeCtrlInterface ()
{
    // 显示读卡成功界面退出动画
    g_InterfaceID = -1;
    g_GoalInterfaceID = 2;
    g_SmartAnim.ShowAnim (3, false);
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
    UploadTemperature ();
    UploadHumidity ();
    UploadDayAndNight ();

    switch (g_InterfaceID) {
        case 0:         // 主界面
            UpdateTemperature ();
            UpdateTime ();
            g_SmartAnim.ShowAnim (1, true);
            break;
        case 2:         // 家庭控制界面
            UpdateTemperature ();
            UpdateDayAndNight ();
            g_SmartAnim.ShowAnim (6, true);
            break;
        case 3:         // 智慧农业界面
            UpdateTemperature ();
            UpdateHumidity ();
            g_SmartAnim.ShowAnim (8, true);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    动画完成回调函数
// Parameters:  animID  int     完成的动画ID
//              obverse bool    指示完成的动画是否是顺放的
// Return:      void
//
void AnimEndProcess (int animID, bool obverse)
{
	// 动画完成
    switch (animID) {
        case 0:                 // 进入主界面动画
            if (obverse) {
                // 主界面载入
                MainInterfaceEntered ();
            }
            else {
                // 主界面退出
                switch (g_GoalInterfaceID) {
                    case 1:
                        // 显示读卡界面进入动画
                        g_SmartAnim.ShowAnim (2, true);
                        break;
                    case 3:
                        // 刷新智慧农业界面信息
                        //UpdateTemperature ();
                        UpdateHumidity ();
                        UpdateHumiditySettingValue ();
                        //SetSpray (false);
                        //SetFan (false);

                        // 显示智慧农业进入动画
                        g_SmartAnim.ShowAnim (7, true);
                        break;
                    case 4:
                        // 清空金额信息
                        g_SmartAnim.ReplacePic (9, 2, "null");
                        g_SmartAnim.ReplacePic (10, 2, "null");
                        g_SmartAnim.ReplacePic (11, 2, "null");

                        g_SmartAnim.ReplacePic (9, 4, "null");
                        g_SmartAnim.ReplacePic (10, 4, "null");
                        g_SmartAnim.ReplacePic (11, 4, "null");

                        g_SmartAnim.ReplacePic (9, 5, "null");
                        g_SmartAnim.ReplacePic (10, 5, "null");
                        g_SmartAnim.ReplacePic (11, 5, "null");

                        g_SmartAnim.ReplacePic (9, 6, "null");
                        g_SmartAnim.ReplacePic (10, 6, "null");
                        g_SmartAnim.ReplacePic (11, 6, "null");

                        // 显示一卡通进入动画
                        g_SmartAnim.ShowAnim (9, true);
                        break;
                    case 5:
                        // 清空信息
                        g_SmartAnim.ReplacePic (12, 0, "food_picture.bmp");
                        g_SmartAnim.ReplacePic (12, 1, "food_read_card.bmp");

                        // 显示食品溯源进入动画
                        g_SmartAnim.ShowAnim (12, true);
                        break;
                    case 6:
                        // 刷新设置界面时间设置项和值
                        g_DateSettingItem = YEAR;
                        GetLocalTime (&g_DateSettingValue);
                        UpdateDateSettingItemAndValue ();
                        
                        // 显示设置进入动画
                        g_SmartAnim.ShowAnim (15, true);
                        break;
                }
            }
            break;
        case 2:                 // 进入读卡界面动画
            if (obverse) {
                // 已进入读卡界面
                ReadCardInterfaceEntered ();
            }
            else {
                // 读卡界面退出
                switch (g_GoalInterfaceID) {
                    case 0:
                        // 刷新主界面信息
                        UpdateTemperature ();
                        UpdateTime ();

                        // 显示主界面进入动画
                        g_SmartAnim.ShowAnim (0, true);
                        break;
                    case 2:
                        // 刷新家庭控制界面信息
                        UpdateTemperature ();
                        UpdateDayAndNight ();
                        //SetAirConditioner (false);
                        //for (int i = 1; i <= 4; i++)
                        //    SetLight (i, false);

                        // 显示家庭控制界面进入动画
                        g_SmartAnim.ShowAnim (5, true);
                        break;
                }
            }
            break;
		case 3:                 // 读卡成功动画
			if (obverse) {
                // 读卡成功
				Sleep (500);
				
                // 进入家庭控制界面
                EnterHomeCtrlInterface ();
			}
            else {
                // 显示读卡界面退出动画
                g_SmartAnim.ShowAnim (2, false);
            }
			break;
		case 4:                 // 读卡失败动画
			if (obverse) {
                // 读卡失败
				Sleep (500);

                // 显示读卡失败退出动画
				g_SmartAnim.ShowAnim (4, false);
			}
            else {
                g_InterfaceID = 1;

                // 开始循环读卡
                ReadCardStart ();
            }
			break;
        case 5:                 // 家庭控制界面动画
            if (obverse)
                // 已进入家庭控制界面
                HomeControlInterfaceEntered ();
            else {
                switch (g_GoalInterfaceID) {
                    case 0:
                        // 刷新主界面信息
                        UpdateTemperature ();
                        UpdateTime ();

                        // 显示主界面进入动画
                        g_SmartAnim.ShowAnim (0, true);
                        break;
                    case 1:
                        // 显示读卡界面进入动画
                        g_SmartAnim.ShowAnim (2, true);
                        break;
                }
            }
            break;
        case 7:                 // 智慧农业界面动画
            if (obverse)
                // 已进入智慧农业界面
                FarmingInterfaceEntered ();
            else {
                // 刷新主界面信息
                UpdateTemperature ();
                UpdateTime ();

                // 显示主界面进入动画
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
        case 9:                 // 一卡通界面动画
            if (obverse)
                // 已进入一卡通界面
                SubwayInterfaceEntered ();
            else {
                // 刷新主界面信息
                UpdateTemperature ();
                UpdateTime ();

                // 显示主界面进入动画
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
        case 10:                // 一卡通读卡成功动画
            g_InterfaceID = 4;

            // 开始循环读卡
            ReadCardStart ();

            break;
        case 11:                // 一卡通读卡失败动画
            g_InterfaceID = 4;

            // 开始循环读卡
            ReadCardStart ();

            break;
        case 12:                // 食品溯源界面动画
            if (obverse)
                // 已进入食品溯源界面
                FoodInterfaceEntered ();
            else {
                // 刷新主界面信息
                UpdateTemperature ();
                UpdateTime ();

                // 显示主界面进入动画
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
        case 13:                // 食品溯源读卡成功动画
            g_InterfaceID = 5;

            // 开始循环读卡
            ReadCardStart ();

            break;
        case 14:                // 食品溯源读卡失败动画
            g_InterfaceID = 5;

            // 开始循环读卡
            ReadCardStart ();

            break;
        case 15:                // 设置界面动画
            if (obverse)
                // 已进入设置界面
                SettingsInterfaceEntered ();
            else {
                // 刷新主界面信息
                UpdateTemperature ();
                UpdateTime ();

                // 显示主界面进入动画
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
    }
}