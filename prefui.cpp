#include "stdafx.h"

#include "common.h"
#include <gdiplus.h>

#include "resource.h"
#include "Eyereach.h"

#include "views.h"

extern SCENEDATA g_SceneData[];

#define GDI(type) Gdiplus::type

struct GdiplusInitData : public GDI(GdiplusStartupInput)
{
	ULONG_PTR _token;

	GdiplusInitData(void) {
		GDI(GdiplusStartup)(&_token, this, NULL);
	}

	~GdiplusInitData(void) {
		GDI(GdiplusShutdown)(_token);
	}
};

GdiplusInitData GdiplusData;

/*
================================================
*/
#define DECL_EMPTY_DLGITEMPOS() \
    { 0, (0), 0, 0, NULL, 0, { 0, 0, 0, 0 }, NULL }

#define AR_ROOT     0
#define AR_PARENT   1
#define AR_SIBLING  2
#define AR_SIZE     3

void WindowPosFromPosDef_HRP(CRect* par3, DLGITEMPOSDEF& posdef, WINDOWPOS& pos)
{
    CRect* prc = &par3[AR_PARENT];
    if(posdef.align & DI_ALIGN_RIGHT) {
        pos.x = prc->right - posdef.margin.right - posdef.width;
    }
    else if(posdef.align & DI_ALIGN_HCENTER) {
        pos.x = prc->left + (prc->Width() - (posdef.margin.left + posdef.margin.right) - posdef.width) / 2;
    }
    else if(posdef.align & DI_ALIGN_HSTRETCH) {
        pos.x = prc->left + posdef.margin.left;
        pos.cx = prc->Width() - posdef.margin.left - posdef.margin.right;
    }
    else {
        pos.x = prc->left + posdef.margin.left;
    }   
}

void WindowPosFromPosDef_HRS(CRect* par3, DLGITEMPOSDEF& posdef, WINDOWPOS& pos)
{
    CRect* prc = &par3[AR_SIBLING];
    if(posdef.align & DI_ALIGN_RIGHT) {
        pos.x = prc->right + posdef.margin.left;
    }
    else if(posdef.align & DI_ALIGN_HCENTER) {
        pos.x = prc->CenterPoint().x + posdef.margin.left;
    }
    else if(posdef.align & DI_ALIGN_HSTRETCH) {
        ATLASSERT(FALSE);
    }
    else {
        pos.x = prc->left + posdef.margin.left;
    }
}

void WindowPosFromPosDef_H(CRect* par3, DLGITEMPOSDEF& posdef, WINDOWPOS& pos)
{
    CRect* prc = &par3[AR_ROOT];
    if(posdef.align & DI_ALIGN_RIGHT) {
        pos.x = prc->right - posdef.margin.right - posdef.width;
    }
    else if(posdef.align & DI_ALIGN_HCENTER) {
        pos.x = (prc->Width() - (posdef.margin.left + posdef.margin.right) - posdef.width) / 2;
    }
    else if(posdef.align & DI_ALIGN_HSTRETCH) {
        pos.x = posdef.margin.left;
        pos.cx = prc->Width() - posdef.margin.left - posdef.margin.right;
    }
    else {
        pos.x = posdef.margin.left;
    }
}

void WindowPosFromPosDef_VRP(CRect* par3, DLGITEMPOSDEF& posdef, WINDOWPOS& pos)
{
    CRect* prc = &par3[AR_PARENT];
    if(posdef.align & DI_ALIGN_BOTTOM) {
        pos.y = prc->bottom - posdef.margin.bottom - posdef.height;
    }
    else if(posdef.align & DI_ALIGN_VCENTER) {
        pos.y = prc->top + (prc->Height() - (posdef.margin.top + posdef.margin.bottom) - posdef.height) / 2;
    }
    else if(posdef.align & DI_ALIGN_VSTRETCH) {
        pos.y = prc->top + posdef.margin.top;
        pos.cy = prc->Height() - posdef.margin.top - posdef.margin.bottom;
    }
    else {
        pos.y = prc->top + posdef.margin.top;
    }
}

void WindowPosFromPosDef_VRS(CRect* par3, DLGITEMPOSDEF& posdef, WINDOWPOS& pos)
{
    CRect* prc = &par3[AR_SIBLING];
    if(posdef.align & DI_ALIGN_BOTTOM) {
        pos.y = prc->bottom + posdef.margin.top;
    }
    else if(posdef.align & DI_ALIGN_VCENTER) {
        pos.y = prc->CenterPoint().y + posdef.margin.top;;
    }
    else if(posdef.align & DI_ALIGN_VSTRETCH) {
        ATLASSERT(FALSE);
    }
    else {
        pos.y = prc->top + posdef.margin.top;
    }
}

void WindowPosFromPosDef_V(CRect* par3, DLGITEMPOSDEF& posdef, WINDOWPOS& pos)
{
    CRect* prc = &par3[AR_ROOT];
    if(posdef.align & DI_ALIGN_BOTTOM) {
        pos.y = prc->bottom - posdef.margin.bottom - posdef.height;
    }
    else if(posdef.align & DI_ALIGN_VCENTER) {
        pos.y = (prc->Height() - (posdef.margin.top + posdef.margin.bottom) - posdef.height) / 2;
    }
    else if(posdef.align & DI_ALIGN_VSTRETCH) {
        pos.y = posdef.margin.top;
        pos.cy = prc->Height() - posdef.margin.top - posdef.margin.bottom;
    }
    else {
        pos.y = posdef.margin.top;
    }
}

