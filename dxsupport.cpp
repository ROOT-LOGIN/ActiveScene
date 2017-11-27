
#include "stdafx.h"

#include "common.h"

#include "Eyereach.h"

#include "dxsupport.h"

#include "views.h"
#include "player.h"

/*
#include <D3dx9core.h>
#pragma comment(lib, "D3dx9.lib")
*/

SCENEDATA g_SceneData[MAX_MONITORS];

void SCENEDATA::updateViewMode(int value) {
    _AtlModule.setPrefDWORD(Pref_ViewMode, this->id, value);
}

void SCENEDATA::updateClipLeft(unsigned short value) {
    _AtlModule.setPrefDWORD(Pref_ClipboxLeft, this->id, value);
}

void SCENEDATA::updateClipTop(unsigned short value) {
    _AtlModule.setPrefDWORD(Pref_ClipboxTop, this->id, value);
}

void SCENEDATA::updateClipRight(unsigned short value) {
    _AtlModule.setPrefDWORD(Pref_ClipboxRight, this->id, value);
}

void SCENEDATA::updateClipBottom(unsigned short value) {
    _AtlModule.setPrefDWORD(Pref_ClipboxBottom, this->id, value);
}

void SCENEDATA::updateRotateAngle(signed short value) {
    _AtlModule.setPrefDWORD(Pref_RotateAngle, this->id, value);
}

void SCENEDATA::updateMirrorHorz(unsigned char value) {
    _AtlModule.setPrefDWORD(Pref_HorzMirrorMode, this->id, value);
}

void SCENEDATA::updateMirrorVert(unsigned char value) {
    _AtlModule.setPrefDWORD(Pref_VertMirrorMode, this->id, value);
}

void SCENEDATA::updateCellCount(signed char value) {
    _AtlModule.setPrefDWORD(Pref_CellCount, this->id, value);
}

// {F66D0D2B-0A03-4B70-AEAB-9F9CA7D0D9E8}
CLSID CLSID_CVMR9SufaceAllocator = { 0xF66D0D2B, 0x0A03, 0x4B70, { 0xAE, 0xAB, 0x9F, 0x9C, 0xA7, 0xD0, 0xD9, 0xE8 } };
CLSID CLSID_CVMR9Compositor =      { 0xBE407BF6, 0x5223, 0x4A62, { 0xA5, 0xF1, 0x52, 0x63, 0x04, 0xD2, 0xEC, 0x30 } };

/// <summary>
/// Initializes a new instance of the <see cref="CVMR9SufaceAllocator"/> class.
/// Create the D3DEx object 
/// </summary>
CVMR9SufaceAllocator::CVMR9SufaceAllocator(void)
{
    m_notifyid = 0;
    m_hwnd = NULL;
    m_Surfaces = NULL;
	
	Direct3DCreate9Ex(D3D_SDK_VERSION, &m_D3d);
}

CVMR9SufaceAllocator::~CVMR9SufaceAllocator(void)
{
    deleteSurfaces();
}

void CVMR9SufaceAllocator::deleteSurfaces(void)
{
    if(m_Surfaces != NULL) {
        m_Surfaces->free();
    }
	m_Surfaces = NULL;
	/*if(m_clientTexture) {
		m_clientTexture.Release();
	}*/
}

HRESULT CVMR9SufaceAllocator::InitializeVMR9(IBaseFilter* pVmr9, HWND hwnd, CComObject<CVMR9SufaceAllocator> **ppAlloc)
{
    CComQIPtr<IVMRFilterConfig9> config = pVmr9;
    if(!config) return E_NOINTERFACE;

    // only support in renderless mode
    HRESULT hr = config->SetRenderingMode(VMR9Mode_Renderless);
    if(FAILED(hr)) return hr;    
        
    CComQIPtr<IVMRSurfaceAllocatorNotify9> notify = pVmr9;
    if(!notify) return E_NOINTERFACE;

    CComObject<CVMR9SufaceAllocator> *pAlloc;        
    hr = CComObject<CVMR9SufaceAllocator>::CreateInstance(&pAlloc);
    if(FAILED(hr)) return hr;
    pAlloc->m_hwnd = hwnd;
	hr = pAlloc->createDevice();
    if(FAILED(hr)) return hr;

	CComQIPtr<IVMRSurfaceAllocator9> allocator = pAlloc;
    if(!allocator) return E_NOINTERFACE;
        
    // advice the surface allocator
    hr = notify->AdviseSurfaceAllocator(GetCurrentThreadId(), allocator);
    if(FAILED(hr)) return hr;
    
    hr = allocator->AdviseNotify(notify);
    if(FAILED(hr)) return hr;

    if(ppAlloc) {
        pAlloc->AddRef();
        *ppAlloc = pAlloc;
    }
    return hr;
}

HRESULT CVMR9SufaceAllocator::createDevice(void)
{
	if(!m_D3d) 
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpres;
    memset(&d3dpres, 0, sizeof(D3DPRESENT_PARAMETERS));
    d3dpres.BackBufferFormat = D3DFMT_UNKNOWN;
    /*d3dpres.BackBufferCount = D3DPRESENT_BACK_BUFFERS_MAX_EX;
    d3dpres.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
	d3dpres.MultiSampleQuality = 4;*/
	d3dpres.SwapEffect = D3DSWAPEFFECT_DISCARD;    
	//d3dpres.MultiSampleType = D3DMULTISAMPLE_NONE;
    //d3dpres.SwapEffect = D3DSWAPEFFECT_FLIP;    
    //d3dpres.SwapEffect = D3DSWAPEFFECT_COPY;
    d3dpres.Windowed = TRUE;
	d3dpres.hDeviceWindow = m_hwnd;    
    
    HRESULT hr = m_D3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd, 
        D3DCREATE_MULTITHREADED | D3DCREATE_SCREENSAVER | D3DCREATE_HARDWARE_VERTEXPROCESSING /*| D3DCREATE_ADAPTERGROUP_DEVICE*/,
        &d3dpres, &m_Dxdev);
    if(FAILED(hr)) return hr;

	hr = m_Dxdev->GetRenderTarget(0, &m_Target);
    /*if(FAILED(hr)) return hr;

    CRect crc;
    GetClientRect(m_hwnd, &crc);
    hr = m_Dxdev->CreateTexture(crc.Width(), crc.Height(), 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_clientTexture, NULL);
    */
	return hr;
}

// Create Direct3D surfaces that match the parameters given in 
// the InitializeDevice method. Optionally, you can use the VMR-9 
// filter's IVMRSurfaceAllocatorNotify9::AllocateSurfaceHelper 
// method to allocate these surfaces. 
// Store the surface pointers in an array. 
// If you want the VMR-9 to draw the video frames onto a texture 
// surface, add the VMR9AllocFlag_TextureSurface flag to the 
// VMR9AllocationInfo structure. If the device does not support 
// textures in the native video format, you might need to create 
// a separate texture surface, and then copy the video frames from 
// the video surface to the texture.
HRESULT STDMETHODCALLTYPE CVMR9SufaceAllocator::InitializeDevice( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ VMR9AllocationInfo *lpAllocInfo,
        /* [out][in] */ DWORD *lpNumBuffers)
{
    if(*lpNumBuffers == 0) return E_OUTOFMEMORY;
   
	D3DCAPS9 caps;
	HRESULT hr = m_Dxdev->GetDeviceCaps(&caps);
	if(FAILED(hr)) return hr;

	DWORD dwWidth = 1;
    DWORD dwHeight = 1;
    float fTU = 1.f;
    float fTV = 1.f;
	if(caps.TextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        while( dwWidth < lpAllocInfo->dwWidth )
            dwWidth = dwWidth << 1;
        while( dwHeight < lpAllocInfo->dwHeight )
            dwHeight = dwHeight << 1;

        fTU = (float)(lpAllocInfo->dwWidth) / (float)(dwWidth);
        fTV = (float)(lpAllocInfo->dwHeight) / (float)(dwHeight);
        m_Scene.SetSrcRect( fTU, fTV );
        lpAllocInfo->dwWidth = dwWidth;
        lpAllocInfo->dwHeight = dwHeight;
    }

	// NOTE:
    // we need to make sure that we create textures because
    // surfaces can not be textured onto a primitive.
    lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;

    ComArray<IDirect3DSurface9>* surface_ary = ComArray<IDirect3DSurface9>::Create(*lpNumBuffers);
    hr = m_VmrNotify->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, surface_ary->array);
    if(FAILED(hr)) {
        surface_ary->free();
        return hr;
    }

	if(m_Surfaces) {
		m_Surfaces->free();
	}
    surface_ary->size = *lpNumBuffers;
    m_Surfaces = surface_ary;    
    m_notifyid = dwUserID;

	return m_Scene.Init(m_Dxdev);
}
    
