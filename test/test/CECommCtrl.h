#pragma once

#define RECEIVE_DATA_LEN_GEMINI2 5

enum MOTOR_STATUS { STOP, FORWARD, BACKWARD };

class CCommCtrl
{
public:
    CCommCtrl(void);
    ~CCommCtrl(void);
    
    bool OpenSerialPort (DWORD dwPortNumber, DWORD dwBaudRate);
    bool CloseSerialPort (DWORD dwPortNumber);

    bool SetLED (DWORD dwPortNumber, int ledID, bool open);
    bool SetMotor (DWORD dwPortNumber, MOTOR_STATUS motorStatus);
    bool GetTemperature (DWORD dwPortNumber, int *temperature);
    bool GetHumidity (DWORD dwPortNumber, int *humidityPercent);
    bool GetLight (DWORD dwPortNumber, bool *bright);

private:
    static BYTE m_ReceiveBuffer_Gemini2[RECEIVE_DATA_LEN_GEMINI2];
    static int m_ReceiveStatus_Gemini2;

    bool SendData_Gemini2 (DWORD dwPortNumber, PBYTE pSendData, int nSendDataLen, bool requestAnswer, DWORD dwTimeout);
    static BOOL ProcessByte_Gemini2 (BYTE byte, DWORD dwPortNumber);
    HANDLE CreatePrivateMutex (LPCTSTR lpNamePrefix, DWORD dwIndex);
};
