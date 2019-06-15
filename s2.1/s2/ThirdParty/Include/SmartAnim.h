#ifdef SMARTANIM_EXPORTS
#define SMARTANIM_API __declspec(dllexport)
#else
#define SMARTANIM_API __declspec(dllimport)
#endif

#ifndef __SMARTANIM_H__
#define __SMARTANIM_H__

#include "FastDraw.h"
#include "AnimRes.h"
#include "PicRes.h"

typedef struct _ANIM_NOW
{
    int animID;
    PANIM_INFO pAnimInfo;
    LARGE_INTEGER startCounter;
    bool isFirstFrame;
	bool obverse;
} ANIM_NOW, *PANIM_NOW;

typedef void (* ANIM_END_PROCESS) (int animID, bool obverse);

class SMARTANIM_API CSmartAnim {
public:
	CSmartAnim(void);
	~CSmartAnim ();

    bool Initialize (HWND hWnd, LPTSTR lpszAnimResFileName, LPTSTR lpszPicResFileName, LPTSTR lpszPicDirPath);
    void Uninitialize ();
    void RegisterAnimEndProcess (ANIM_END_PROCESS pAnimEndProcess);
    bool ShowAnim (int animID, bool obverse);
	bool ReplacePic (int animID, int picIndex, PCHAR pszNewPicName);
	
private:
    CFastDraw m_FastDraw;
	CAnimRes m_AnimRes;
	CPicRes m_PicRes;
    bool m_ShowAnimThreadStarted;
    bool m_StopAnimCtrlThread;
    LARGE_INTEGER m_Frequency;
    ANIM_END_PROCESS m_pAnimEndProcess;
    PANIM_NOW m_pAnimNow;               // 重做线程同步

    friend DWORD ThreadShowAnim(LPVOID lpvoid);
};

#endif // __SMARTANIM_H__