void DeferWindowPosViaPosDef(HWND hDlgWnd, HDWP& hDwp, CRect* par3, DLGITEMPOSDEF& posdef, WINDOWPOS& pos)
{   
    // psibpos[0] //top-level window
    // psibpos[1] //parent control
    // psibpos[2] //sibling
    pos.flags = 0;    
    pos.cx = posdef.width;
    pos.cy = posdef.height;
    if(posdef.idAfter) pos.hwndInsertAfter = GetDlgItem(hDlgWnd, posdef.idAfter);
    else pos.hwndInsertAfter = 0;    
    if(!posdef.idAfter) pos.flags |= SWP_NOZORDER;    
    pos.hwnd = GetDlgItem(hDlgWnd, posdef.id);

    CRect* prc;
    if(posdef.align & DI_ALIGN_HRELPARENT)
    {
        WindowPosFromPosDef_HRP(par3, posdef, pos);
    }
    else if(posdef.align & DI_ALIGN_HRELSIBLING)
    {
        WindowPosFromPosDef_HRS(par3, posdef, pos);
    }
    else
    {
        WindowPosFromPosDef_H(par3, posdef, pos);
    }
    
    if(posdef.align & DI_ALIGN_VRELPARENT)
    {
        WindowPosFromPosDef_VRP(par3, posdef, pos);
    }
    else if(posdef.align & DI_ALIGN_VRELSIBLING)
    {
        WindowPosFromPosDef_VRS(par3, posdef, pos);
    }
    else
    {
        WindowPosFromPosDef_V(par3, posdef, pos);
    }
        
    hDwp = DeferWindowPos(hDwp, pos.hwnd, pos.hwndInsertAfter, pos.x, pos.y, pos.cx, pos.cy, pos.flags);
    CRect ar[AR_SIZE];
    memcpy(ar, par3, sizeof(ABSRECT));
    ar[AR_PARENT] = CRect(CPoint(pos.x, pos.y), CSize(pos.cx, pos.cy));
    memset(ar+2, 0, sizeof(ABSRECT));
    for(int i=0; i<posdef.logchild_count; i++)
    {
        DeferWindowPosViaPosDef(hDlgWnd, hDwp, ar, posdef.logical_children[i], pos);
    }
    par3[AR_SIBLING] = ar[AR_PARENT];
}


/*
================================================
*/

#define EnableDlgItem(id, enabled) \
    ::EnableWindow(GetDlgItem(id), enabled)

class CEyereachPrefMonitorPage : public CDialogImpl<CEyereachPrefMonitorPage>
{
public:
    enum { IDD = IDD_DIALOG2 };
    CEyereachPrefMonitorPage(SCENEDATA* psd) : mp_SceneData(psd)
    {
    }

    BEGIN_MSG_MAP(CEyereachPrefMonitorPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnWmInitDialog)

        COMMAND_RANGE_HANDLER(IDC_EDIT1, IDC_EDIT5, OnIdEdit)
        COMMAND_RANGE_HANDLER(IDC_RADIO1, IDC_RADIO4, OnIdRadio)
        COMMAND_RANGE_HANDLER(IDC_RADIO5, IDC_RADIO7, OnIdRadio)
        COMMAND_RANGE_HANDLER(IDC_CHECK2, IDC_CHECK4, OnIdCheck)
    END_MSG_MAP();

