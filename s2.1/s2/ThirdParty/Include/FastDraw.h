////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
//                         Function: FastDraw ��ͼ��                              //
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

// ��ʽ����DirectDraw���ú���ָ�����Ͷ��壭DirectDrawCreate
typedef HRESULT (WINAPI *PFDirectDrawCreate) (LPGUID lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);

#define MAX_BUFFER_SURFACE 200      // ��󻺴��������
#define MAX_PRE_BACKGROUND 10       // ���Ԥ���屳������������������Ĭ�ϱ������棩

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function:    FastDraw��
//
class FASTDRAW_API CFastDraw {
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ���캯��
    // Remark:      ��ʼ����Ҫ��
    //
	CFastDraw();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ��ʼ��FastDraw
    // Parameters:	hWnd    HWND    [IN]    ��������
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      ��ʹ��FastDraw����ǰ����ɹ����øú���������ɱ�Ҫ�ĳ�ʼ��������
    //              ���ѳɹ����øú��������ڳ������ǰ�������ٵ���һ��Uninitialize���������ͷŸú�������Ŀռ䡣
    //              �ú��������ڽ���Ӧ�ó�����������߳��е��ã�����IDirectDraw��Ҫ�ڸ��߳����ͷţ������ڸ��߳��з��䣩��
    //
    bool Initialize (HWND hWnd);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    �ͷ�FastDraw
    // Parameters:	NULL
    // Return:      NULL
    // Remark:      �ú����ͷ���Initialize�������ٵĿռ䡣
    //              ���ɹ�����Initialize��������������ٵ���һ�θú������ͷ�Initialize��������Ŀռ䡣
    //              ��δ���û�δ�ɹ�����Initialize����������Բ����øú���������Ȼ���øú�������ú��������κδ���
    //              �ú��������ڽ���Ӧ�ó�����������߳��е��ã�����IDirectDraw��Ҫ�ڸ��߳����ͷţ���
    //
	void Uninitialize ();
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ��ӻ������
    // Parameters:	nBufferSurfaceIndex     int         [IN]    �������洢����
    //              hSourceBitmap           HBITMAP     [IN]    �����뻺������ԭʼλͼ���
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      ��nBufferSurfaceIndex����ֵλ�õĻ�������Ѿ���������ú��������ͷ��Ѵ��ڵĻ�����棬�ٽ����±��档
    //
    bool AddBufferSurface (int nBufferSurfaceIndex, HBITMAP hSourceBitmap);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ���������������ͼ����
    // Parameters:	nBufferSurfaceIndex     int                     [IN]    ������Դ�����������ֵ
    //              center                  POINT                   [IN]    ��������������꣨��Ŀ���������ϵ�У�
    //              zoomX                   float                   [IN]    �������ˮƽ�������ű���
    //              zoomY                   float                   [IN]    ���������ֱ�������ű���
    //              lpWindow                LPRECT                  [IN]    Ŀ�������ƴ���ָ�루��Ŀ���������ϵ�У���
    //                                                                      ��ֵΪNULL�����޻��ƴ������ơ�
    //                                                                      ���򣬳����ô���Ŀ������ݽ����ضϡ�
    //              lpTransColor            LPRGBQUARD              [IN]    ͸��ɫָ�롣��ֵΪNULL������͸��ɫ��
    //                                                                      ����Դ�����и���ɫ������������
    //              transScale              float                   [IN]    ͸���ȡ�ȡֵ��Χ��0��1��0Ϊȫ͸����1Ϊ��͸����
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú�����ָ���Ļ�����澭��һϵ�б任��������ͼ���档
    //              �����ݲ�֧��ͬʱ����͸��ɫ��͸���ȡ�
    //              �����һ�ο�����δ��������ú�����ȴ������������ٿ�ʼ������
    //
    bool BlitToDrawSurface (int nBufferSurfaceIndex, POINT center, float zoomX, float zoomY, LPRECT lpWindow, LPRGBQUAD lpTransColor, 
                            float transScale);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ���������������������
    // Parameters:	nBackgroundIndex        int                     [IN]    ������Ŀ�걳����������ֵ
    //              nBufferSurfaceIndex     int                     [IN]    ������Դ�����������ֵ
    //              center                  POINT                   [IN]    ��������������꣨��Ŀ���������ϵ�У�
    //              zoomX                   float                   [IN]    �������ˮƽ�������ű���
    //              zoomY                   float                   [IN]    ���������ֱ�������ű���
    //              lpWindow                LPRECT                  [IN]    Ŀ�������ƴ���ָ�루��Ŀ���������ϵ�У���
    //                                                                      ��ֵΪNULL�����޻��ƴ������ơ�
    //                                                                      ���򣬳����ô���Ŀ������ݽ����ضϡ�
    //              lpTransColor            LPRGBQUARD              [IN]    ͸��ɫָ�롣��ֵΪNULL������͸��ɫ��
    //                                                                      ����Դ�����и���ɫ������������
    //              transScale              float                   [IN]    ͸���ȡ�ȡֵ��Χ��0��1��0Ϊȫ͸����1Ϊ��͸����
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú�����ָ���Ļ�����澭��һϵ�б任������Ŀ�걳�����档
    //              �����ݲ�֧��ͬʱ����͸��ɫ��͸���ȡ�
    //              �����һ�ο�����δ��������ú�����ȴ������������ٿ�ʼ������
    //
    bool BlitToBackgroundSurface (int nBackgroundIndex, int nBufferSurfaceIndex, POINT center, float zoomX, float zoomY, LPRECT lpWindow, 
                                  LPRGBQUAD lpTransColor, float transScale);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ��������������������
    // Parameters:	NULL
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú���������ǰ�������������������档
    //              �����һ�ο�����δ��������ú�����ȴ������������ٿ�ʼ������
    //
    bool BlitPrimarySurfaceToBackgroundSurface ();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ����������������ͼ����
    // Parameters:	nBackgroundIndex    int     [IN]    ������������ֵ
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú�����ָ������ֵ�ı������濽������ͼ����
    //              �����һ�ο�����δ��������ú�����ȴ������������ٿ�ʼ������
    //
    bool BlitPreBackgroundSurefaceBackground (int nBackgroundIndex);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ����Ԥ���屳������ı���
    // Parameters:	nBackgroundIndex    int     [IN]    ������������ֵ����ֵ��ӦΪ0��0ΪĬ�ϱ������������ֵ��
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú�������һ��������ֵ��1��Ԥ���屳������Ĭ�ϱ�������Ϊ������������nBackgroundIndex����ֵ��ָ���
    //              Ԥ���屳�������С������λ�õ�Ԥ���屳����δ�����������Ƚ�����Ԥ���屳���ռ䣬Ȼ����ִ�п�����
    //              �����һ�ο�����δ��������ú�����ȴ������������ٿ�ʼ������
    //
    bool BlitBackgroundSurfaceToDrawSurface (int nBackgroundIndex);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ��ת����
    // Parameters:	NULL
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú�������ͼ���淭ת����Ļ�����棬����ʾ��
    //              ����ϴη�ת������δ��ɣ���ú�����ȴ���ת�������ٿ�ʼ��ת��
    //
    bool Flipping ();

private:
    // DirectDraw�ӿ�
    HINSTANCE m_hInstDDLib;                                         // DirectDraw�⵼����
	LPDIRECTDRAW m_pDD;					                            // DirectDraw�ӿ�ʵ��
	LPDIRECTDRAWSURFACE m_pDDSPrimary;			                    // DirectDraw������
    LPDIRECTDRAWSURFACE m_pDDSDraw;                                 // DirectDraw��ͼ����
    LPDIRECTDRAWSURFACE m_pDDSBackground;                           // DirectDrawĬ�ϱ�������
	DDCAPS m_DDDriverCaps;						                    // DirectDrawӲ�����ٹ���
	DDCAPS m_DDEmulCaps;						                    // DirectDraw���ģ�⹦��
    LPDIRECTDRAWSURFACE m_pDDSBuffers[MAX_BUFFER_SURFACE];          // DirectDraw�������
    LPDIRECTDRAWSURFACE m_pDDSPreBackground[MAX_PRE_BACKGROUND];    // DirectDrawԤ���屳������ 
    PFDirectDrawCreate m_pfDirectDrawCreate;                        // DirectDraw��ʽ���뺯����DirectDrawCreate

