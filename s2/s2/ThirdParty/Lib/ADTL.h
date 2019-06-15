////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//          Function: WinCE 下 C 语言版 ADTL (Advanced Tool Library) 库           //
//                                                                                //
//                               Author: Cloud                                    //
//                             Date: 2012/06/27                                   //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

#ifdef ADTL_EXPORTS
#define ADTL_API __declspec(dllexport)
#else
#define ADTL_API __declspec(dllimport)
#endif

#ifndef __ADTL_H__
#define __ADTL_H__

// RGB24位色转RGB565颜色值
#define RGB565(r,g,b) ((DWORD)(((r&0xf8)<<8)|((g&0xfc)<<3)|(b>>3)))

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    判断带有位置信息的消息是否在某个区域中
// Parameters:  lParam      LPARAM  [IN]    Windows消息的lParam参数（包含位置信息）
//              rectArea    RECT    [IN]    待判断的矩形区域
// Return:      如果lParam所包含的位置信息在rectArea所指示的矩形区域内，函数返回true；否则返回false。
//
ADTL_API bool IsInArea(LPARAM lParam, RECT rectArea);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    获取字节数组中的一行文本
// Parameters:  pNow	PBYTE	[IN]		字节数组首地址。
//				dwLen	DWORD	[IN]		字节数组长度。
//				szLine	PCHAR	[IN/OUT]	输出读取到的一行文本（调用者开辟空间）
// Return:      如果成功读取一行文本，则返回未读取区域首地址；否则返回NULL。
// Remark:		函数将略过以“//”开头的注释，以及只包含空白的空行，只返回包含有效字符的行。
//				函数只对ASCII编码文本的字节数组有效。
//
ADTL_API void GetString (PCHAR *ppNow, PCHAR string);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    获取字符串中第一个有效的脚本字符串
// Parameters:  ppNow	PCHAR*	[IN/OUT]	输入指向字符串首地址的指针的地址
//											函数执行后，该地址的值（指针）将指向字符串中未扫描的区域首地址。
//				string	PCHAR	[IN/OUT]	输出读取到的脚本字符串（调用者开辟空间）
// Return:      NULL
// Remark:		第一个有效的脚本字符串，指的是第一个出现在“：”后（如果其后为“{”则略过此“：”）的字符串，
//				该字符串以空格、“{”、“}”或行尾为结尾。
//				函数只对ASCII编码文本的字节数组有效。
//				特别注意：该字符串需以NULL结尾。
//
ADTL_API float GetNumber (PCHAR *ppNow);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    获取字节数组中的一行文本
// Parameters:  pNow	PBYTE	[IN]		字节数组首地址。
//				pEnd	PBYTE	[IN]		字节数组尾地址加1。
//				szLine	PCHAR	[IN/OUT]	输出读取到的一行文本（调用者开辟空间）
// Return:      如果成功读取一行文本，则返回未读取区域首地址；否则返回NULL。
// Remark:		函数将略过以“//”开头的注释，以及只包含空白的空行，只返回包含有效字符的行。
//				函数只对ASCII编码文本的字节数组有效。
//
ADTL_API PBYTE GetOneLine(PBYTE pNow, PBYTE pEnd, PCHAR szLine);