// When streaming stops, the VMR-9 calls.
// Should release all of its Direct3D resources
HRESULT STDMETHODCALLTYPE CVMR9SufaceAllocator::TerminateDevice( 
    /* [in] */ DWORD_PTR dwID)
{
    if(dwID != m_notifyid) return E_FAIL;
    
	if(m_Surfaces) {
		m_Surfaces->free();
		m_Surfaces = NULL;
	}
    return S_OK;
}

// During streaming, the VMR-9 gets surfaces from the allocator-presenter 
HRESULT STDMETHODCALLTYPE CVMR9SufaceAllocator::GetSurface( 
    /* [in] */ DWORD_PTR dwUserID,
    /* [in] */ DWORD SurfaceIndex,
    /* [in] */ DWORD SurfaceFlags,
    /* [out] */ IDirect3DSurface9 **lplpSurface)
{
    if(dwUserID != m_notifyid) return E_FAIL;

    *lplpSurface = m_Surfaces->at(SurfaceIndex);
	(*lplpSurface)->AddRef();
    return S_OK;
}

// Call IVMRSurfaceAllocatorNotify9::SetD3DDevice 
HRESULT STDMETHODCALLTYPE CVMR9SufaceAllocator::AdviseNotify( 
    /* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify)
{    
	HMONITOR hM = m_D3d->GetAdapterMonitor(D3DADAPTER_DEFAULT);
    HRESULT hr = lpIVMRSurfAllocNotify->SetD3DDevice(m_Dxdev, hM);
    if(FAILED(hr)) return hr;
	
    m_VmrNotify = lpIVMRSurfAllocNotify;
    lpIVMRSurfAllocNotify->AddRef();	
    return S_OK;
}

// IVMRImagePresenter9
HRESULT STDMETHODCALLTYPE CVMR9SufaceAllocator::StartPresenting( 
    /* [in] */ DWORD_PTR dwUserID)
{
	return m_Dxdev ? S_OK : E_FAIL;
}

HRESULT STDMETHODCALLTYPE CVMR9SufaceAllocator::StopPresenting( 
    /* [in] */ DWORD_PTR dwUserID)
{
    return S_OK;
}

// Present the image when the VMR-9 calls 
HRESULT STDMETHODCALLTYPE CVMR9SufaceAllocator::PresentImage( 
    /* [in] */ DWORD_PTR dwUserID,
    /* [in] */ VMR9PresentationInfo *lpPresInfo)
{
    m_Dxdev->SetRenderTarget(0, m_Target);
	
	CComPtr<IDirect3DTexture9> texture;
    HRESULT hr = lpPresInfo->lpSurf->GetContainer(IID_IDirect3DTexture9, (void**)&texture);
	if(FAILED(hr)) return hr;

/*
    RECT crc;
    GetClientRect(m_hwnd, &crc);
    
    int w = crc.right-crc.left;
    int h = crc.bottom-crc.top;
    CComPtr<IDirect3DTexture9> foregroundTxt;
    m_Dxdev->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &foregroundTxt, NULL);
    
    CComPtr<IDirect3DSurface9> foregroundSrf;
    foregroundTxt->GetSurfaceLevel(0, &foregroundSrf);

    HDC hdc9;
    foregroundSrf->GetDC(&hdc9);

    HDC hdc = GetDC(m_hwnd);
    BitBlt(hdc9, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
*/    

    m_Scene.UpdateSetting(XPref_AspectRatio, MONITORID_MASK, &lpPresInfo->szAspectRatio);
	hr = m_Scene.DrawScene(m_Dxdev, texture, NULL);
	if(FAILED(hr)) return hr;

	// IMPORTANT: device can be lost when user changes the resolution
    // or when (s)he presses Ctrl + Alt + Delete.
    // We need to restore our video memory after that
	hr = m_Dxdev->Present(NULL, NULL, NULL, NULL);
    if( hr == D3DERR_DEVICELOST)
    {
        if (m_Dxdev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        {
			if(m_Surfaces) {
				m_Surfaces->free(); m_Surfaces = NULL;
			}
            deleteSurfaces();
            hr = createDevice();
			if(FAILED(hr)) return hr;
            HMONITOR hMonitor = m_D3d->GetAdapterMonitor(D3DADAPTER_DEFAULT);
			hr = m_VmrNotify->ChangeD3DDevice(m_Dxdev, hMonitor);
			if(FAILED(hr)) return hr;
        }
        hr = S_OK;
    }

    return hr;
}

HRESULT CVMR9SufaceAllocator::UpdateSetting(DWORD sid, int mid, void* value)
{
    return m_Scene.UpdateSetting(sid, mid, value);
}

HRESULT CVMR9SufaceAllocator::QuerySetting(DWORD sid, int mid, LPVOID* pvalue)
{
    return m_Scene.QuerySetting(sid, mid, pvalue);
}

/*HRESULT CVMR9SufaceAllocator::DrawClientTexture(BYTE *pBits)
{
    if(m_clientTexture) {       
        CRect crc;
        GetClientRect(m_hwnd, crc);

        D3DLOCKED_RECT locked;                  
        if(SUCCEEDED(m_clientTexture->LockRect(0, &locked, &crc, D3DLOCK_NOSYSLOCK))) {

            // need reverse
            //memcpy(locked.pBits, pBits, ((crc.Width() * 32 + 31) / 32) * 4 * crc.Height());

            int width = ((crc.Width() * 32 + 31) / 32) * 4;
            pBits += width * (crc.Height() - 1);
            BYTE* ps = (BYTE*)locked.pBits;
            for (int r = 0; r < crc.Height(); r++) {
                memcpy(ps, pBits, width);
                ps += width;
                pBits -= width;
            }
            m_clientTexture->UnlockRect(0);
        }
    }
    return S_OK;
}
*/

// =======================================

 #define _USE_MATH_DEFINES
 #include <math.h>


//#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 )
#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZRHW | D3DFVF_TEX1 )
//#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_TEX1 )


// Matrix functions
D3DMATRIX* MatrixPerspectiveFovLH(
    D3DMATRIX * pOut,
    FLOAT fovy,
    FLOAT Aspect,
    FLOAT zn,
    FLOAT zf
    );


D3DMATRIX* MatrixLookAtLH( 
    D3DMATRIX *pOut, 
    const D3DVECTOR *pEye, 
    const D3DVECTOR *pAt,
    const D3DVECTOR *pUp 
    );


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlaneScene::CPlaneScene()
{
    memset(m_vertices, 0, sizeof(m_vertices));
    g_SceneData[0].id = 0;
    g_SceneData[1].id = 1;    

    /*m_vertices[0].position = CUSTOMVERTEX::Position(-1.0f,  1.0f, 0.0f); // top left
    m_vertices[1].position = CUSTOMVERTEX::Position(-1.0f, -1.0f, 0.0f); // bottom left
    m_vertices[2].position = CUSTOMVERTEX::Position( 1.0f,  1.0f, 0.0f); // top right
    m_vertices[3].position = CUSTOMVERTEX::Position( 1.0f, -1.0f, 0.0f); // bottom right
    */
        
    // set up diffusion:
 //   m_vertices[0].color = 0xffffffff;
 //   m_vertices[1].color = 0xffffffff;
 //   m_vertices[2].color = 0xffffffff;
 //   m_vertices[3].color = 0xffffffff;
	//m_vertices[0+4].color = 0xffffffff;
 //   m_vertices[1+4].color = 0xffffffff;
 //   m_vertices[2+4].color = 0xffffffff;
 //   m_vertices[3+4].color = 0xffffffff;

    // set up texture coordinates
    for(int i = 0; i< MAX_MONITORS; i++) {
        CUSTOMVERTEX* pvertex = &this->vertexFor(i, false); 
        pvertex[0].tu = 0.0f; pvertex[0].tv = 0.0f; // low left
        pvertex[1].tu = 0.0f; pvertex[1].tv = 2.0f; // high left
        pvertex[2].tu = 2.0f; pvertex[2].tv = 0.0f; // low right
        pvertex[3].tu = 2.0f; pvertex[3].tv = 2.0f; // high right

        pvertex = &this->vertexFor(i, true); 
        pvertex[0].tu = 0.0f; pvertex[0].tv = 0.0f; // low left
        pvertex[1].tu = 0.0f; pvertex[1].tv = 1.0f; // high left
        pvertex[2].tu = 1.0f; pvertex[2].tv = 0.0f; // low right
        pvertex[3].tu = 1.0f; pvertex[3].tv = 1.0f; // high right
    }    
}

CPlaneScene::~CPlaneScene()
{

}

#define ASSIGN_IF_NOTEQU_RET_EQU_HR(left, right, hr) \
    if(left != right) left = right; \
    else return hr

HRESULT CPlaneScene::UpdateSetting(DWORD sid, int mid, void* value)
{
    switch(sid)
    __beginScope()
    default: return E_INVALIDARG;
    case Pref_ClipboxLeft:    ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].clipLeft, (WORD)value, S_FALSE); break;
    case Pref_ClipboxTop:     ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].clipTop, (WORD)value, S_FALSE); break;
    case Pref_ClipboxRight:   ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].clipRight, (WORD)value, S_FALSE); break;
    case Pref_ClipboxBottom:  ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].clipBottom, (WORD)value, S_FALSE); break;
    case Pref_RotateAngle:    ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].rotateAngle, (WORD)value, S_FALSE); break;
    case Pref_HorzMirrorMode: ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].mirrorHorz, (WORD)value, S_FALSE); break;
    case Pref_VertMirrorMode: ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].mirrorVert, (WORD)value, S_FALSE); break;
    case Pref_ViewMode:       ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].viewMode, (unsigned char)(int)value, S_FALSE); 
        switch (g_SceneData[mid].viewMode)
        {
        case SceneViewMode::PSVM_Stretch:
        case SceneViewMode::PSVM_Center:
            g_SceneData[mid].cellCount = 1; break;
        case SceneViewMode::PSVM_MirrorTile:
        case SceneViewMode::PSVM_WrapTile:
            if (g_SceneData[mid].cellCount == 1)
                g_SceneData[mid].cellCount = 4;
            break;
        }
        break;
    case Pref_CellCount:      ASSIGN_IF_NOTEQU_RET_EQU_HR(g_SceneData[mid].cellCount, (signed char)(int)value, S_FALSE); break;
    case XPref_AspectRatio: {
        LPSIZE psz = (LPSIZE)value;
        if((m_aspectRatioX != (float)psz->cx) || (m_aspectRatioX != (float)psz->cx)) {
            m_aspectRatioX = (float)psz->cx; m_aspectRatioY = (float)psz->cy;
        } else return S_FALSE;
    }break;
    __endScope()

    internalUpdateViewSetting(mid);
    return S_OK;
}

