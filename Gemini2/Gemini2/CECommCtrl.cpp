#include "StdAfx.h"
#include "CECommCtrl.h"

#include "SerialPort.h"
#pragma comment (lib, "SerialPort")

#define RECEIVE_TIMEOUT_NORMAL 500

BYTE CCommCtrl::m_ReceiveBuffer_Gemini2[RECEIVE_DATA_LEN_GEMINI2];
int CCommCtrl::m_ReceiveStatus_Gemini2;

CCommCtrl::CCommCtrl(void)
{
}

CCommCtrl::~CCommCtrl(void)
{
}

bool CCommCtrl::OpenSerialPort (DWORD dwPortNumber, DWORD dwBaudRate)
{
    if (SP_Open (dwPortNumber, dwBaudRate, 0))
        return true;
    else
        return false;
}

bool CCommCtrl::CloseSerialPort (DWORD dwPortNumber)
{
    if (SP_Close (dwPortNumber, 0))
        return true;
    else
        return false;
}

bool CCommCtrl::SetLED (DWORD dwPortNumber, int ledID, bool open)
{
    // prepare BYTE sendData[ ]
    int sendDataLen = 7;
    BYTE sendData[7] = { 0xAA, 0x01, 0x01, open ? 0x01 : 0x02, 0x00, (BYTE)ledID, 0x55 };

    return SendData_Gemini2 (dwPortNumber, sendData, sendDataLen, false, RECEIVE_TIMEOUT_NORMAL);
}

bool CCommCtrl::SetMotor (DWORD dwPortNumber, MOTOR_STATUS motorStatus)
{
    // todo: prepare BYTE sendData[ ]

	int sendDataLen = 7;
    BYTE sendData[7] = { 0xAA, 0x01, 0x02, (BYTE)(motorStatus) , 0x00, 0x00 , 0x55 };

    return SendData_Gemini2 (dwPortNumber, sendData, sendDataLen, false, RECEIVE_TIMEOUT_NORMAL);
}

bool CCommCtrl::GetTemperature (DWORD dwPortNumber, int *temperature)
{
    // todo: prepare BYTE sendData[ ]

	int sendDataLen = 7;
    BYTE sendData[7] = { 0xAA, 0x01, 0x03, 0x01 , 0x00, 0x00 , 0x55 };

    if (!SendData_Gemini2 (dwPortNumber, sendData, sendDataLen, true, RECEIVE_TIMEOUT_NORMAL))
        return false;

    // 检查返回值
    if (m_ReceiveBuffer_Gemini2[0] != 0x01 || m_ReceiveBuffer_Gemini2[1] != 0x03 || m_ReceiveBuffer_Gemini2[2] != 0x01)
        return false;

    if (m_ReceiveBuffer_Gemini2[3] == 0x00 && m_ReceiveBuffer_Gemini2[4] == 0x01)
        return false;

    // 计算温度值
    *temperature = (m_ReceiveBuffer_Gemini2[3] << 8) + m_ReceiveBuffer_Gemini2[4];

    return true;
}

bool CCommCtrl::GetHumidity (DWORD dwPortNumber, int *humidityPercent)
{
    // todo: prepare BYTE sendData[ ]
	int sendDataLen = 7;
    BYTE sendData[7] = { 0xAA, 0x01, 0x03, 0x02 , 0x00, 0x00 , 0x55 };


    if (!SendData_Gemini2 (dwPortNumber, sendData, sendDataLen, true, RECEIVE_TIMEOUT_NORMAL))
        return false;

    // 检查返回值
    if (m_ReceiveBuffer_Gemini2[0] != 0x01 || m_ReceiveBuffer_Gemini2[1] != 0x03 || m_ReceiveBuffer_Gemini2[2] != 0x02)
        return false;

    if (m_ReceiveBuffer_Gemini2[3] == 0x00 && m_ReceiveBuffer_Gemini2[4] == 0x01)
        return false;

    // 计算湿度值
    *humidityPercent = (m_ReceiveBuffer_Gemini2[3] << 8) + m_ReceiveBuffer_Gemini2[4];

    return true;
}

