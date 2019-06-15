#include "stdafx.h"
#include "M104BPC.h"

// 引入SerialPort库
#include "SerialPort.h"
#pragma comment (lib, "SerialPort")

#define MAX_RECEIVE_DATA_LEN 256            // 最大收取数据字节数
#define MAX_SEND_DATA_LEN 256               // 最大发送数据字节数
#define TIMEOUT_NORMAL 500                  // 一般指令超时时间

// 前向声明
BOOL SendData (DWORD dwPortNumber, PBYTE pSendData, DWORD dwSendDataLen, DWORD dwTimeout);
BOOL ProcessByte (BYTE byte, DWORD dwPortNumber);
void MakeSendData (BYTE *pSendData, int *pnSendLen);

int g_nReceiveStatus = 0;                   // 返回数据处理状态
int g_nReceiveDataLen = 0;                  // 返回数据长度
BYTE g_ReceiveData[MAX_RECEIVE_DATA_LEN];   // 返回数据（执行结果 + 数据域）

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
// Function:    打开端口
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              dwBaudRate      DWORD   [IN]    串口波特率
// Return:      打开串口成功返回TRUE，否则返回FALSE。
//              调用GetLastError获取详细错误代码
// Remark:      如果端口已经打开，则直接返回TRUE
//
M104BPC_API BOOL WINAPI m104bpc_OpenPort (DWORD dwPortNumber, DWORD dwBaudRate)
{
    return SP_Open (dwPortNumber, dwBaudRate, 0);
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    关闭端口
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
// Remark:      如果端口已关闭，则直接返回TRUE。
//
M104BPC_API BOOL WINAPI m104bpc_ClosePort (DWORD dwPortNumber)
{
    return SP_Close (dwPortNumber, 0);
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    射频控制
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              open            BOOL    [IN]    指示打开或关闭射频
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_RFCtrl (DWORD dwPortNumber, BOOL open)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x05, open ? 0x01 : 0x00 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    设置模块工作于TypeA模式
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_SetTypeA (DWORD dwPortNumber)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x3A, 0x41 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    激活卡片
// Parameters:  dwPortNumber    DWORD   [IN]        串口号，范围1-255
//              activeAll       BOOL    [IN]        指示只激活未进入睡眠状态的卡
//                                                  还是激活所有卡
//              pCardModel      CARD_MODEL  [OUT]   返回卡片型号
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_ActiveCard (DWORD dwPortNumber, BOOL activeAll, CARD_MODEL *pCardModel)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x46, activeAll ? 0x52 : 0x26 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    防冲突
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              pCardNo         BYTE*   [OUT]   返回4字节卡号，用户开辟4字节空间
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_Anticollision (DWORD dwPortNumber, BYTE *pCardNo)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x47, 0x04 };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    选卡
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              pCardNo         BYTE*   [IN]    选卡卡号
//              pCardSize       BYTE*   [OUT]   返回卡片容量，用户开辟空间
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_SelectCard (DWORD dwPortNumber, BYTE *pCardNo, BYTE *pCardSize)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x48, pCardNo[0], pCardNo[1], pCardNo[2], pCardNo[3] };
    int nSendLen = 5;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    验证密钥
// Parameters:  dwPortNumber    DWORD       [IN]    串口号，范围1-255
//              keyModel        KEY_MODEL   [IN]    密钥模式，出厂为KEY_A
//              blockIndex      BYTE        [IN]    绝对块号。S50卡: 0x00-0x3F(64块) S70卡: 0x00-0xFF(256块)
//              pKey            BYTE*       [IN]    6字节扇区密钥，出厂为0xFFFFFFFFFFFF
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_CheckKey (DWORD dwPortNumber, KEY_MODEL keyModel, BYTE blockIndex, BYTE *pKey)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x4A, (BYTE)keyModel, blockIndex, pKey[0], pKey[1], pKey[2], pKey[3], pKey[4], pKey[5] };
    int nSendLen = 9;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    读块
// Parameters:  dwPortNumber    DWORD       [IN]    串口号，范围1-255
//              blockIndex      BYTE        [IN]    绝对块号。S50卡: 0x00-0x3F(64块) S70卡: 0x00-0xFF(256块)
//              pData           BYTE*       [OUT]   返回块数据，用户开辟16字节空间
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_ReadBlock (DWORD dwPortNumber, BYTE blockIndex, BYTE *pData)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x4B, blockIndex };
    int nSendLen = 2;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    写块