HRESULT CPlaneScene::QuerySetting(DWORD sid, int mid, LPVOID* pvalue)
{
    return S_OK;
}

HRESULT CPlaneScene::Init(IDirect3DDevice9* d3ddev)
{
    HRESULT hr;

    if( ! d3ddev )
        return E_POINTER;
    m_d3ddev = d3ddev;

    /*hr = d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    hr = d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    hr = d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    hr = d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    hr = d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	hr = d3ddev->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_MAX);
    hr = d3ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    hr = d3ddev->SetRenderState(D3DRS_ALPHAREF, 0x0);
    hr = d3ddev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	hr = d3ddev->SetRenderState(D3DRS_LASTPIXEL, TRUE);
    hr = d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    hr = d3ddev->SetRenderState(D3DRS_LOCALVIEWER, 1);
    */
    hr = d3ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    hr = d3ddev->SetRenderState(D3DRS_ZENABLE, D3DZBUFFERTYPE::D3DZB_FALSE);
    hr = d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
    hr = d3ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    
    hr = d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    hr = d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
    hr = d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);
    
    hr = d3ddev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
    hr = d3ddev->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
    hr = d3ddev->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);

    hr = d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);

    hr = d3ddev->SetRenderState(D3DRS_LOCALVIEWER, TRUE);
    

	//hr = d3ddev->SetRenderState(D3DRS_TEXTUREFACTOR, 0x10101010);
	//hr = d3ddev->SetRenderState(D3DRS_BLENDFACTOR, 0xFFFFFFFF);

    /*FAIL_RET(hr = d3ddev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    FAIL_RET(hr = d3ddev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
    FAIL_RET(hr = d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
    FAIL_RET(hr = d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
    FAIL_RET(hr = d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));*/

    hr = d3ddev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    hr = d3ddev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    hr = d3ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_GAUSSIANQUAD);
    hr = d3ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    hr = d3ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

    m_vertexBuffer = NULL;

    int kk = sizeof(m_vertices);
    // hr = d3ddev->CreateVertexBuffer(sizeof(m_vertices),D3DUSAGE_WRITEONLY,D3DFVF_CUSTOMVERTEX,D3DPOOL_MANAGED,& m_vertexBuffer, NULL );
	hr = d3ddev->CreateVertexBuffer(sizeof(m_vertices), 0,D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT,& m_vertexBuffer, NULL );
	if(FAILED(hr)) return hr;

    CComPtr<IDirect3DSurface9> backBuffer;
    FAIL_RET(d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, & backBuffer ));

    D3DSURFACE_DESC backBufferDesc;
    backBuffer->GetDesc(&backBufferDesc);

    // Set the projection matrix
    D3DMATRIX matProj;
    FLOAT fAspect = backBufferDesc.Width / (float)backBufferDesc.Height;
    MatrixPerspectiveFovLH( &matProj, (float)M_PI_4, fAspect, 1.0f, 100.0f );
   // FAIL_RET( d3ddev->SetTransform( D3DTS_PROJECTION, &matProj ) );


    D3DVECTOR from = { 1.0f, 1.0f, -3.0f };
    D3DVECTOR at = { 0.0f, 0.0f, 0.0f };
    D3DVECTOR up = { 0.0f, 1.0f, 0.0f };

    D3DMATRIX matView;
    MatrixLookAtLH( &matView, & from, & at, & up);
   // FAIL_RET( d3ddev->SetTransform( D3DTS_VIEW, &matView ) );

	m_time = GetTickCount();

    MonitorWrap MI;
    MI.rcMonitor.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    MI.rcMonitor.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    MI.rcMonitor.right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + MI.rcMonitor.left;
    MI.rcMonitor.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + MI.rcMonitor.top;
    CUSTOMVERTEX* pvertex = &this->vertexFor(MONITORID_MASK, true);
    pvertex[0].position = CUSTOMVERTEX::Position(MI.rcMonitor.left, MI.rcMonitor.top, 0.0f); // top left
    pvertex[1].position = CUSTOMVERTEX::Position(MI.rcMonitor.left, MI.rcMonitor.bottom, 0.0f); // bottom left
    pvertex[2].position = CUSTOMVERTEX::Position(MI.rcMonitor.right, MI.rcMonitor.top, 0.0f); // top right
    pvertex[3].position = CUSTOMVERTEX::Position(MI.rcMonitor.right, MI.rcMonitor.bottom, 0.0f); // bottom right
    pvertex[0].tu = 0.0; pvertex[0].tv = 0.0;
    pvertex[1].tu = 0.0; pvertex[1].tv = 1.0;
    pvertex[2].tu = 1.0; pvertex[2].tv = 0.0;
    pvertex[3].tu = 1.0; pvertex[3].tv = 1.0;

    internalUpdateViewSetting(MONITORID_MASK);
    return hr;
}

C_ASSERT(REGIONS_PER_MONITOR == 4);
C_ASSERT(VERTICES_PER_REGION == 4);

void CPlaneScene::internalUpdateViewSetting(int mid)
{
    MonitorWrap MI;
    int CM = MI.getMonitorCount();
    CUSTOMVERTEX* pvertex;
    mid &= 0xFF;
    if(mid < CM) CM = mid + 1;
    else if(mid == 0xFF) mid = 0;
    else return;
    
    for(; mid < CM; mid++)
    {
        MI.Fill(mid);
        pvertex = &this->vertexFor(mid, false);
        switch (g_SceneData[mid].viewMode)
        {
        default: g_SceneData[mid].viewMode = PSVM_MirrorTile;
        case PSVM_MirrorTile: _updateVertexDataTileView(mid, MI.rcMonitor, pvertex, false); break;
        case PSVM_Stretch: _updateVertexDataStretchView(mid, MI.rcMonitor, pvertex); break;        
        case PSVM_WrapTile: _updateVertexDataTileView(mid, MI.rcMonitor, pvertex, true); break;
        case PSVM_Center: _updateVertexDataCenterView(mid, MI.rcMonitor, pvertex); break;
        }
    }
}

union D2DMATRIX
{
    struct
    {
        float _11, _12, _13;
        float _21, _22, _23;
        float _31, _32, _33;
    };
    float m[3][3];
};

union D2DPOINT
{
    struct
    {
        float x, y, _1;
    };
    float m[3];
};

D2DPOINT operator*(D2DPOINT& pp, D2DMATRIX& pm)
{
    D2DPOINT pr;
    pr.x = pp.x * pm._11 + pp.y * pm._21 + pm._31;
    pr.y = pp.x * pm._12 + pp.y * pm._22 + pm._32;
    return pr;
}

D2DPOINT& operator*=(D2DPOINT& pp, D2DMATRIX& pm)
{
    pp._1 = pp.x * pm._11 + pp.y * pm._21 + pm._31;
    pp.y = pp.x * pm._12 + pp.y * pm._22 + pm._32;
    pp.x = pp._1;
    pp._1 = 1.0f;
    return pp;
}

