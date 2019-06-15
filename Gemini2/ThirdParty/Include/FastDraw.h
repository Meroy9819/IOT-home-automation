////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                         Function: FastDraw 绘图库                              //
//                                                                                //
//                               Author: Cloud                                    //
//                             Date: 2012/06/08                                   //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////

#ifdef FASTDRAW_EXPORTS
#define FASTDRAW_API __declspec(dllexport)
#else
#define FASTDRAW_API __declspec(dllimport)
#endif

#ifndef __FASTDRAW_H__
#define __FASTDRAW_H__

#ifndef __DDRAW_INCLUDED__
#include "ddraw.h"
#endif

// 隐式调用DirectDraw所用函数指针类型定义－DirectDrawCreate
typedef HRESULT (WINAPI *PFDirectDrawCreate) (LPGUID lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);

#define MAX_BUFFER_SURFACE 200      // 最大缓存表面数量
#define MAX_PRE_BACKGROUND 10       // 最大预定义背景表面数量（不包括默认背景表面）

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    FastDraw类
//
class FASTDRAW_API CFastDraw {
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    构造函数
    // Remark:      初始化必要项
    //
	CFastDraw();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    初始化FastDraw
    // Parameters:	hWnd    HWND    [IN]    主窗体句柄
    // Return:      成功返回true，失败返回false。
    // Remark:      在使用FastDraw对象前，需成功调用该函数，以完成必要的初始化工作。
    //              若已成功调用该函数，则在程序结束前，需至少调用一次Uninitialize函数，以释放该函数分配的空间。
    //              该函数必须在建立应用程序主窗体的线程中调用（由于IDirectDraw需要在该线程中释放，故须在该线程中分配）。
    //
    bool Initialize (HWND hWnd);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    释放FastDraw
    // Parameters:	NULL
    // Return:      NULL
    // Remark:      该函数释放由Initialize函数开辟的空间。
    //              若成功调用Initialize函数，则必须至少调用一次该函数以释放Initialize函数分配的空间。
    //              若未调用或未成功调用Initialize函数，则可以不调用该函数；若仍然调用该函数，则该函数不做任何处理。
    //              该函数必须在建立应用程序主窗体的线程中调用（由于IDirectDraw需要在该线程中释放）。
    //
	void Uninitialize ();
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    添加缓存表面
    // Parameters:	nBufferSurfaceIndex     int         [IN]    缓存表面存储索引
    //              hSourceBitmap           HBITMAP     [IN]    待存入缓存表面的原始位图句柄
    // Return:      成功返回true，失败返回false。
    // Remark:      如nBufferSurfaceIndex索引值位置的缓存表面已经建立，则该函数首先释放已存在的缓存表面，再建立新表面。
    //
    bool AddBufferSurface (int nBufferSurfaceIndex, HBITMAP hSourceBitmap);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    拷贝缓存表面至绘图表面
    // Parameters:	nBufferSurfaceIndex     int                     [IN]    拷贝的源缓存表面索引值
    //              center                  POINT                   [IN]    拷贝后的中心坐标（在目标表面坐标系中）
    //              zoomX                   float                   [IN]    拷贝后的水平方向缩放比例
    //              zoomY                   float                   [IN]    拷贝后的竖直方向缩放比例
    //              lpWindow                LPRECT                  [IN]    目标表面绘制窗体指针（在目标表面坐标系中）。
    //                                                                      该值为NULL，则无绘制窗体限制。
    //                                                                      否则，超出该窗体的拷贝内容将被截断。
    //              lpTransColor            LPRGBQUARD              [IN]    透明色指针。该值为NULL，则无透明色。
    //                                                                      否则，源表面中该颜色将不被拷贝。
    //              transScale              float                   [IN]    透明度。取值范围：0～1，0为全透明，1为不透明。
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将指定的缓存表面经过一系列变换拷贝到绘图表面。
    //              函数暂不支持同时设置透明色和透明度。
    //              如果上一次拷贝还未结束，则该函数会等待拷贝结束后再开始拷贝。
    //
    bool BlitToDrawSurface (int nBufferSurfaceIndex, POINT center, float zoomX, float zoomY, LPRECT lpWindow, LPRGBQUAD lpTransColor, 
                            float transScale);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    拷贝缓存表面至背景表面
    // Parameters:	nBackgroundIndex        int                     [IN]    拷贝的目标背景表面索引值
    //              nBufferSurfaceIndex     int                     [IN]    拷贝的源缓存表面索引值
    //              center                  POINT                   [IN]    拷贝后的中心坐标（在目标表面坐标系中）
    //              zoomX                   float                   [IN]    拷贝后的水平方向缩放比例
    //              zoomY                   float                   [IN]    拷贝后的竖直方向缩放比例
    //              lpWindow                LPRECT                  [IN]    目标表面绘制窗体指针（在目标表面坐标系中）。
    //                                                                      该值为NULL，则无绘制窗体限制。
    //                                                                      否则，超出该窗体的拷贝内容将被截断。
    //              lpTransColor            LPRGBQUARD              [IN]    透明色指针。该值为NULL，则无透明色。
    //                                                                      否则，源表面中该颜色将不被拷贝。
    //              transScale              float                   [IN]    透明度。取值范围：0～1，0为全透明，1为不透明。
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将指定的缓存表面经过一系列变换拷贝到目标背景表面。
    //              函数暂不支持同时设置透明色和透明度。
    //              如果上一次拷贝还未结束，则该函数会等待拷贝结束后再开始拷贝。
    //
    bool BlitToBackgroundSurface (int nBackgroundIndex, int nBufferSurfaceIndex, POINT center, float zoomX, float zoomY, LPRECT lpWindow, 
                                  LPRGBQUAD lpTransColor, float transScale);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    拷贝主表面至背景表面
    // Parameters:	NULL
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数拷贝当前主表面内容至背景表面。
    //              如果上一次拷贝还未结束，则该函数会等待拷贝结束后再开始拷贝。
    //
    bool BlitPrimarySurfaceToBackgroundSurface ();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    拷贝背景表面至绘图表面
    // Parameters:	nBackgroundIndex    int     [IN]    背景表面索引值
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将指定索引值的背景表面拷贝至绘图表面
    //              如果上一次拷贝还未结束，则该函数会等待拷贝结束后再开始拷贝。
    //
    bool BlitPreBackgroundSurefaceBackground (int nBackgroundIndex);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    拷贝预定义背景表面的背景
    // Parameters:	nBackgroundIndex    int     [IN]    背景表面索引值。该值不应为0（0为默认背景表面的索引值）
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将上一个（索引值减1）预定义背景（或默认背景）作为背景，拷贝到nBackgroundIndex索引值所指向的
    //              预定义背景表面中。如果该位置的预定义背景还未建立，则首先建立该预定义背景空间，然后再执行拷贝。
    //              如果上一次拷贝还未结束，则该函数会等待拷贝结束后再开始拷贝。
    //
    bool BlitBackgroundSurfaceToDrawSurface (int nBackgroundIndex);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    翻转表面
    // Parameters:	NULL
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将绘图表面翻转至屏幕主表面，并显示。
    //              如果上次翻转操作还未完成，则该函数会等待翻转结束后再开始翻转。
    //
    bool Flipping ();

private:
    // DirectDraw接口
    HINSTANCE m_hInstDDLib;                                         // DirectDraw库导入句柄
	LPDIRECTDRAW m_pDD;					                            // DirectDraw接口实例
	LPDIRECTDRAWSURFACE m_pDDSPrimary;			                    // DirectDraw主表面
    LPDIRECTDRAWSURFACE m_pDDSDraw;                                 // DirectDraw绘图表面
    LPDIRECTDRAWSURFACE m_pDDSBackground;                           // DirectDraw默认背景表面
	DDCAPS m_DDDriverCaps;						                    // DirectDraw硬件加速功能
	DDCAPS m_DDEmulCaps;						                    // DirectDraw软件模拟功能
    LPDIRECTDRAWSURFACE m_pDDSBuffers[MAX_BUFFER_SURFACE];          // DirectDraw缓存表面
    LPDIRECTDRAWSURFACE m_pDDSPreBackground[MAX_PRE_BACKGROUND];    // DirectDraw预定义背景表面 
    PFDirectDrawCreate m_pfDirectDrawCreate;                        // DirectDraw隐式导入函数－DirectDrawCreate

