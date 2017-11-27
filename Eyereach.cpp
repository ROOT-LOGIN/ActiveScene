// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"

#include "dllmain.h"
#include "xdlldata.h"

#include "common.h"

#include "Eyereach.h"

#include "dxsupport.h"

#include "views.h"
#include "player.h"

#include <gdiplus.h>

WCHAR SCENEDATA::s_path[1024];
unsigned short SCENEDATA::s_mute;

static void make_config_file_name(wchar_t* config_file_path) {
    GetModuleFileName(_AtlModule.getModule(), config_file_path, 1024);
    PathRemoveFileSpec(config_file_path);
    PathCombine(config_file_path, config_file_path, L"eyereach.pref");
}

void SCENEDATA::LoadConfig(void) {
    wchar_t config_file_path[USN_PAGE_SIZE];
    make_config_file_name(config_file_path);

    HANDLE hFile = CreateFile(config_file_path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, NULL, NULL);
    DWORD dw = 0;
    if(hFile && hFile != INVALID_HANDLE_VALUE) {
        ReadFile(hFile, config_file_path, 4000, &dw, NULL);
        CloseHandle(hFile);
    }
    if (dw < (sizeof(SCENEDATA) + 4))
        goto label_MakeDefault;

    if ((*(USHORT*)config_file_path) != SCENEDATA_SD)
        goto label_MakeDefault;
    
    if ((*((USHORT*)config_file_path + 1)) != SCENEDATA_VERSION)
        goto label_MakeDefault;

    dw = 4;
    memcpy(s_path, ((BYTE*)config_file_path) + dw, sizeof(s_path));
    dw += sizeof(s_path);
    memcpy(&s_mute, ((BYTE*)config_file_path) + dw, sizeof(s_mute));
    dw += sizeof(s_mute);
    memcpy(g_SceneData, ((BYTE*)config_file_path) + dw, sizeof(g_SceneData));
    return;

label_MakeDefault:
    for (int i = 0; i < MAX_MONITORS; i++)
    {
        SCENEDATA& p = g_SceneData[i];
        p.id = i;
        p.cellCount = 4;
        p.viewMode = SceneViewMode::PSVM_MirrorTile;
    }
    s_path[0] = L'\0';
}