void CPlaneScene::_updateVertexDataCenterView(int mid, RECT monitor, CUSTOMVERTEX* pVertex) 
{
    CRect rc(monitor);    
    pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, monitor.top, 0.0); // l, t
    pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, monitor.bottom, 0.0); // l, b
    pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, monitor.top, 0.0); // r, t
    pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, monitor.bottom, 0.0); // r, b
    // rotate
    float angle = g_SceneData[mid].rotateAngle;
    angle *= 3.1415926f / 180.0f;
    for(int i=0; i<4;i++) {
        D2DPOINT pt = { pVertex[i].position.x - rc.CenterPoint().x, pVertex[i].position.y - rc.CenterPoint().y, };
        D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, rc.CenterPoint().x, rc.CenterPoint().y, 1.0f };
        pt *= mx;
        pVertex[i].position.x = pt.x;
        pVertex[i].position.y = pt.y;
    }
    // clip    
    /*float t, x, y, invx, invy;
    t = tanf(angle); x = cosf(angle); y = sinf(angle);
    invx = rc.Width() / 2.0f; invy = rc.Height() / 2.0f;
    D2DMATRIX translate_center = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, invx, invy, 1.0f };
    D2DMATRIX translate_left   = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -invy * x, -invy * y, 1.0f };
    D2DMATRIX translate_right  = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, invy * x, invy * y, 1.0f };

    pVertex[0].position.y = monitor.top;
    pVertex[1].position.y = monitor.bottom;
    pVertex[2].position.y = monitor.top;
    pVertex[3].position.y = monitor.bottom;
    
    pVertex[0].position.x = (pVertex[0].position.y - invy + invy * x) / t + invx - invy * y;
    pVertex[1].position.x = (pVertex[1].position.y - invy + invy * x) / t + invx - invy * y;
    pVertex[2].position.x = (pVertex[2].position.y + invy + invy * x) / t + invx + invy * y;
    pVertex[3].position.x = (pVertex[3].position.y + invy + invy * x) / t + invx + invy * y;
    // tanf(angle) * (x - rc.Width() /2 + rc.Height() * sinf(angle) /2) - rc.Height() /2 + rc.Height() * cosf(angle) /2
    // tanf(angle) * (x - rc.Width() /2 - rc.Height() * sinf(angle) /2) - rc.Height() /2 - rc.Height() * cosf(angle) /2
    */
    //mirror
    float x = -(float)g_SceneData[mid].clipLeft / m_aspectRatioX;
    float y = -(float)g_SceneData[mid].clipTop / m_aspectRatioY;
    float invx = 1 + (float)g_SceneData[mid].clipRight / m_aspectRatioX;
    float invy = 1 + (float)g_SceneData[mid].clipBottom / m_aspectRatioY;

    if(g_SceneData[mid].mirrorHorz)
    {
        pVertex[0].tu = invx;    
        pVertex[1].tu = invx;    
        pVertex[2].tu = x; 
        pVertex[3].tu = x; 
    }
    else
    {
        pVertex[0].tu = x;    
        pVertex[1].tu = x;    
        pVertex[2].tu = invx; 
        pVertex[3].tu = invx; 
    }
    if(g_SceneData[mid].mirrorVert)
    {
        pVertex[0].tv = invy;
        pVertex[1].tv = y;
        pVertex[2].tv = invy;
        pVertex[3].tv = y;
    }
    else
    {
        pVertex[0].tv = y;
        pVertex[1].tv = invy;
        pVertex[2].tv = y;
        pVertex[3].tv = invy;
    }
}

void CPlaneScene::_updateVertexDataStretchView(int mid, RECT monitor, CUSTOMVERTEX* pVertex) 
{
    CRect rc(monitor);    
    pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, monitor.top, 0.0); // l, t
    pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, monitor.bottom, 0.0); // l, b
    pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, monitor.top, 0.0); // r, t
    pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, monitor.bottom, 0.0); // r, b
    // rotate
    float angle = g_SceneData[mid].rotateAngle;
    angle *= 3.1415926f / 180.0f;
    for (int i = 0; i < 4; i++) {
        D2DPOINT pt = { pVertex[i].position.x - rc.CenterPoint().x, pVertex[i].position.y - rc.CenterPoint().y, };
        D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, rc.CenterPoint().x, rc.CenterPoint().y, 1.0f };
        pt *= mx;
        pVertex[i].position.x = pt.x;
        pVertex[i].position.y = pt.y;
    }
    // clip    
    /*float t, x, y, invx, invy;
    t = tanf(angle); x = cosf(angle); y = sinf(angle);
    invx = rc.Width() / 2.0f; invy = rc.Height() / 2.0f;
    D2DMATRIX translate_center = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, invx, invy, 1.0f };
    D2DMATRIX translate_left   = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -invy * x, -invy * y, 1.0f };
    D2DMATRIX translate_right  = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, invy * x, invy * y, 1.0f };

    pVertex[0].position.y = monitor.top;
    pVertex[1].position.y = monitor.bottom;
    pVertex[2].position.y = monitor.top;
    pVertex[3].position.y = monitor.bottom;
    
    pVertex[0].position.x = (pVertex[0].position.y - invy + invy * x) / t + invx - invy * y;
    pVertex[1].position.x = (pVertex[1].position.y - invy + invy * x) / t + invx - invy * y;
    pVertex[2].position.x = (pVertex[2].position.y + invy + invy * x) / t + invx + invy * y;
    pVertex[3].position.x = (pVertex[3].position.y + invy + invy * x) / t + invx + invy * y;
    // tanf(angle) * (x - rc.Width() /2 + rc.Height() * sinf(angle) /2) - rc.Height() /2 + rc.Height() * cosf(angle) /2
    // tanf(angle) * (x - rc.Width() /2 - rc.Height() * sinf(angle) /2) - rc.Height() /2 - rc.Height() * cosf(angle) /2
    */
    //mirror
    float x = (float)g_SceneData[mid].clipLeft / m_aspectRatioX;
    float y = (float)g_SceneData[mid].clipTop / m_aspectRatioY;
    float invx = 1 - (float)g_SceneData[mid].clipRight / m_aspectRatioX;
    float invy = 1 - (float)g_SceneData[mid].clipBottom / m_aspectRatioY;

    if(g_SceneData[mid].mirrorHorz)
    {
        pVertex[0].tu = invx;    
        pVertex[1].tu = invx;    
        pVertex[2].tu = x; 
        pVertex[3].tu = x; 
    }
    else
    {
        pVertex[0].tu = x;    
        pVertex[1].tu = x;    
        pVertex[2].tu = invx; 
        pVertex[3].tu = invx; 
    }
    if(g_SceneData[mid].mirrorVert)
    {
        pVertex[0].tv = invy;
        pVertex[1].tv = y;
        pVertex[2].tv = invy;
        pVertex[3].tv = y;
    }
    else
    {
        pVertex[0].tv = y;
        pVertex[1].tv = invy;
        pVertex[2].tv = y;
        pVertex[3].tv = invy;
    }
}

void CPlaneScene::_updateVertexDataFitView(int mid, RECT monitor, CUSTOMVERTEX* pVertex) 
{
    float cl = g_SceneData[mid].clipLeft, cr = g_SceneData[mid].clipRight, 
          ct = g_SceneData[mid].clipTop,  cb = g_SceneData[mid].clipBottom;

    float Dx = m_aspectRatioX - cl - cr, Dy = m_aspectRatioY - ct - cb;    
    float Sx = abs(monitor.right - monitor.left), Sy = abs(monitor.bottom - monitor.top);

    float d = 1.0; 
    while(Dx < Sx || Dy < Sy) {
        d += 0.25;
        Dx *= d;
        Dy *= d;        
    };
        
    float ox = (Dx - Sx) / Dx / d / 2;
    float oy = (Dy - Sy) / Dy / d / 2;    
    Dx /= d; Dy /= d;

    float x = (ox * Dx + cl) / m_aspectRatioX,
          y = (oy * Dy + ct) / m_aspectRatioY,
          invx = 1 - (ox * Dx + cr) / m_aspectRatioX,
          invy = 1 - (oy * Dy + cb) / m_aspectRatioY;

    pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, monitor.top, 0.0); // l, t
    pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, monitor.bottom, 0.0); // l, b
    pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, monitor.top, 0.0); // r, t
    pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, monitor.bottom, 0.0); // r, b
    pVertex[0].tu = x;    pVertex[0].tv = y;
    pVertex[1].tu = x;    pVertex[1].tv = invy;
    pVertex[2].tu = invx; pVertex[2].tv = y;
    pVertex[3].tu = invx; pVertex[3].tv = invy;
}