bool CCommCtrl::GetLight (DWORD dwPortNumber, bool *bright)
{
    // todo: prepare BYTE sendData[ ]
	int sendDataLen = 7;
    BYTE sendData[7] = { 0xAA, 0x01, 0x04,  0x01 , 0x00, 0x00, 0x55 };


    if (!SendData_Gemini2 (dwPortNumber, sendData, sendDataLen, true, RECEIVE_TIMEOUT_NORMAL))
        return false;

    // 检查返回值
    if (m_ReceiveBuffer_Gemini2[0] != 0x01 || m_ReceiveBuffer_Gemini2[1] != 0x04 || m_ReceiveBuffer_Gemini2[2] != 0x01 || m_ReceiveBuffer_Gemini2[3] != 0x00)
        return false;

    // 保存光敏电阻状态
    if (m_ReceiveBuffer_Gemini2[4] == 0x00)
        *bright = false;
    else if (m_ReceiveBuffer_Gemini2[4] == 0x01)
        *bright = true;
    else
        return false;

    return true;
}

bool CCommCtrl::SendData_Gemini2 (DWORD dwPortNumber, PBYTE pSendData, int nSendDataLen, bool requestAnswer, DWORD dwTimeout)
{
    DWORD dwErr = 0;
    HANDLE hSendDataMutex = NULL;
    bool bRet = false;

    __try
    {        
        // 建立发送接收数据互斥体
        if (hSendDataMutex = CreatePrivateMutex (TEXT ("MUTEX_SEND_RECEIVE_SP_GEMINI2"), dwPortNumber))
        {
            switch (WaitForSingleObject (hSendDataMutex, INFINITE))
            {
            case WAIT_OBJECT_0:
                if (requestAnswer) {
                    m_ReceiveStatus_Gemini2 = 0;
                    if (SP_Write_Read (dwPortNumber, ProcessByte_Gemini2, pSendData, nSendDataLen, dwTimeout))
                        bRet = true;
                }
                else {
                    if (SP_Write (dwPortNumber, pSendData, nSendDataLen, dwTimeout))
                        bRet = true;
                }
                break;
            case WAIT_ABANDONED:
                dwErr = ERROR_WAIT_MUTEX;
                break;
            default:
                dwErr = GetLastError ();
            }
        }
        else
            dwErr = GetLastError ();
    }
    __finally
    {
        if (hSendDataMutex)
        {
            ReleaseMutex (hSendDataMutex);
            CloseHandle (hSendDataMutex);
        }
    }

    if (dwErr)
        SetLastError (dwErr);

    // 数据发送/接收成功，等待一个时间，避开协调器通信时间
    if (bRet)
        Sleep (100);

    return bRet;
}

BOOL CCommCtrl::ProcessByte_Gemini2 (BYTE byte, DWORD dwPortNumber)
{
	// process a frame and save data to m_ReceiveBuffer_Gemini2
    switch (m_ReceiveStatus_Gemini2) {
        case 0:     // 帧头
            if (byte == 0x55)
                m_ReceiveStatus_Gemini2++;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:     // 数据
            m_ReceiveBuffer_Gemini2[m_ReceiveStatus_Gemini2 - 1] = byte;
            m_ReceiveStatus_Gemini2++;
            break;
        case 6:     // 帧尾
            if (byte == 0xAA)
                return TRUE;
            else
                m_ReceiveStatus_Gemini2 = 0;
            break;
        default:
            m_ReceiveStatus_Gemini2 = 0;
    }

    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    建立包含特定索引的互斥体
// Parameters:  lpNamePrefix    LPCTSTR [IN]    NULL结尾字符串。如果dwIndex值为0，互斥体名即为lpNamePrefix，
//                                              否则互斥体名为lpNamePrefix + _ + dwIndex的字符串表示。
//              dwIndex         DWORD   [IN]    该函数建立的互斥体名将与该索引相关，详见lpNamePrefix参数说明。
// Return:      成功返回Mutex句柄，失败返回NULL。详细错误信息请调用GetLastError
//
HANDLE CCommCtrl::CreatePrivateMutex (LPCTSTR lpNamePrefix, DWORD dwIndex)
{
    HANDLE hMutex;
    TCHAR mutexName[MAX_PATH];

    // 生成互斥体名
    if (dwIndex)
    {
        if (_stprintf (mutexName, TEXT ("%s_%i"), lpNamePrefix, dwIndex) == 0)
            return NULL;
    }
    else
        if (_tcscpy_s (mutexName, MAX_PATH, lpNamePrefix) != 0)
            return NULL;

    // 打开或建立互斥体
    if (!(hMutex = CreateMutex (NULL, FALSE, mutexName)))
        return NULL;

    return hMutex;
}