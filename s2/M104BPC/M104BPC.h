#ifdef M104BPC_EXPORTS
#define M104BPC_API __declspec(dllexport)
#else
#define M104BPC_API __declspec(dllimport)
#endif

#define MODEL_ERROR_PREFIX  0x20010000                      // Ä£¿é´íÎó´úÂëÇ°×º

enum CARD_MODEL { UNKNOWN, S50 = 0x04, S70 = 0x02 };        // ¿¨Æ¬ÐÍºÅ
enum KEY_MODEL { KEY_A = 0x60, KEY_B = 0x61 };              // ÃÜÔ¿ÐÍºÅ

M104BPC_API BOOL WINAPI m104bpc_OpenPort (DWORD dwPortNumber, DWORD dwBaudRate);
M104BPC_API BOOL WINAPI m104bpc_ClosePort (DWORD dwPortNumber);
M104BPC_API BOOL WINAPI m104bpc_RFCtrl (DWORD dwPortNumber, BOOL open);
M104BPC_API BOOL WINAPI m104bpc_SetTypeA (DWORD dwPortNumber);
M104BPC_API BOOL WINAPI m104bpc_ActiveCard (DWORD dwPortNumber, BOOL activeAll, CARD_MODEL *pCardModel);
M104BPC_API BOOL WINAPI m104bpc_Anticollision (DWORD dwPortNumber, BYTE *pCardNo);
M104BPC_API BOOL WINAPI m104bpc_SelectCard (DWORD dwPortNumber, BYTE *pCardNo, BYTE *pCardSize);
M104BPC_API BOOL WINAPI m104bpc_CheckKey (DWORD dwPortNumber, KEY_MODEL keyModel, BYTE blockIndex, BYTE *pKey);
M104BPC_API BOOL WINAPI m104bpc_ReadBlock (DWORD dwPortNumber, BYTE blockIndex, BYTE *pData);
M104BPC_API BOOL WINAPI m104bpc_WriteBlock (DWORD dwPortNumber, BYTE blockIndex, BYTE *pData);