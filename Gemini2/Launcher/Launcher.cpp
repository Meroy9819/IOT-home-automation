// Launcher.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// ��������
    if (!CreateProcess (TEXT ("\\PocketMory1\\Gemini2\\Gemini2.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL)) {
        // �������������
        if (!FindWindow (L"DesktopExplorerWindow", NULL))
            CreateProcess (TEXT ("explorer.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);

	    return 1;
    }
    else 
        return 0;
}