void CPlaneScene::_updateVertexDataTileView(int mid, RECT monitor, CUSTOMVERTEX* pVertex, bool wrapMode) 
{
    if (abs(g_SceneData[mid].cellCount) == 2)
    {
        _updateVertexDataTileViewSplit(mid, monitor, pVertex, wrapMode);
        return;
    }
    /*float Dx = m_aspectRatioX, Dy = m_aspectRatioY;
    float Sx = abs(monitor.right - monitor.left), Sy = abs(monitor.bottom - monitor.top);

    float d = 1.0; 
    while(Dx < Sx || Dy < Sy) {
        d += 0.25;
        Dx *= d;
        Dy *= d;        
    };
        
    float x = (Dx - Sx) / Dx / d / 2;
    float y = (Dy - Sy) / Dy / d / 2;
    
    Sx /= 2.0;
    Sy /= 2.0;*/
    
    CRect rc(monitor);
    float x, y, invx, invy;
    if(g_SceneData[mid].mirrorHorz)
    {
        invx = (float)g_SceneData[mid].clipLeft / m_aspectRatioX;
        x = 1 - (float)g_SceneData[mid].clipRight / m_aspectRatioX;
    }
    else
    {
        x = (float)g_SceneData[mid].clipLeft / m_aspectRatioX;
        invx = 1 - (float)g_SceneData[mid].clipRight / m_aspectRatioX;
    }

    if(g_SceneData[mid].mirrorVert)
    {
        invy = (float)g_SceneData[mid].clipTop / m_aspectRatioY;
        y = 1 - (float)g_SceneData[mid].clipBottom / m_aspectRatioY;
    }
    else
    {
        y = (float)g_SceneData[mid].clipTop / m_aspectRatioY;
        invy = 1 - (float)g_SceneData[mid].clipBottom / m_aspectRatioY;
    }
    
    // rotate
    float angle = g_SceneData[mid].rotateAngle;
    angle *= 3.1415926f / 180.0f;    

    // left, top
    pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, monitor.top, 0.0); // l, t
    pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, rc.CenterPoint().y, 0.0); // l, b
    pVertex[2].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.top, 0.0); // r, t
    pVertex[3].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, rc.CenterPoint().y, 0.0); // r, b
    pVertex[0].tu = x;    pVertex[0].tv = y;
    pVertex[1].tu = x;    pVertex[1].tv = invy;
    pVertex[2].tu = invx; pVertex[2].tv = y;
    pVertex[3].tu = invx; pVertex[3].tv = invy;
    CRect crc(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
    for(int i=0; i<4;i++) {
        D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
        D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
        pt *= mx;
        pVertex[i].position.x = pt.x;
        pVertex[i].position.y = pt.y;
    }
    // left, bottom
    pVertex += 4;
    pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, rc.CenterPoint().y, 0.0);
    pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, monitor.bottom, 0.0);
    pVertex[2].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, rc.CenterPoint().y, 0.0);
    pVertex[3].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.bottom, 0.0);
    if(wrapMode) {
        pVertex[0].tu = x;    pVertex[0].tv = y;
        pVertex[1].tu = x;    pVertex[1].tv = invy;
        pVertex[2].tu = invx; pVertex[2].tv = y;
        pVertex[3].tu = invx; pVertex[3].tv = invy;
    }
    else {
        pVertex[0].tu = x;    pVertex[0].tv = invy;
        pVertex[1].tu = x;    pVertex[1].tv = y;
        pVertex[2].tu = invx; pVertex[2].tv = invy;
        pVertex[3].tu = invx; pVertex[3].tv = y;
    }
    crc = CRect(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
    for(int i=0; i<4;i++) {
        D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
        D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
        pt *= mx;
        pVertex[i].position.x = pt.x;
        pVertex[i].position.y = pt.y;
    }

    // right, top
    pVertex += 4;
    pVertex[0].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.top, 0.0);
    pVertex[1].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, rc.CenterPoint().y, 0.0);
    pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, monitor.top, 0.0);
    pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, rc.CenterPoint().y, 0.0);
    if(wrapMode) {
        pVertex[0].tu = x;    pVertex[0].tv = y;
        pVertex[1].tu = x;    pVertex[1].tv = invy;
        pVertex[2].tu = invx; pVertex[2].tv = y;
        pVertex[3].tu = invx; pVertex[3].tv = invy;
    }
    else {
        pVertex[0].tu = invx; pVertex[0].tv = y;
        pVertex[1].tu = invx; pVertex[1].tv = invy;
        pVertex[2].tu = x;    pVertex[2].tv = y;
        pVertex[3].tu = x;    pVertex[3].tv = invy;
    }
    crc = CRect(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
    for(int i=0; i<4;i++) {
        D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
        D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
        pt *= mx;
        pVertex[i].position.x = pt.x;
        pVertex[i].position.y = pt.y;
    }

    // right, bottom
    pVertex += 4;
    pVertex[0].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, rc.CenterPoint().y, 0.0);
    pVertex[1].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.bottom, 0.0);    
    pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, rc.CenterPoint().y, 0.0);
    pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, monitor.bottom, 0.0);
    if(wrapMode) {
        pVertex[0].tu = x;       pVertex[0].tv = y;
        pVertex[1].tu = x;       pVertex[1].tv = invy;
        pVertex[2].tu = invx; pVertex[2].tv = y;
        pVertex[3].tu = invx; pVertex[3].tv = invy;
    }
    else {
        pVertex[0].tu = invx; pVertex[0].tv = invy;
        pVertex[1].tu = invx; pVertex[1].tv = y;
        pVertex[2].tu = x;    pVertex[2].tv = invy;
        pVertex[3].tu = x;    pVertex[3].tv = y;
    }
    crc = CRect(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
    for(int i=0; i<4;i++) {
        D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
        D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
        pt *= mx;
        pVertex[i].position.x = pt.x;
        pVertex[i].position.y = pt.y;
    }

    /*// rotate
    float angle = g_SceneData[mid].rotateAngle;
    angle *= 3.1415926f / 180.0f;
    pVertex -= 12;
    for(int i=0; i<16;i++) {
        D2DPOINT pt = { pVertex[i].position.x - rc.CenterPoint().x, pVertex[i].position.y - rc.CenterPoint().y, };
        D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, rc.CenterPoint().x, rc.CenterPoint().y, 1.0f };
        pt *= mx;
        pVertex[i].position.x = pt.x;
        pVertex[i].position.y = pt.y;
    }
    */
}

