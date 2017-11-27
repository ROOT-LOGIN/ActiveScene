
#ifndef ___DXSUPPORT_H_INCLUDE___
#define ___DXSUPPORT_H_INCLUDE___

// {F66D0D2B-0A03-4B70-AEAB-9F9CA7D0D9E8}
extern CLSID CLSID_CVMR9SufaceAllocator;
// {BE407BF6-5223-4A62-A5F1-526304D2EC30}
extern CLSID CLSID_CVMR9Compositor;

#ifdef MAX_MONITORS
#undef MAX_MONITORS
#endif

#define MAX_MONITORS         2
#define REGIONS_PER_MONITOR  4
#define VERTICES_PER_REGION  4
#define FRG_VERTICES         (1 * VERTICES_PER_REGION)
#define BKG_VERTICES         (VERTICES_PER_REGION * REGIONS_PER_MONITOR * MAX_MONITORS)
#define TOTAL_VERTICES       (BKG_VERTICES + FRG_VERTICES)

const int XPref_AspectRatio = Pref_CustomUsage + 1;

extern SCENEDATA g_SceneData[MAX_MONITORS]; // global data that holds user preference

/// <summary>
/// Represent a plane scene, which draws the video eyereach scene
/// </summary>
class CPlaneScene
{
public:

    CPlaneScene();
    virtual ~CPlaneScene();

    HRESULT Init(IDirect3DDevice9* d3ddev);

    HRESULT DrawScene(IDirect3DDevice9* d3ddev, IDirect3DTexture9* texture, IDirect3DTexture9* foregroundTexture);

    void SetSrcRect( float fTU, float fTV );

    void setAspectRatio(SIZE ratio);
    
    HRESULT UpdateSetting(DWORD sid, int mid, void* value);
    HRESULT QuerySetting(DWORD sid, int mid, LPVOID* pvalue);

private:

    struct CUSTOMVERTEX
    {
        struct Position {
			float x, y, z, rhw;

            Position() : x(0.0f),y(0.0f),z(0.0f),rhw(1.0f) { };
            Position(float x_, float y_, float z_) : x(x_),y(y_),z(z_),rhw(1.0f) { };            
        };

        Position    position; // The position
        //D3DCOLOR    color;    // The color
        FLOAT       tu, tv;   // The texture coordinates
    };
    
    float m_aspectRatioX, m_aspectRatioY;
    void internalUpdateViewSetting(int mid);
    void _updateVertexDataFitView(int mid, RECT monitor, CUSTOMVERTEX* pVertex);
    void _updateVertexDataCenterView(int mid, RECT monitor, CUSTOMVERTEX* pVertex);
    void _updateVertexDataStretchView(int mid, RECT monitor, CUSTOMVERTEX* pVertex);    
    void _updateVertexDataTileView(int mid, RECT monitor, CUSTOMVERTEX* pVertex, bool wrapMode = false);
    void _updateVertexDataTileViewSplit(int mid, RECT monitor, CUSTOMVERTEX* pVertex, bool wrapMode);

    CUSTOMVERTEX& vertexFor(int iMonitor, bool bIsFg) {
        if(bIsFg) {
            return m_vertices[BKG_VERTICES];
        }
        else {
            return m_vertices[iMonitor * VERTICES_PER_REGION * REGIONS_PER_MONITOR];
        }
    }

    CUSTOMVERTEX                    m_vertices[TOTAL_VERTICES];
    CComPtr<IDirect3DVertexBuffer9> m_vertexBuffer;

    CComPtr<IDirect3DDevice9> m_d3ddev;

    DWORD  m_time;
};

