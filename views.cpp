#include "stdafx.h"
#include "common.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <ShObjIdl.h>

#include "Eyereach.h"

#include "dxsupport.h"

#include "views.h"
#include "player.h"

SUBCLASS_WINDOW::SUBCLASS_WINDOW(void) 
{
	m_hWnd = NULL;
	m_pfnSubProc = NULL;
	m_uId = 0;
	//memset(this, 0, sizeof(SUBCLASS_WINDOW));
}

BOOL SUBCLASS_WINDOW::Subclass(void)
{
	if(!IsWindow(m_hWnd)) return FALSE;
	return SetWindowSubclass(m_hWnd, m_pfnSubProc, m_uId, (DWORD_PTR)this);
}

BOOL SUBCLASS_WINDOW::Restore(void)
{
	if(!IsWindow(m_hWnd)) return TRUE;
	return RemoveWindowSubclass(m_hWnd, m_pfnSubProc, m_uId);
}

/*
=========================================================
*/

#define WM_APP_PROGNOTIFY WM_APP + 0x800

HHOOK hhookProg;
LRESULT CALLBACK ProgHOOKPROC(int code, WPARAM wParam, LPARAM lParam)
{
    LPMSG msg = (LPMSG)lParam;
    if (msg->message == WM_APP_PROGNOTIFY)
    {
        if (_AtlModule.getPlayer())
        {
            if (!(BOOL)wParam)
            {
                _AtlModule.Stop();
            }
        }
        else
        {
            if ((BOOL)wParam)
            {
                _AtlModule.ToggleRunStop();
            }
        }

        UnhookWindowsHookEx(hhookProg);
        hhookProg = NULL;
        return 0;
    }
    else
    {
        return CallNextHookEx(hhookProg, code, wParam, lParam);
    }
}

void __dllmainSubclassProg(HWND hwnd, DWORD tid, BOOL fAttach)
{
    hhookProg = NULL;
    hhookProg = SetWindowsHookEx(WH_GETMESSAGE, ProgHOOKPROC, NULL, tid);
    
    PostMessage(hwnd, WM_APP_PROGNOTIFY, fAttach, 0);
}

#undef WM_APP_PROGNOTIFY

/*
=========================================================
*/

void _PlayerOpenFile(SUBCLASS_WINDOW* pThis)
{
    if (_AtlModule.getPlayer() == NULL)
        return;

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    WCHAR szFileName[MAX_PATH];
    szFileName[0] = L'\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = pThis->m_hWnd;
    ofn.hInstance = GetModuleHandle(NULL);
    ofn.lpstrFilter = L"All (*.*)\0*.*\0\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
    HRESULT hr;
    if (GetOpenFileName(&ofn))
    {
        hr = _AtlModule.getPlayer()->OpenFile(szFileName);
        if (SUCCEEDED(hr))
        {
            // ::InvalidateRect(hwnd, NULL, FALSE);
            _AtlModule.getPlayer()->Play();
            // If this file has a video stream, notify the video renderer 
            // about the size of the destination rectangle.
            /*RECT rc;
            ::GetClientRect(hwnd, &rc);*/
            // mp_Player->UpdateVideoWindow(NULL);
        }
        else
        {
            ::MessageBox(pThis->m_hWnd, TEXT("Cannot open this file."), L"Error", MB_ICONERROR | MB_OK);
        }
    }
}

__SUBCLASSPROC(ListViewOnKeyUp)
{
    if (wp != VK_ESCAPE) {
        bHandled = false; return 0;
    }

    _PlayerOpenFile(pThis);
    return 0;
}

__SUBCLASSMAIN(ListViewSubclassProc)
{
    BEGIN_SUBCLASSPROC()
        SUBCLASSPROC_ENTRY(WM_KEYUP, ListViewOnKeyUp)
        END_SUBCLASSPROC()
}

/*
=========================================================
*/

__SUBCLASSPROC(DefViewOnKeyUp)
{
    if (wp != VK_ESCAPE) {
        bHandled = false; return 0;
    }

    _PlayerOpenFile(pThis);
    return 0;
}

