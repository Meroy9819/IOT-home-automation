#pragma once

#ifdef SMARTANIM_EXPORTS
#define SMARTANIM_API __declspec(dllexport)
#else
#define SMARTANIM_API __declspec(dllimport)
#endif

#include "FastDraw.h"

#define MAX_NAME_LENGTH 64

typedef struct _PIC_INFO
{
	CHAR picName[MAX_NAME_LENGTH];
	int surfaceIndex;
	_PIC_INFO *pNext;
} PIC_INFO, *PPIC_INFO;

class SMARTANIM_API CPicRes
{
public:
	CPicRes(void);
	~CPicRes(void);

	bool Initialize (LPTSTR lpszResFileName, LPTSTR lpszPicDirPath, CFastDraw *pFastDraw);
	void Uninitialize ();
	int GetInterfaceIndex (PCHAR pszPicName);

private:
	PPIC_INFO m_pPicInfo;

	PBYTE GetOneLine(PBYTE pNow, PBYTE pEnd, PCHAR szLine);
	float GetNumber (PCHAR *ppNow);
	void GetString (PCHAR *ppNow, PCHAR string);
};