void CPlaneScene::_updateVertexDataTileViewSplit(int mid, RECT monitor, CUSTOMVERTEX* pVertex, bool wrapMode)
{
    CRect rc(monitor);
    float x, y, invx, invy;
    if (g_SceneData[mid].mirrorHorz)
    {
        invx = (float)g_SceneData[mid].clipLeft / m_aspectRatioX;
        x = 1 - (float)g_SceneData[mid].clipRight / m_aspectRatioX;
    }
    else
    {
        x = (float)g_SceneData[mid].clipLeft / m_aspectRatioX;
        invx = 1 - (float)g_SceneData[mid].clipRight / m_aspectRatioX;
    }

    if (g_SceneData[mid].mirrorVert)
    {
        invy = (float)g_SceneData[mid].clipTop / m_aspectRatioY;
        y = 1 - (float)g_SceneData[mid].clipBottom / m_aspectRatioY;
    }
    else
    {
        y = (float)g_SceneData[mid].clipTop / m_aspectRatioY;
        invy = 1 - (float)g_SceneData[mid].clipBottom / m_aspectRatioY;
    }

    // rotate
    float angle = g_SceneData[mid].rotateAngle;
    angle *= 3.1415926f / 180.0f;

    if (g_SceneData[mid].cellCount == -2)
    {
        // horz split
        // top
        pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, monitor.top, 0.0); // l, t
        pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, rc.CenterPoint().y, 0.0); // l, b
        pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, monitor.top, 0.0); // r, t
        pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, rc.CenterPoint().y, 0.0); // r, b
        pVertex[0].tu = x;    pVertex[0].tv = y;
        pVertex[1].tu = x;    pVertex[1].tv = invy;
        pVertex[2].tu = invx; pVertex[2].tv = y;
        pVertex[3].tu = invx; pVertex[3].tv = invy;
        CRect crc(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
        for (int i = 0; i<4; i++) {
            D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
            D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
            pt *= mx;
            pVertex[i].position.x = pt.x;
            pVertex[i].position.y = pt.y;
        }
        // bottom
        pVertex += 4;
        pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, rc.CenterPoint().y, 0.0);
        pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, monitor.bottom, 0.0);
        pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, rc.CenterPoint().y, 0.0);
        pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, monitor.bottom, 0.0);
        if (wrapMode) {
            pVertex[0].tu = x;    pVertex[0].tv = y;
            pVertex[1].tu = x;    pVertex[1].tv = invy;
            pVertex[2].tu = invx; pVertex[2].tv = y;
            pVertex[3].tu = invx; pVertex[3].tv = invy;
        }
        else {
            pVertex[0].tu = x;    pVertex[0].tv = invy;
            pVertex[1].tu = x;    pVertex[1].tv = y;
            pVertex[2].tu = invx; pVertex[2].tv = invy;
            pVertex[3].tu = invx; pVertex[3].tv = y;
        }
        crc = CRect(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
        for (int i = 0; i<4; i++) {
            D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
            D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
            pt *= mx;
            pVertex[i].position.x = pt.x;
            pVertex[i].position.y = pt.y;
        }
    }
    else
    {
        // vert split
        // left
        pVertex[0].position = CUSTOMVERTEX::Position(monitor.left, monitor.top, 0.0); // l, t
        pVertex[1].position = CUSTOMVERTEX::Position(monitor.left, monitor.bottom, 0.0); // l, b
        pVertex[2].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.top, 0.0); // r, t
        pVertex[3].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.bottom, 0.0); // r, b
        pVertex[0].tu = x;    pVertex[0].tv = y;
        pVertex[1].tu = x;    pVertex[1].tv = invy;
        pVertex[2].tu = invx; pVertex[2].tv = y;
        pVertex[3].tu = invx; pVertex[3].tv = invy;
        CRect crc(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
        for (int i = 0; i<4; i++) {
            D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
            D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
            pt *= mx;
            pVertex[i].position.x = pt.x;
            pVertex[i].position.y = pt.y;
        }
        // right
        pVertex += 4;
        pVertex[0].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.top, 0.0);
        pVertex[1].position = CUSTOMVERTEX::Position(rc.CenterPoint().x, monitor.bottom, 0.0);
        pVertex[2].position = CUSTOMVERTEX::Position(monitor.right, monitor.top, 0.0);
        pVertex[3].position = CUSTOMVERTEX::Position(monitor.right, monitor.bottom, 0.0);
        if (wrapMode) {
            pVertex[0].tu = x;    pVertex[0].tv = y;
            pVertex[1].tu = x;    pVertex[1].tv = invy;
            pVertex[2].tu = invx; pVertex[2].tv = y;
            pVertex[3].tu = invx; pVertex[3].tv = invy;
        }
        else {
            pVertex[0].tu = invx; pVertex[0].tv = y;
            pVertex[1].tu = invx; pVertex[1].tv = invy;
            pVertex[2].tu = x;    pVertex[2].tv = y;
            pVertex[3].tu = x;    pVertex[3].tv = invy;
        }
        crc = CRect(pVertex[0].position.x, pVertex[0].position.y, pVertex[3].position.x, pVertex[3].position.y);
        for (int i = 0; i<4; i++) {
            D2DPOINT pt = { pVertex[i].position.x - crc.CenterPoint().x, pVertex[i].position.y - crc.CenterPoint().y, };
            D2DMATRIX mx = { cosf(angle), -sinf(angle), 0.0f, sinf(angle), cosf(angle), 0.0f, crc.CenterPoint().x, crc.CenterPoint().y, 1.0f };
            pt *= mx;
            pVertex[i].position.x = pt.x;
            pVertex[i].position.y = pt.y;
        }
    }       
}

D3DMATRIX* d3dmatrix_identity(D3DMATRIX* pmx) {
    memset(pmx, 0, sizeof(D3DMATRIX));    
    pmx->_11 = 1.0f;
    pmx->_22 = 1.0f;
    pmx->_33 = 1.0f;
    pmx->_44 = 1.0f;
    return pmx;
}

D3DMATRIX* d3dmatrix_scale_xy(D3DMATRIX* pmx, float fx, float fy) {
    d3dmatrix_identity(pmx);
    pmx->_11 = fx;
    pmx->_22 = fy;
    return pmx;
}

D3DMATRIX* d3dmatrix_rotate_xy(D3DMATRIX* pmx, float angle) {
    d3dmatrix_identity(pmx);
    pmx->_22 = pmx->_11 = cosf(angle);
    pmx->_21 = sinf(angle);
    pmx->_12 = 0.0f - pmx->_21;
    return pmx;
}

D3DMATRIX* d3dmatrix_translate_xy(D3DMATRIX* pmx, float w, float h) {
    d3dmatrix_identity(pmx);
    pmx->_41 = w;
    pmx->_42 = h;
    return pmx;
}

D3DMATRIX* d3dmatrix_mirror_xy(D3DMATRIX* pmx, float a, float b, float c) {
    d3dmatrix_identity(pmx);
    float p = 1.0f / (a*a + b*b);
    pmx->_11 = (b*b-a*a) * p;
    pmx->_22 = (0.0f - pmx->_11) * p;
    pmx->_12 = pmx->_21 = -2.0f * a * b * p;
    pmx->_41 = -2.0f * a * c * p;
    pmx->_42 = -2.0f * b * c * p;
    return pmx;
}

D3DMATRIX IDENTITYMATRIX = { 
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

HRESULT 
CPlaneScene::DrawScene(IDirect3DDevice9* d3ddev, IDirect3DTexture9* texture, IDirect3DTexture9* foregroundTexture)
{
    HRESULT hr;    

    if( !( d3ddev && texture ) )
    {
        return E_POINTER;
    }

    if( m_vertexBuffer == NULL )
    {
        return D3DERR_INVALIDCALL;
    }

    // get the difference in time
    DWORD dwCurrentTime;
    dwCurrentTime = GetTickCount();
    double difference = 0;// m_time - dwCurrentTime ;
    
    // figure out the rotation of the plane
    float x = (float) ( -cos(difference / 2000.0 ) ) ;
    float y = (float) ( cos(difference / 2000.0 ) ) ;
    float z = (float) ( sin(difference / 2000.0 ) ) ;

    // update the two rotating vertices with the new position
    //m_vertices[0].position = CUSTOMVERTEX::Position(x,  y, z);   // top left
    //m_vertices[3].position = CUSTOMVERTEX::Position(-x, -y, -z); // bottom right
    
   /* if(MI.getMonitorCount() == 1)
    {
        MI.Fill(GetDesktopWindow());
    }
    else {
        MI.Fill(1);
    }*/
    MonitorWrap MI;
    int CM = MI.getMonitorCount();    
    CUSTOMVERTEX* pvertex;

    

    // Adjust the color so the blue is always on the bottom.
    // As the corner approaches the bottom, get rid of all the other
    // colors besides blue
   /* DWORD mask0 = (DWORD) (255 * ( ( y + 1.0  )/ 2.0 ));
    DWORD mask3 = (DWORD) (255 * ( ( -y + 1.0  )/ 2.0 ));
    m_vertices[0].color = 0xff0000ff | ( mask0 << 16 ) | ( mask0 << 8 );
    m_vertices[3].color = 0xff0000ff | ( mask3 << 16 ) | ( mask3 << 8 );
	*/
	
	void* pData;
    FAIL_RET( m_vertexBuffer->Lock(0,sizeof(pData), &pData,0) );
    memcpy(pData,m_vertices,sizeof(m_vertices));                            
    FAIL_RET( m_vertexBuffer->Unlock() );  
    
    // write the new vertex information into the buffer
    // clear the scene so we don't have any articats left
    d3ddev->Clear( 0L, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0x0,0x0,0x0,0x0), 1.0f, 0L);

    FAIL_RET( d3ddev->BeginScene() );
    
    hr = d3ddev->SetStreamSource(0, m_vertexBuffer, 0, sizeof(CPlaneScene::CUSTOMVERTEX)  );
    hr = d3ddev->SetFVF( D3DFVF_CUSTOMVERTEX );

    hr = d3ddev->SetTexture(0, texture);  

    //d3ddev->SetTransform(D3DTS_WORLD, &IDENTITYMATRIX);
    D3DMATRIX mx;
    d3dmatrix_scale_xy(&mx, 2.0f, 2.0f);
    //d3dmatrix_mirror_xy(&mx, 1.0f, 0.0f, -0.5f);
    //d3ddev->MultiplyTransform(D3DTS_WORLD, &mx);
    //d3dmatrix_mirror_xy(&mx, 0.0f, 1.0f, -0.5f);
    //d3ddev->MultiplyTransform(D3DTS_TEXTURE0, &mx);

    //d3ddev->SetTransform(D3DTS_TEXTURE0, 
    for(int i=0; i< CM; i++) {    
        switch (g_SceneData[i].viewMode) {
            case PSVM_Stretch: case PSVM_Center: {
                hr = d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                    (i * REGIONS_PER_MONITOR * VERTICES_PER_REGION), 2);  //draw quad 
            } break;
            default: g_SceneData[i].viewMode = PSVM_MirrorTile;
            case PSVM_WrapTile: case PSVM_MirrorTile: {
                if (g_SceneData[i].cellCount == 4)
                {
                    for (int j = 0; j < 4; j++) {
                        hr = d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                            (i * REGIONS_PER_MONITOR * VERTICES_PER_REGION) + j * VERTICES_PER_REGION, 2);  //draw quad 
                    }
                }
                else
                {
                    for (int j = 0; j < 2; j++) {
                        hr = d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                            (i * REGIONS_PER_MONITOR * VERTICES_PER_REGION) + j * VERTICES_PER_REGION, 2);  //draw quad 
                    }
                }
            } break;
        }
    }

    d3ddev->SetTexture(0, NULL);

