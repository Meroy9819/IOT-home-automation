#include "stdafx.h"
#include "M104BPC.h"

// ����SerialPort��
#include "SerialPort.h"
#pragma comment (lib, "SerialPort")

#define MAX_RECEIVE_DATA_LEN 256            // �����ȡ�����ֽ���
#define MAX_SEND_DATA_LEN 256               // ����������ֽ���
#define TIMEOUT_NORMAL 500                  // һ��ָ�ʱʱ��

// ǰ������
BOOL SendData (DWORD dwPortNumber, PBYTE pSendData, DWORD dwSendDataLen, DWORD dwTimeout);
BOOL ProcessByte (BYTE byte, DWORD dwPortNumber);
void MakeSendData (BYTE *pSendData, int *pnSendLen);

int g_nReceiveStatus = 0;                   // �������ݴ���״̬
int g_nReceiveDataLen = 0;                  // �������ݳ���
BYTE g_ReceiveData[MAX_RECEIVE_DATA_LEN];   // �������ݣ�ִ�н�� + ������

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    �򿪶˿�
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              dwBaudRate      DWORD   [IN]    ���ڲ�����
// Return:      �򿪴��ڳɹ�����TRUE�����򷵻�FALSE��
//              ����GetLastError��ȡ��ϸ�������
// Remark:      ����˿��Ѿ��򿪣���ֱ�ӷ���TRUE
//
M104BPC_API BOOL WINAPI m104bpc_OpenPort (DWORD dwPortNumber, DWORD dwBaudRate)
{
    return SP_Open (dwPortNumber, dwBaudRate, 0);
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    �رն˿�
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
// Remark:      ����˿��ѹرգ���ֱ�ӷ���TRUE��
//
M104BPC_API BOOL WINAPI m104bpc_ClosePort (DWORD dwPortNumber)
{
    return SP_Close (dwPortNumber, 0);
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ��Ƶ����
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              open            BOOL    [IN]    ָʾ�򿪻�ر���Ƶ
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_RFCtrl (DWORD dwPortNumber, BOOL open)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x05, open ? 0x01 : 0x00 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0]) {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
    else
        return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ����ģ�鹤����TypeAģʽ
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_SetTypeA (DWORD dwPortNumber)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x3A, 0x41 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0]) {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
    else
        return TRUE;
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ���Ƭ
// Parameters:  dwPortNumber    DWORD   [IN]        ���ںţ���Χ1-255
//              activeAll       BOOL    [IN]        ָʾֻ����δ����˯��״̬�Ŀ�
//                                                  ���Ǽ������п�
//              pCardModel      CARD_MODEL  [OUT]   ���ؿ�Ƭ�ͺ�
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_ActiveCard (DWORD dwPortNumber, BOOL activeAll, CARD_MODEL *pCardModel)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x46, activeAll ? 0x52 : 0x26 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0] == 0) {
        *pCardModel = (CARD_MODEL)g_ReceiveData[1];
        return TRUE;
    }
    else {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ����ͻ
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              pCardNo         BYTE*   [OUT]   ����4�ֽڿ��ţ��û�����4�ֽڿռ�
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_Anticollision (DWORD dwPortNumber, BYTE *pCardNo)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x47, 0x04 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0] == 0) {
        memcpy (pCardNo, g_ReceiveData + 1, 4);
        return TRUE;
    }
    else {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ѡ��
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              pCardNo         BYTE*   [IN]    ѡ������
//              pCardSize       BYTE*   [OUT]   ���ؿ�Ƭ�������û����ٿռ�
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_SelectCard (DWORD dwPortNumber, BYTE *pCardNo, BYTE *pCardSize)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x48, pCardNo[0], pCardNo[1], pCardNo[2], pCardNo[3] };
    int nSendLen = 5;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0] == 0) {
        *pCardSize = g_ReceiveData[1];
        return TRUE;
    }
    else {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ��֤��Կ
// Parameters:  dwPortNumber    DWORD       [IN]    ���ںţ���Χ1-255
//              keyModel        KEY_MODEL   [IN]    ��Կģʽ������ΪKEY_A
//              blockIndex      BYTE        [IN]    ���Կ�š�S50��: 0x00-0x3F(64��) S70��: 0x00-0xFF(256��)
//              pKey            BYTE*       [IN]    6�ֽ�������Կ������Ϊ0xFFFFFFFFFFFF
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_CheckKey (DWORD dwPortNumber, KEY_MODEL keyModel, BYTE blockIndex, BYTE *pKey)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x4A, (BYTE)keyModel, blockIndex, pKey[0], pKey[1], pKey[2], pKey[3], pKey[4], pKey[5] };
    int nSendLen = 9;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0] == 0)     
        return TRUE;
    else {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ����
// Parameters:  dwPortNumber    DWORD       [IN]    ���ںţ���Χ1-255
//              blockIndex      BYTE        [IN]    ���Կ�š�S50��: 0x00-0x3F(64��) S70��: 0x00-0xFF(256��)
//              pData           BYTE*       [OUT]   ���ؿ����ݣ��û�����16�ֽڿռ�
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_ReadBlock (DWORD dwPortNumber, BYTE blockIndex, BYTE *pData)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x4B, blockIndex };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0] == 0) {
        memcpy (pData, g_ReceiveData + 1, 16);
        return TRUE;
    }
    else {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    д��
// Parameters:  dwPortNumber    DWORD       [IN]    ���ںţ���Χ1-255
//              blockIndex      BYTE        [IN]    ���Կ�š�S50��: 0x00-0x3F(64��) S70��: 0x00-0xFF(256��)
//              pData           BYTE*       [IN]    16�ֽڴ�д�������
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
M104BPC_API BOOL WINAPI m104bpc_WriteBlock (DWORD dwPortNumber, BYTE blockIndex, BYTE *pData)
{
    // ׼������������
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x4C, blockIndex };
    memcpy (pSendData + 2, pData, 16);
    int nSendLen = 18;
    MakeSendData (pSendData, &nSendLen);

    // ��������
    if (!SendData (dwPortNumber, pSendData, nSendLen, TIMEOUT_NORMAL))
        return FALSE;

    if (g_ReceiveData[0] == 0)
        return TRUE;
    else {
        SetLastError (MODEL_ERROR_PREFIX + g_ReceiveData[0]);
        return FALSE;
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ׼�������͵�����
// Parameters:  pSendData       BYTE*   [IN/OUT]    ����ָ���е� �����֣�������
//                                                  ���������Ĵ���������
//              pnSendLen       int*    [IN/OUT]    ����ָ���е� �����֣������� ����
//                                                  ������������ݳ���
// Return:      NULL
//
void MakeSendData (BYTE *pSendData, int *pnSendLen)
{
    BYTE pSendDataTemp[MAX_SEND_DATA_LEN] = { 0x02, 0x00, 0x00, *pnSendLen + 2 };
    BYTE checkSum = *pnSendLen + 2;

    // ������֯����
    int nIndex = 4;
    for (int i = 0; i < *pnSendLen; i++) {
        if (pSendData[i] == 0x02 || pSendData[i] == 0x03 || pSendData[i] == 0x10)
            pSendDataTemp[nIndex++] = 0x10;
        pSendDataTemp[nIndex++] = pSendData[i];
        checkSum += pSendData[i];
    }

    // ���У��ͼ�֡β
    pSendDataTemp[nIndex++] = checkSum;
    pSendDataTemp[nIndex++] = 0x03;

    // ��������
    memcpy (pSendData, pSendDataTemp, nIndex);
    *pnSendLen = nIndex;
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    ��������
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              pSendData       PBYTE   [IN]    �����͵�����
//              dwSendDataLen   DWORD   [IN]    ���������ݳ���
//              dwTimeout       DWORD   [IN]    ���ͼ��������ݳ�ʱʱ�䣬��λms
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
BOOL SendData (DWORD dwPortNumber, PBYTE pSendData, DWORD dwSendDataLen, DWORD dwTimeout)
{
    g_nReceiveStatus = 0;
    return SP_Write_Read (dwPortNumber, ProcessByte, pSendData, dwSendDataLen, dwTimeout);
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    �����յ��ֽڵĻص�����
// Parameters:  byte            BYTE    �յ����ֽ�
//              dwPortNumber    DWORD   �յ����ֽڵĴ��ںţ���Χ1-255
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
BOOL ProcessByte (BYTE byte, DWORD dwPortNumber)
{
    static bool escape = false;     // �Ƿ���ֹ�ת���
    static BYTE checkSum = 0;       // У���
    static int dataReceived = 0;    // �ѽ��յ����ݳ��ȣ�ִ�н�� + ������

    // �����ַ�ת��
    switch (byte) {
        case 0x02:
            if (escape)
                escape = false;
            else {
                // ֡ͷ
                g_nReceiveStatus = 1;
                g_nReceiveDataLen = 0;
                checkSum = 0;
                dataReceived = 0;
                return false;
            }
            break;
        case 0x03:
            if (escape)
                escape = false;
            break;
        case 0x10:
            if (escape)
                escape = false;
            else {
                escape = true;
                return false;
            }
            break;
    }

    // �����������
    switch (g_nReceiveStatus) {
        case 0:     // �ȴ�֡ͷ
            break;
        case 1:     // ģ���ַ�����ԣ�
        case 2:
            checkSum += byte;
            g_nReceiveStatus++;
            break;
        case 3:     // ����
            g_nReceiveDataLen = byte - 2;
            checkSum += byte;
            g_nReceiveStatus++;
            break;
        case 4:     // ������ԣ�
            checkSum += byte;
            g_nReceiveStatus++;
            break;
        case 5:     // ���ݣ�ִ�н�� + ������
            g_ReceiveData[dataReceived++] = byte;
            checkSum += byte;

            if (dataReceived >= g_nReceiveDataLen)
                g_nReceiveStatus++;
            break;
        case 6:     // У��
            if (checkSum == byte)
                g_nReceiveStatus++;
            else
                g_nReceiveStatus = 0;
            break;
        case 7:     // ֡β
            if (byte == 0x03)
                return TRUE;
            else
                g_nReceiveStatus = 0;
            break;
        default:
            g_nReceiveStatus = 0;
    }

    return FALSE;
}