    LRESULT OnWmInitDialog(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled) {
        _updateLayout();
        CheckDlgButton(IDC_CHECK2, mp_SceneData->mirrorHorz ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(IDC_CHECK3, mp_SceneData->mirrorVert ? BST_CHECKED : BST_UNCHECKED);

        int id = IDC_RADIO1;
        switch(mp_SceneData->viewMode)
        {
        case PSVM_MirrorTile: id = IDC_RADIO2; break;
        case PSVM_WrapTile:   id = IDC_RADIO3; break;
        case PSVM_Center:     id = IDC_RADIO4; break;
        }
        CheckRadioButton(IDC_RADIO1, IDC_RADIO4, id);
        
        id = IDC_RADIO5;
        switch (mp_SceneData->viewMode)
        {
        case PSVM_MirrorTile:
        case PSVM_WrapTile: 
            ::EnableWindow(GetDlgItem(IDC_RADIO5), FALSE);
            ::EnableWindow(GetDlgItem(IDC_RADIO6), TRUE);
            ::EnableWindow(GetDlgItem(IDC_RADIO7), TRUE);
            id = (mp_SceneData->cellCount == 4) ? IDC_RADIO7 : IDC_RADIO6; break;
        default:
            ::EnableWindow(GetDlgItem(IDC_RADIO5), TRUE);
            ::EnableWindow(GetDlgItem(IDC_RADIO6), FALSE);
            ::EnableWindow(GetDlgItem(IDC_RADIO7), FALSE);
            break;
        }
        CheckDlgButton(IDC_CHECK4, mp_SceneData->cellCount < 0);
        ::EnableWindow(GetDlgItem(IDC_CHECK4), id == IDC_RADIO6);
        CheckRadioButton(IDC_RADIO5, IDC_RADIO7, id);

        
        WCHAR txt[32];
        _itow(mp_SceneData->clipLeft, txt, 10);
        SetDlgItemText(IDC_EDIT1, txt);
        _itow(mp_SceneData->clipTop, txt, 10);
        SetDlgItemText(IDC_EDIT2, txt);
        _itow(mp_SceneData->clipRight, txt, 10);
        SetDlgItemText(IDC_EDIT3, txt);
        _itow(mp_SceneData->clipBottom, txt, 10);
        SetDlgItemText(IDC_EDIT4, txt);
        _itow(mp_SceneData->rotateAngle, txt, 10);
        SetDlgItemText(IDC_EDIT5, txt);

        SetDlgItemText(IDC_EDITFILE, mp_SceneData->s_path);

        return 0;
    }

    LRESULT OnIdEdit(WORD code, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnIdRadio(WORD cod, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnIdCheck(WORD cod, WORD id, HWND hwnd, BOOL& bHandled);    

    static DLGITEMPOSDEF m_dlgItemPosDefs_Clip[];
    static DLGITEMPOSDEF m_dlgItemPosDefs_View[];
    static DLGITEMPOSDEF m_dlgItemPosDefs_Cells[];
    static DLGITEMPOSDEF m_dlgItemPosDefs_Effect[];
    static DLGITEMPOSDEF m_dlgItemPosDefs[];

private:
    SCENEDATA* mp_SceneData;
    void _updateLayout(void);
};

DLGITEMPOSDEF CEyereachPrefMonitorPage::m_dlgItemPosDefs_View[] = {
    DECL_DLGITEMPOS(IDC_RADIO1, 0, 130, 20,   (DI_ALIGN_LEFT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 16, 22, 4, 0),
    DECL_DLGITEMPOS(IDC_RADIO2, 0, 130, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 6, 4, 0),
    DECL_DLGITEMPOS(IDC_RADIO3, 0, 130, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 6, 4, 0),
    DECL_DLGITEMPOS(IDC_RADIO4, 0, 130, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 6, 4, 0)
};

DLGITEMPOSDEF CEyereachPrefMonitorPage::m_dlgItemPosDefs_Cells[] = {
    DECL_DLGITEMPOS(IDC_RADIO5, 0, 100, 20,   (DI_ALIGN_LEFT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 16, 22, 4, 0),
    DECL_DLGITEMPOS(IDC_RADIO6, 0, 100, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 6, 4, 0),
    DECL_DLGITEMPOS(IDC_CHECK4, 0, 80, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 16, 6, 4, 0),
    DECL_DLGITEMPOS(IDC_RADIO7, 0, 100, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), -16, 6, 4, 0)
};

DLGITEMPOSDEF CEyereachPrefMonitorPage::m_dlgItemPosDefs_Effect[] = {
    DECL_DLGITEMPOS(IDC_CHECK2, 0, 130, 20,   (DI_ALIGN_LEFT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 16, 22, 4, 0),    
    DECL_DLGITEMPOS(IDC_CHECK3, 0, 130, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 6, 4, 0),    
    //DECL_DLGITEMPOS(IDC_CHECK3, 0, 160, 20,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 6, 4, 0),
    //DECL_DLGITEMPOS(IDC_CHECK4, 0, 160, 16,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 12, 4, 0)
};

DLGITEMPOSDEF CEyereachPrefMonitorPage::m_dlgItemPosDefs_Clip[] = {
    DECL_DLGITEMPOS(ID_STATIC00, 0, 48, 24,   (DI_ALIGN_LEFT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 6, 22, 4, 0),
    DECL_DLGITEMPOS(ID_STATIC01, 0, 48, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0),
    DECL_DLGITEMPOS(ID_STATIC02, 0, 48, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0),
    DECL_DLGITEMPOS(ID_STATIC03, 0, 48, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0),
    DECL_DLGITEMPOS(ID_STATIC04, 0, 48, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0),
    DECL_DLGITEMPOS(IDC_EDIT1, 0, 130, 24,   (DI_ALIGN_LEFT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 62, 22, 4, 0),    
    DECL_DLGITEMPOS(IDC_EDIT2, 0, 130, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0),    
    DECL_DLGITEMPOS(IDC_EDIT3, 0, 130, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0),    
    DECL_DLGITEMPOS(IDC_EDIT4, 0, 130, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0),
    DECL_DLGITEMPOS(IDC_EDIT5, 0, 130, 24,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 4, 0)
};

DLGITEMPOSDEF CEyereachPrefMonitorPage::m_dlgItemPosDefs[] = {
    //DECL_DLGITEMPOS(IDCANCEL,       0, 80, 28,   (DI_ALIGN_RIGHT | DI_ALIGN_BOTTOM),      0, 0, 8, 8),
    //DECL_DLGITEMPOS(IDOK,           0, 80, 28,   (DI_ALIGN_RIGHT | DI_ALIGN_BOTTOM),      0, 0, 96, 8),
    //DECL_DLGITEMPOS_CHILD(ID_GROUPFILE,   0, 0,  52,   (DI_ALIGN_HSTRETCH | DI_ALIGN_TOP),      8, 4, 8, 0, CEyereachPrefUI::m_dlgItemPosDefs_File),
    DECL_DLGITEMPOS_CHILD(ID_GROUPTYPE,   0, 170, 130, (DI_ALIGN_LEFT | DI_ALIGN_TOP), 8, 8, 0, 0, CEyereachPrefMonitorPage::m_dlgItemPosDefs_View),
    DECL_DLGITEMPOS_CHILD(ID_GROUPEFFECT, 0, 170, 80, (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 0, 0, CEyereachPrefMonitorPage::m_dlgItemPosDefs_Effect),
    DECL_DLGITEMPOS_CHILD(ID_GROUPCELLS,   0, 120, 140, (DI_ALIGN_LEFT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 188, 8, 0, 0, CEyereachPrefMonitorPage::m_dlgItemPosDefs_Cells),
    DECL_DLGITEMPOS_CHILD(ID_GROUPCLIP,   0, 210, 218, (DI_ALIGN_RIGHT | DI_ALIGN_TOP | DI_ALIGN_RELSIBLING), 10, 0, 8, 8, CEyereachPrefMonitorPage::m_dlgItemPosDefs_Clip),
    //DECL_DLGITEMPOS_CHILD(ID_GROUPVIEW,   0, 0, 0,     (DI_ALIGN_HSTRETCH | DI_ALIGN_VSTRETCH), 216, 64, 8, 8, CEyereachPrefUI::m_dlgItemPosDefs_Preview)
};

void CEyereachPrefMonitorPage::_updateLayout(void) {
    CRect ar[AR_SIZE];
    GetClientRect(ar[AR_ROOT]);
    const int c = sizeof(m_dlgItemPosDefs) / sizeof(DLGITEMPOSDEF);
    HDWP hDwp = BeginDeferWindowPos(c);
    WINDOWPOS pos;
    for(int i = 0; i<c; i++) {
        //pos.hwnd = m_hWnd;
        ::DeferWindowPosViaPosDef(m_hWnd, hDwp, ar, m_dlgItemPosDefs[i], pos);
        //hDwp = ::DeferWindowPos(hDwp, pos.hwnd, pos.hwndInsertAfter, pos.x, pos.y, pos.cx, pos.cy, pos.flags);
    }
    EndDeferWindowPos(hDwp);
}

LRESULT CEyereachPrefMonitorPage::OnIdEdit(WORD code, WORD id, HWND hwnd, BOOL& bHandled)
{
    if(code != EN_CHANGE) {
        bHandled = FALSE; return 0;
    }

    //_AtlModule.setPrefDWORD(Pref_ClipboxLeft, EPBUF.left); 
    TCHAR txt[16];
    GetDlgItemText(id, txt, 15);
    switch(id) {
    case IDC_EDIT1: mp_SceneData->updateClipLeft(min(abs(_tstoi(txt)), 800)); break;
    case IDC_EDIT2: mp_SceneData->updateClipTop(min(abs(_tstoi(txt)), 800)); break;
    case IDC_EDIT3: mp_SceneData->updateClipRight(min(abs(_tstoi(txt)), 800)); break;
    case IDC_EDIT4: mp_SceneData->updateClipBottom(min(abs(_tstoi(txt)), 800)); break;
    case IDC_EDIT5: mp_SceneData->updateRotateAngle(_tstoi(txt) % 360); break;
    }
    return 0;
}

LRESULT CEyereachPrefMonitorPage::OnIdRadio(WORD code, WORD id, HWND hwnd, BOOL& bHandled)
{
    if(code != BN_CLICKED) {
        bHandled = FALSE; return 0;
    }
    
    switch (id)
    {
    case IDC_RADIO1: 
        EnableDlgItem(IDC_RADIO5, TRUE);
        EnableDlgItem(IDC_RADIO6, FALSE);
        EnableDlgItem(IDC_RADIO7, FALSE);
        CheckRadioButton(IDC_RADIO5, IDC_RADIO7, IDC_RADIO5);
        mp_SceneData->updateViewMode(PSVM_Stretch); break;
    case IDC_RADIO2: 
        EnableDlgItem(IDC_RADIO5, FALSE);
        EnableDlgItem(IDC_RADIO6, TRUE);
        EnableDlgItem(IDC_RADIO7, TRUE);
        if(IsDlgButtonChecked(IDC_RADIO5))
            CheckRadioButton(IDC_RADIO5, IDC_RADIO7, IDC_RADIO7);
        mp_SceneData->updateViewMode(PSVM_MirrorTile); break;
    case IDC_RADIO3: 
        EnableDlgItem(IDC_RADIO5, FALSE);
        EnableDlgItem(IDC_RADIO6, TRUE);
        EnableDlgItem(IDC_RADIO7, TRUE);
        if (IsDlgButtonChecked(IDC_RADIO5))
            CheckRadioButton(IDC_RADIO5, IDC_RADIO7, IDC_RADIO7);
        mp_SceneData->updateViewMode(PSVM_WrapTile); break;
    case IDC_RADIO4: 
        EnableDlgItem(IDC_RADIO5, TRUE);
        EnableDlgItem(IDC_RADIO6, FALSE);
        EnableDlgItem(IDC_RADIO7, FALSE);
        EnableDlgItem(IDC_CHECK4, FALSE);
        CheckRadioButton(IDC_RADIO5, IDC_RADIO7, IDC_RADIO5);
        mp_SceneData->updateViewMode(PSVM_Center); break;
    case IDC_RADIO5: EnableDlgItem(IDC_CHECK4, FALSE); mp_SceneData->updateCellCount(1); break;
    case IDC_RADIO6: EnableDlgItem(IDC_CHECK4, TRUE); mp_SceneData->updateCellCount(IsDlgButtonChecked(IDC_CHECK4) ? -2 : 2); break;
    case IDC_RADIO7: EnableDlgItem(IDC_CHECK4, FALSE); mp_SceneData->updateCellCount(4); break;
    default: return 0;
    }
    return 0;
}

LRESULT CEyereachPrefMonitorPage::OnIdCheck(WORD code, WORD id, HWND hwnd, BOOL& bHandled)
{
    if(code != BN_CLICKED) {
        bHandled = FALSE; return 0;
    }

    switch (id)
    {
    case IDC_CHECK2: mp_SceneData->updateMirrorHorz(IsDlgButtonChecked(id) ? 0xFF : 0); break;
    case IDC_CHECK3: mp_SceneData->updateMirrorVert(IsDlgButtonChecked(id) ? 0xFF : 0); break;
    case IDC_CHECK4: mp_SceneData->updateCellCount(IsDlgButtonChecked(id) ? -2 : 2); break;
    }
    return 0;
}

/*
================================================
*/

DLGITEMPOSDEF CEyereachPrefUI::m_dlgItemPosDefs_File[] = {
    DECL_DLGITEMPOS(IDC_EDITFILE, 0, 0, 24,   (DI_ALIGN_HSTRETCH|DI_ALIGN_TOP|DI_ALIGN_RELPARENT), 8, 20, 90, 8),
    DECL_DLGITEMPOS(IDC_BUTTON1,  0, 72, 28, (DI_ALIGN_RIGHT|DI_ALIGN_TOP|DI_ALIGN_RELPARENT), 0, 18, 8, 0)
};

DLGITEMPOSDEF CEyereachPrefUI::m_dlgItemPosDefs_Preview[] = {
    DECL_DLGITEMPOS(IDC_STATICPREV, 0, 0, 0,   (DI_ALIGN_STRETCH | DI_ALIGN_RELPARENT), 8, 22, 8, 8)
};

DLGITEMPOSDEF CEyereachPrefUI::m_dlgItemPosDefs_Control[] = {
    DECL_DLGITEMPOS(IDC_CHECK1, 0, 140, 26,   (DI_ALIGN_LEFT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 8, 16, 4, 6),    
    DECL_DLGITEMPOS(IDC_CHECK5, 0, 120, 26,   (DI_ALIGN_RIGHT | DI_ALIGN_TOP | DI_ALIGN_RELSIBLING), 8, 0, 4, 6),
    DECL_DLGITEMPOS(IDC_BUTTON3, 0, 65, 24,   (DI_ALIGN_RIGHT | DI_ALIGN_TOP | DI_ALIGN_RELSIBLING), 4, 0, 4, 6),
    DECL_DLGITEMPOS(IDC_BUTTON4, 0, 65, 24,   (DI_ALIGN_RIGHT | DI_ALIGN_TOP | DI_ALIGN_RELSIBLING), 8, 0, 4, 6),

    DECL_DLGITEMPOS(ID_VOLUME, 0, 140, 22,   (DI_ALIGN_RIGHT | DI_ALIGN_TOP | DI_ALIGN_RELPARENT), 8, 22, 8, 6),

    DECL_DLGITEMPOS(ID_SLIDER, 0, 0, 26,   (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_HSTRETCH | DI_ALIGN_RELPARENT), 8, 0, 8, 6)
};

DLGITEMPOSDEF CEyereachPrefUI::m_dlgItemPosDefs[] = {
    //DECL_DLGITEMPOS(IDCANCEL,       0, 80, 28,   (DI_ALIGN_RIGHT | DI_ALIGN_BOTTOM),      0, 0, 8, 8),
    //DECL_DLGITEMPOS(IDOK,           0, 80, 28,   (DI_ALIGN_RIGHT | DI_ALIGN_BOTTOM),      0, 0, 96, 8),
    DECL_DLGITEMPOS_CHILD(ID_GROUPFILE,   0, 0, 54,   (DI_ALIGN_HSTRETCH | DI_ALIGN_TOP),      8, 4, 8, 0, CEyereachPrefUI::m_dlgItemPosDefs_File),
    DECL_DLGITEMPOS_CHILD(ID_GROUPCONTROL,   0, 0, 76,   (DI_ALIGN_HSTRETCH | DI_ALIGN_TOP),      8, 58, 8, 0, CEyereachPrefUI::m_dlgItemPosDefs_Control),
    //DECL_DLGITEMPOS_CHILD(ID_GROUPCLIP,   0, 200, 150, (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 0, 0, CEyereachPrefUI::m_dlgItemPosDefs_Clip),
    //DECL_DLGITEMPOS_CHILD(ID_GROUPTYPE,   0, 200, 130, (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 0, 0, CEyereachPrefUI::m_dlgItemPosDefs_View),
    //DECL_DLGITEMPOS_CHILD(ID_GROUPEFFECT, 0, 200, 130, (DI_ALIGN_LEFT | DI_ALIGN_BOTTOM | DI_ALIGN_RELSIBLING), 0, 8, 0, 0, CEyereachPrefUI::m_dlgItemPosDefs_Effect),    
    //DECL_DLGITEMPOS_CHILD(ID_GROUPVIEW,   0, 0, 0,     (DI_ALIGN_HSTRETCH | DI_ALIGN_VSTRETCH), 216, 64, 8, 8, CEyereachPrefUI::m_dlgItemPosDefs_Preview)
    DECL_DLGITEMPOS(IDC_TAB1,   0, 0, 266,     (DI_ALIGN_HSTRETCH | DI_ALIGN_TOP), 8, 140, 8, 8, CEyereachPrefUI::m_dlgItemPosDefs_Preview)
};

CEyereachPrefUI::CEyereachPrefUI(void)
{
    mp_Image = NULL;
}

CEyereachPrefUI::~CEyereachPrefUI(void)
{
    delete ((GDI(Image)*)mp_Image);
}

HRESULT STDMETHODCALLTYPE CEyereachPrefUI::Initialize(IEyereachControl *pControl)
{
    HRSRC hrsrc = FindResource(_AtlModule.m_hInst, MAKEINTRESOURCE(IDB_PNG1), L"PNG");
    HGLOBAL hg = LoadResource(_AtlModule.m_hInst, hrsrc);
    void* p = LockResource(hg);
    int sz = SizeofResource(_AtlModule.m_hInst, hrsrc);
    IStream* ps = SHCreateMemStream((BYTE*)p, sz);    
    mp_Image = GDI(Image)::FromStream(ps, FALSE);    
    ps->Release();
    FreeResource(hg);

    if (pControl != (IEyereachControl*)0x1) 
    {
        this->Create(NULL);
        this->ShowWindow(SW_SHOW);
    }
    return S_OK;
}

void CEyereachPrefUI::_updateLayout() {
    CRect ar[AR_SIZE];
    GetClientRect(ar[AR_ROOT]);
    const int c = sizeof(m_dlgItemPosDefs) / sizeof(DLGITEMPOSDEF);
    HDWP hDwp = BeginDeferWindowPos(c);
    WINDOWPOS pos;
    for(int i = 0; i<c; i++) {
        //pos.hwnd = m_hWnd;
        ::DeferWindowPosViaPosDef(m_hWnd, hDwp, ar, m_dlgItemPosDefs[i], pos);
        //hDwp = ::DeferWindowPos(hDwp, pos.hwnd, pos.hwndInsertAfter, pos.x, pos.y, pos.cx, pos.cy, pos.flags);
    }
    EndDeferWindowPos(hDwp);
};

LRESULT CEyereachPrefUI::OnWmKeyUp(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)
{
    switch (wp)
    {
    case VK_ESCAPE: _doQuitOrApply(false); break;
    default:
        break;
    }
    return 0;
}

#define SLIDER_MAX_I 15000
#define SLIDER_MAX_F 15000.0f
#define SLIDER_MAX_D 15000.0


LRESULT CEyereachPrefUI::OnWmInitDialog(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled) {
    CRect rc(0,0,WIN_WIDTH,WIN_HEIGHT);
    AdjustWindowRectEx(&rc, GetWindowLong(GWL_STYLE), FALSE, GetWindowLong(GWL_EXSTYLE));
    this->SetWindowPos(NULL, 0,0, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOMOVE);

    CheckDlgButton(IDC_CHECK4, BST_CHECKED);
    ::Button_Enable(::GetDlgItem(m_hWnd, IDC_CHECK4), FALSE);
    
    SetDlgItemText(IDC_EDITFILE, g_SceneData->s_path);
    CheckDlgButton(IDC_CHECK1, g_SceneData->s_mute ? BST_CHECKED : BST_UNCHECKED);
    
    ::Button_Enable(::GetDlgItem(m_hWnd, IDC_BUTTON1), _AtlModule.getPlayer() != NULL);
        
    HWND hwndSlider = ::GetDlgItem(m_hWnd, ID_SLIDER);
    LONG sl = ::GetWindowLong(hwndSlider, GWL_STYLE);
    ::SetWindowLong(hwndSlider, GWL_STYLE, sl | TBS_ENABLESELRANGE);
    this->SendDlgItemMessage(ID_SLIDER, TBM_SETRANGEMIN, FALSE, (LPARAM)0);
    this->SendDlgItemMessage(ID_SLIDER, TBM_SETRANGEMAX, FALSE, (LPARAM)15000);
    this->SendDlgItemMessage(ID_SLIDER, TBM_SETSELSTART, FALSE, (LPARAM)0);
    this->SendDlgItemMessage(ID_SLIDER, TBM_SETSELEND, FALSE, (LPARAM)15000);

    hwndSlider = ::GetDlgItem(m_hWnd, ID_VOLUME);
    this->SendDlgItemMessage(ID_VOLUME, TBM_SETRANGEMIN, FALSE, (LPARAM)-50);
    this->SendDlgItemMessage(ID_VOLUME, TBM_SETRANGEMAX, FALSE, (LPARAM)0);
    this->SendDlgItemMessage(ID_VOLUME, TBM_SETPOS, TRUE, (LPARAM)-10);

    _updateLayout();

    MonitorWrap mi, xmi;
    POINT pt;
    GetCursorPos(&pt);
    HMONITOR HM = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfo(HM, &xmi);

    int c = mi.getMonitorCount();    

    HWND hTab1 = ::GetDlgItem(m_hWnd, IDC_TAB1);

    TCITEM tci;
    tci.mask = TCIF_TEXT;
    tci.pszText = L"Monitor 1";
    TabCtrl_InsertItem(hTab1 , 0, &tci);
    CEyereachPrefMonitorPage* pmwin = new CEyereachPrefMonitorPage(&g_SceneData[0]);
    pmwin->Create(m_hWnd);
    mp_MWin[0] = pmwin;
    int i = 0;
    if(c>1) {
        tci.pszText = L"Monitor 2";
        TabCtrl_InsertItem(hTab1 , 1, &tci);
        pmwin = new CEyereachPrefMonitorPage(&g_SceneData[1]);
        pmwin->Create(m_hWnd);
        mp_MWin[1] = pmwin;    
        
        for(;i<c; i++) {
            mi.Fill(i);
            if(memcmp(&mi.rcMonitor, &xmi.rcMonitor, sizeof(RECT)) == 0) {
                break;
            }
        }
        if(i == c) i = 0;
    }
    else {
        mp_MWin[1] = NULL;
    }

    ::GetClientRect(hTab1, rc);
    CRect wrc = rc;
    ::GetWindowRect(hTab1, wrc);
    ScreenToClient(wrc);
    TabCtrl_AdjustRect(hTab1, FALSE, rc);
    for(int j=0; j<c && j<2; j++) {
        mp_MWin[j]->SetWindowPos(HWND_TOP, wrc.left + rc.left, wrc.top + rc.top, rc.Width(), rc.Height(), 0);
    }

    TabCtrl_SetCurSel(hTab1, i);
    mp_MWin[i]->ShowWindow(SW_SHOW);
    
    GetWindowRect(wrc);
    SetWindowPos(NULL, 
        ((xmi.rcMonitor.right - xmi.rcMonitor.left) - wrc.Width()) / 2 + xmi.rcMonitor.left,
        ((xmi.rcMonitor.bottom - xmi.rcMonitor.top) - wrc.Height()) / 2 + xmi.rcMonitor.top,
        0, 0, SWP_NOSIZE|SWP_NOZORDER);

    return 0;
}

void CEyereachPrefUI::_doQuitOrApply(BOOL apply) {
    mp_MWin[0]->DestroyWindow();
    delete mp_MWin[0];
    if(mp_MWin[1]) {
        mp_MWin[1]->DestroyWindow();   
        delete mp_MWin[1];
    }
    mp_MWin[0] = NULL;
    mp_MWin[1] = NULL;

    if (m_bModal) {
        EndDialog(0);
    }
    else {
        DestroyWindow();
        PostThreadMessage(GetCurrentThreadId(), WM_QUIT, apply, 0);
    }
}

LRESULT CEyereachPrefUI::OnTab1Notify(int id, LPNMHDR lp, BOOL& bHandled)
{
    switch (lp->code)
    {
    default: bHandled = FALSE; break;
    case TCN_SELCHANGE:
        switch(GetDlgItem(IDC_TAB1).SendMessageW(TCM_GETCURSEL))
        {
        case 1:  mp_MWin[0]->ShowWindow(SW_HIDE); mp_MWin[1]->ShowWindow(SW_SHOW); break;
        default: mp_MWin[0]->ShowWindow(SW_SHOW); if(mp_MWin[1]) { mp_MWin[1]->ShowWindow(SW_HIDE); } break;
        }
        break;
    }
    return 0;
}

#define DEFLATE 6
LRESULT CEyereachPrefUI::OnWmDrawItem(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)
{
    LPDRAWITEMSTRUCT ds = (LPDRAWITEMSTRUCT)lp;
    switch (wp)
    {
    case IDC_STATICPREV: __beginScope()
        GDI(Graphics) graphic(ds->hDC);
        GDI(SolidBrush) blackBrush(GDI(Color(0,0,0)));
        GDI(SolidBrush) silverBrush(GDI(Color(220,220,220)));
        GDI(Pen) pen(GDI(Color)(255,0,0), 2.0f);
        GDI(Pen) silverPen(&silverBrush, 2.0f);
        GDI(TextureBrush) imgBrush((GDI(Image)*)mp_Image, GDI(WrapModeTile));

        CRect crc(ds->rcItem);
        graphic.FillRectangle(&blackBrush, crc.left, crc.top, crc.Width(), crc.Height());
        
        crc.DeflateRect(DEFLATE, DEFLATE, DEFLATE, DEFLATE);

        CRect virtualScreen(CPoint(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN)),
            CSize(GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN)));
        
        int dx =0, dy = 0;
        if(virtualScreen.left < 0) {
            //virtualScreen.right -= virtualScreen.left;
            dx = 0 - virtualScreen.left;
            //virtualScreen.left = 0;
        }
        if(virtualScreen.top < 0) {
            //virtualScreen.bottom -= virtualScreen.top;
            dy = 0 - virtualScreen.top;
            //virtualScreen.top = 0;
        }

        float rx = (float)crc.Width() / (float)virtualScreen.Width();
        float ry = (float)crc.Height() / (float)virtualScreen.Height();

        CRect monitors[4];
        MonitorWrap MI;
        int cm = MI.getMonitorCount();
        for(int c=0; c<cm&&c<4; c++)
        {
            MI.Fill(c);
            monitors[c] = CRect(&MI.rcMonitor);
            monitors[c].OffsetRect(DEFLATE + dx, DEFLATE + dy);            
            graphic.DrawRectangle(&silverPen, 
                monitors[c].left * rx, monitors[c].top * ry, monitors[c].Width() * rx, monitors[c].Height() * ry);
            
            GDI(Matrix) matrix;
            monitors[c].DeflateRect(4.0f, 4.0f);
            matrix.Scale(monitors[c].Width() * rx / 128, monitors[c].Height() * ry / 128);            
            matrix.Translate(monitors[c].left * rx, monitors[c].top * ry, GDI(MatrixOrderAppend));
                        
            imgBrush.SetTransform(&matrix);
            graphic.FillRectangle(&imgBrush, monitors[c].left * rx, monitors[c].top * ry, monitors[c].Width() * rx, monitors[c].Height() * ry);
            imgBrush.ResetTransform();

            graphic.Flush();
        }

        __endScope() break;
    default: bHandled = false; break;
    }
    return 0l;
}

LRESULT CEyereachPrefUI::OnIdButton1(WORD code, WORD id, HWND hwnd, BOOL& bHandled)
{
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = _AtlModule.m_hInst;
    WCHAR filename[1024];    
    filename[0] = L'\0';
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 1020;
    ofn.lpstrTitle = L"Choose a video file to play";
    ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;
    if(!GetOpenFileName(&ofn)) return 0;

    HRESULT hr = E_FAIL;
    if(_AtlModule.getPlayer()) 
        hr = _AtlModule.Play(filename);

    if(SUCCEEDED(hr)) SetDlgItemText(IDC_EDITFILE, g_SceneData->s_path);

    return 0;
}

LRESULT CEyereachPrefUI::OnIdCheck(WORD code, WORD id, HWND hwnd, BOOL& bHandled)
{
    if(code != BN_CLICKED) {
        bHandled = FALSE; return 0;
    }

    switch (id)
    {
    case IDC_CHECK1: 
		g_SceneData->s_mute = IsDlgButtonChecked(id) ? 0xFFFFFFFF : 0; 		
		break;
    }
    return 0;
}

LRESULT CEyereachPrefUI::OnSliderReleaseCapture(int id, LPNMHDR lp, BOOL& bHandled)
{
    if (!_AtlModule.getPlayer()) return 0;

    if (id == ID_VOLUME)
    {
        LONG pos = this->SendDlgItemMessage(ID_VOLUME, TBM_GETPOS, 0, 0);
        _AtlModule.SetVolume(pos * 100);
    }
    else if (id == ID_SLIDER)
    {
        // selection mode
        if (this->IsDlgButtonChecked(IDC_CHECK5) == BST_CHECKED)
            return 0;

        int pos = this->SendDlgItemMessage(ID_SLIDER, TBM_GETPOS, 0, 0);

        _AtlModule.SetDuration((float)pos / SLIDER_MAX_F, (float)pos / SLIDER_MAX_F);
    }
    return 0;
}

LRESULT CEyereachPrefUI::OnIdButton3(WORD code, WORD id, HWND hwnd, BOOL& bHandled)
{
    // seeking mode
    if (this->IsDlgButtonChecked(IDC_CHECK5) == BST_UNCHECKED)
        return 0;

    if (code != BN_CLICKED) {
        bHandled = FALSE; return 0;
    }

    int pos = this->SendDlgItemMessage(ID_SLIDER, TBM_GETPOS, 0, 0);
    int start = this->SendDlgItemMessage(ID_SLIDER, TBM_GETSELSTART, 0, 0);
    int end = this->SendDlgItemMessage(ID_SLIDER, TBM_GETSELEND, 0, 0);

    int msg = 0;
    if (id == IDC_BUTTON3)
    {
        if (pos >= end) return 0;
        msg = TBM_SETSELSTART;
        start = pos;
    }
    else if (id == IDC_BUTTON4)
    {
        if (pos <= start) return 0;
        msg = TBM_SETSELEND;
        end = pos;
    }
    
    this->SendDlgItemMessage(ID_SLIDER, msg, TRUE, (LPARAM)pos);
    if (_AtlModule.getPlayer())
    {
        _AtlModule.SetDuration((float)start / SLIDER_MAX_F, (float)end / SLIDER_MAX_F);
    }
    return 0;
}

/*
============================================================
*/
void CALLBACK Eyereach_TestPrefUIW(void* arg1, void* arg2, LPWSTR arg3, void* arg4)
{
#ifdef DEBUG
    CEyereachPrefUI ui;
    ui.Initialize((IEyereachControl*)0x1);
    ui.DoModal(NULL);
    TerminateProcess(GetCurrentProcess(), 0);
#endif
}
