#ifdef SERIALPORT_EXPORTS
#define SERIALPORT_API __declspec(dllexport)
#else
#define SERIALPORT_API __declspec(dllimport)
#endif

#ifndef SERIALPORTCE_DEFINE
#define SERIALPORTCE_DEFINE

// SerialPort.dll Definition
#define USER_ERROR_CODE 0x20000000						                // 用户自定义错误
#define ERROR_PORT_NOT_OPEN		        USER_ERROR_CODE + 0x101	        // 端口未打开
#define ERROR_WAIT_MUTEX                USER_ERROR_CODE + 0x102         // 等待互斥体错误
#define ERROR_WRITE_NOT_COMPLETE        USER_ERROR_CODE + 0x103         // 实际写入数据量与待写入数据量不符
#define ERROR_READ_STARTED              USER_ERROR_CODE + 0x104         // 该端口上的双工收发通信已经启动
#define ERROR_READ_STOPPED              USER_ERROR_CODE + 0x105         // 该端口上不存在本进程启动的双工收发通信
// SerialPort.dll Definition End

#endif

typedef BOOL (WINAPI *FuncProcessByte) (BYTE byte, DWORD dwPortNumber); // 处理每个字节的回调函数

///////////////////////////////////////////////////////////////////////////
//
// Function:    打开串口
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              dwBaudRate      DWORD   [IN]    串口波特率
//              StopBits        DWORD   [IN]    停止位。0-1停止位，1-1.5停止位，
//                                              2-2停止位。默认为1停止位。
// Return:      打开串口成功返回TRUE，否则返回FALSE。
//              调用GetLastError获取详细错误代码
// Remark:      如果串口已经打开，则直接返回TRUE
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Open (DWORD dwPortNumber, DWORD dwBaudRate, BYTE StopBits);

///////////////////////////////////////////////////////////////////////////
//
// Function:    关闭串口
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              dwTimeout       DWORD   [IN]    如果该进程已启用双工通信方式，则超过此时间后，
//                                              如双工读取线程仍未停止，则强制终止双工读取线程
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
// Remark:      如果串口已关闭，则直接返回TRUE。
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Close (DWORD dwPortNumber, DWORD dwTimeout);

///////////////////////////////////////////////////////////////////////////
//
// Function:    清空串口读取缓冲区并停止以前的读取操作
// Parameters:  dwPortNumber    DWORD   [IN]    串口号，范围1-255
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_ClearReadBuffer (DWORD dwPortNumber);

///////////////////////////////////////////////////////////////////////////
//
// Function:    串口写数据
// Parameters:  dwPortNumber            DWORD   [IN]    串口号，范围1-255
//              lpWriteBuffer           BYTE*   [IN]    写入数据缓存
//              dwNumberOfBytesToWrite  BYTE*   [IN]    要写入的数据字节数。
//                                                      该值不应大于lpWriteBuffer
//                                                      指向的空间。
//              dwTimeout               BYTE*   [IN]    写入超时时间，单位ms
// Return:      操作成功返回TRUE，操作失败返回FALSE。调用GetLastError获取
//              详细错误代码
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Write (DWORD dwPortNumber, BYTE* lpWriteBuffer, DWORD dwNumberOfBytesToWrite, DWORD dwTimeout);

///////////////////////////////////////////////////////////////////////////
//
// Function:    串口读数据
// Parameters:  dwPortNumber    DWORD           [IN]    串口号，范围1-255
//              processByte     FuncProcessByte [IN]    处理串口收到的每个字节函数指针
//              dwTimeout       BYTE*           [IN]    读取超时时间，单位ms
// Return:      processByte指向的函数返回TRUE，函数将返回TRUE。如已到超时时间，但
//              processByte未返回TRUE，则函数将返回FALSE。调用GetLastError可获取
//              详细错误代码
// Remark:      函数读取到每1个数据后，将调用processByte指向的函数处理该数据，
//              并按照剩余超时间重设超时，并继续读取。
// Caution:     读取数据过程中出现的异常将被屏蔽，并继续循环读取，其目的是增加读取
//              的容错性，但同时也会带来部分异常被屏蔽，最终只获得超时异常。用户需
//              注意谨慎判断。被屏蔽的错误包括：单次读取的字节数非1；异步读取未完成，
//              但错误代码非ERROR_IO_PENDING；等待异步读取事件返回WAIT_FAILED；
//              获取异步结果返回FALSE。
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Read (DWORD dwPortNumber, FuncProcessByte processByte, DWORD dwTimeout);

///////////////////////////////////////////////////////////////////////////
//
// Function:    发送并接收数据
// Parameters:  dwPortNumber            DWORD           [IN]    串口号，范围1-255
//              processByte             FuncProcessByte [IN]    处理串口收到的每个
//                                                              字节函数指针
//              lpWriteBuffer           BYTE*           [IN]    写入数据缓存
//              dwNumberOfBytesToWrite  BYTE*           [IN]    要写入的数据字节数。
//                                                              该值不应大于lpWriteBuffer
//                                                              指向的空间。
//              dwTimeout               BYTE*           [IN]    处理超时时间，单位ms
// Return:      设置成功返回TRUE，设置失败返回FALSE。调用GetLastError获取详细错误代码
//
///////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_Write_Read (DWORD dwPortNumber, FuncProcessByte processByte, BYTE* lpWriteBuffer, DWORD dwNumberOfBytesToWrite, 
                                          DWORD dwTimeout);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    启动双工收发方式
// Parameter:   dwPortNumber    DWORD           [IN]    串口号，范围1-255
//              processByte     FuncProcessByte [IN]    回调函数，处理串口收到的每个字节，忽略函数返回值
// Return:      执行成功返回TRUE，否则返回FALSE。调用GetLastError函数获取详细错误代码
// Caution:     由于数据处理在单独的线程中完成，请注意其与界面线程的协调。双工收发须调用SP_EndRead或SP_Close结束。
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_BeginRead (DWORD dwPortNumber, FuncProcessByte processByte);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    结束双工收发方式
// Parameter:   dwPortNumber    DWORD   [IN]    串口号，范围1-255
//              hReadThread     DWORD   [IN]    待结束的收发线程
//              dwTimeout       DWORD   [IN]    如果超过此时间，读取线程仍未停止，则强制终止读取线程
// Return:      执行成功返回TRUE，否则返回FALSE。调用GetLastError函数获取详细错误代码
// Remark:      如果线程在dwTimeout时间内未终止，则强制终止线程，返回TRUE，线程ExitCode设为0x200。
//              如果线程在dwTimeout时间内终止，则线程ExitCode为0，返回TRUE。否则返回FALSE。
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SERIALPORT_API BOOL WINAPI SP_EndRead (DWORD dwPortNumber, DWORD dwTimeout);