	bool m_bCOMInitialized;						                    // COM是否已成功初始化
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    枚举表面回调函数
    // Parameters:	参见MSDN
    // Return:      参见MSDN
    // Remark:      获取与主表面连接的第一个表面（后备表面）作为绘图表面
    //
    friend HRESULT WINAPI EnumSurfacesCallback (LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    拷贝表面
    // Parameters:	lpSourceSurface         LPDIRECTDRAWSURFACE     [IN]    源表面指针
    //              lpDestinationSurface    LPDIRECTDRAWSURFACE     [IN]    目标表面指针
    //              center                  POINT                   [IN]    拷贝后的中心坐标（在目标表面坐标系中）
    //              zoomX                   float                   [IN]    拷贝后的水平方向缩放比例
    //              zoomY                   float                   [IN]    拷贝后的竖直方向缩放比例
    //              lpWindow                LPRECT                  [IN]    目标表面绘制窗体指针（在目标表面坐标系中）。
    //                                                                      该值为NULL，则无绘制窗体限制。
    //                                                                      否则，超出该窗体的拷贝内容将被截断。
    //              lpTransColor            LPRGBQUARD              [IN]    透明色指针。该值为NULL，则无透明色。
    //                                                                      否则，源表面中该颜色将不被拷贝。
    //              transScale              float                   [IN]    透明度。取值范围：0～1，0为全透明，1为不透明。
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将源表面经过一系列变换拷贝到目标表面。
    //              函数暂不支持同时设置透明色和透明度。
    //              如果上一次拷贝还未结束，则该函数会等待拷贝结束后再开始拷贝。
    //
    bool BlitSurface (LPDIRECTDRAWSURFACE lpSourceSurface, LPDIRECTDRAWSURFACE lpDestinationSurface, POINT center, float zoomX, 
					  float zoomY, LPRECT lpWindow, LPRGBQUAD lpTransColor, float transScale);
        
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    获取设备DirectDraw支持的功能
    // Parameters:	NULL
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将测试设备DirectDraw支持的功能，同时将功能列表保存至文件。
    //
	bool GetDirectDrawCapabilities ();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    测试DirectDraw各项操作时间（暂未实现）
    // Parameters:	NULL
    // Return:      成功返回true，失败返回false。
    // Remark:      该函数将测试设备DirectDraw各项操作的具体时间，并将测试结果保存至文件。
    //
    bool TestDirectDrawOperationTime ();    
};

#endif // __FASTDRAW_H__