#if false       
    hr = d3ddev->SetTexture(0, foregroundTexture);
    
    D3DMATRIX mx0 = { 0 };
    mx0._11 = 1;
    mx0._22 = -1;
    mx0._33 = 1;
    mx0._44 = 1;
    //d3ddev->SetTransform(D3DTS_TEXTURE0, &mx0);

    //d3ddev->SetTransform(D3DTS_VIEW, &IDENTITYMATRIX);    
    hr = d3ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    hr = d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    hr = d3ddev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    hr = d3ddev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

    //hr = d3ddev->SetStreamSource(0, m_vertexBuffer, 0, sizeof(CPlaneScene::CUSTOMVERTEX));
    //hr = d3ddev->SetFVF(D3DFVF_CUSTOMVERTEX);

    //set next source ( NEW )     
    // hr = d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, MAX_MONITORS * VERTEX_PER_SURFACE, 2);  //draw quad 	
    hr = d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, BKG_VERTICES, 2);  //draw quad 	 
    d3ddev->SetTexture(0, NULL);
#endif
 //   FAIL_RET( d3ddev->SetTexture( 0, NULL));
    hr = d3ddev->EndScene();
    return hr;
}

void
CPlaneScene::SetSrcRect( float fTU, float fTV )
{
/*
    m_vertices[0].tu = 0.0f; m_vertices[0].tv = 0.0f; // low left
    m_vertices[1].tu = 0.0f; m_vertices[1].tv = fTV;  // high left
    m_vertices[2].tu = fTU;  m_vertices[2].tv = 0.0f; // low right
    m_vertices[3].tu = fTU;  m_vertices[3].tv = fTV;  // high right
*/
}

//////////////////////////////////////////////////////////////////////
//
// Matrix functions
//
// The purpose of these functions is to remove any dependencies on
// the D3DX utility library from this sample. The functions are
// modeled after the equivalent D3DX functions. In a real application,
// you should use the D3DX library instead.
//
//////////////////////////////////////////////////////////////////////

template <class T>
inline T SQUARED(T x)
{
    return x * x;
}

D3DVECTOR* VecSubtract(D3DVECTOR *pOut, const D3DVECTOR *pV1, const D3DVECTOR *pV2)
{
    pOut->x = pV1->x - pV2->x;
    pOut->y = pV1->y - pV2->y;
    pOut->z = pV1->z - pV2->z;
    return pOut;
}

D3DVECTOR* VecNormalize(D3DVECTOR *pOut, const D3DVECTOR *pV1)
{
    FLOAT norm_sq = SQUARED(pV1->x) + SQUARED(pV1->y) + SQUARED(pV1->z);

    if (norm_sq > FLT_MIN)
    {
        FLOAT f = sqrtf(norm_sq);
        pOut->x = pV1->x / f;
        pOut->y = pV1->y / f;
        pOut->z = pV1->z / f;
    }
    else
    {
        pOut->x = 0.0f;
        pOut->y = 0.0f;
        pOut->z = 0.0f;
    }
    return pOut;
}

D3DVECTOR* VecCross(D3DVECTOR *pOut, const D3DVECTOR *pV1, const D3DVECTOR *pV2)
{
    pOut->x = pV1->y * pV2->z - pV1->z * pV2->y;
    pOut->y = pV1->z * pV2->x - pV1->x * pV2->z;
    pOut->z = pV1->x * pV2->y - pV1->y * pV2->x;

    return pOut;
}



FLOAT VecDot(const D3DVECTOR *pV1, const D3DVECTOR *pV2)
{
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

// MatrixLookAtLH: Approximately equivalent to D3DXMatrixLookAtLH.

D3DMATRIX* MatrixLookAtLH( 
    D3DMATRIX *pOut, 
    const D3DVECTOR *pEye, 
    const D3DVECTOR *pAt,
    const D3DVECTOR *pUp 
    )
{

    D3DVECTOR vecX, vecY, vecZ;

    // Compute direction of gaze. (+Z)

    VecSubtract(&vecZ, pAt, pEye);
    VecNormalize(&vecZ, &vecZ);

    // Compute orthogonal axes from cross product of gaze and pUp vector.
    VecCross(&vecX, pUp, &vecZ);
    VecNormalize(&vecX, &vecX);
    VecCross(&vecY, &vecZ, &vecX);

    // Set rotation and translate by pEye
    pOut->_11 = vecX.x;
    pOut->_21 = vecX.y;
    pOut->_31 = vecX.z;
    pOut->_41 = -VecDot(&vecX, pEye);

    pOut->_12 = vecY.x;
    pOut->_22 = vecY.y;
    pOut->_32 = vecY.z;
    pOut->_42 = -VecDot(&vecY, pEye);

    pOut->_13 = vecZ.x;
    pOut->_23 = vecZ.y;
    pOut->_33 = vecZ.z;
    pOut->_43 = -VecDot(&vecZ, pEye);

    pOut->_14 = 0.0f;
    pOut->_24 = 0.0f;
    pOut->_34 = 0.0f;
    pOut->_44 = 1.0f;

    return pOut;
}


// MatrixPerspectiveFovLH: Approximately equivalent to D3DXMatrixPerspectiveFovLH.

D3DMATRIX* MatrixPerspectiveFovLH(
    D3DMATRIX * pOut,
    FLOAT fovy,
    FLOAT Aspect,
    FLOAT zn,
    FLOAT zf
    )
{   
    // yScale = cot(fovy/2)

    FLOAT yScale = cosf(fovy * 0.5f) / sinf(fovy * 0.5f);
    FLOAT xScale = yScale / Aspect;

    ZeroMemory(pOut, sizeof(D3DMATRIX));

    pOut->_11 = xScale;

    pOut->_22 = yScale;

    pOut->_33 = zf / (zf - zn);
    pOut->_34 = 1.0f;

    pOut->_43 = -pOut->_33 * zn;

    return pOut;
}

// ================================================
// IVMRImageCompositor9
HRESULT STDMETHODCALLTYPE CVMR9Compositor::InitCompositionDevice(IUnknown *pD3DDevice)
{
	CComQIPtr<IDirect3DDevice9> d3ddev = pD3DDevice;
	if(!d3ddev) return E_FAIL;

    //FAIL_RET( SetUpFog( d3ddev ) );
    //
    // Set the projection matrix
    //
    CComPtr<IDirect3DSurface9> backBuffer;
    FAIL_RET( d3ddev->GetRenderTarget( 0, &backBuffer ) );

    D3DSURFACE_DESC backBufferDesc;
    backBuffer->GetDesc( & backBufferDesc );

    //FAIL_RET( adjustViewMatrix( d3ddev ) );

    D3DMATRIX matProj;
    FLOAT fAspect = backBufferDesc.Width / 
                    (float)backBufferDesc.Height;
    MatrixPerspectiveFovLH( &matProj, (float)M_PI_4, fAspect, 
                                1.0f, 100.0f );
    //FAIL_RET( d3ddev->SetTransform( D3DTS_PROJECTION, &matProj ) );

    // 
    // vertex buffer
    // 
	m_vertexBuffer.Release();
    FAIL_RET( d3ddev->CreateVertexBuffer(sizeof(m_vertices), 0, D3DFVF_CUSTOMVERTEX,D3DPOOL_DEFAULT,& m_vertexBuffer, NULL ) );

    if( m_createTexture ) {
        FAIL_RET( createTexture( d3ddev, backBufferDesc.Width, backBufferDesc.Height ) );
    }

	m_zSurface = NULL;
	FAIL_RET(  d3ddev->CreateDepthStencilSurface( backBufferDesc.Width, backBufferDesc.Height,
        D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_zSurface, NULL )  ); 

    return S_OK;
}
	
HRESULT STDMETHODCALLTYPE CVMR9Compositor::TermCompositionDevice(IUnknown *pD3DDevice)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CVMR9Compositor::SetStreamMediaType(DWORD dwStrmID, AM_MEDIA_TYPE *pmt, BOOL fTexture)
{
	m_createTexture = !fTexture;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CVMR9Compositor::CompositeImage(IUnknown *pD3DDevice, 
	IDirect3DSurface9 *pddsRenderTarget, AM_MEDIA_TYPE *pmtRenderTarget, REFERENCE_TIME rtStart, 
	REFERENCE_TIME rtEnd, D3DCOLOR dwClrBkGnd, VMR9VideoStreamInfo *pVideoStreamInfo, UINT cStreams)
{
    if( pD3DDevice == NULL ) {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
	CComQIPtr<IDirect3DDevice9> d3ddev = pD3DDevice;
    if(!d3ddev) return S_FALSE;

    FAIL_RET( d3ddev->SetRenderState( D3DRS_AMBIENT, 0x00202020 ) );
    FAIL_RET( d3ddev->SetRenderState( D3DRS_LIGHTING, FALSE ) );
    FAIL_RET( d3ddev->SetRenderState( D3DRS_CULLMODE,D3DCULL_NONE)); 
    // FAIL_RET( d3ddev->SetRenderState( D3DRS_ZENABLE,TRUE)); 

    // FAIL_RET( adjustViewMatrix( d3ddev ) );

    // write the new vertex information into the buffer
    /*void* pData;
    FAIL_RET( m_vertexBuffer->Lock(0, sizeof m_vertices, &pData,0) );
    memcpy(pData,m_vertices,sizeof(m_vertices));
    FAIL_RET( m_vertexBuffer->Unlock() );  
	*/
	// FAIL_RET( d3ddev->SetDepthStencilSurface( m_zSurface) );

    // clear the scene so we don't have any articats left
    d3ddev->Clear( 0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
                   D3DCOLOR_XRGB(128,128,255), 1.0f, 0L );

    FAIL_RET( d3ddev->BeginScene() );
    FAIL_RET( d3ddev->SetStreamSource(0, m_vertexBuffer, 0, sizeof( CUSTOMVERTEX)  ) );
    FAIL_RET( d3ddev->SetFVF( D3DFVF_CUSTOMVERTEX ) );
    FAIL_RET( d3ddev->SetTexture( 0, getTexture( d3ddev, pVideoStreamInfo + 0 % cStreams ) ) );
    FAIL_RET( d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2) );  
    FAIL_RET( d3ddev->SetTexture( 0, getTexture( d3ddev, pVideoStreamInfo + 1 % cStreams ) ) );
    FAIL_RET( d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,4,2) );  
    FAIL_RET( d3ddev->SetTexture( 0, getTexture( d3ddev, pVideoStreamInfo + 2 % cStreams ) ) );
    FAIL_RET( d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,8,2) );
	FAIL_RET( d3ddev->SetTexture( 0, getTexture( d3ddev, pVideoStreamInfo + 3 % cStreams ) ) );
    FAIL_RET( d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP,12,2) );
    FAIL_RET( d3ddev->EndScene());
    return S_OK;
}

