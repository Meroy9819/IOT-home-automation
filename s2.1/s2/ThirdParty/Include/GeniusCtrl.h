////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                  Function: CGeniusCtrl ��Ļ�������ƿ�                          //
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

#define MAX_PROCESS_NAME_LEN 64                     // ������������ַ���

#define GCPROCESS                                   // ��ʶGeniusCtrl��Ļ����������
typedef GCPROCESS void (*MOUSE_DOWN_PROCESS) ();    // �����������ָ������

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����ȵ�ṹ��
// Remark:      �ýṹ���¼���λ�á���Ӧ�Ĵ��������ơ���Ӧ�Ĵ�������
//              ��һ����������Ϣ������ڵ�
//
typedef struct _HOT_DOT
{
    RECT area;                                  // ���λ��
    CHAR szProcessName[MAX_PROCESS_NAME_LEN];   // ����������
	MOUSE_DOWN_PROCESS pProcess;                // ������
    _HOT_DOT *pNext;                            // ��һ���ȵ�ṹ��ָ��
} HOT_DOT, *PHOT_DOT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    ����״̬�ṹ��
// Remark:      �ýṹ�����ĳ������ID���ý��������е���ȵ���Ϣ��
//              ��һ������ĵ�������Ϣ������ڵ�
//
typedef struct _INTERFACE_STATUS
{
    int interfaceID;            // ����ID
    PHOT_DOT pHotDot;           // �����ȵ���Ϣ����
    _INTERFACE_STATUS *pNext;   // ��һ������״̬�ṹ��ָ��
} INTERFACE_STATUS, *PINTERFACE_STATUS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    CGeniusCtrl��
//
class GENIUSCTRL_API CGeniusCtrl {
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ���캯��
    // Remark:      ��ʼ����Ҫ��
    //
	CGeniusCtrl(void);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ��ʼ��GeniusCtrl
    // Parameters:	lpResFileName   LPTSTR  [IN]    �ű��ļ����ƣ�����·����
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      ��ʹ��GeniusCtrl����ǰ����ɹ����øú���������ɱ�Ҫ�ĳ�ʼ��������
    //              ���ѳɹ����øú��������ڳ������ǰ�������ٵ���һ��Uninitialize���������ͷŸú�������Ŀռ䡣
    //
	bool Initialize (LPTSTR lpResFileName);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    �ͷ�GeniusCtrl
    // Parameters:	NULL
    // Return:      NULL
    // Remark:      �ú����ͷ���Initialize�������ٵĿռ䡣
    //              ���ɹ�����Initialize��������������ٵ���һ�θú������ͷ�Initialize��������Ŀռ䡣
    //              ��δ���û�δ�ɹ�����Initialize����������Բ����øú���������Ȼ���øú�������ú��������κδ���
    //
	void Uninitialize ();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ע����갴�´�����
    // Parameters:	lpszProcessName     LPSTR               [IN]    ����������
    //              pProcess            MOUSE_DOWN_PROCESS  [IN]    ������ָ��
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú���ע���ض��Ĵ�������pProcess�����������ƣ�lpszProcessName����
    //              ���ű�ָ������ĳ��������ʱ��lpszProcessName�������Զ����øô�������pProcess����
    //              ����������DoLButtonDown�������ڵ��߳��б����ã�һ��Ϊ��������Ϣ�����̣߳���
    //
	bool RegisterMouseDownProcess (LPSTR lpszProcessName, MOUSE_DOWN_PROCESS pProcess);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ������갴����Ϣ��WM_LBUTTONDOWN)
    // Parameters:	interfaceID     int     [IN]    ��ǰ����ID
    //              lParam          LPARAM  [IN]    Windows��Ϣ��lParam�������������λ�ã�
    // Return:      NULL
    // Remark:      �˺���һ����WM_LBUTTONDOWN��Ϣ�Ĵ�������ֱ�ӵ��ã���ʹ�õ���ű���ָ���ĺ��������ض����桢
    //              �ض�λ�õĵ������
    //
    void DoLButtonDown (int interfaceID, LPARAM lParam);

private:
    PINTERFACE_STATUS m_pInterfaceStatus;   // �������н�������Ϣ������
};