#include "stdafx.h"
#include "common.h"

#include <Commoncontrols.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include <ShlObj.h>
#include <ShObjIdl.h>

#include "Eyereach_i.h"
#include "Eyereach.h"

#include "dxsupport.h"
#include "views.h"
#include "player.h"

// {052E8567-05F3-4710-BFC2-538838702891}
extern const CLSID CLSID_EyereachMenu = { 0x052E8567, 0x05F3, 0x4710, { 0xBF, 0xC2, 0x53, 0x88, 0x38, 0x70, 0x28, 0x91 } };

class __declspec(uuid("052E8567-05F3-4710-BFC2-538838702891"))
CEyereachMenu : public CComObjectRoot,
    public CComCoClass<CEyereachMenu, &CLSID_EyereachMenu>,
    public IShellExtInit,
    public IContextMenu
{
public:
    DECLARE_NO_REGISTRY()

    DECLARE_CLASSFACTORY()

    DECLARE_NOT_AGGREGATABLE(CEyereachMenu)

    BEGIN_COM_MAP(CEyereachMenu)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)        
    END_COM_MAP()

public:
    // IShellExtInit
    virtual HRESULT STDMETHODCALLTYPE Initialize( 
        _In_opt_  PCIDLIST_ABSOLUTE pidlFolder,
        _In_opt_  IDataObject *pdtobj,
        _In_opt_  HKEY hkeyProgID)
    {        
        if(!pidlFolder) return E_FAIL;
        TCHAR path0[1024];
        TCHAR path1[1024];
        SHGetPathFromIDList(pidlFolder, path0);
        SHGetFolderLocation(NULL, CSIDL_DESKTOP, NULL, NULL, (LPITEMIDLIST*)&pidlFolder);
        if(!pidlFolder) return E_FAIL;

        SHGetPathFromIDList(pidlFolder, path1);
        HRESULT hr = _tcsicmp(path0, path1) == 0 ? S_OK : E_FAIL;
        ILFree((LPITEMIDLIST)pidlFolder);
        return hr;
    }
        
    // IContextMenu
    virtual HRESULT STDMETHODCALLTYPE QueryContextMenu( 
        _In_  HMENU hmenu,
        _In_  UINT indexMenu,
        _In_  UINT idCmdFirst,
        _In_  UINT idCmdLast,
        _In_  UINT uFlags)
    {
        DShowPlayer* player = _AtlModule.getPlayer();

        m_CmdFirst = idCmdFirst;
        
        InsertMenu(hmenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        MENUITEMINFO mmi = {0};
        mmi.cbSize = sizeof(MENUITEMINFO);
        SIZE size = { 16, 16 };
        
        HBITMAP hBmp[5];
        CreateBitmapFromIcon(LoadIcon(_AtlModule.m_hInst, MAKEINTRESOURCE(IDI_ICON1)), 0, size, &hBmp[0]);
        CreateBitmapFromIcon(LoadIcon(_AtlModule.m_hInst, MAKEINTRESOURCE(IDI_ICON2)), 0, size, &hBmp[1]);
        CreateBitmapFromIcon(LoadIcon(_AtlModule.m_hInst, MAKEINTRESOURCE(IDI_ICON3)), 0, size, &hBmp[2]);
        CreateBitmapFromIcon(LoadIcon(_AtlModule.m_hInst, MAKEINTRESOURCE(IDI_ICON4)), 0, size, &hBmp[3]);
        CreateBitmapFromIcon(LoadIcon(_AtlModule.m_hInst, MAKEINTRESOURCE(IDI_ICON5)), 0, size, &hBmp[4]);

#ifdef DEBUG
        mmi.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_BITMAP;
        mmi.hbmpItem = hBmp[4];
        mmi.fType = MFT_STRING;
        mmi.fState = MFS_ENABLED;
        mmi.wID = idCmdFirst++;
        mmi.dwTypeData = _T("Raise Exception (DEBUG)");
        mmi.cch = sizeof("Raise Exception (DEBUG)") - 1;
        InsertMenuItem(hmenu, -1, TRUE, &mmi);            
        memset(&mmi, 0, sizeof(mmi));
        mmi.cbSize = sizeof(MENUITEMINFO);
#endif
        MonitorWrap mi;
        int u = mi.indexFromCursor();
        int cc = g_SceneData[u].cellCount;

        switch (g_SceneData[u].viewMode)
        {
        case PSVM_Stretch: u = 0; break;
        case PSVM_WrapTile: u = 2; break;
        case PSVM_Center: u = 3; break;
        default: case PSVM_MirrorTile: u = 1; break;
        }

        HMENU hSubMenu = LoadMenu(_AtlModule.m_hInst, MAKEINTRESOURCE(IDR_MENU1));
        UINT mc = GetMenuItemCount(hSubMenu);
        for(UINT i=0; i<mc; i++) {
            mmi.cch = 0;
            GetMenuItemInfo(hSubMenu, i, TRUE, &mmi);
            if(mmi.fType & MFT_SEPARATOR) continue;
            mmi.fMask |= MIIM_BITMAP | MIIM_ID;
            mmi.wID = idCmdFirst++;
            if(i == 11) {
                mmi.hbmpItem = hBmp[0];
                mmi.fMask |= MIIM_STRING;
                if(player) {
                    mmi.dwTypeData = L"Stop Eyereach";
                    mmi.cch = sizeof("Stop Eyereach") - 1;
                }
                else {
                    mmi.dwTypeData = L"Run Eyereach";
                    mmi.cch = sizeof("Run Eyereach") - 1;
                }
            }
            else if(i == 9) {
                mmi.fMask &= (~MIIM_STRING);
                mmi.hbmpItem = hBmp[2];
            }
            else if(i == 13) {
                mmi.fMask &= (~MIIM_STRING);
                mmi.hbmpItem = hBmp[1];
            }
            else if (i >= 5 && i <= 7)
            {
                mmi.fMask |= MIIM_STATE;
                mmi.fMask &= (~MIIM_BITMAP);
                mmi.fState = MFS_DISABLED;
                if (player) {
                    switch (i)
                    {
                    case 5:
                        if (u == 0 || u == 3)
                            mmi.fState = MFS_ENABLED;
                        break;
                    default:
                        if (u == 1 || u == 2)
                            mmi.fState = MFS_ENABLED;
                        break;
                    }
                }
            }
            else if(i != 13) {
                mmi.fMask |= MIIM_STATE;
                mmi.fMask &= (~MIIM_BITMAP);
                if(player) {
                    mmi.fState = MFS_ENABLED;
                }
                else {
                    mmi.fState = MFS_DISABLED;
                }
            }
            SetMenuItemInfo(hSubMenu, i, TRUE, &mmi);
        }

        CheckMenuRadioItem(hSubMenu, 0, 3, u, MF_BYPOSITION);

        switch (abs(cc))
        {
        case 1: u = 5; break;
        case 2: u = 6; break;
        case 4: u = 7; break;
        default: u = 0; break;
        }
        if (u != 0)
        {
            CheckMenuRadioItem(hSubMenu, 5, 7, u, MF_BYPOSITION);
        }

        mmi.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_BITMAP | MIIM_SUBMENU; 
        mmi.hbmpItem = hBmp[0];
        mmi.fType = MFT_STRING;
        mmi.fState = MFS_ENABLED;        
        mmi.dwTypeData = _T("Eyereach Miracle Space");
        mmi.cch = sizeof("Eyereach Miracle Space") - 1;
        mmi.hSubMenu = hSubMenu;

        InsertMenuItem(hmenu, -1, TRUE, &mmi);
        if(player) {
            mmi.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_BITMAP | MIIM_STATE;
            mmi.hbmpItem = hBmp[2];
            mmi.fType = MFT_STRING;
            mmi.fState = MFS_ENABLED;
            mmi.wID = idCmdFirst++;
            if(player->getState() == STATE_RUNNING) {
                mmi.dwTypeData = _T("Pause Eyereach");
                mmi.cch = sizeof("Pause Eyereach") - 1;
            }
            else {
                mmi.dwTypeData = _T("Play Eyereach");
                mmi.cch = sizeof("Play Eyereach") - 1;
                if(player->getState() != STATE_PAUSED) 
                    mmi.fState = MFS_DISABLED;
            }
            InsertMenuItem(hmenu, -1, TRUE, &mmi);            
        }

        InsertMenu(hmenu, -1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, idCmdFirst - m_CmdFirst);
    }
    
#ifdef DEBUG
#define VERBIDX(i) (i)
#else
#define VERBIDX(i) (i-1)
#endif

    virtual HRESULT STDMETHODCALLTYPE InvokeCommand(         
        _In_  CMINVOKECOMMANDINFO *pici)
    {
        if(!pici) return E_POINTER;
        MonitorWrap mi;
        int i = mi.indexFromCursor();
        if(IS_INTRESOURCE(pici->lpVerb)) {
            switch(INT_PTR(pici->lpVerb)) {
            case VERBIDX(1): _AtlModule.setPrefDWORD(Pref_ViewMode, i, PSVM_Stretch); break;
            case VERBIDX(2): _AtlModule.setPrefDWORD(Pref_ViewMode, i, PSVM_MirrorTile); break;
            case VERBIDX(3): _AtlModule.setPrefDWORD(Pref_ViewMode, i, PSVM_WrapTile); break;
			case VERBIDX(4): _AtlModule.setPrefDWORD(Pref_ViewMode, i, PSVM_Center); break;
            case VERBIDX(6): _AtlModule.setPrefDWORD(Pref_CellCount, i, 1); break;
            case VERBIDX(7): _AtlModule.setPrefDWORD(Pref_CellCount, i, 2); break;
            case VERBIDX(8): _AtlModule.setPrefDWORD(Pref_CellCount, i, 4); break;
            case VERBIDX(10): _AtlModule.SetVolume(10000); break;
            case VERBIDX(12): _AtlModule.ToggleRunStop(); break;
            case VERBIDX(14): _AtlModule.ShowPreferenceUI(); break;
            case VERBIDX(15): _AtlModule.TogglePlayPause(); break;
            default : {
                memset((void*)4, 0, 0xFF); // FOR RaiseError!!!
                } break;
            }
        }
        return S_OK;
    }
    
    virtual HRESULT STDMETHODCALLTYPE GetCommandString( 
        _In_  UINT_PTR idCmd,
        _In_  UINT uType,
        _Reserved_  UINT *pReserved,
        _Out_writes_bytes_((uType & GCS_UNICODE) ? (cchMax * sizeof(wchar_t)) : cchMax) _When_(!(uType & (GCS_VALIDATEA | GCS_VALIDATEW)), _Null_terminated_) 
        CHAR *pszName,
        _In_  UINT cchMax)
    {
        switch (uType)
        {
        case GCS_VALIDATEA: case GCS_VALIDATEW:
            return (idCmd >= m_CmdFirst || (idCmd - m_CmdFirst) <= 10) ? S_OK : S_FALSE;
        case GCS_VERBA:
            if(idCmd == m_CmdFirst+1) {
                memcpy(pszName, "asConfig", sizeof("asConfig"));
                return S_OK;
            }
            else if(idCmd == m_CmdFirst+2) {
                memcpy(pszName, "asPlay", sizeof("asPlay"));
                return S_OK;
            }
            break;
        case GCS_VERBW:
            if(idCmd == m_CmdFirst+1) {
                memcpy(pszName, L"asConfig", sizeof(L"asConfig"));
                return S_OK;
            }
            else if(idCmd == m_CmdFirst+2) {
                memcpy(pszName, L"asPlay", sizeof(L"asPlay"));
                return S_OK;
            }
            break;
        }
        return E_NOT_SET;
    }

private:
    HRESULT __stdcall Create32BitHBITMAP(HDC hDC, const SIZE& size, PVOID* bytes, HBITMAP* phBmp)
    {
        BITMAPINFO bmp = { 0 };
        bmp.bmiHeader.biSize = sizeof(bmp.bmiHeader);
        bmp.bmiHeader.biWidth = size.cx;
        bmp.bmiHeader.biHeight = size.cy;
        bmp.bmiHeader.biPlanes = 1;
        bmp.bmiHeader.biBitCount = 32;
        bmp.bmiHeader.biCompression = BI_RGB;

        if(hDC == NULL) hDC = GetDC(NULL);
        *phBmp = CreateDIBSection(hDC, &bmp, DIB_RGB_COLORS, (void**)bytes, NULL, 0);
        ReleaseDC(NULL, hDC);

        return *phBmp ? S_OK : E_FAIL;
    }
    
    HRESULT __stdcall CreateBitmapFromIcon(HICON hIcon, int, const SIZE& size, HBITMAP* phBmp)
    {
        DWORD* byte;

        if(FAILED(Create32BitHBITMAP(NULL, size, (void**)&byte, phBmp)))
            return E_FAIL;

        HDC hdc = CreateCompatibleDC(NULL);
        HGDIOBJ hold = SelectObject(hdc, *phBmp);
        
        HIMAGELIST hlist = ImageList_Create(size.cx, size.cy, ILC_COLOR32|ILC_MASK, 1, 1);
        ImageList_ReplaceIcon(hlist, -1, hIcon);
        
        CComPtr<IImageList2> plist;
        HRESULT hr = HIMAGELIST_QueryInterface(hlist, IID_IImageList2, (void**)&plist);
        ImageList_Destroy(hlist);

        if(plist) {
            IMAGELISTDRAWPARAMS imgparam;
            memset(&imgparam, 0, sizeof(IMAGELISTDRAWPARAMS));
            imgparam.cbSize = sizeof(IMAGELISTDRAWPARAMS);
            imgparam.himl = hlist;
            imgparam.hdcDst = hdc;
            imgparam.cx = size.cx;
            imgparam.cy = size.cy;
            imgparam.rgbBk = CLR_DEFAULT;
            imgparam.rgbFg = CLR_DEFAULT;
            imgparam.fStyle = ILD_TRANSPARENT | 0x50000;
            plist->Draw(&imgparam);
            plist.Release();
        }
        SelectObject(hdc, hold);
        DeleteObject(hdc);
        
        return hr;
    }

private:
    UINT m_CmdFirst;
};

OBJECT_ENTRY_AUTO(CLSID_EyereachMenu, CEyereachMenu)
