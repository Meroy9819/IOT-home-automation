#include "stdafx.h"
#include "Gemini2.h"
#include <assert.h>

#include "CECommCtrl.h"

// ʹ��ADTL��
#include "ADTL.h"
#pragma comment (lib, "ADTL")

// ����SmartAnim��
#include "SmartAnim.h"
#pragma comment (lib, "SmartAnim")

// ����GeniusCtrl��
#include "GeniusCtrl.h"
#pragma comment (lib, "GeniusCtrl")

// ����M104BPC��
#include "M104BPC.h"
#pragma comment (lib, "M104BPC")

#define MAX_LOADSTRING 100              // ��������⼰������󳤶�
#define MAX_LINE_LENGTH 128             // ֧�ֵ������ļ�ÿ����ַ���

#define TIMER_ID_REFRESH 8000           // ����ˢ�¼�ʱ��ID
#define SEND_DATA_TIMES 3               // �������ݵĴ��������践��ֵ���޷���ѯ��ָ�Ϊȷ���������������͵��ظ�������

#define PORT_NUMBER_COORDINATOR 2       // ������Э�����˿�
#define BAUND_RATE_COORDINATOR 115200   // ������Э����������

#define PORT_NUMBER_M104BPC 3           // M104BPC�˿�
#define BAUND_RATE_M104BPC 19200        // M104BPC������

#define BASE_DIR TEXT("\\PocketMory1\\")                            // ��������ļ���
#define MAKE_FULL_PATH(dayNumber) (BASE_DIR TEXT(#dayNumber))       // չ���ļ�����·������������ļ���

#define ANIM_RES_FILENAME MAKE_FULL_PATH(AnimRes.grc)               // ������Դ�ļ�����·��
#define PIC_RES_FILENAME MAKE_FULL_PATH(PicRes.grc)                 // ͼƬ��Դ�ļ�����·��
#define CTRL_RES_FILENAME MAKE_FULL_PATH(CtrlRes.grc)               // ������Դ�ļ�����·��
#define PIC_DIR_PATH MAKE_FULL_PATH(Pics\\)                         // ͼƬ��Դ�ļ�������·��

#define SOUND_FAMILY_SUCCEED MAKE_FULL_PATH(Family_Succeed.wav)     // �ǻۼ�ͥ�����ɹ������ļ�·��
#define SOUND_FAMILY_FAILED MAKE_FULL_PATH(Family_Failed.wav)       // �ǻۼ�ͥ����ʧ�������ļ�·��
#define SOUND_SUBWAY_SUCCEED MAKE_FULL_PATH(Subway_Succeed.wav)     // һ��ͨ�����ɹ������ļ�·��
#define SOUND_SUBWAY_FAILED MAKE_FULL_PATH(Subway_Failed.wav)       // һ��ͨ����ʧ�������ļ�·��
#define SOUND_FOOD_SUCCEED MAKE_FULL_PATH(Food_Succeed.wav)         // ʳƷ��Դ�����ɹ������ļ�·��
#define SOUND_FOOD_FAILED MAKE_FULL_PATH(Food_Failed.wav)           // ʳƷ��Դ����ʧ�������ļ�·��

#define SERVER_PATH MAKE_FULL_PATH(Gemini2\\DeviceServer.exe)       // �豸������·��
#define RFID_COF_FILE MAKE_FULL_PATH(RFID.txt)                      // RFID��ID·��
#define SERVER_ADDR_FILE MAKE_FULL_PATH(ServerAddr.txt)             // ����ͨ�ŵ�ַ·��

#define LOG_FILE MAKE_FULL_PATH(log.txt)                            // ��־�ļ�

// ������״̬�ṹ��
typedef struct _SENSOR_STATUS {
    bool bAirConditioner;               // �յ�״̬
    bool bLight[4];                     // ��״̬
    bool bSpray;                        // ��ˮͷ״̬
    bool bFan;                          // ������״̬
    bool bHumidityAuto;                 // ʪ���Զ�����״̬
    int nHumiditySettingValue;          // ʪ������ֵ
} SENSOR_STATUS, *PSENSOR_STATUS;

enum DateSettingItem { YEAR, MONTH, DAY, HOUR, MINUTE };     // ����������Ŀö��

// ȫ�ֱ���:
HINSTANCE g_hInst;					    // ��ǰӦ�ó���ʵ��
HWND g_hWnd;						    // ��������
HWND g_hDeviceServer = NULL;            // �豸��������Ϣ������

UINT WM_SEND_WINDOW_HANDLE = 0;         // �豸��������������Ϣ������
UINT WM_SET_WEBSITE_ADDR = 0;           // �趨�豸��������վ���ӵ�ַ
UINT WM_SET_LED = 0;                    // ����LED��״̬
UINT WM_SET_MOTOR = 0;                  // ���õ��״̬
UINT WM_UPDATE_TEMPERATURE = 0;         // �����¶�
UINT WM_UPDATE_HUMIDITY = 0;            // ����ʪ��
UINT WM_UPDATE_DAY_NIGHT = 0;           // ���¹���״̬
UINT WM_CLOSE_SERVER = 0;               // �ر��豸������

int g_InterfaceID = -1;                 // ��ǰ����ID
int g_GoalInterfaceID = -1;             // Ŀ�����ID����IDָʾ��Ҫ����Ľ���

CSmartAnim g_SmartAnim;                 // SmartAnimʵ��
CGeniusCtrl g_GeniusCtrl;               // GeniusCtrlʵ��
CCommCtrl g_CommCtrl;                   // CommCtrlʵ��

HANDLE g_hReadCardThread = NULL;        // �����߳̾��
bool g_bStopReadCardThread = false;     // �Ƿ���ֹ�����߳�

bool g_bRechargeCard = false;           // �Ƿ��ֵ��Ƭ

SYSTEMTIME g_DateSettingValue;          // ��������ֵ
DateSettingItem g_DateSettingItem;      // ����������

BYTE g_RFID[4];                         // ����ͨ����RFID
DWORD g_LocalAddr = 0;                  // ���������ַ
DWORD g_RemoteAddr = 0;                 // Զ�������ַ

// ������״̬
SENSOR_STATUS g_sensorStatus = { false, 
                                 { false, false, false, false }, 
                                 false, 
                                 false, 
                                 false, 
                                 50 };

