////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                  Function: CGeniusCtrl 屏幕交互控制库                          //
//                                                                                //
//                               Author: Cloud                                    //
//                             Date: 2012/06/20                                   //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

#ifdef GENIUSCTRL_EXPORTS
#define GENIUSCTRL_API __declspec(dllexport)
#else
#define GENIUSCTRL_API __declspec(dllimport)
#endif

#define MAX_PROCESS_NAME_LEN 64                     // 最大处理函数名称字符数

#define GCPROCESS                                   // 标识GeniusCtrl屏幕交互处理函数
typedef GCPROCESS void (*MOUSE_DOWN_PROCESS) ();    // 鼠标点击处理函数指针类型

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    点击热点结构体
// Remark:      该结构体记录点击位置、对应的处理函数名称、对应的处理函数。
//              是一个点击相关信息的链表节点
//
typedef struct _HOT_DOT
{
    RECT area;                                  // 点击位置
    CHAR szProcessName[MAX_PROCESS_NAME_LEN];   // 处理函数名称
	MOUSE_DOWN_PROCESS pProcess;                // 处理函数
    _HOT_DOT *pNext;                            // 下一个热点结构体指针
} HOT_DOT, *PHOT_DOT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    界面状态结构体
// Remark:      该结构体包含某个界面ID，该界面下所有点击热点信息。
//              是一个界面的点击相关信息的链表节点
//
typedef struct _INTERFACE_STATUS
{
    int interfaceID;            // 界面ID
    PHOT_DOT pHotDot;           // 所有热点信息链表
    _INTERFACE_STATUS *pNext;   // 下一个界面状态结构体指针
} INTERFACE_STATUS, *PINTERFACE_STATUS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    CGeniusCtrl类
//
class GENIUSCTRL_API CGeniusCtrl {
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    构造函数
    // Remark:      初始化必要项
    //
	CGeniusCtrl(void);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    初始化GeniusCtrl
    // Parameters:	lpResFileName   LPTSTR  [IN]    脚本文件名称（完整路径）
    // Return:      成功返回true，失败返回false。
    // Remark:      在使用GeniusCtrl对象前，需成功调用该函数，以完成必要的初始化工作。
    //              若已成功调用该函数，则在程序结束前，需至少调用一次Uninitialize函数，以释放该函数分配的空间。
    //
	bool Initialize (LPTSTR lpResFileName);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    释放GeniusCtrl
    // Parameters:	NULL
    // Return:      NULL
    // Remark:      该函数释放由Initialize函数开辟的空间。
    //              若成功调用Initialize函数，则必须至少调用一次该函数以释放Initialize函数分配的空间。
    //              若未调用或未成功调用Initialize函数，则可以不调用该函数；若仍然调用该函数，则该函数不做任何处理。
    //
	void Uninitialize ();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    注册鼠标按下处理函数
    // Parameters:	lpszProcessName     LPSTR               [IN]    处理函数名称
    //              pProcess            MOUSE_DOWN_PROCESS  [IN]    处理函数指针
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数注册特定的处理函数（pProcess）到函数名称（lpszProcessName）。
    //              当脚本指定调用某函数名称时（lpszProcessName），将自动调用该处理函数（pProcess）。
    //              处理函数将在DoLButtonDown函数所在的线程中被调用（一般为主界面消息处理线程）。
    //
	bool RegisterMouseDownProcess (LPSTR lpszProcessName, MOUSE_DOWN_PROCESS pProcess);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    处理鼠标按下消息（WM_LBUTTONDOWN)
    // Parameters:	interfaceID     int     [IN]    当前界面ID
    //              lParam          LPARAM  [IN]    Windows消息的lParam参数（包含点击位置）
    // Return:      NULL
    // Remark:      此函数一般在WM_LBUTTONDOWN消息的处理函数中直接调用，以使用点击脚本中指定的函数处理特定界面、
    //              特定位置的点击操作
    //
    void DoLButtonDown (int interfaceID, LPARAM lParam);

private:
    PINTERFACE_STATUS m_pInterfaceStatus;   // 保存所有界面点击信息的链表
};