
#ifndef ___INCLUDE_EYEREACHMODULE_H___
#define ___INCLUDE_EYEREACHMODULE_H___

#include "Resource.h"

#include "Eyereach_i.h"

//extern const IID LIBID_EyereachLib;

enum SceneViewMode
{
    // PlaneScene    
    PSVM_Stretch,
    PSVM_WrapTile,
    PSVM_MirrorTile,
    PSVM_Center
};

class CEyereachModule : 
#ifdef _USRDLL
    public CAtlDllModuleT<CEyereachModule>,
#else
    public CAtlExeModuleT<CEyereachModule>,
#endif
    
    public IEyereachControl
{
public :
    CEyereachModule(void);
    ~CEyereachModule(void);

	DECLARE_LIBID(LIBID_EyereachTypelib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_EYEREACH, "{B1B32A06-068C-4FBB-A2D6-55341626A62A}")
    DECLARE_NO_REGISTRY()    
    	/*static LPCOLESTR GetAppId() throw() { return OLESTR("{B1B32A06-068C-4FBB-A2D6-55341626A62A}"); }
	static TCHAR* GetAppIdT() throw() { return _T("{B1B32A06-068C-4FBB-A2D6-55341626A62A}"); }*/
    
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    // IEyereachControl
    virtual HRESULT STDMETHODCALLTYPE Run(LPCWSTR filepath);
    virtual HRESULT STDMETHODCALLTYPE Stop(void);
    virtual HRESULT STDMETHODCALLTYPE Play(LPCWSTR filepath);
    virtual HRESULT STDMETHODCALLTYPE Pause(void);
    virtual HRESULT STDMETHODCALLTYPE SetVolume(LONG volume);
    virtual HRESULT STDMETHODCALLTYPE SetDuration(float start, float end);

    virtual HRESULT STDMETHODCALLTYPE setPrefDWORD(DWORD dwPrefId, int iMonitor, DWORD value);
    virtual HRESULT STDMETHODCALLTYPE getPrefDWORD(DWORD dwPrefId, int iMonitor, /* [out] */ DWORD *pValue);
    virtual HRESULT STDMETHODCALLTYPE setPrefString(DWORD dwPrefId, int iMonitor, BSTR value);
    virtual HRESULT STDMETHODCALLTYPE getPrefString(DWORD dwPrefId, int iMonitor, /* [out] */ BSTR *pValue);

    HRESULT ToggleRunStop(void);
    HRESULT TogglePlayPause(void);
    HRESULT ShowPreferenceUI(void);

    HINSTANCE m_hInst;
    HMODULE getModule(void) const { return m_hInst; }

	class DShowPlayer* getPlayer(void) { return mp_Player; }
    class CEyereachPrefUI* getPrefUI(void) { return mp_PrefUI; }
    void setPrefUI(class CEyereachPrefUI* prefui);
    
protected:
    ULONG STDMETHODCALLTYPE InternalAddRef(void);
    ULONG STDMETHODCALLTYPE InternalRelease(void);

private:
    class DShowPlayer* mp_Player;
    class CEyereachPrefUI* mp_PrefUI; 

    HRESULT STDMETHODCALLTYPE CreatePlayer(HWND hwndDefView, HWND hwndWorker, LPCWSTR filepath);
    static void CALLBACK CreateWorkerMsgProc(HWND hwnd, UINT msg, ULONG_PTR lparam, LRESULT lresult);
};

extern class CEyereachModule _AtlModule;

/// <summary>
/// Representing a memory bitmap
/// </summary>
struct MemBitmap
{
	HBITMAP hBmp;
	BYTE* pBits;
#if SAVE_BITMAP
    int iWidth;
    int iHeight;
#endif
	~MemBitmap(void) {
		Delete();
	}
	
	void Delete(void) {
		if(hBmp) DeleteObject(hBmp);
		hBmp = NULL;
		pBits = NULL;
	}
	
	void Create(HDC hdc, int w, int h);
#if SAVE_BITMAP
    void SaveBitmap();
#endif
};

// The PlaybackState enumeration describes the 
// current state of the DShowPlayer object. 
enum PlaybackState
{
    STATE_NO_GRAPH,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_STOPPED
};

// The constant WM_GRAPH_EVENT defines a private 
// window message.  This message is used to notify 
// the application about filter graph events.
#define WM_GRAPH_EVENT (WM_APP + 1)

#endif
