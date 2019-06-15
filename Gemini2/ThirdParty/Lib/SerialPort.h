#ifdef SERIALPORT_EXPORTS
#define SERIALPORT_API __declspec(dllexport)
#else
#define SERIALPORT_API __declspec(dllimport)
#endif

#ifndef SERIALPORTCE_DEFINE
#define SERIALPORTCE_DEFINE

// SerialPort.dll Definition
#define USER_ERROR_CODE 0x20000000						                // �û��Զ������
#define ERROR_PORT_NOT_OPEN		        USER_ERROR_CODE + 0x101	        // �˿�δ��
#define ERROR_WAIT_MUTEX                USER_ERROR_CODE + 0x102         // �ȴ����������
#define ERROR_WRITE_NOT_COMPLETE        USER_ERROR_CODE + 0x103         // ʵ��д�����������д������������
#define ERROR_READ_STARTED              USER_ERROR_CODE + 0x104         // �ö˿��ϵ�˫���շ�ͨ���Ѿ�����
#define ERROR_READ_STOPPED              USER_ERROR_CODE + 0x105         // �ö˿��ϲ����ڱ�����������˫���շ�ͨ��
// SerialPort.dll Definition End

#endif

typedef BOOL (WINAPI *FuncProcessByte) (BYTE byte, DWORD dwPortNumber); // ����ÿ���ֽڵĻص�����

///////////////////////////////////////////////////////////////////////////
//
// Function:    �򿪴���
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              dwBaudRate      DWORD   [IN]    ���ڲ�����
//              StopBits        DWORD   [IN]    ֹͣλ��0-1ֹͣλ��1-1.5ֹͣλ��
//                                              2-2ֹͣλ��Ĭ��Ϊ1ֹͣλ��
// Return:      �򿪴��ڳɹ�����TRUE�����򷵻�FALSE��
//              ����GetLastError��ȡ��ϸ�������
// Remark:      ��������Ѿ��򿪣���ֱ�ӷ���TRUE
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Open (DWORD dwPortNumber, DWORD dwBaudRate, BYTE StopBits);

///////////////////////////////////////////////////////////////////////////
//
// Function:    �رմ���
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              dwTimeout       DWORD   [IN]    ����ý���������˫��ͨ�ŷ�ʽ���򳬹���ʱ���
//                                              ��˫����ȡ�߳���δֹͣ����ǿ����ֹ˫����ȡ�߳�
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
// Remark:      ��������ѹرգ���ֱ�ӷ���TRUE��
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Close (DWORD dwPortNumber, DWORD dwTimeout);

///////////////////////////////////////////////////////////////////////////
//
// Function:    ��մ��ڶ�ȡ��������ֹͣ��ǰ�Ķ�ȡ����
// Parameters:  dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_ClearReadBuffer (DWORD dwPortNumber);

///////////////////////////////////////////////////////////////////////////
//
// Function:    ����д����
// Parameters:  dwPortNumber            DWORD   [IN]    ���ںţ���Χ1-255
//              lpWriteBuffer           BYTE*   [IN]    д�����ݻ���
//              dwNumberOfBytesToWrite  BYTE*   [IN]    Ҫд��������ֽ�����
//                                                      ��ֵ��Ӧ����lpWriteBuffer
//                                                      ָ��Ŀռ䡣
//              dwTimeout               BYTE*   [IN]    д�볬ʱʱ�䣬��λms
// Return:      �����ɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ
//              ��ϸ�������
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Write (DWORD dwPortNumber, BYTE* lpWriteBuffer, DWORD dwNumberOfBytesToWrite, DWORD dwTimeout);

///////////////////////////////////////////////////////////////////////////
//
// Function:    ���ڶ�����
// Parameters:  dwPortNumber    DWORD           [IN]    ���ںţ���Χ1-255
//              processByte     FuncProcessByte [IN]    �������յ���ÿ���ֽں���ָ��
//              dwTimeout       BYTE*           [IN]    ��ȡ��ʱʱ�䣬��λms
// Return:      processByteָ��ĺ�������TRUE������������TRUE�����ѵ���ʱʱ�䣬��
//              processByteδ����TRUE������������FALSE������GetLastError�ɻ�ȡ
//              ��ϸ�������
// Remark:      ������ȡ��ÿ1�����ݺ󣬽�����processByteָ��ĺ�����������ݣ�
//              ������ʣ�೬ʱ�����賬ʱ����������ȡ��
// Caution:     ��ȡ���ݹ����г��ֵ��쳣�������Σ�������ѭ����ȡ����Ŀ�������Ӷ�ȡ
//              ���ݴ��ԣ���ͬʱҲ����������쳣�����Σ�����ֻ��ó�ʱ�쳣���û���
//              ע������жϡ������εĴ�����������ζ�ȡ���ֽ�����1���첽��ȡδ��ɣ�
//              ����������ERROR_IO_PENDING���ȴ��첽��ȡ�¼�����WAIT_FAILED��
//              ��ȡ�첽�������FALSE��
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Read (DWORD dwPortNumber, FuncProcessByte processByte, DWORD dwTimeout);

///////////////////////////////////////////////////////////////////////////
//
// Function:    ���Ͳ���������
// Parameters:  dwPortNumber            DWORD           [IN]    ���ںţ���Χ1-255
//              processByte             FuncProcessByte [IN]    �������յ���ÿ��
//                                                              �ֽں���ָ��
//              lpWriteBuffer           BYTE*           [IN]    д�����ݻ���
//              dwNumberOfBytesToWrite  BYTE*           [IN]    Ҫд��������ֽ�����
//                                                              ��ֵ��Ӧ����lpWriteBuffer
//                                                              ָ��Ŀռ䡣
//              dwTimeout               BYTE*           [IN]    ����ʱʱ�䣬��λms
// Return:      ���óɹ�����TRUE������ʧ�ܷ���FALSE������GetLastError��ȡ��ϸ�������
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Write_Read (DWORD dwPortNumber, FuncProcessByte processByte, BYTE* lpWriteBuffer, DWORD dwNumberOfBytesToWrite, 
                                          DWORD dwTimeout);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����˫���շ���ʽ
// Parameter:   dwPortNumber    DWORD           [IN]    ���ںţ���Χ1-255
//              processByte     FuncProcessByte [IN]    �ص��������������յ���ÿ���ֽڣ����Ժ�������ֵ
// Return:      ִ�гɹ�����TRUE�����򷵻�FALSE������GetLastError������ȡ��ϸ�������
// Caution:     �������ݴ����ڵ������߳�����ɣ���ע����������̵߳�Э����˫���շ������SP_EndRead��SP_Close������
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_BeginRead (DWORD dwPortNumber, FuncProcessByte processByte);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����˫���շ���ʽ
// Parameter:   dwPortNumber    DWORD   [IN]    ���ںţ���Χ1-255
//              hReadThread     DWORD   [IN]    ���������շ��߳�
//              dwTimeout       DWORD   [IN]    ���������ʱ�䣬��ȡ�߳���δֹͣ����ǿ����ֹ��ȡ�߳�
// Return:      ִ�гɹ�����TRUE�����򷵻�FALSE������GetLastError������ȡ��ϸ�������
// Remark:      ����߳���dwTimeoutʱ����δ��ֹ����ǿ����ֹ�̣߳�����TRUE���߳�ExitCode��Ϊ0x200��
//              ����߳���dwTimeoutʱ������ֹ�����߳�ExitCodeΪ0������TRUE�����򷵻�FALSE��
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_EndRead (DWORD dwPortNumber, DWORD dwTimeout);