void SCENEDATA::SaveConfig(void) {
    wchar_t config_file_path[USN_PAGE_SIZE];
    make_config_file_name(config_file_path);

    HANDLE hFile = CreateFile(config_file_path, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD dw, dy;
    if(hFile && hFile != INVALID_HANDLE_VALUE) {        
        dy = SCENEDATA_SD;
        WriteFile(hFile, &dy, 2, &dw, NULL);
        dy = SCENEDATA_VERSION;
        WriteFile(hFile, &dy, 2, &dw, NULL);
        WriteFile(hFile, s_path, sizeof(s_path), &dw, NULL); 
        WriteFile(hFile, &s_mute, sizeof(s_mute), &dw, NULL);
		WriteFile(hFile, g_SceneData, sizeof(g_SceneData), &dw, NULL);
        CloseHandle(hFile);
    }
}

/*
=================================================
*/

void MemBitmap::Create(HDC hdc, int w, int h)
{
    Delete();

    BITMAPINFO bmp = {0};
    bmp.bmiHeader.biSize = sizeof(bmp.bmiHeader);
    bmp.bmiHeader.biWidth = 
#if SAVE_BITMAP
        iWidth = 
#endif
        w;
    bmp.bmiHeader.biHeight = 
#if SAVE_BITMAP
        iHeight = 
#endif
        h;
    bmp.bmiHeader.biPlanes = 1;
    bmp.bmiHeader.biBitCount = 32;
    bmp.bmiHeader.biCompression = BI_RGB;
    hBmp = CreateDIBSection(hdc, &bmp, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
}

#if SAVE_BITMAP
void MemBitmap::SaveBitmap()
{
    if (PathFileExists(L"C:\\200G\\trunk\\ActiveScene\\x64\\Debug\\captureqwsx.bmp"))
        return;

    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = iWidth;
    bi.biHeight = iHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    HANDLE hFile = CreateFile(L"C:\\200G\\trunk\\ActiveScene\\x64\\Debug\\captureqwsx.bmp",
        GENERIC_ALL,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD dwBmpSize = ((iWidth * bi.biBitCount + 31) / 32) * 4 * iHeight;
    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwSizeofDIB;
    bmfHeader.bfType = 0x4D42; //BM   

    DWORD dwBytesWritten = 0;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, this->pBits, dwBmpSize, &dwBytesWritten, NULL);

    CloseHandle(hFile);
}
#endif

/*
================================================
*/

CEyereachModule::CEyereachModule(void)
{
    Lock();
    mp_PrefUI = NULL;
    mp_Player = NULL;
}

CEyereachModule::~CEyereachModule(void)
{
    Unlock();
    Stop();
    setPrefUI(NULL);
}

void CEyereachModule::setPrefUI(CEyereachPrefUI* prefui) { 
    if(!prefui && mp_PrefUI) { 
        if(mp_PrefUI->IsWindow())
            mp_PrefUI->DestroyWindow();
        delete mp_PrefUI; 
        mp_PrefUI = NULL; return; 
    }
    else if(mp_PrefUI == NULL) {
        mp_PrefUI = prefui; 
    }
}

// IUnknown
HRESULT STDMETHODCALLTYPE CEyereachModule::QueryInterface(REFIID riid, void **ppvObject) {
    if(!ppvObject) return E_POINTER;

    HRESULT hr = E_NOINTERFACE;
    if(riid == IID_IUnknown) {
        *ppvObject = (IUnknown*)this; 
        AddRef(); hr = S_OK;
    }
    else if(riid == IID_IEyereachControl) {
        *ppvObject = (IEyereachControl*)this; 
        AddRef(); hr = S_OK;
    }    
    return hr;
}

ULONG STDMETHODCALLTYPE CEyereachModule::AddRef(void) {
    return InternalAddRef();
}

ULONG STDMETHODCALLTYPE CEyereachModule::Release(void) {
    return InternalRelease();
}

typedef unsigned (__stdcall * _StartAddress) (void *);

ULONG STDMETHODCALLTYPE CEyereachModule::InternalAddRef(void)
{
    return this->Lock();
}

ULONG STDMETHODCALLTYPE CEyereachModule::InternalRelease(void)
{
    if(this->GetLockCount() > 0) 
        return this->Unlock();
    return 0;
}

struct HWND_DATA
{
    HWND hwndProg;
    HWND hwndDefvw;
    LPCWSTR filepath;
};

#define WM_USER_CREATEWORKER 0x52C

void CALLBACK CEyereachModule::CreateWorkerMsgProc(HWND hwnd, UINT msg, ULONG_PTR lparam, LRESULT lresult)
{
    HWND_DATA* p = (HWND_DATA*)lparam;
    HWND hwndWorker = GetParent(p->hwndDefvw);
    if (hwndWorker == p->hwndProg)
        goto label_Final; // parent is not Worker, Failure

    // Get the next sibling Worker
    hwndWorker = FindWindowEx(GetDesktopWindow(), hwndWorker, L"WorkerW", L"");
    if(hwndWorker == NULL)
        goto label_Final; // it have no sibling, Failure

    _AtlModule.CreatePlayer(p->hwndDefvw, hwndWorker, p->filepath);

label_Final:
    delete p;
}

HRESULT STDMETHODCALLTYPE CEyereachModule::CreatePlayer(HWND hwndDefView, HWND hwndWorker, LPCWSTR filepath)
{
    if (!IsWindow(hwndWorker))
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    mp_Player = new DShowPlayer(hwndDefView, hwndWorker);

    g_SceneData->LoadConfig();
    // Lock the server, now the ref count should be at least 2
    if (!filepath)
        filepath = g_SceneData->s_path;
    if (filepath && filepath[0]) {
        hr = mp_Player->OpenFile(filepath);
        if (SUCCEEDED(hr))
            hr = mp_Player->Play();
    }
    else hr = S_OK;
    return hr;
}

/*
* NOW, We change the strategy.
* Since Explorer use WorkerW to blend wallpaper, 
* we could also use WorkerW to do dx-play.
* 
* In this way, we doesn't need to draw desktop icon texture,
* that simplify our work. But, it may not work when Microsoft
* changes the implement of Explorer. 
*
* WorkerW is not created by default when user login on.
* It is created when user change the wallpaper.
* Use the WM_USER_CREATEWORKER to trigger Explorer to create Worker
* (And change the window hierarchy).
*
* Remember, Worker never receive any message after it's created.
*/
HRESULT STDMETHODCALLTYPE CEyereachModule::Run(LPCWSTR filepath) {  
    if(mp_Player) return S_FALSE;

    HWND hwndDefvw = GetDesktopWindow();
    HWND hwndProg = FindWindowEx(hwndDefvw, NULL, L"Progman", L"Program Manager");
    hwndDefvw = FindWindowEx(hwndProg, NULL, L"SHELLDLL_DefView", NULL);
    HWND hwndWorker = NULL;
    while (!hwndDefvw)
    {
        hwndWorker = FindWindowEx(GetDesktopWindow(), hwndWorker, L"WorkerW", L"");
        if(hwndWorker == NULL) break;
        hwndDefvw = FindWindowEx(hwndWorker, NULL, L"SHELLDLL_DefView", NULL);
    }
    
    if (hwndWorker == NULL)
    {
        // Create Worker and make parent of hwndDefvw
        HWND_DATA* phd = new HWND_DATA();
        phd->hwndDefvw = hwndDefvw;
        phd->hwndProg = hwndProg;
        phd->filepath = filepath;
        SendMessageCallback(hwndProg, WM_USER_CREATEWORKER, (WPARAM)0, (LPARAM)0, CreateWorkerMsgProc, (ULONG_PTR)phd);
        return S_FALSE;
    }
    else
    {
        hwndWorker = FindWindowEx(GetDesktopWindow(), hwndWorker, L"WorkerW", L"");
        if (!hwndWorker)
        {
            HWND_DATA* phd = new HWND_DATA();
            phd->hwndDefvw = hwndDefvw;
            phd->hwndProg = hwndProg;
            phd->filepath = filepath;
            SendMessageCallback(hwndProg, WM_USER_CREATEWORKER, (WPARAM)0, (LPARAM)0, CreateWorkerMsgProc, (ULONG_PTR)phd);
            return S_FALSE;
        }
    }

    return CreatePlayer(hwndDefvw, hwndWorker, filepath);    
}

HRESULT STDMETHODCALLTYPE CEyereachModule::Stop(void) {    
    if(!mp_Player) return S_FALSE;
        
    g_SceneData->SaveConfig();

    mp_Player->Stop(true); 
    delete mp_Player;
    mp_Player = NULL;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CEyereachModule::Play(LPCWSTR filepath) {
    if(!mp_Player) return E_FAIL;
    HRESULT hr = E_FAIL;
    if(filepath) {
        hr = mp_Player->OpenFile(filepath);
        if(SUCCEEDED(hr))
            return mp_Player->Play();
    }
    if(mp_Player->getState() == STATE_PAUSED)
        return mp_Player->Play();
    return hr;
}

HRESULT STDMETHODCALLTYPE CEyereachModule::Pause(void) {
    if(!mp_Player) return E_FAIL;    
    if(mp_Player->getState() == STATE_RUNNING)
        return mp_Player->Pause();
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CEyereachModule::SetVolume(LONG volume) {
    if(!mp_Player) return S_FALSE;
    return mp_Player->Volume(volume);
}

HRESULT STDMETHODCALLTYPE CEyereachModule::SetDuration(float start, float end)
{
    if (!mp_Player) return S_FALSE;
    return mp_Player->Duration(start, end);
}

HRESULT STDMETHODCALLTYPE CEyereachModule::setPrefDWORD(DWORD dwPrefId, int iMonitor, DWORD value)
{
    if(!mp_Player) return E_FAIL;

    switch (dwPrefId)
    {            
    default:
        return mp_Player->UpdateSetting(dwPrefId, iMonitor, (VOID*)value);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CEyereachModule::getPrefDWORD(DWORD dwPrefId, int iMonitor, /* [out] */ DWORD *pValue)
{
    if(!mp_Player) return E_FAIL;

    switch (dwPrefId)
    {            
    default:
        return mp_Player->QuerySetting(dwPrefId, iMonitor, (LPVOID*)pValue);
        break;
    }
    return S_OK;

}

HRESULT STDMETHODCALLTYPE CEyereachModule::setPrefString(DWORD dwPrefId, int iMonitor, BSTR value)
{
    if(!mp_Player) return E_FAIL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CEyereachModule::getPrefString(DWORD dwPrefId, int iMonitor, /* [out] */ BSTR *pValue)
{
    if(!mp_Player) return E_FAIL;
    return S_OK;
}

// CEyereachModule
HRESULT CEyereachModule::ToggleRunStop(void)
{
    return mp_Player ? Stop() : Run(NULL);
}

HRESULT CEyereachModule::TogglePlayPause(void)
{
    if(!mp_Player) return E_FAIL;    
    switch(mp_Player->getState()) {
    case STATE_PAUSED: return mp_Player->Play(); break;
    case STATE_RUNNING: return mp_Player->Pause(); break;
    default: return E_FAIL;
    }
}

#define CLSID_EyereachPrefUI IID_IEyereachPrefUI

void CALLBACK EyereachPreferenceUIThread(LPVOID)
{   
    CEyereachPrefUI* pui = _AtlModule.getPrefUI();
    if(pui) {
        //pui->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
        return;
    }

    CComPtr<IEyereachControl> pCtrl;
    if(FAILED(_AtlModule.QueryInterface(IID_IEyereachControl, (void**)&pCtrl)))
        return;

    pui = new CEyereachPrefUI();
    pui->Initialize(pCtrl);
    _AtlModule.setPrefUI(pui);

    HICON hicon = LoadIcon(_AtlModule.m_hInst, MAKEINTRESOURCE(IDI_ICON1));
    pui->SetIcon(hicon, FALSE);
    pui->SetIcon(hicon, TRUE);

    MSG msg;
    while (GetMessage(&msg, pui->m_hWnd, 0, 0))
    {
        if(msg.message == WM_QUIT) {            
            break;
        }

        if(!IsDialogMessage(pui->m_hWnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } 

    _AtlModule.setPrefUI(NULL);
}

HRESULT CEyereachModule::ShowPreferenceUI(void)
{    
    //CComPtr<IStream> pStm;
    //SHCreateStreamOnFileEx(L"", STGM_READWRITE | STGM_DIRECT | STGM_CREATE | STGM_SIMPLE, 0, TRUE, NULL, &pStm);

    /*CComPtr<IEyereachPrefUI> pUI;
    CoCreateInstance(CLSID_EyereachPrefUI, NULL, CLSCTX_LOCAL_SERVER, IID_IEyereachPrefUI, (void**)&pUI);
    this->AddRef();
    pUI->Initialize((IEyereachControl*)this);*/
    
    CEyereachPrefUI* pui = _AtlModule.getPrefUI();
    if(pui) {
        pui->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
        FLASHWINFO fwi;
        fwi.cbSize = sizeof(FLASHWINFO);
        fwi.hwnd = pui->m_hWnd;
        fwi.dwFlags = FLASHW_CAPTION;
        fwi.uCount = 2;
        fwi.dwTimeout = 0;
        FlashWindowEx(&fwi);        
        return S_OK;
    }

    // Explorer hold's the message loop,
    // We must create our own loop to get keyboard event message    
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)EyereachPreferenceUIThread, (LPVOID)NULL, 0, NULL);
    return S_OK;
}