/*
    If the Direct3D device is lost at any time, the allocator-presenter must restore 
    the device and recreate the surfaces. For example, the device can be lost if the 
    display mode changes or the user moves the window to another monitor. If the Direct3D 
    device changes, call the VMR-9 filter's IVMRSurfaceAllocatorNotify9::ChangeD3DDevice method. 
*/
class __declspec(uuid("F66D0D2B-0A03-4B70-AEAB-9F9CA7D0D9E8")) 
CVMR9SufaceAllocator : public CComObjectRoot,
    public IVMRSurfaceAllocator9, 
    public IVMRImagePresenter9
{
public:
    CVMR9SufaceAllocator(void);
    ~CVMR9SufaceAllocator(void);

    BEGIN_COM_MAP(CVMR9SufaceAllocator)
        COM_INTERFACE_ENTRY(IVMRSurfaceAllocator9)
        COM_INTERFACE_ENTRY(IVMRImagePresenter9)
    END_COM_MAP()
    
    static HRESULT InitializeVMR9(IBaseFilter* pVmr9, HWND hwnd, CComObject<CVMR9SufaceAllocator> **ppAlloc);

    // IVMRSurfaceAllocator9
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
    virtual HRESULT STDMETHODCALLTYPE InitializeDevice( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ VMR9AllocationInfo *lpAllocInfo,
        /* [out][in] */ DWORD *lpNumBuffers);
    
    // When streaming stops, the VMR-9 calls.
    // Should release all of its Direct3D resources
    virtual HRESULT STDMETHODCALLTYPE TerminateDevice( 
        /* [in] */ DWORD_PTR dwID);
    
    // During streaming, the VMR-9 gets surfaces from the allocator-presenter 
    virtual HRESULT STDMETHODCALLTYPE GetSurface( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ DWORD SurfaceIndex,
        /* [in] */ DWORD SurfaceFlags,
        /* [out] */ IDirect3DSurface9 **lplpSurface);
    
    // Call IVMRSurfaceAllocatorNotify9::SetD3DDevice 
    virtual HRESULT STDMETHODCALLTYPE AdviseNotify( 
        /* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);
    
    // IVMRImagePresenter9
    virtual HRESULT STDMETHODCALLTYPE StartPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    virtual HRESULT STDMETHODCALLTYPE StopPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    // Present the image when the VMR-9 calls 
    virtual HRESULT STDMETHODCALLTYPE PresentImage( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ VMR9PresentationInfo *lpPresInfo);

    HRESULT SetViewMode(SceneViewMode viewMode);
    HRESULT UpdateSetting(DWORD sid, int mid, void* value);
    HRESULT QuerySetting(DWORD sid, int mid, LPVOID* pvalue);

    //HRESULT DrawClientTexture(BYTE *pBits);
private:
	HRESULT createDevice(void);
	void deleteSurfaces(void);

private:
    DWORD_PTR m_notifyid;
    HWND m_hwnd;
	CComPtr<IDirect3D9Ex> m_D3d;
	CComPtr<IDirect3DDevice9> m_Dxdev;
	CComPtr<IDirect3DSurface9> m_Target;
    ComArray<IDirect3DSurface9>* m_Surfaces;
    CComPtr<IVMRSurfaceAllocatorNotify9> m_VmrNotify;
    //CComPtr<IDirect3DTexture9> m_clientTexture;
	CPlaneScene m_Scene;
};

/// <summary>
/// A compositor is which composes image into one.
/// </summary>
/// <seealso cref="CComObjectRoot" />
/// <seealso cref="IVMRImageCompositor9" />
class __declspec(uuid("BE407BF6-5223-4A62-A5F1-526304D2EC30"))
CVMR9Compositor : public CComObjectRoot,
	public IVMRImageCompositor9
{
public:
	CVMR9Compositor(void);
	~CVMR9Compositor(void);

	BEGIN_COM_MAP(CVMR9Compositor)
		COM_INTERFACE_ENTRY(IVMRImageCompositor9)
	END_COM_MAP()

public:
	// IVMRImageCompositor9
	virtual HRESULT STDMETHODCALLTYPE InitCompositionDevice( 
		/* [in] */ IUnknown *pD3DDevice);
	
	virtual HRESULT STDMETHODCALLTYPE TermCompositionDevice( 
		/* [in] */ IUnknown *pD3DDevice);
	
	virtual HRESULT STDMETHODCALLTYPE SetStreamMediaType( 
        /* [in] */ DWORD dwStrmID,
        /* [in] */ AM_MEDIA_TYPE *pmt,
        /* [in] */ BOOL fTexture);
	
	virtual HRESULT STDMETHODCALLTYPE CompositeImage( 
        /* [in] */ IUnknown *pD3DDevice,
        /* [in] */ IDirect3DSurface9 *pddsRenderTarget,
        /* [in] */ AM_MEDIA_TYPE *pmtRenderTarget,
        /* [in] */ REFERENCE_TIME rtStart,
        /* [in] */ REFERENCE_TIME rtEnd,
        /* [in] */ D3DCOLOR dwClrBkGnd,
        /* [in] */ VMR9VideoStreamInfo *pVideoStreamInfo,
        /* [in] */ UINT cStreams);

private:
	IDirect3DTexture9* getTexture(IDirect3DDevice9* d3ddev, VMR9VideoStreamInfo *pVideoStreamInfo);
	HRESULT adjustViewMatrix( IDirect3DDevice9* d3ddev );
	HRESULT createTexture( IDirect3DDevice9* d3ddev, DWORD x, DWORD y  );
	HRESULT setUpFog( IDirect3DDevice9* d3ddev );

	struct CUSTOMVERTEX
    {
        struct Position {
            Position() : 
                x(0.0f),y(0.0f),z(0.0f) {            
            };
            Position(float x_, float y_, float z_) :
                x(x_),y(y_),z(z_) {
            };
            float x,y,z;
        };

        Position    position; // The position
        FLOAT       tu, tv;   // The texture coordinates
    };

	float m_x;
	float m_y;

    CUSTOMVERTEX                    m_vertices[TOTAL_VERTICES];
    CComPtr<IDirect3DTexture9>      m_Texture;
    CComPtr<IDirect3DSurface9>      m_zSurface;
    CComPtr<IDirect3DVertexBuffer9> m_vertexBuffer;

	BOOL m_createTexture;
};

