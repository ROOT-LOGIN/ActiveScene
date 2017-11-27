
#ifndef ___INCLUDE_COMMON_H___
#define ___INCLUDE_COMMON_H___

#define DIRECT3D_VERSION 0x0900

// #include <gdiplus.h>

#include <uuids.h>
#include <dshow.h>
#include <d3d.h>
#include <d3d9.h>
#include <vmr9.h>
#include <evr9.h>
#include <Audiopolicy.h>

//#pragma comment(lib, "dshow.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")

/*
#include <Commctrl.h>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
*/

#define __beginScope(...) {
#define __endScope(...) }

#define IS_VALID_FILE(hFile) hIn && hIn != INVALID_HANDLE_VALUE
#define SUCCEEDEDEX(hr) (hr == S_OK)

#define FAIL_RET(hr) if(FAILED(hr)) return hr;

/// <summary>
/// A template class for constrcting an array that has known size.
/// </summary>
template<class T>
struct Array
{
    int capacity; // the capacity
    int size;     // the current holding object count 
    BYTE rawbytes[0];

    T* at(int idx) { 
#if CHEC_ARRAY_BOUND
        if(idx < 0 || idx > size - 1) {
            ATLASSERT(false);
        }
#endif
        return (T*)&rawbytes[idx * sizeof(T)]; 
    }

    static Array* Create(int cap) {
        int sz = sizeof(int) * 2 + sizeof(T) * cap;
        Array* ptr = (Array*)::malloc(sz);
        memset(ptr, 0, sz);
        ptr->capacity = cap;
        return ptr;
    }

    void free(void) {        
        ::free(this);
    }
};

/// <summary>
/// A template class for constrcting an COM object array that has known size.
/// </summary>
template<class T>
struct ComArray
{
    int capacity; // the capacity
    int size;     // the current holding object count 
    T* array[0];

    T*& at(int idx) { 
#if CHEC_ARRAY_BOUND
        if(idx < 0 || idx > size - 1) {
            ATLASSERT(false);
        }
#endif
        return array[idx]; 
    }

    static ComArray* Create(int cap) {
        int sz = sizeof(int) * 2 + sizeof(T*) * cap;
        ComArray* ptr = (ComArray*)malloc(sz);
        memset(ptr, 0, sz);
        ptr->capacity = cap;
        ptr->size = 0;
        return ptr;
    }

    void free(void) {
        for(int i = 0; i < size; i++) {
            array[i]->Release();
        }
        ::free(this);
    }
};

/// <summary>
/// A template class for converting module address to target type
/// </summary>
template<class T, int offset>
class ModulePointer
{
private:
    T* ptr;

public:
    ModulePointer() { ptr = NULL; }
    ModulePointer(HMODULE hModule) { Initialize(hModule); }

    operator T*() { return ptr; }

    void Initialize(HMODULE hModule) {
        ptr = (T*)((BYTE*)hModule + offset);
    }
};

/// <summary>
/// A template class for construct an object array which size is known on compling time
/// </summary>
template<int i>
struct PTR_ARRAY
{
public:
    void* ptr[i];
};

/// <summary>
/// A template class for representing a vftable
/// </summary>
template<int offset>
struct VFTABLE
{    
public:    
    PTR_ARRAY<24>* ptr;

    void Initialize(HMODULE hModule) {
        ptr = (PTR_ARRAY<24>*)((BYTE*)hModule + offset);
    }
};

/// <summary>
/// A template class that help implement a COM interface.
/// It is never queryable and its reference count is always 1.
/// </summary>
template<typename TInterface>
class FakeIUnknownImpl : public TInterface
{
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) { return E_FAIL; }
    ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
    ULONG STDMETHODCALLTYPE Release(void) { return 1; }
};

HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, REFGUID clsid, IBaseFilter **ppF, LPCWSTR wszName);

HRESULT IsPinConnected(IPin *pPin, bool *pResult);

HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, bool *pResult);

HRESULT MatchPin(IPin *pPin, PIN_DIRECTION direction, bool bShouldBeConnected, bool *pResult);

HRESULT FindFirstPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, bool Connected, IPin **ppPin);

HRESULT RemoveUnconnectedRenderer(IGraphBuilder *pGraph, IBaseFilter *pRenderer, bool *pbRemoved);

HRESULT ConnectFilter(IGraphBuilder *pGraph, PIN_DIRECTION direction, IPin *pPin, IBaseFilter *pFilter);
__forceinline HRESULT ConnectOutputFilter(IGraphBuilder *pGraph, IPin *pOut, IBaseFilter *pDest) {
    return ConnectFilter(pGraph, PINDIR_INPUT, pOut, pDest);
}

__forceinline HRESULT ConnectInputFilter(IGraphBuilder *pGraph, IPin *pIn, IBaseFilter *pSrc) {
    return ConnectFilter(pGraph, PINDIR_OUTPUT, pIn, pSrc);
}

