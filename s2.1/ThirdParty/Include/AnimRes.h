#pragma once

#ifdef SMARTANIM_EXPORTS
#define SMARTANIM_API __declspec(dllexport)
#else
#define SMARTANIM_API __declspec(dllimport)
#endif

#define MAX_NAME_LENGTH 64

typedef struct _PIC_KEY
{
	float time;
	POINT center;
	float zoomX;
	float zoomY;
	float transScale;
	_PIC_KEY *pNext;
} PIC_KEY, *PPIC_KEY;

typedef struct _PIC_ELEMENT
{
	CHAR szPicName[MAX_NAME_LENGTH];
	LPRGBQUAD lpTransColor;
	LPRECT lpWindow;
	PPIC_KEY pPicKey;
	_PIC_ELEMENT *pNext;
} PIC_ELEMENT, *PPIC_ELEMENT;

typedef struct _PRE_BACKGROUND
{
    float time;
    _PRE_BACKGROUND *pNext;
} PRE_BACKGROUND, *PPRE_BACKGROUND;

typedef struct _ANIM_INFO
{
	CHAR szBackground[MAX_NAME_LENGTH];
	bool loop;
	float animTime;
    PPRE_BACKGROUND pPreBackground;
} ANIM_INFO, *PANIM_INFO;

typedef struct _ANIM_ELEMENT
{
	int animID;
    ANIM_INFO animInfo;
	PPIC_ELEMENT pPicElement;
	_ANIM_ELEMENT *pNext;
} ANIM_ELEMENT, *PANIM_ELEMENT;

typedef struct _PIC_STATUS
{
	CHAR szPicName[MAX_NAME_LENGTH];
	LPRGBQUAD lpTransColor;
	LPRECT lpWindow;
	POINT center;
	float zoomX;
	float zoomY;
	float transScale;
	_PIC_STATUS *pNext;
} PIC_STATUS, *PPIC_STATUS;

class SMARTANIM_API CAnimRes
{
public:
	CAnimRes(void);
	~CAnimRes(void);

	bool Initialize (LPTSTR lpszResFileName);
	void Uninitialize ();
	PPIC_STATUS GetPicStatus (int animID, float time);
	PANIM_INFO GetAnimInfo (int animID);
	bool ReplacePic (int animID, int picIndex, PCHAR pszNewPicName);

private:
	PANIM_ELEMENT m_pAnim;
	PPIC_STATUS m_pPicStatus;

	void ReleasePicStatus ();

	PBYTE GetOneLine(PBYTE pNow, PBYTE pEnd, PCHAR szLine);
	float GetNumber (PCHAR *ppNow);
	void GetString (PCHAR *ppNow, PCHAR string);
	bool GetBool (PCHAR *ppNow);
};