IDirect3DTexture9* CVMR9Compositor::getTexture(IDirect3DDevice9* d3ddev, VMR9VideoStreamInfo *pVideoStreamInfo)
{
	CComPtr<IDirect3DTexture9> texture;
	HRESULT hr = pVideoStreamInfo->pddsVideoSurface->GetContainer( IID_IDirect3DTexture9, (LPVOID*)&texture );
	if( FAILED ( hr ) ) {
		CComPtr<IDirect3DSurface9> surface;
		if( FAILED( m_Texture->GetSurfaceLevel( 0 , & surface ) ) ) {
			return NULL;
		}

        // copy the full surface onto the texture's surface
        if( FAILED( d3ddev->StretchRect( pVideoStreamInfo->pddsVideoSurface, NULL, surface, NULL, D3DTEXF_NONE ) )) {
			return NULL;
		}

        texture = m_Texture;
    }
	return texture.Detach();
}


CVMR9Compositor::CVMR9Compositor(void)
{
	m_vertices[0].position =    CUSTOMVERTEX::Position( 0.0f,  0.0f, 0.0f); // - forward bottom left
    m_vertices[1].position =    CUSTOMVERTEX::Position( 0.0f,  0.5f, 0.0f); // - tip
    m_vertices[2].position =    CUSTOMVERTEX::Position( 0.5f,  0.0f, 0.0f); // - forward bottom right
    m_vertices[3].position =    CUSTOMVERTEX::Position( 0.5f,  0.5f, 0.0f); // - back bottom right 
    
    m_vertices[4].position =    CUSTOMVERTEX::Position( 0.5f,  0.0f, 0.0f ); // - forward bottom left
    m_vertices[5].position =    CUSTOMVERTEX::Position( 0.5f,  0.5f, 0.0f ); // - tip
    m_vertices[6].position =    CUSTOMVERTEX::Position( 1.0f,  0.0f, 0.0f);  // - back bottom left
    m_vertices[7].position =    CUSTOMVERTEX::Position( 1.0f,  0.5f, 0.0f);  // - back bottom right
    
    m_vertices[8].position =    CUSTOMVERTEX::Position( 0.0f,  0.5f, 0.0f ); // - forward  
    m_vertices[9].position =    CUSTOMVERTEX::Position( 0.0f,  1.0f, 0.0f ); // - back
    m_vertices[10].position =   CUSTOMVERTEX::Position( 0.5f,  0.5f, 0.0f ); // - forward 
    m_vertices[11].position =   CUSTOMVERTEX::Position( 0.5f,  1.0f, 0.0f ); // - back

	m_vertices[12].position =   CUSTOMVERTEX::Position( 0.5f,  0.5f, 0.0f ); // - back
	m_vertices[13].position =   CUSTOMVERTEX::Position( 0.5f,  1.0f, 0.0f ); // - back
	m_vertices[14].position =   CUSTOMVERTEX::Position( 1.0f,  0.5f, 0.0f ); // - back
	m_vertices[15].position =   CUSTOMVERTEX::Position( 1.0f,  1.0f, 0.0f ); // - back


    // set up texture coordinates
    m_vertices[0].tu = 0.0f; m_vertices[0].tv = 0.0f; // low left
    m_vertices[1].tu = 0.0f; m_vertices[1].tv = 0.5f; // high left
    m_vertices[2].tu = 0.5f; m_vertices[2].tv = 0.0f; // low right
    m_vertices[3].tu = 0.5f; m_vertices[3].tv = 0.5f; // high right
    
    m_vertices[4].tu = 0.5f; m_vertices[4].tv = 0.0f; 
    m_vertices[5].tu = 0.5f; m_vertices[5].tv = 0.5f; 
    m_vertices[6].tu = 1.0f; m_vertices[6].tv = 0.0f; 
    m_vertices[7].tu = 1.0f; m_vertices[7].tv = 0.5f; 
    
    m_vertices[8].tu =  0.0f; m_vertices[8].tv =  0.5f; 
    m_vertices[9].tu =  0.0f; m_vertices[9].tv =  1.0f; 
    m_vertices[10].tu = 0.5f; m_vertices[10].tv = 0.5f; 
    m_vertices[11].tu = 0.5f; m_vertices[11].tv = 1.0f; 

	m_vertices[11].tu = 0.5f; m_vertices[11].tv = 0.5f; 
	m_vertices[12].tu = 0.5f; m_vertices[11].tv = 1.0f; 
	m_vertices[13].tu = 1.0f; m_vertices[11].tv = 0.5f; 
	m_vertices[14].tu = 1.0f; m_vertices[11].tv = 1.0f; 
}

CVMR9Compositor::~CVMR9Compositor(void)
{
}

HRESULT CVMR9Compositor::adjustViewMatrix( IDirect3DDevice9* d3ddev )
{
	SYSTEMTIME stm;
	GetSystemTime(&stm);
	float x = stm.wMilliseconds, y = x;
    x = float( x ) / 100 ;
    y = float( y ) / 100 ;
    //
    // view matrix
    //
    
    D3DVECTOR from = { x, y, -4.0f };
    D3DVECTOR at = { 0.0f, 1.0f, 0.0f };
    D3DVECTOR up = { 0.0f, 1.0f, 0.0f };

    D3DMATRIX matView;
    MatrixLookAtLH(&matView, &from, &at, &up);
    HRESULT hr = d3ddev->SetTransform( D3DTS_VIEW, &matView );
    return hr;
}

HRESULT CVMR9Compositor::createTexture( IDirect3DDevice9* d3ddev, DWORD x, DWORD y  )
{
    HRESULT hr;
    D3DDISPLAYMODE dm; 
    FAIL_RET( d3ddev->GetDisplayMode(NULL,  & dm ) );

	m_Texture.Release();
	// create the private texture
    hr = d3ddev->CreateTexture(x, y, 1, D3DUSAGE_RENDERTARGET, dm.Format, D3DPOOL_DEFAULT, & m_Texture, NULL );
    return hr;

}

HRESULT CVMR9Compositor::setUpFog( IDirect3DDevice9* d3ddev )
{
    HRESULT hr = S_OK;
    float fogStart = 2.0;
    float fogEnd = 6.0;

    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGENABLE, TRUE ));
    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGCOLOR, 0x00001020));
    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGSTART, DWORD( fogStart ) ));
    FAIL_RET( d3ddev->SetRenderState( D3DRS_FOGEND, DWORD( fogEnd ) ));
    return hr;
}