HRESULT ConnectFilter(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest);

HRESULT FindFilterInterface(IGraphBuilder *pGraph, REFGUID iid, void **ppUnk);

template<typename T>
inline HRESULT FindFilterInterface(IGraphBuilder *pGraph, T **ppUnk) {
    return FindFilterInterface(pGraph, __uuidof(T), (void**)ppUnk);
}

HRESULT FindPinInterface(IBaseFilter *pFilter, REFGUID iid, void **ppUnk);

template<typename T>
inline HRESULT FindPinInterface(IGraphBuilder *pGraph, T **ppUnk) {
    return FindPinInterface(pGraph, __uuidof(T), (void**)ppUnk);
}

HRESULT FindInterfaceAnywhere(IGraphBuilder *pGraph, REFGUID iid, void **ppUnk);

template<typename T>
inline HRESULT FindInterfaceAnywhere(IGraphBuilder *pGraph, T **ppUnk) {
    return FindInterfaceAnywhere(pGraph, __uuidof(T), (void**)ppUnk);
}

HRESULT GetNextFilter(IBaseFilter *pFilter, PIN_DIRECTION Dir, IBaseFilter **ppNext);

void RemoveAllFilters(IGraphBuilder *pGraph);

/* ========================================== */

Array<DISPLAY_DEVICE>* GetAllDisplayDevices(void);

/* ========================================== */

#define DECL_MSG_HANDLER(handler) \
    LRESULT handler(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)

#define IMPL_MSG_HANDLER(class, handler) \
    LRESULT class::handler(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)

/// <summary>
/// Helper class for reading information from a MONITORINFO.
/// </summary>
struct MonitorWrap : MONITORINFO
{
public:
    MonitorWrap(void) { cbSize = sizeof(MONITORINFO); }

    int getMonitorCount(void) {
        int dw = 0;
        EnumDisplayMonitors(NULL, NULL, MonitorEnumProc4Count, (LPARAM)&dw);
        return dw;
    }

    void Fill(HWND hwnd) {
        GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), this);
    }

    void Fill(int i) {
        this->rcWork.left = i;        
        EnumDisplayMonitors(NULL, NULL, MonitorEnumProc4Fill, (LPARAM)this);
    }

    int indexFromCursor(void) {
        this->rcMonitor.left = 0;
        GetCursorPos((LPPOINT)&this->rcWork);
        EnumDisplayMonitors(NULL, NULL, MonitorEnumProc4Index, (LPARAM)this);
        return this->rcMonitor.left;
    }

private:
    static BOOL CALLBACK MonitorEnumProc4Count(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
    {
        (*(DWORD*)dwData)++;
        return TRUE;
    }

    static BOOL CALLBACK MonitorEnumProc4Index(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
    {
        MonitorWrap* pm = (MonitorWrap*)dwData;
        CRect rc(lprcMonitor);
        if(PtInRect(lprcMonitor, *(LPPOINT)&pm->rcWork))
            return FALSE;
        pm->rcMonitor.left++;
        return TRUE;
    }

    static BOOL CALLBACK MonitorEnumProc4Fill(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
    {
        MonitorWrap* pm = (MonitorWrap*)dwData;
        if(pm->rcWork.left == 0)
        {
            GetMonitorInfo(hMonitor, pm);
            return FALSE;
            
        }
        else {
            pm->rcWork.left--;
            return TRUE;
        }
    }
};

#include <pshpack1.h>

/// <summary>
/// A structure for recording user preference.
/// </summary>
typedef struct tagSCENEDATA
{
    unsigned long id : 8;
    volatile mutable unsigned long viewMode : 24;
    volatile mutable unsigned short clipLeft;
    volatile mutable unsigned short clipTop;
    volatile mutable unsigned short clipRight;
    volatile mutable unsigned short clipBottom;
    volatile mutable signed short rotateAngle;
    volatile mutable unsigned char mirrorHorz;
    volatile mutable unsigned char mirrorVert;
    volatile mutable char cellCount;
    volatile mutable char reserved__[3];

    static unsigned short s_mute;
    static WCHAR s_path[1024];

    static void LoadConfig(void);
    static void SaveConfig(void);

    void updateViewMode(int value);
    void updateClipLeft(unsigned short value);
    void updateClipTop(unsigned short value);
    void updateClipRight(unsigned short value);
    void updateClipBottom(unsigned short value);
    void updateRotateAngle(signed short value);
    void updateMirrorHorz(unsigned char value);
    void updateMirrorVert(unsigned char value);
    void updateCellCount(signed char value);
} SCENEDATA, *LPSCENEDATA;

C_ASSERT(sizeof(SCENEDATA) == 20);

#define SCENEDATA_SD 'SD'
#define SCENEDATA_VERSION 1

#include <poppack.h>

#define MONITORID_MASK 0xFF

#endif // ___INCLUDE_COMMON_H___