	bool m_bCOMInitialized;						                    // COM�Ƿ��ѳɹ���ʼ��
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ö�ٱ���ص�����
    // Parameters:	�μ�MSDN
    // Return:      �μ�MSDN
    // Remark:      ��ȡ�����������ӵĵ�һ�����棨�󱸱��棩��Ϊ��ͼ����
    //
    friend HRESULT WINAPI EnumSurfacesCallback (LPDIRECTDRAWSURFACE lpDDSurface, LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext);
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ��������
    // Parameters:	lpSourceSurface         LPDIRECTDRAWSURFACE     [IN]    Դ����ָ��
    //              lpDestinationSurface    LPDIRECTDRAWSURFACE     [IN]    Ŀ�����ָ��
    //              center                  POINT                   [IN]    ��������������꣨��Ŀ���������ϵ�У�
    //              zoomX                   float                   [IN]    �������ˮƽ�������ű���
    //              zoomY                   float                   [IN]    ���������ֱ�������ű���
    //              lpWindow                LPRECT                  [IN]    Ŀ�������ƴ���ָ�루��Ŀ���������ϵ�У���
    //                                                                      ��ֵΪNULL�����޻��ƴ������ơ�
    //                                                                      ���򣬳����ô���Ŀ������ݽ����ضϡ�
    //              lpTransColor            LPRGBQUARD              [IN]    ͸��ɫָ�롣��ֵΪNULL������͸��ɫ��
    //                                                                      ����Դ�����и���ɫ������������
    //              transScale              float                   [IN]    ͸���ȡ�ȡֵ��Χ��0��1��0Ϊȫ͸����1Ϊ��͸����
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú�����Դ���澭��һϵ�б任������Ŀ����档
    //              �����ݲ�֧��ͬʱ����͸��ɫ��͸���ȡ�
    //              �����һ�ο�����δ��������ú�����ȴ������������ٿ�ʼ������
    //
    bool BlitSurface (LPDIRECTDRAWSURFACE lpSourceSurface, LPDIRECTDRAWSURFACE lpDestinationSurface, POINT center, float zoomX, 
					  float zoomY, LPRECT lpWindow, LPRGBQUAD lpTransColor, float transScale);
        
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ��ȡ�豸DirectDraw֧�ֵĹ���
    // Parameters:	NULL
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú����������豸DirectDraw֧�ֵĹ��ܣ�ͬʱ�������б������ļ���
    //
	bool GetDirectDrawCapabilities ();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Function:    ����DirectDraw�������ʱ�䣨��δʵ�֣�
    // Parameters:	NULL
    // Return:      �ɹ�����true��ʧ�ܷ���false��
    // Remark:      �ú����������豸DirectDraw��������ľ���ʱ�䣬�������Խ���������ļ���
    //
    bool TestDirectDrawOperationTime ();    
};

#endif // __FASTDRAW_H__