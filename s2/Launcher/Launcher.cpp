// Launcher.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// 启动程序
    if (!CreateProcess (TEXT ("\\PocketMory1\\Gemini2\\Gemini2.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL)) {
        // 启动任务管理器
        if (!FindWindow (L"DesktopExplorerWindow", NULL))
            CreateProcess (TEXT ("explorer.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);

	    return 1;
    }
    else 
        return 0;
}