#define MAX_VIDEO_STREAMS 4
//#define VMR9_WINDOWLESS 1

/// <summary>
/// The base class for video renderer
/// </summary>
class CVideoRenderer
{
public:
    virtual ~CVideoRenderer(void) 
#ifdef DEBUG
        throw(...)
#endif
    { }

    // The HasVideo method returns TRUE if the video renderer has been created. 
    virtual bool getHasVido() const = 0;
    // The AddToGraph method adds the video renderer to the filter graph. 
    virtual HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd) = 0;
    // The FinalizeGraph method completes the graph-building step. 
    virtual HRESULT FinalizeGraph(IGraphBuilder *pGraph) = 0;
    // The UpdateVideoWindow method updates the video destination rectangle. 
    virtual HRESULT UpdateVideoWindow(HWND hwnd, LPCRECT prc) = 0;
    // The Repaint method redraws the current video frame. 
    virtual HRESULT Repaint(HWND hwnd, HDC hdc) = 0;
    // The DisplayModeChanged method handles display-mode changes.
    virtual HRESULT DisplayModeChanged(void) = 0;

    /*=======================*/

    virtual HRESULT UpdateSetting(DWORD sid, int mid, void* value) = 0;
    virtual HRESULT QuerySetting(DWORD sid, int mid, LPVOID* pvalue) = 0;

    //virtual HRESULT UpdateClient(BYTE* pBits) = 0;
};

#if 0
class CDShowTargetWindow : public CWindowImpl<CDShowTargetWindow, CWindow, CControlWinTraits>
{
public:    
    static CDShowTargetWindow* FromHWND(class DShowPlayer* player, HWND hwnd);
    void WithHWND(class DShowPlayer* player, HWND hwnd);
    void GiveUpHWND(void);

    CDShowTargetWindow(void);  
    
public:
    BEGIN_MSG_MAP(CDShowTargetWindow)
        MESSAGE_HANDLER(WM_GRAPH_EVENT, OnGraphEvent)
        MESSAGE_HANDLER(WM_DISPLAYCHANGE, OnDisplayChange)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
    END_MSG_MAP()

    DECL_MSG_HANDLER(OnGraphEvent);

    DECL_MSG_HANDLER(OnDisplayChange);

    DECL_MSG_HANDLER(OnEraseBkgnd);

    DECL_MSG_HANDLER(OnPaint);

    DECL_MSG_HANDLER(OnSize);

    void OpenFile(HWND hwnd);

    DECL_MSG_HANDLER(OnKeyUp);    

    virtual void OnFinalMessage(HWND hwnd);

private:  
    MemBitmap m_Bitmap;
    class DShowPlayer* mp_Player;     
};

class CDShowTargetWindowParent : public CWindowImpl<CDShowTargetWindow, CWindow, CControlWinTraits>
{
public:
    void WithHWND(class DShowPlayer* player, HWND hwnd);
    void GiveUpHWND(void);

public:
    BEGIN_MSG_MAP(CDShowTargetWindowParent)
        //MESSAGE_HANDLER(WM_PRINTCLIENT, OnPrintClient)
        //MESSAGE_HANDLER(WM_PAINT, OnPaint)
    END_MSG_MAP()

    DECL_MSG_HANDLER(OnPrintClient) { return 0; }
    DECL_MSG_HANDLER(OnPaint) { return 0; }
};
#endif

#endif