// �˴���ģ���а����ĺ�����ǰ������:
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
    
	// ִ��Ӧ�ó����ʼ��:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}
        
    // ����������
	HWND hwndTask = FindWindow (L"HHTaskBar", NULL);
	ShowWindow (hwndTask, SW_HIDE);

	// ���ع��
	HCURSOR hPrevCursor = SetCursor (NULL);

	// ��ʼ��SmartAnim
	if (!g_SmartAnim.Initialize (g_hWnd, ANIM_RES_FILENAME, PIC_RES_FILENAME, PIC_DIR_PATH))
		return 1;

    // ��ʼ��GeniusCtrl
    if (!g_GeniusCtrl.Initialize (CTRL_RES_FILENAME))
        return 2;

	// ע����洦��ص�����
	if (!RegisterGeniusCtrlProcess ())
        return 3;

    // ע�ᶯ�������ص�����
    g_SmartAnim.RegisterAnimEndProcess (AnimEndProcess);

    // �򿪴�����Э�����˿�
    if (!g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR))
        return 4;

    // ��M104BPC�˿�
    if (!m104bpc_OpenPort (PORT_NUMBER_M104BPC, BAUND_RATE_M104BPC))
        return 5;

    // ��ȡRFID����
    if (!LoadRFID (RFID_COF_FILE))
        return 6;

    // ��ȡ����ͨ�ŵ�ַ
    if (!LoadServerAddr (SERVER_ADDR_FILE))
        return 7;

    // ����RFIDģ��TypeAģʽ
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
        ASSERTMSG (TEXT ("��Ƶģ������ģʽʧ�ܣ������������豸��"), FALSE);
        return 8;
    }

    // ע�����豸����������ͨ�ŵ���Ϣ
    WM_SEND_WINDOW_HANDLE = RegisterWindowMessage (TEXT ("WM_SEND_WINDOW_HANDLE"));
    WM_SET_WEBSITE_ADDR = RegisterWindowMessage (TEXT ("WM_SET_WEBSITE_ADDR"));
    WM_SET_LED = RegisterWindowMessage (TEXT ("WM_SET_LED"));
    WM_SET_MOTOR = RegisterWindowMessage (TEXT ("WM_SET_MOTOR"));
    WM_UPDATE_TEMPERATURE = RegisterWindowMessage (TEXT ("WM_UPDATE_TEMPERATURE"));
    WM_UPDATE_HUMIDITY = RegisterWindowMessage (TEXT ("WM_UPDATE_HUMIDITY"));
    WM_UPDATE_DAY_NIGHT = RegisterWindowMessage (TEXT ("WM_UPDATE_DAY_NIGHT"));
    WM_CLOSE_SERVER = RegisterWindowMessage (TEXT ("WM_CLOSE_SERVER"));

    // �����豸������
	PROCESS_INFORMATION procInfo;
	procInfo.hProcess = NULL;
	TCHAR commLine[16];
    swprintf (commLine, TEXT ("%d"), g_hWnd);
	CreateProcess (SERVER_PATH, commLine, NULL, NULL, FALSE, 0, NULL, NULL, NULL, &procInfo);

    // ��������ˢ�¶�ʱ��
    SetTimer (g_hWnd, TIMER_ID_REFRESH, 1000, (TIMERPROC)TimerProcess_Refresh);

    // ��ʾ������
	EnterMainInterface ();

    //AddLog ("Application Started!", LOG_FILE);
    
	// ����Ϣѭ��:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    // ֹͣ����ˢ�¶�ʱ��
    KillTimer (g_hWnd, TIMER_ID_REFRESH);

	// ֹͣ�豸������
    if (procInfo.hProcess) {
        SendMessage (g_hDeviceServer, WM_CLOSE_SERVER, 0, 0);
        TerminateProcess (procInfo.hProcess, 0);
    }

    // �رմ�����Э�����˿�
    g_CommCtrl.CloseSerialPort (PORT_NUMBER_COORDINATOR);

    // �ر�M104BPC�˿�
    m104bpc_ClosePort (PORT_NUMBER_M104BPC);

    // �ͷ�GeniusCtrl
    g_GeniusCtrl.Uninitialize ();

	// �ͷ�SmartAnim
	g_SmartAnim.Uninitialize ();

    // �������������
    if (!FindWindow (L"DesktopExplorerWindow", NULL))
        CreateProcess (TEXT ("explorer.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);

	// �ָ����
	SetCursor (hPrevCursor);

    // ��ʾ������
	ShowWindow (hwndTask, SW_SHOW);

    //AddLog ("Application Stopped!", LOG_FILE);

	return (int) msg.wParam;
}

//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��:
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
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    TCHAR szTitle[MAX_LOADSTRING];		    // �������ı�
    TCHAR szWindowClass[MAX_LOADSTRING];	// ����������

    g_hInst = hInstance;                    // ��ʵ������洢��ȫ�ֱ�����

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_GEMINI2, szWindowClass, MAX_LOADSTRING);

    //������Ѿ������У��򽫽������ڴ����ϣ�Ȼ���˳�
    g_hWnd = FindWindow(szWindowClass, szTitle);	
    if (g_hWnd) 
    {
        // ������������ǰ����Ӵ���
        // ��| 0x00000001�����ڽ����и�����������ǰ̨��
        // ������Щ���ڡ�
        SetForegroundWindow((HWND)((ULONG) g_hWnd | 0x00000001));
        return 0;
    }

    if (!MyRegisterClass(hInstance, szWindowClass))
    	return FALSE;

    // ����ȫ������
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
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
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
            // �����豸��������Ϣ������
            g_hDeviceServer = (HWND)wParam;

            // �����豸��������վ���ӵ�ַ
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

    // ���ļ�
	hFile = CreateFile (lpszLogFileName,
		                GENERIC_WRITE,
		                FILE_SHARE_WRITE,
		                NULL,
		                OPEN_ALWAYS,
		                FILE_ATTRIBUTE_NORMAL,
		                NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto End;

    // ��ȡϵͳʱ��
    SYSTEMTIME systemTime;
    GetLocalTime (&systemTime);

    // ��֯Log
    if (sprintf (szLogStr, "%4d-%02d-%02d %02d:%02d:%02d %s %d\r\n", systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
                 systemTime.wHour, systemTime.wMinute, systemTime.wSecond, lpszLogStr, dwErr) == -1)
        goto End;

    // д��Log
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
// Function:    ��ȡRFID���ļ�
// Parameters:	lpCofFileName   LPTSTR  [IN]    �����ļ����ƣ�����·����
// Return:      �ɹ�����true��ʧ�ܷ���false��
//
bool LoadRFID (PCTSTR lpCofFileName)
{
    bool bRet = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BYTE* pFileContent = NULL;

	// ������Χ���
	if (!lpCofFileName)
		goto End;

	// ���ļ�
	hFile = CreateFile (lpCofFileName,
						GENERIC_READ,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto End;

	// ��ȡ�ļ�����
	DWORD dwLen = GetFileSize (hFile, NULL);
	if (dwLen == 0xFFFFFFFF) 
		goto End;

	// ��ȡ�ļ�
	pFileContent = new BYTE[dwLen];
	DWORD dwNumberOfBytesRead;
	if (!ReadFile (hFile, pFileContent, dwLen, &dwNumberOfBytesRead, NULL))
		goto End;
	if (dwNumberOfBytesRead != dwLen) 
		goto End;

	// ��ȡ�ļ���ÿһ��
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
// Function:    ��ȡ��������ַ
// Parameters:	lpCofFileName   LPTSTR  [IN]    �����ļ����ƣ�����·����
// Return:      �ɹ�����true��ʧ�ܷ���false��
//
bool LoadServerAddr (PCTSTR lpCofFileName)
{
    bool bRet = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	BYTE* pFileContent = NULL;

	// ������Χ���
	if (!lpCofFileName)
		goto End;

	// ���ļ�
	hFile = CreateFile (lpCofFileName,
						GENERIC_READ,
						0,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		goto End;

	// ��ȡ�ļ�����
	DWORD dwLen = GetFileSize (hFile, NULL);
	if (dwLen == 0xFFFFFFFF) 
		goto End;

	// ��ȡ�ļ�
	pFileContent = new BYTE[dwLen];
	DWORD dwNumberOfBytesRead;
	if (!ReadFile (hFile, pFileContent, dwLen, &dwNumberOfBytesRead, NULL))
		goto End;
	if (dwNumberOfBytesRead != dwLen) 
		goto End;

	// ��ȡ�ļ���ÿһ��
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
// Function:    ����ĳ�µ�����
// Parameters:  year    int     [IN]    ���
//              month   int     [IN]    �·�
// Return:      �����ض����ض��µ�����
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
// Function:    ����ʱ��
// Parameters:  NULL
// Return:      void
// Remark:		�ú���������Ҫ��ʾʱ�����Ӧ������Դ��������ˢ�½���
//
void UpdateTime ()
{
	// ��ȡϵͳʱ��
	SYSTEMTIME systemTime;
	GetLocalTime (&systemTime);

	// ��������-��
	CHAR szDate[11] = "date_0.bmp";
	int year[4] = { systemTime.wYear / 1000, (systemTime.wYear % 1000) / 100, (systemTime.wYear % 100) / 10, systemTime.wYear % 10 };
	for (int i = 0; i < 4; i++) {
		szDate[5] = year[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 1 + i, szDate);
		g_SmartAnim.ReplacePic (1, 1 + i, szDate);
	}

	// ��������-��
	int month[2] = { systemTime.wMonth / 10, systemTime.wMonth % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = month[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 6 + i, szDate);
		g_SmartAnim.ReplacePic (1, 6 + i, szDate);
	}

	// ��������-��
	int day[2] = { systemTime.wDay / 10, systemTime.wDay % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = day[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 9 + i, szDate);
		g_SmartAnim.ReplacePic (1, 9 + i, szDate);
	}

	// ��������-����
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

	// ����ʱ��-ʱ
	CHAR szTime[11] = "time_0.bmp";
	int hour[2] = { systemTime.wHour / 10, systemTime.wHour % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = hour[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 11 + i, szTime);
		g_SmartAnim.ReplacePic (1, 11 + i, szTime);
	}

	// ����ʱ��-��
	int minute[2] = { systemTime.wMinute / 10, systemTime.wMinute % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = minute[i] + 0x30;
		g_SmartAnim.ReplacePic (0, 14 + i, szTime);
		g_SmartAnim.ReplacePic (1, 14 + i, szTime);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �����¶�
// Parameters:  NULL
// Return:      ���³ɹ�����true�����򷵻�false��
// Remark:		�ú���������Ҫ��ʾ�¶ȵ���Ӧ������Դ��������ˢ�½���
//
bool UpdateTemperature ()
{
    bool bRet;

    // ��ȡ�¶���ֵ
    int temperature;
    bRet = g_CommCtrl.GetTemperature (PORT_NUMBER_COORDINATOR, &temperature);

    // �����¶ȣ�����0���϶ȣ�
    if (bRet && temperature != 0) {
        CHAR szTemp[11] = "temp_0.bmp";
        bool isMinus = temperature < 0 ? true : false;
        int temp[2] = { (temperature % 100) / 10, temperature % 10 };
        if (abs (temperature) <= 9) {
            // ����
            g_SmartAnim.ReplacePic (0, 16, "null");
		    g_SmartAnim.ReplacePic (1, 16, "null");
            g_SmartAnim.ReplacePic (5, 7, "null");
            g_SmartAnim.ReplacePic (6, 7, "null");
            g_SmartAnim.ReplacePic (7, 13, "null");
            g_SmartAnim.ReplacePic (8, 13, "null");
		    if (!isMinus) {
                // ʮλ
                g_SmartAnim.ReplacePic (0, 17, "null");
			    g_SmartAnim.ReplacePic (1, 17, "null");
                g_SmartAnim.ReplacePic (5, 8, "null");
                g_SmartAnim.ReplacePic (6, 8, "null");
                g_SmartAnim.ReplacePic (7, 14, "null");
                g_SmartAnim.ReplacePic (8, 14, "null");
		    }
		    else {
                // ʮλ
                g_SmartAnim.ReplacePic (0, 17, "temp_minus.bmp");
			    g_SmartAnim.ReplacePic (1, 17, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (5, 8, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (6, 8, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (7, 14, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (8, 14, "temp_minus.bmp");
		    }
            szTemp[5] = temp[1] + 0x30;
            // ��λ
            g_SmartAnim.ReplacePic (0, 18, szTemp);
		    g_SmartAnim.ReplacePic (1, 18, szTemp);
            g_SmartAnim.ReplacePic (5, 9, szTemp);
            g_SmartAnim.ReplacePic (6, 9, szTemp);
            g_SmartAnim.ReplacePic (7, 15, szTemp);
            g_SmartAnim.ReplacePic (8, 15, szTemp);
        }
        else {
		    if (!isMinus) {
                // ����
                g_SmartAnim.ReplacePic (0, 16, "null");
			    g_SmartAnim.ReplacePic (1, 16, "null");
                g_SmartAnim.ReplacePic (5, 7, "null");
                g_SmartAnim.ReplacePic (6, 7, "null");
                g_SmartAnim.ReplacePic (7, 13, "null");
                g_SmartAnim.ReplacePic (8, 13, "null");
		    }
		    else {
                // ����
                g_SmartAnim.ReplacePic (0, 16, "temp_minus.bmp");
			    g_SmartAnim.ReplacePic (1, 16, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (5, 7, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (6, 7, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (7, 13, "temp_minus.bmp");
                g_SmartAnim.ReplacePic (8, 13, "temp_minus.bmp");
		    }
            // ʮλ
            szTemp[5] = temp[0] + 0x30;
            g_SmartAnim.ReplacePic (0, 17, szTemp);
		    g_SmartAnim.ReplacePic (1, 17, szTemp);
            g_SmartAnim.ReplacePic (5, 8, szTemp);
            g_SmartAnim.ReplacePic (6, 8, szTemp);
            g_SmartAnim.ReplacePic (7, 14, szTemp);
            g_SmartAnim.ReplacePic (8, 14, szTemp);


            // ��λ
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
// Function:    �ϴ��¶���ֵ���豸������
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
// Function:    ����ʪ��
// Parameters:  NULL
// Return:      ���³ɹ�����true�����򷵻�false��
// Remark:		�ú���������Ҫ��ʾʪ�ȵ���Ӧ������Դ��������ˢ�½���
//
bool UpdateHumidity ()
{
    bool bRet;

    // ��ȡʪ����ֵ
    int humidity;
    bRet = g_CommCtrl.GetHumidity (PORT_NUMBER_COORDINATOR, &humidity);

    // ����ʪ��
    if (bRet) {
        CHAR szTemp[11] = "temp_0.bmp";
        int temp[2] = { (humidity % 100) / 10, humidity % 10 };
        if (humidity < 10) {
            // ʮλ
            g_SmartAnim.ReplacePic (7, 2, "null");
            g_SmartAnim.ReplacePic (8, 2, "null");

            // ��λ
            szTemp[5] = temp[1] + 0x30;
            g_SmartAnim.ReplacePic (7, 3, szTemp);
            g_SmartAnim.ReplacePic (8, 3, szTemp);
        }
        else {
            // ʮλ
            szTemp[5] = temp[0] + 0x30;
            g_SmartAnim.ReplacePic (7, 2, szTemp);
            g_SmartAnim.ReplacePic (8, 2, szTemp);

            // ��λ
            szTemp[5] = temp[1] + 0x30;
            g_SmartAnim.ReplacePic (7, 3, szTemp);
            g_SmartAnim.ReplacePic (8, 3, szTemp);
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ϴ�ʪ����ֵ���豸������
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
// Function:    ����ʪ������ֵ
// Parameters:  NULL
// Return:      ���³ɹ�����true�����򷵻�false��
// Remark:		�ú���������Ҫ��ʾʪ������ֵ����Ӧ������Դ��������ˢ�½���
//
void UpdateHumiditySettingValue ()
{
    return;

    CHAR szTemp[11] = "time_0.bmp";
    int temp[2] = { (g_sensorStatus.nHumiditySettingValue % 100) / 10, g_sensorStatus.nHumiditySettingValue % 10 };
    if (g_sensorStatus.nHumiditySettingValue < 10) {
        // ʮλ
        g_SmartAnim.ReplacePic (7, 6, "null");
        g_SmartAnim.ReplacePic (8, 6, "null");

        // ��λ
        szTemp[5] = temp[1] + 0x30;
        g_SmartAnim.ReplacePic (7, 7, szTemp);
        g_SmartAnim.ReplacePic (8, 7, szTemp);
    }
    else {
        // ʮλ
        szTemp[5] = temp[0] + 0x30;
        g_SmartAnim.ReplacePic (7, 6, szTemp);
        g_SmartAnim.ReplacePic (8, 6, szTemp);

        // ��λ
        szTemp[5] = temp[1] + 0x30;
        g_SmartAnim.ReplacePic (7, 7, szTemp);
        g_SmartAnim.ReplacePic (8, 7, szTemp);
    }   
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ��������������Ŀ��ֵ
// Parameters:  NULL
// Return:      ���³ɹ�����true�����򷵻�false��
// Remark:		�ú���������Ҫ��ʾ����������Ŀ��ֵ����Ӧ������Դ��������ˢ�½���
//
void UpdateDateSettingItemAndValue ()
{
    // ��������������
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

    // ��������-��
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

	// ��������-��
	int month[2] = { g_DateSettingValue.wMonth / 10, g_DateSettingValue.wMonth % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = month[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 6 + i, szDate);
		g_SmartAnim.ReplacePic (16, 6 + i, szDate);
	}

	// ��������-��
	int day[2] = { g_DateSettingValue.wDay / 10, g_DateSettingValue.wDay % 10 };
	for (int i = 0; i < 2; i++) {
		szDate[5] = day[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 9 + i, szDate);
		g_SmartAnim.ReplacePic (16, 9 + i, szDate);
	}

	// ��������-����
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

	// ����ʱ��-ʱ
	CHAR szTime[11] = "time_0.bmp";
	int hour[2] = { g_DateSettingValue.wHour / 10, g_DateSettingValue.wHour % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = hour[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 11 + i, szTime);
		g_SmartAnim.ReplacePic (16, 11 + i, szTime);
	}

	// ����ʱ��-��
	int minute[2] = { g_DateSettingValue.wMinute / 10, g_DateSettingValue.wMinute % 10 };
	for (int i = 0; i < 2; i++) {
		szTime[5] = minute[i] + 0x30;
		g_SmartAnim.ReplacePic (15, 14 + i, szTime);
		g_SmartAnim.ReplacePic (16, 14 + i, szTime);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ϴ�������Ϣ���豸������
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
// Function:    ���¹�����Ϣ
// Parameters:  NULL
// Return:      ���³ɹ�����true�����򷵻�false��
// Remark:		�ú���������Ҫ��ʾ���յ���Ӧ������Դ��������ˢ�½���
//
bool UpdateDayAndNight ()
{
    bool bRet;

    // ��ȡ������ֵ
    bool bright;
    bRet = g_CommCtrl.GetLight (PORT_NUMBER_COORDINATOR, &bright);

    // ���¹�����Ϣ
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
// Function:    ���ÿյ�״̬
// Parameters:  open    bool    [IN]    ָʾ�Ƿ�򿪿յ�
// Return:      void
// Remark:      �ú������ƿյ�״̬����ˢ�½�����ʾ
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

    // ˢ�¼�ͥ���ƽ���

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ���õ�״̬
// Parameters:  lightID     int     [IN]    Ҫ���Ƶĵ���ţ���1��ʼ��
//              open        bool    [IN]    ָʾ�Ƿ�򿪵�
// Return:      void
// Remark:      �ú������Ƶ�״̬����ˢ�½�����ʾ
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

    // ˢ�¼�ͥ���ƽ���

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ������ˮͷ״̬
// Parameters:  open        bool    [IN]    ָʾ�Ƿ����ˮͷ
// Return:      void
// Remark:      �ú���������ˮͷ״̬����ˢ�½�����ʾ
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

    // ˢ���ǻ�ũҵ����

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����������״̬
// Parameters:  open        bool    [IN]    ָʾ�Ƿ��������
// Return:      void
// Remark:      �ú�������������״̬����ˢ�½�����ʾ
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

    // ˢ���ǻ�ũҵ����

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı��״̬
// Parameters:  lightID int [IN]    Ҫ���Ƶĵ���ţ���1��ʼ��
// Return:      void
// Remark:      �ú����ı���Ӧ�Ƶ�״̬����ˢ�½�����ʾ
//
void ChangeLight (int lightID)
{
    g_sensorStatus.bLight[lightID - 1] = !g_sensorStatus.bLight[lightID - 1];
    SetLight (lightID, g_sensorStatus.bLight[lightID - 1]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �˳�����
// Parameters:  NULL
// Return:      void
// Remark:		�ú���Ϊ������������
//
GCPROCESS void DestroyWindow ()
{
	DestroyWindow (g_hWnd);
}

GCPROCESS void CardRecharge ()
{
    // ���ó�ֵ��־
    g_bRechargeCard = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı�յ�״̬
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı䵱ǰ�յ���״̬����ˢ�½�����ʾ
//
GCPROCESS void ChangeAirConditioner ()
{
    g_sensorStatus.bAirConditioner = !g_sensorStatus.bAirConditioner;
    SetAirConditioner (g_sensorStatus.bAirConditioner);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı��1״̬
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı��1��״̬����ˢ�½�����ʾ
//              �ú���Ϊ������������
//
GCPROCESS void ChangeLight1 ()
{
    ChangeLight (1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı��2״̬
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı��2��״̬����ˢ�½�����ʾ
//              �ú���Ϊ������������
//
GCPROCESS void ChangeLight2 ()
{
    ChangeLight (2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı��3״̬
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı��3��״̬����ˢ�½�����ʾ
//              �ú���Ϊ������������
//
GCPROCESS void ChangeLight3 ()
{
    ChangeLight (3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı��4״̬
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı��4��״̬����ˢ�½�����ʾ
//              �ú���Ϊ������������
//
GCPROCESS void ChangeLight4 ()
{
    ChangeLight (4);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı���ˮͷ״̬
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı䵱ǰ��ˮͷ��״̬����ˢ�½�����ʾ
//
GCPROCESS void ChangeSpray ()
{
    g_sensorStatus.bSpray = !g_sensorStatus.bSpray;
    SetSpray (g_sensorStatus.bSpray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı�������״̬
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı䵱ǰ�����ȵ�״̬����ˢ�½�����ʾ
//
GCPROCESS void ChangeFan ()
{
    g_sensorStatus.bFan = !g_sensorStatus.bFan;
    SetFan (g_sensorStatus.bFan);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ı���������
// Parameters:  NULL
// Return:      void
// Remark:      �ú����ı���������״̬����ˢ�½�����ʾ
//
GCPROCESS void ChangeDateSettings ()
{
    // �ı�������
    if (g_DateSettingItem == MINUTE)
        g_DateSettingItem = YEAR;
    else
        g_DateSettingItem = (DateSettingItem)(g_DateSettingItem + 1);

    // ����������
    UpdateDateSettingItemAndValue ();

    // ˢ�½�����ʾ
    g_SmartAnim.ShowAnim (16, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �����ɹ�
// Parameters:  NULL
// Return:      void
// Remark:      �ú���ģ������ɹ���������Ӳ��������ʹ��
//              �ú���Ϊ������������
//
GCPROCESS void ReadCardSucceed ()
{
    // ��ֹ�����߳�
    ReadCardStop ();
    
    // ��ʾ�����ɹ�����
    switch (g_InterfaceID) {
        case 1:         // ��������
            // ��������
            PlaySound (SOUND_FAMILY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (3, true);
            break;
        case 4:         // һ��ͨ����
            // ��������
            PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (10, true);
            break;
        case 5:         // ʳƷ��Դ����
            // ��������
            PlaySound (SOUND_FOOD_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (13, true);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����ʧ��
// Parameters:  NULL
// Return:      void
// Remark:      �ú���ģ�����ʧ�ܣ�������Ӳ��������ʹ��
//              �ú���Ϊ������������
//
GCPROCESS void ReadCardFailed ()
{
    // ��ֹ�����߳�
    ReadCardStop ();

	// ��ʾ����ʧ�ܶ���
    switch (g_InterfaceID) {
        case 1:         // ��������
            // ��������
            PlaySound (SOUND_FAMILY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

	        g_InterfaceID = -1;
	        g_SmartAnim.ShowAnim (4, true);
            break;
        case 4:         // һ��ͨ����
            // ��������
            PlaySound (SOUND_SUBWAY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (11, true);
            break;
        case 5:         // ʳƷ��Դ����
            // ��������
            PlaySound (SOUND_FOOD_FAILED, NULL, SND_FILENAME | SND_ASYNC);

            g_InterfaceID = -1;
            g_SmartAnim.ShowAnim (14, true);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����������üӰ�ť
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void DateSettingAdd ()
{
    // ������������ֵ
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

    // ����ʱ��
    SetLocalTime (&g_DateSettingValue);
    GetLocalTime (&g_DateSettingValue);

    // ����������
    UpdateDateSettingItemAndValue ();

    // ˢ�½�����ʾ
    g_SmartAnim.ShowAnim (16, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����������ü���ť
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void DateSettingMinus ()
{
    // ������������ֵ
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

    // ����ʱ��
    SetLocalTime (&g_DateSettingValue);
    GetLocalTime (&g_DateSettingValue);

    // ����������
    UpdateDateSettingItemAndValue ();

    // ˢ�½�����ʾ
    g_SmartAnim.ShowAnim (16, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����������
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void EnterMainInterface ()
{
    // ���ֽ���ý���ǰ�Ľ���
	switch (g_InterfaceID) {
		case -1:					            // ��������
            // ˢ����������Ϣ
            UpdateTime ();
            UpdateTemperature ();

			// ��ʾ��������붯��
			g_SmartAnim.ShowAnim (0, true);
			break;
		case 1:						            // ��������
            // ��ֹ�����߳�
            ReadCardStop ();

			// �˳��������棬׼������������
			g_InterfaceID = -1;
            g_GoalInterfaceID = 0;
			g_SmartAnim.ShowAnim (2, false);
            break;
        case 2:                                 // ��ͥ���ƽ���
            // �رռ�ͥ���ƽ��������
            SetAirConditioner (false);
            g_sensorStatus.bAirConditioner = false;
            for (int i = 1; i <= 4; i++) {
                SetLight (i, false);
                g_sensorStatus.bLight[i - 1] = false;
            }

            // ��ʾ��ͥ���ƽ����˳�������׼������������
            g_InterfaceID = -1;
            g_GoalInterfaceID = 0;
            g_SmartAnim.ShowAnim (5, false);
            break;
        case 3:                                 // �ǻ�ũҵ����
            // �ر��ǻ�ũҵ���������
            SetSpray (false);
            SetFan (false);
            g_sensorStatus.bSpray = false;
            g_sensorStatus.bFan = false;

            // �˳��ǻ�ũҵ���棬׼������������
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (7, false);
            break;
        case 4:                                 // һ��ͨ����
            // ��ֹ�����߳�
            ReadCardStop ();

            // �˳�һ��ͨ���棬׼������������
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (9, false);
            break;
        case 5:                                 // ʳƷ��Դ����
            // ��ֹ�����߳�
            ReadCardStop ();

            // �˳�ʳƷ��Դ���棬׼������������
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (12, false);
            break;
        case 6:                                 // ���ý���
            // �˳����ý��棬׼������������
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (15, false);   
            break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �����������
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void EnterReadCardInterface ()
{
    // ���ֽ���ý���ǰ�Ľ���
    switch (g_InterfaceID) {
        case 0:                                 // ������
			// �˳������棬׼�������������
			g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (0, false);
            break;
        case 2:                                 // ��ͥ���ƽ���
            // �˳���ͥ���ƽ��棬׼�������������
            g_InterfaceID = -1;
            g_GoalInterfaceID = 1;
            g_SmartAnim.ShowAnim (5, false);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �����ǻ�ũҵ����
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void EnterFarmingInterface ()
{
    // �˳������棬׼�������ǻ�ũҵ����
	g_InterfaceID = -1;
    g_GoalInterfaceID = 3;
    g_SmartAnim.ShowAnim (0, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����һ��ͨ����
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void EnterSubwayInterface ()
{
    // �˳������棬׼������һ��ͨ����
	g_InterfaceID = -1;
    g_GoalInterfaceID = 4;
    g_SmartAnim.ShowAnim (0, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����ʳƷ��Դ����
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void EnterFoodInterface ()
{
    // �˳������棬׼������ʳƷ��Դ����
	g_InterfaceID = -1;
    g_GoalInterfaceID = 5;
    g_SmartAnim.ShowAnim (0, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �������ý���
// Parameters:  NULL
// Return:      void
// Remark:      �ú���Ϊ������������
//
GCPROCESS void EnterSettingsInterface ()
{
    // �˳������棬׼���������ý���
	g_InterfaceID = -1;
    g_GoalInterfaceID = 6;
    g_SmartAnim.ShowAnim (0, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ע����갴�´�����
// Parameters:  NULL
// Return:      �ɹ�����true��ʧ�ܷ���false��
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
// Function:    ��ʼRFID����
// Parameters:  NULL
// Return:      �ɹ����������̷߳���true�����򷵻�false��
// Remark:		�ú�������RFID�����߳�
//
bool ReadCardStart ()
{
    // ֹͣ�������Ķ����߳�
    ReadCardStop ();

    // ���������߳�
    g_bStopReadCardThread = false;
    if (!(g_hReadCardThread = CreateThread (NULL, 0, ThreadReadCard, NULL, 0, NULL)))
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ��ֹRFID����
// Parameters:  NULL
// Return:      NULL
// Remark:		�ú�����ֹRFID�����̣߳����ȴ��߳̽���
//
void ReadCardStop ()
{
    // ��ֹ�����߳�
    g_bStopReadCardThread = true;

    // �ȴ������߳̽���
    WaitForSingleObject (g_hReadCardThread, INFINITE);

    // �ͷ��߳̾��
    if (g_hReadCardThread) {
        CloseHandle (g_hReadCardThread);
        g_hReadCardThread = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    RFID�����߳�
// Parameters:  lpvoid  LPVOID  �̲߳���ָ��
// Return:      �����߳��˳���
// Remark:		���߳̽��ڵ���ReadCardStop����������һ�ſ�Ƭ���˳�
//              �����ڶ�ȡ��һ�ſ�Ƭ�����кź󣬻ᰴ�յ�ǰ�����Ҫ�������Ӧ�Ĳ�����
//
DWORD ThreadReadCard (LPVOID lpvoid)
{
    bool bSuccess;
    BYTE cardSize;
    BYTE pKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    BYTE blockBytes[16];

    // RFID����Ƶ
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
        ASSERTMSG (TEXT ("��Ƶģ������ʧ�ܣ������������豸��"), FALSE);
    }

    while (!g_bStopReadCardThread) {
        // ���Ƭ
        CARD_MODEL cardModel;
        if (!m104bpc_ActiveCard (PORT_NUMBER_M104BPC, true, &cardModel))
            continue;

        // ����ͻ����ȡ��Ƭ���кţ�
        BYTE cardNo[4];
        if (!m104bpc_Anticollision (PORT_NUMBER_M104BPC, cardNo))
            continue;

        switch (g_InterfaceID) {
            case 1:                         // �ǻۼ�ͥˢ������
                // �ж��Ƿ�Ϊ�ѵǼǿ���
                if (cardNo[0] == g_RFID[0] &&
                    cardNo[1] == g_RFID[1] &&
                    cardNo[2] == g_RFID[2] &&
                    cardNo[3] == g_RFID[3]) {
                    // ��������
                    PlaySound (SOUND_FAMILY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

	                // ��ʾ�����ɹ�����
	                g_InterfaceID = -1;
	                g_SmartAnim.ShowAnim (3, true);
                }
                else {
                    // ��������
                    PlaySound (SOUND_FAMILY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

                    // ��ʾ����ʧ�ܶ���
                    g_InterfaceID = -1;
                    g_SmartAnim.ShowAnim (4, true);
                }
                break;
            case 4:                         // һ��ͨ����
                bSuccess = false;

                // ����ȷ�Ŀ�
                if (cardNo[0] != g_RFID[0] ||
                    cardNo[1] != g_RFID[1] ||
                    cardNo[2] != g_RFID[2] ||
                    cardNo[3] != g_RFID[3])
                        goto SubWayEnd;

                // ѡ��
                if (!m104bpc_SelectCard (PORT_NUMBER_M104BPC, cardNo, &cardSize))
                    goto SubWayEnd;

                // ��֤����
                if (!m104bpc_CheckKey (PORT_NUMBER_M104BPC, KEY_A, 2, pKey))
                    goto SubWayEnd;
                    
                // ����
                if (!m104bpc_ReadBlock (PORT_NUMBER_M104BPC, 2, blockBytes)) 
                    goto SubWayEnd;
                
                if (g_bRechargeCard) {  // ��ֵ
                    // д��
                    blockBytes[0] = 100;
                    if (!m104bpc_WriteBlock (PORT_NUMBER_M104BPC, 2, blockBytes))
                        goto SubWayEnd;
                    
                    // ���½���
                    g_SmartAnim.ReplacePic (9, 2, "date_0.bmp");
                    g_SmartAnim.ReplacePic (10, 2, "date_0.bmp");
                    g_SmartAnim.ReplacePic (11, 2, "date_0.bmp");

                    CHAR szTemp[11] = "date_0.bmp";

                    // ��λ
                    g_SmartAnim.ReplacePic (9, 4, "date_1.bmp");
                    g_SmartAnim.ReplacePic (10, 4, "date_1.bmp");
                    g_SmartAnim.ReplacePic (11, 4, "date_1.bmp");

                    // ʮλ
                    g_SmartAnim.ReplacePic (9, 5, "date_0.bmp");
                    g_SmartAnim.ReplacePic (10, 5, "date_0.bmp");
                    g_SmartAnim.ReplacePic (11, 5, "date_0.bmp");

                    // ��λ
                    g_SmartAnim.ReplacePic (9, 6, "date_0.bmp");
                    g_SmartAnim.ReplacePic (10, 6, "date_0.bmp");
                    g_SmartAnim.ReplacePic (11, 6, "date_0.bmp");

                    // ��ֵ���
                    g_bRechargeCard = false;
                    bSuccess = true;

                    // ��������
                    PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

                    // ��ʾ�����ɹ�����
                    g_InterfaceID = -1;
                    g_SmartAnim.ShowAnim (10, true);
                }
                else {                  // ����
                    if (blockBytes[0] >= 2) {   // ����㹻
                        // �۳����
                        blockBytes[0] -= 2;
                        if (!m104bpc_WriteBlock (PORT_NUMBER_M104BPC, 2, blockBytes))
                            goto SubWayEnd;
                        
                        // ���½���
                        g_SmartAnim.ReplacePic (9, 2, "date_2.bmp");
                        g_SmartAnim.ReplacePic (10, 2, "date_2.bmp");
                        g_SmartAnim.ReplacePic (11, 2, "date_2.bmp");

                        int money[3];
                        money[0] = blockBytes[0] % 10;
                        money[1] = (blockBytes[0] / 10) % 10;
                        money[2] = (blockBytes[0] / 100) % 10;
                        CHAR szTemp[11] = "date_0.bmp";
                        if (blockBytes[0] < 10) {
                            // ��λ
                            szTemp[5] = 0x30 + money[0];
                            g_SmartAnim.ReplacePic (9, 4, szTemp);
                            g_SmartAnim.ReplacePic (10, 4, szTemp);
                            g_SmartAnim.ReplacePic (11, 4, szTemp);

                            // ����
                            g_SmartAnim.ReplacePic (9, 5, "null");
                            g_SmartAnim.ReplacePic (10, 5, "null");
                            g_SmartAnim.ReplacePic (11, 5, "null");
                            g_SmartAnim.ReplacePic (9, 6, "null");
                            g_SmartAnim.ReplacePic (10, 6, "null");
                            g_SmartAnim.ReplacePic (11, 6, "null");
                        }
                        else if (blockBytes[0] < 100) {
                            // ʮλ
                            szTemp[5] = 0x30 + money[1];
                            g_SmartAnim.ReplacePic (9, 4, szTemp);
                            g_SmartAnim.ReplacePic (10, 4, szTemp);
                            g_SmartAnim.ReplacePic (11, 4, szTemp);

                            // ��λ
                            szTemp[5] = 0x30 + money[0];
                            g_SmartAnim.ReplacePic (9, 5, szTemp);
                            g_SmartAnim.ReplacePic (10, 5, szTemp);
                            g_SmartAnim.ReplacePic (11, 5, szTemp);

                            // ����
                            g_SmartAnim.ReplacePic (9, 6, "null");
                            g_SmartAnim.ReplacePic (10, 6, "null");
                            g_SmartAnim.ReplacePic (11, 6, "null");
                        }
                        else {
                            // ��λ
                            szTemp[5] = 0x30 + money[2];
                            g_SmartAnim.ReplacePic (9, 4, szTemp);
                            g_SmartAnim.ReplacePic (10, 4, szTemp);
                            g_SmartAnim.ReplacePic (11, 4, szTemp);

                            // ʮλ
                            szTemp[5] = 0x30 + money[1];
                            g_SmartAnim.ReplacePic (9, 5, szTemp);
                            g_SmartAnim.ReplacePic (10, 5, szTemp);
                            g_SmartAnim.ReplacePic (11, 5, szTemp);

                            // ��λ
                            szTemp[5] = 0x30 + money[0];
                            g_SmartAnim.ReplacePic (9, 6, szTemp);
                            g_SmartAnim.ReplacePic (10, 6, szTemp);
                            g_SmartAnim.ReplacePic (11, 6, szTemp);
                        }

                        // �������
                        bSuccess = true;

                        // ��������
                        PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

                        // ��ʾ�����ɹ�����
                        g_InterfaceID = -1;
                        g_SmartAnim.ShowAnim (10, true);
                    }
                    else {                      // ����
                        // ���½���
                        g_SmartAnim.ReplacePic (9, 2, "date_0.bmp");
                        g_SmartAnim.ReplacePic (11, 2, "date_0.bmp");

                        CHAR szTemp[11] = "date_0.bmp";

                        // ��λ
                        szTemp[5] = 0x30 + blockBytes[0];
                        g_SmartAnim.ReplacePic (9, 4, szTemp);
                        g_SmartAnim.ReplacePic (11, 4, szTemp);

                        // ����
                        g_SmartAnim.ReplacePic (9, 5, "null");
                        g_SmartAnim.ReplacePic (11, 5, "null");
                        g_SmartAnim.ReplacePic (9, 6, "null");
                        g_SmartAnim.ReplacePic (11, 6, "null");
                    }
                }
SubWayEnd:
                if (!bSuccess) {
                    // ��������
                    PlaySound (SOUND_SUBWAY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

                    // ��ʾ����ʧ�ܶ���
                    g_InterfaceID = -1;
	                g_SmartAnim.ShowAnim (11, true);
                }

                break;
            case 5:                         // ʳƷ��Դ����
                bSuccess = false;

                // �ж��Ƿ�Ϊ�ѵǼǿ���
                if (cardNo[0] == g_RFID[0] &&
                    cardNo[1] == g_RFID[1] &&
                    cardNo[2] == g_RFID[2] &&
                    cardNo[3] == g_RFID[3]) {
                    // ��������
                    PlaySound (SOUND_FOOD_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);

	                // ��ʾ�����ɹ�����
	                g_InterfaceID = -1;
	                g_SmartAnim.ShowAnim (13, true);
                }
                else {
                    // ��������
                    PlaySound (SOUND_FOOD_FAILED, NULL, SND_FILENAME | SND_ASYNC);

                    // ��ʾ����ʧ�ܶ���
                    g_InterfaceID = -1;
                    g_SmartAnim.ShowAnim (14, true);
                }

                break;
        }

        // ������һ�ſ�Ƭ���˳��߳�
        break;
    }

    // RFID����Ƶ
    m104bpc_RFCtrl (PORT_NUMBER_M104BPC, false);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �����ͥ���ƽ���
// Parameters:  NULL
// Return:      void
//
void EnterHomeCtrlInterface ()
{
    // ��ʾ�����ɹ������˳�����
    g_InterfaceID = -1;
    g_GoalInterfaceID = 2;
    g_SmartAnim.ShowAnim (3, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ѽ���������
// Parameters:  NULL
// Return:      void
//
void MainInterfaceEntered ()
{
	// ���ý���ID
    g_InterfaceID = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ѽ����������
// Parameters:  NULL
// Return:      void
//
void ReadCardInterfaceEntered ()
{
	// ���ý���ID
	g_InterfaceID = 1;

    // ��ʼѭ������
    ReadCardStart ();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ѽ����ͥ���ƽ���
// Parameters:  NULL
// Return:      void
//
void HomeControlInterfaceEntered ()
{
    // ���ý���ID
    g_InterfaceID = 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ѽ����ǻ�ũҵ����
// Parameters:  NULL
// Return:      void
//
void FarmingInterfaceEntered ()
{
    // ���ý���ID
    g_InterfaceID = 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ѽ���һ��ͨ����
// Parameters:  NULL
// Return:      void
//
void SubwayInterfaceEntered ()
{
    // ���ý���ID
    g_InterfaceID = 4;

    // ��ʼѭ������
    ReadCardStart ();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ѽ���ʳƷ��Դ����
// Parameters:  NULL
// Return:      void
//
void FoodInterfaceEntered ()
{
    // ���ý���ID
    g_InterfaceID = 5;

    // ��ʼѭ������
    ReadCardStart ();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    �ѽ���ʳƷ��Դ����
// Parameters:  NULL
// Return:      void
//
void SettingsInterfaceEntered ()
{
    // ���ý���ID
    g_InterfaceID = 6;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����ˢ�¶�ʱ��������
// Parameters:  hWnd    HWND    �붨ʱ�����ӵĴ�����
//              uMsg    UINT    WM_TIMER��Ϣ
//              idEvent UINT    ��ʱ���¼����
//              dwTimer DOWRD   ����ϵͳ��������ĺ�����
// Return:      void
//
void CALLBACK TimerProcess_Refresh (HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    // �ϴ����������ݵ��豸������
    UploadTemperature ();
    UploadHumidity ();
    UploadDayAndNight ();

    switch (g_InterfaceID) {
        case 0:         // ������
            UpdateTemperature ();
            UpdateTime ();
            g_SmartAnim.ShowAnim (1, true);
            break;
        case 2:         // ��ͥ���ƽ���
            UpdateTemperature ();
            UpdateDayAndNight ();
            g_SmartAnim.ShowAnim (6, true);
            break;
        case 3:         // �ǻ�ũҵ����
            UpdateTemperature ();
            UpdateHumidity ();
            g_SmartAnim.ShowAnim (8, true);
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ������ɻص�����
// Parameters:  animID  int     ��ɵĶ���ID
//              obverse bool    ָʾ��ɵĶ����Ƿ���˳�ŵ�
// Return:      void
//
void AnimEndProcess (int animID, bool obverse)
{
	// �������
    switch (animID) {
        case 0:                 // ���������涯��
            if (obverse) {
                // ����������
                MainInterfaceEntered ();
            }
            else {
                // �������˳�
                switch (g_GoalInterfaceID) {
                    case 1:
                        // ��ʾ����������붯��
                        g_SmartAnim.ShowAnim (2, true);
                        break;
                    case 3:
                        // ˢ���ǻ�ũҵ������Ϣ
                        //UpdateTemperature ();
                        UpdateHumidity ();
                        UpdateHumiditySettingValue ();
                        //SetSpray (false);
                        //SetFan (false);

                        // ��ʾ�ǻ�ũҵ���붯��
                        g_SmartAnim.ShowAnim (7, true);
                        break;
                    case 4:
                        // ��ս����Ϣ
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

                        // ��ʾһ��ͨ���붯��
                        g_SmartAnim.ShowAnim (9, true);
                        break;
                    case 5:
                        // �����Ϣ
                        g_SmartAnim.ReplacePic (12, 0, "food_picture.bmp");
                        g_SmartAnim.ReplacePic (12, 1, "food_read_card.bmp");

                        // ��ʾʳƷ��Դ���붯��
                        g_SmartAnim.ShowAnim (12, true);
                        break;
                    case 6:
                        // ˢ�����ý���ʱ���������ֵ
                        g_DateSettingItem = YEAR;
                        GetLocalTime (&g_DateSettingValue);
                        UpdateDateSettingItemAndValue ();
                        
                        // ��ʾ���ý��붯��
                        g_SmartAnim.ShowAnim (15, true);
                        break;
                }
            }
            break;
        case 2:                 // ����������涯��
            if (obverse) {
                // �ѽ����������
                ReadCardInterfaceEntered ();
            }
            else {
                // ���������˳�
                switch (g_GoalInterfaceID) {
                    case 0:
                        // ˢ����������Ϣ
                        UpdateTemperature ();
                        UpdateTime ();

                        // ��ʾ��������붯��
                        g_SmartAnim.ShowAnim (0, true);
                        break;
                    case 2:
                        // ˢ�¼�ͥ���ƽ�����Ϣ
                        UpdateTemperature ();
                        UpdateDayAndNight ();
                        //SetAirConditioner (false);
                        //for (int i = 1; i <= 4; i++)
                        //    SetLight (i, false);

                        // ��ʾ��ͥ���ƽ�����붯��
                        g_SmartAnim.ShowAnim (5, true);
                        break;
                }
            }
            break;
		case 3:                 // �����ɹ�����
			if (obverse) {
                // �����ɹ�
				Sleep (500);
				
                // �����ͥ���ƽ���
                EnterHomeCtrlInterface ();
			}
            else {
                // ��ʾ���������˳�����
                g_SmartAnim.ShowAnim (2, false);
            }
			break;
		case 4:                 // ����ʧ�ܶ���
			if (obverse) {
                // ����ʧ��
				Sleep (500);

                // ��ʾ����ʧ���˳�����
				g_SmartAnim.ShowAnim (4, false);
			}
            else {
                g_InterfaceID = 1;

                // ��ʼѭ������
                ReadCardStart ();
            }
			break;
        case 5:                 // ��ͥ���ƽ��涯��
            if (obverse)
                // �ѽ����ͥ���ƽ���
                HomeControlInterfaceEntered ();
            else {
                switch (g_GoalInterfaceID) {
                    case 0:
                        // ˢ����������Ϣ
                        UpdateTemperature ();
                        UpdateTime ();

                        // ��ʾ��������붯��
                        g_SmartAnim.ShowAnim (0, true);
                        break;
                    case 1:
                        // ��ʾ����������붯��
                        g_SmartAnim.ShowAnim (2, true);
                        break;
                }
            }
            break;
        case 7:                 // �ǻ�ũҵ���涯��
            if (obverse)
                // �ѽ����ǻ�ũҵ����
                FarmingInterfaceEntered ();
            else {
                // ˢ����������Ϣ
                UpdateTemperature ();
                UpdateTime ();

                // ��ʾ��������붯��
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
        case 9:                 // һ��ͨ���涯��
            if (obverse)
                // �ѽ���һ��ͨ����
                SubwayInterfaceEntered ();
            else {
                // ˢ����������Ϣ
                UpdateTemperature ();
                UpdateTime ();

                // ��ʾ��������붯��
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
        case 10:                // һ��ͨ�����ɹ�����
            g_InterfaceID = 4;

            // ��ʼѭ������
            ReadCardStart ();

            break;
        case 11:                // һ��ͨ����ʧ�ܶ���
            g_InterfaceID = 4;

            // ��ʼѭ������
            ReadCardStart ();

            break;
        case 12:                // ʳƷ��Դ���涯��
            if (obverse)
                // �ѽ���ʳƷ��Դ����
                FoodInterfaceEntered ();
            else {
                // ˢ����������Ϣ
                UpdateTemperature ();
                UpdateTime ();

                // ��ʾ��������붯��
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
        case 13:                // ʳƷ��Դ�����ɹ�����
            g_InterfaceID = 5;

            // ��ʼѭ������
            ReadCardStart ();

            break;
        case 14:                // ʳƷ��Դ����ʧ�ܶ���
            g_InterfaceID = 5;

            // ��ʼѭ������
            ReadCardStart ();

            break;
        case 15:                // ���ý��涯��
            if (obverse)
                // �ѽ������ý���
                SettingsInterfaceEntered ();
            else {
                // ˢ����������Ϣ
                UpdateTemperature ();
                UpdateTime ();

                // ��ʾ��������붯��
                g_SmartAnim.ShowAnim (0, true);
            }
            break;
    }
}