// Parameters:  dwPortNumber    DWORD       [IN]    串口号，范围1-255
//              blockIndex      BYTE        [IN]    绝对块号。S50卡: 0x00-0x3F(64块) S70卡: 0x00-0xFF(256块)
//              pData           BYTE*       [IN]    16字节待写入块数据
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
M104BPC_API BOOL WINAPI m104bpc_WriteBlock (DWORD dwPortNumber, BYTE blockIndex, BYTE *pData)
{
    // 准备待发送数据
    BYTE pSendData[MAX_SEND_DATA_LEN] = { 0x4C, blockIndex };
    memcpy (pSendData + 2, pData, 16);
    int nSendLen = 18;
    MakeSendData (pSendData, &nSendLen);

    // 发送数据
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
// Function:    准备待发送的数据
// Parameters:  pSendData       BYTE*   [IN/OUT]    输入指令中的 命令字＋数据域
//                                                  返回整理后的待发送数据
//              pnSendLen       int*    [IN/OUT]    输入指令中的 命令字＋数据域 长度
//                                                  输出待发送数据长度
// Return:      NULL
//
void MakeSendData (BYTE *pSendData, int *pnSendLen)
{
    BYTE pSendDataTemp[MAX_SEND_DATA_LEN] = { 0x02, 0x00, 0x00, *pnSendLen + 2 };
    BYTE checkSum = *pnSendLen + 2;

    // 重新组织数据
    int nIndex = 4;
    for (int i = 0; i < *pnSendLen; i++) {
        if (pSendData[i] == 0x02 || pSendData[i] == 0x03 || pSendData[i] == 0x10)
            pSendDataTemp[nIndex++] = 0x10;
        pSendDataTemp[nIndex++] = pSendData[i];
        checkSum += pSendData[i];
    }

    // 添加校验和及帧尾
    pSendDataTemp[nIndex++] = checkSum;
    pSendDataTemp[nIndex++] = 0x03;

    // 拷贝数据
    memcpy (pSendData, pSendDataTemp, nIndex);
    *pnSendLen = nIndex;
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    发送数据
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              pSendData       PBYTE   [IN]    待发送的数据
//              dwSendDataLen   DWORD   [IN]    待发送数据长度
//              dwTimeout       DWORD   [IN]    发送及接收数据超时时间，单位ms
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
BOOL SendData (DWORD dwPortNumber, PBYTE pSendData, DWORD dwSendDataLen, DWORD dwTimeout)
{
    g_nReceiveStatus = 0;
    return SP_Write_Read (dwPortNumber, ProcessByte, pSendData, dwSendDataLen, dwTimeout);
}

///////////////////////////////////////////////////////////////////////////
//
// Function:    串口收到字节的回调函数
// Parameters:  byte            BYTE    收到的字节
//              dwPortNumber    DWORD   收到该字节的串口号，范围1-255
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
BOOL ProcessByte (BYTE byte, DWORD dwPortNumber)
{
    static bool escape = false;     // 是否出现过转义符
    static BYTE checkSum = 0;       // 校验和
    static int dataReceived = 0;    // 已接收的数据长度（执行结果 + 数据域）

    // 处理字符转义
    switch (byte) {
        case 0x02:
            if (escape)
                escape = false;
            else {
                // 帧头
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

    // 处理接收数据
    switch (g_nReceiveStatus) {
        case 0:     // 等待帧头
            break;
        case 1:     // 模块地址（忽略）
        case 2:
            checkSum += byte;
            g_nReceiveStatus++;
            break;
        case 3:     // 长度
            g_nReceiveDataLen = byte - 2;
            checkSum += byte;
            g_nReceiveStatus++;
            break;
        case 4:     // 命令（忽略）
            checkSum += byte;
            g_nReceiveStatus++;
            break;
        case 5:     // 数据（执行结果 + 数据域）
            g_ReceiveData[dataReceived++] = byte;
            checkSum += byte;

            if (dataReceived >= g_nReceiveDataLen)
                g_nReceiveStatus++;
            break;
        case 6:     // 校验
            if (checkSum == byte)
                g_nReceiveStatus++;
            else
                g_nReceiveStatus = 0;
            break;
        case 7:     // 帧尾
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