__SUBCLASSMAIN(DefViewSubclassProc)
{
    BEGIN_SUBCLASSPROC()
        SUBCLASSPROC_ENTRY(WM_KEYUP, DefViewOnKeyUp)
    END_SUBCLASSPROC()
}

/*
=========================================================
*/

__SUBCLASSMAIN(WorkerSubclassProc)
{
	BEGIN_SUBCLASSPROC()
        //SUBCLASSPROC_ENTRY(WM_PRINTCLIENT, WorkerOnPrintClient);
        //SUBCLASSPROC_ENTRY(WM_ERASEBKGND, WorkerOnEraseBkgnd);
		//SUBCLASSPROC_ENTRY(WM_PAINT, WorkerOnPaint);
    	END_SUBCLASSPROC()
}

/*
=========================================================
*/

LRESULT CALLBACK DShowPlayer::WorkerGetMsgProc(int code, WPARAM wp, LPARAM lp)
{
    DShowPlayer* player = _AtlModule.getPlayer();
    MSG* pmsg = (MSG*)lp;
    if (!player || code < 0 || wp != PM_REMOVE || pmsg->hwnd != player->getHwnd() || pmsg->message != WM_GRAPH_EVENT)
    {
        return CallNextHookEx(player->m_hhookWorker, code, wp, lp);
    }

    player->HandleGraphEvent();
    return 0;
}

DShowPlayer::DShowPlayer(HWND hwndDefView, HWND hwndWorker)
{
    memset(g_SceneData, 0, sizeof(g_SceneData));
    g_SceneData[0].id = 0;
    g_SceneData[1].id = 1;

    m_state = STATE_NO_GRAPH;
    mp_Video = NULL;

    if (::IsWindow(hwndDefView)) {
        // Subclass DefView and ListView, so we can set accelerate key
        m_DefView.m_hWnd = hwndDefView;
        m_DefView.m_pfnSubProc = (SUBCLASSPROC)DefViewSubclassProc;
        m_DefView.m_uId = 'DefV';
        m_DefView.Subclass();

        HWND hwndListView = FindWindowEx(hwndDefView, NULL, L"SysListView32", L"FolderView");
        if (::IsWindow(hwndListView)) {
            m_ListView.m_hWnd = hwndListView;
            m_ListView.m_pfnSubProc = (SUBCLASSPROC)ListViewSubclassProc;
            m_ListView.m_uId = 'LSTV';
            m_ListView.Subclass();
        }
    }

    if(::IsWindow(hwndWorker)) {
        m_hwndWorkerW = hwndWorker;
        // WorkerW is disable and transparent, 
        // And explorer remove all message send to WorkerW,
        // We must set hook to get graph event notification.
        DWORD tid = GetWindowThreadProcessId(hwndWorker, NULL);
        m_hhookWorker = SetWindowsHookEx(WH_GETMESSAGE, WorkerGetMsgProc, NULL, tid);
	}	
}

HRESULT DShowPlayer::Stop(bool isFinal) {
	
	if(m_Control) {
		m_Control->Stop();
	}

    if(isFinal) {
        m_ListView.Restore();
        m_DefView.Restore();

        UnhookWindowsHookEx(m_hhookWorker);
        m_hhookWorker = NULL;

        // FOR windows 6.2 and later, Restore the wallpaper
        CComPtr<IDesktopWallpaper> dwp;
        if (SUCCEEDED(CoCreateInstance(CLSID_DesktopWallpaper, NULL, CLSCTX_INPROC_SERVER, IID_IDesktopWallpaper, (LPVOID*)&dwp))) 
        {
            UINT uc;
            if (SUCCEEDED(dwp->GetMonitorDevicePathCount(&uc)))
            {
                for (UINT i = 0; i < uc; i++)
                {
                    LPWSTR mpath, wpath;
                    if (SUCCEEDED(dwp->GetMonitorDevicePathAt(i, &mpath)))
                    {
                        if (SUCCEEDED(dwp->GetWallpaper(mpath, &wpath)))
                        {
                            dwp->SetWallpaper(mpath, wpath);
                        }
                    }
                }
            }
        }
    }

    m_state = STATE_STOPPED;
	return S_OK;
}

/*
==============================================
*/

