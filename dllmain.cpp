// Eyereach.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "resource.h"
#include "Eyereach_i.h"
#include "dllmain.h"
#include "xdlldata.h"

#include "Eyereach.h"

CEyereachModule _AtlModule;

#ifdef _USRDLL

void __dllmainSubclassProg(HWND, DWORD, BOOL);

DWORD WINAPI EyereachBoostThread(LPVOID lp)
{
    struct data {
        DWORD tid;
        DWORD reason;
    };
    data* pv = (data*)lp;

    // wait for Program Manager creating
    HWND hwndProg = FindWindowEx(GetDesktopWindow(), NULL, L"Progman", L"Program Manager");
    while (!hwndProg)
    {
        Sleep(500);
        hwndProg = FindWindowEx(GetDesktopWindow(), NULL, L"Progman", L"Program Manager");
    }

    if (pv->reason == DLL_PROCESS_ATTACH)
    {
        Sleep(1000);
        __dllmainSubclassProg(hwndProg, pv->tid, TRUE);
    }
    else if (pv->reason == DLL_PROCESS_ATTACH)
    {
        __dllmainSubclassProg(hwndProg, pv->tid, FALSE);
    }

    delete pv;
    return 0;
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
    DisableThreadLibraryCalls(hInstance);
    _AtlModule.m_hInst = hInstance;
        
    _AtlModule.DllMain(dwReason, lpReserved); 

    TCHAR path[600];
    if (GetModuleFileName(NULL, path, 600))
    {
        LPCTSTR lpsz = PathFindFileName(path);
        if (_tcsicmp(lpsz, _T("explorer.exe")) == 0)
        {
            if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_PROCESS_DETACH)
            {
                DWORD* ptr = new DWORD[2];
                ptr[0] = GetCurrentThreadId();
                ptr[1] = dwReason;
                CreateThread(NULL, 0, EyereachBoostThread, ptr, 0, NULL);
                return TRUE;
            }
        }
    }
    
    return FALSE;
}

// Used to determine whether the DLL can be unloaded by OLE.
STDAPI DllCanUnloadNow(void)
{    
#ifdef _MERGE_PROXYSTUB
	HRESULT hr = PrxDllCanUnloadNow();
	if (hr != S_OK)
		return hr;
#endif
    return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type.
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
	HRESULT hr = PrxDllGetClassObject(rclsid, riid, ppv);
	if (hr != CLASS_E_CLASSNOTAVAILABLE)
		return hr;
#endif
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry.
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();
#ifdef _MERGE_PROXYSTUB
	if (FAILED(hr))
		return hr;
	hr = PrxDllRegisterServer();
#endif
    return hr;
}

// DllUnregisterServer - Removes entries from the system registry.
STDAPI DllUnregisterServer(void)
{
HRESULT hr = _AtlModule.DllUnregisterServer();
	#ifdef _MERGE_PROXYSTUB
	if (FAILED(hr))
		return hr;
	hr = PrxDllRegisterServer();
	if (FAILED(hr))
		return hr;
	hr = PrxDllUnregisterServer();
#endif
		return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user per machine.
STDAPI DllInstall(BOOL bInstall, _In_opt_  LPCWSTR pszCmdLine)
{
HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
		{
			ATL::AtlSetPerUserRegistration(true);
		}
	}

	if (bInstall)
	{	
		hr = DllRegisterServer();
		if (FAILED(hr))
		{
			DllUnregisterServer();
		}
	}
	else
	{
		hr = DllUnregisterServer();
	}

	return hr;
}

#else

void CALLBACK EyereachPreferenceUIThread(LPVOID);

//#define DWORD_style (LVS_EX_TRANSPARENTBKGND | LVS_EX_TRANSPARENTSHADOWTEXT | LVS_EX_DOUBLEBUFFER)
#define DWORD_style (LVS_EX_TRANSPARENTBKGND | LVS_EX_TRANSPARENTSHADOWTEXT)
// #define DWORD_style LVS_EX_TRANSPARENTSHADOWTEXT
//#define DWORD_style 0

#define DWORD_Style LVS_EX_HIDELABELS

//#define XSCE

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
#ifdef XSCE
    HWND hwndDefvw = NULL;//GetDesktopWindow();
    HWND hWorker = NULL;
    while (!hwndDefvw)
    {
        hWorker = FindWindowEx(GetDesktopWindow(), hWorker, L"WorkerW", L"");
        if(hWorker == NULL) break;
        hwndDefvw = FindWindowEx(hWorker, NULL, L"SHELLDLL_DefView", NULL);
        ShowWindow(hwndDefvw, SW_SHOW);
    }
    // hwndDefvw = FindWindowEx(hwndDefvw, NULL, L"Progman", L"Program Manager");
    // hwndDefvw = FindWindowEx(hwndDefvw, NULL, L"SHELLDLL_DefView", NULL);
    HWND hwndLstvw = FindWindowEx(hwndDefvw, NULL, L"SysListView32", L"FolderView");
    while (hWorker)
    {
        hWorker = FindWindowEx(GetDesktopWindow(), hWorker, L"WorkerW", L"");
        //ShowWindow(hWorker, SW_HIDE);
        DestroyWindow(hWorker);
    }
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    
    WCHAR arg[260];
    wsprintf(arg, L"C:\\Windows\\System32\\ribbons.scr /p %u", hwndDefvw);
    //CreateProcess(NULL, arg, NULL, NULL, NULL, 0, NULL, NULL, &si, &pi);
    Sleep(100);

    HWND hwndScr = FindWindowEx(hwndDefvw, NULL, L"D3DSaverWndClass", NULL);
    if(hwndScr) {
        /*ListView_SetExtendedListViewStyle(hwndLstvw,
			ListView_GetExtendedListViewStyle(hwndLstvw) & (~DWORD_style) | DWORD_Style);*/
        //SetParent(hwndLstvw, hwndScr);
        SetWindowPos(hwndLstvw, hwndScr, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    }
#endif
    _AtlModule.m_hInst = hInstance;
    
    EyereachPreferenceUIThread(NULL);
#ifdef XSCE
    if(hwndScr) {
        /*ListView_SetExtendedListViewStyle(hwndLstvw,
			ListView_GetExtendedListViewStyle(hwndLstvw) | DWORD_style & (~DWORD_Style)); */
        //SetParent(hwndLstvw, hwndDefvw);
        DestroyWindow(hwndScr);
    }

    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
#endif
    return 0;
}

#endif
