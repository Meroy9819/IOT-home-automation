// s2Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "s2.h"
#include "CECommCtrl.h"
#include "s2Dlg.h"
#include "SerialPort.h"
#include "stdafx.h"
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
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID_REFRESH 8000           // ����ˢ�¼�ʱ��ID
#define SEND_DATA_TIMES 3               // �������ݵĴ��������践��ֵ���޷���ѯ��ָ�Ϊȷ���������������͵��ظ�������

#define PORT_NUMBER_COORDINATOR 2       // ������Э�����˿�
#define BAUND_RATE_COORDINATOR 115200   // ������Э����������

#define PORT_NUMBER_M104BPC 3           // M104BPC�˿�
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

// Cs2Dlg �Ի���

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


// Cs2Dlg ��Ϣ�������
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
	m_font.CreatePointFont(150, _T("΢���ź�"));
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
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
	//����ͼƬID
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
	SetTimer(1,1000,NULL);// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetTimer(2,1000,NULL);
	SetTimer(3,1000,NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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



//ʮ�붨ʱ
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

//ʱ��




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
int balance;
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
    // ��������
    PlaySound (SOUND_FAMILY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);
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

            // ��������
    PlaySound (SOUND_FAMILY_FAILED, NULL, SND_FILENAME | SND_ASYNC);

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

            g_sensorStatus.bAirConditioner = false;
            for (int i = 1; i <= 4; i++) {
    
                g_sensorStatus.bLight[i - 1] = false;
            }

            // ��ʾ��ͥ���ƽ����˳�������׼������������
            g_InterfaceID = -1;
            g_GoalInterfaceID = 0;
            g_SmartAnim.ShowAnim (5, false);
            break;
        case 3:                                 // �ǻ�ũҵ����
            // �ر��ǻ�ũҵ���������
        

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
bool bSuccess;
DWORD ThreadReadCard (LPVOID lpvoid)
{
    BYTE cardSize;
    BYTE pKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    // RFID����Ƶ
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
					printf("g recharge %d\n",g_bRechargeCard);
                    blockBytes[0] = 100;
					if (!m104bpc_WriteBlock (PORT_NUMBER_M104BPC, 2, blockBytes))
                        goto SubWayEnd;

   
                    
        
                    // ��ֵ���
                    g_bRechargeCard = false;
                    bSuccess = true;


                    // ��������
                    PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);
					 g_InterfaceID = -1;
                }
                else {                  // ����
                    if (blockBytes[0] >= 1) {   // ����㹻
                        // �۳����
                        blockBytes[0] -= 1;
				
						//showbalance(blockBytes[0]);
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
 

    switch (g_InterfaceID) {
        case 0:         // ������
  
            g_SmartAnim.ShowAnim (1, true);
            break;
        case 2:         // ��ͥ���ƽ���
  
            g_SmartAnim.ShowAnim (6, true);
            break;
        case 3:         // �ǻ�ũҵ����
   
            g_SmartAnim.ShowAnim (8, true);
            break;
    }
}



void Cs2Dlg::OnBnClickedButton8()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR);
	g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR);
	LoadServerAddr (SERVER_ADDR_FILE);
	m104bpc_OpenPort (PORT_NUMBER_M104BPC, BAUND_RATE_M104BPC);
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
	g_CommCtrl.OpenSerialPort (PORT_NUMBER_COORDINATOR, BAUND_RATE_COORDINATOR);
	RegisterGeniusCtrlProcess(); 
	LoadRFID (RFID_COF_FILE);
	g_InterfaceID=4;
	ReadCardStart ();

}

void Cs2Dlg::OnBnClickedButton9()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CardRecharge ();
	g_InterfaceID=4;
	ReadCardStart ();
	showbalance();
	printf("%d\n",blockBytes[0]);
	PlaySound (SOUND_SUBWAY_SUCCEED, NULL, SND_FILENAME | SND_ASYNC);
}


void Cs2Dlg::OnBnClickedButton11()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�򿪴���ͨ��

	//�Զ�����
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

	//�Զ���ʪ
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
	//���ص�
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
	aut = 0;	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	SetTimer(1,1000,NULL);// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetTimer(2,1000,NULL);
	SetTimer(3,1000,NULL);
}

void Cs2Dlg::OnBnClickedButton7()
{
	g_InterfaceID=1;
	showbalance();
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

	}// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialog::OnTimer(nIDEvent);
}

void Cs2Dlg::OnBnClickedButton4()
{
	LED1=LED2=true;
	
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
	Sleep(500);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
	Sleep(500);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}

void Cs2Dlg::OnBnClickedButton5()
{
	LED1=LED2=false;
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,1,LED1);
	Sleep(500);
	g_CommCtrl.SetLED(PORT_NUMBER_COORDINATOR,2,LED2);
	Sleep(500);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		s = "�������ȹر��Զ����ƣ�";
		GetDlgItem(IDC_STATIC30)->SetWindowText(s);
		//GetDlgItem(IDC_STATIC30)->s
		//MessageBox(_T("�Զ����Ƴ�ͻ��ϵͳ�Զ��ر��Զ����ƣ�"));
		//Dlg2
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		//s = "�������ȹر��Զ����ƣ�";
		//GetDlgItem(IDC_STATIC30)->SetWindowText(s);
		//MessageBox(_T("�Զ����Ƴ�ͻ��ϵͳ�Զ��ر��Զ����ƣ�"));
		//Dlg2
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}
