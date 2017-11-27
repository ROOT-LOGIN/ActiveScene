#ifndef ___VIEWS_H_INCLUDE___
#define ___VIEWS_H_INCLUDE___

/// <summary>
/// A class for subclassing a window
/// </summary>
class SUBCLASS_WINDOW
{
public:
	HWND m_hWnd;
	SUBCLASSPROC m_pfnSubProc;
	UINT_PTR m_uId;

	SUBCLASS_WINDOW(void);

	virtual BOOL Subclass(void);
	virtual BOOL Restore(void);

    operator HWND() const {
        return m_hWnd;
    }
};

#define __SUBCLASSMAIN(procName) \
	LRESULT CALLBACK procName(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR uIdSubclass, SUBCLASS_WINDOW* pThis)

#define __SUBCLASSPROC(handler) \
	LRESULT handler(SUBCLASS_WINDOW* pThis, UINT msg, WPARAM wp, LPARAM lp, bool& bHandled)

#define __SUBCLASSPROC_NOTIFY(handler, notifyStruct) \
	LRESULT handler(SUBCLASS_WINDOW* pThis, notifyStruct lp, bool& bHandled)

#define BEGIN_SUBCLASSPROC() \
	bool bHandled = true; LRESULT lRet = 0; \
	switch(msg) {

#define SUBCLASSPROC_ENTRY(wm, handler) \
	case wm: { bHandled = true; \
		lRet = handler(pThis, msg, wp, lp, bHandled); \
		if(bHandled) return lRet; } break;

#define BEGIN_SUBCLASSPROC_NOTIFY() \
    case WM_NOTIFY: { bHandled = true; \
        LPNMHDR lpnmhdr = reinterpret_cast<LPNMHDR>(lp); \
        switch(lpnmhdr->code) {

#define SUBCLASSPROC_NOTIFY_ENTRY(nm, handler, notifyStruct) \
    case nm: { bHandled = true; \
        lRet = handler(pThis, (notifyStruct)lp, bHandled); \
        if(bHandled) return lRet; } break;

#define END_SUBCLASSPROC_NOTIFY() \
    } }

#define END_SUBCLASSPROC() \
	} return DefSubclassProc(hwnd, msg, wp, lp);

/*
=================================================
*/

#define DI_ALIGN_LEFT         0x000
#define DI_ALIGN_HCENTER      0x001
#define DI_ALIGN_HSTRETCH     0x002
#define DI_ALIGN_RIGHT        0x004
#define DI_ALIGN_TOP          0x000
#define DI_ALIGN_VCENTER      0x010
#define DI_ALIGN_VSTRETCH     0x020
#define DI_ALIGN_BOTTOM       0x040
#define DI_ALIGN_HRELPARENT   0x100
#define DI_ALIGN_HRELSIBLING  0x200
#define DI_ALIGN_VRELPARENT   0x400
#define DI_ALIGN_VRELSIBLING  0x800
#define DI_ALIGN_RELROOT      0x000
#define DI_ALIGN_CENTER       (DI_ALIGN_HCENTER | DI_ALIGN_VCENTER)
#define DI_ALIGN_STRETCH      (DI_ALIGN_HSTRETCH | DI_ALIGN_VSTRETCH)
#define DI_ALIGN_RELSIBLING   (DI_ALIGN_HRELSIBLING|DI_ALIGN_VRELSIBLING)
#define DI_ALIGN_RELPARENT    (DI_ALIGN_HRELPARENT|DI_ALIGN_VRELPARENT)

/// <summary>
/// A structure that defines the dialog item position
/// </summary>
struct DLGITEMPOSDEF {
    WORD id;    
    WORD idAfter;
    WORD width;
    WORD height;    
    WORD align;
    WORD logchild_count;
    union {
        struct {
            short left;
            short top;
            short right;
            short bottom;
        } margin;
        ULONGLONG margin_value;
    };    
    DLGITEMPOSDEF* logical_children;
};

/// <summary>
/// A rectangle that has 64bits size.
/// </summary>
union ABSRECT {
    struct {
        short x;
        short y;
        unsigned short width;
        unsigned short height;
    };
    ULONGLONG value;
};

#define DECL_DLGITEMPOS(id, idAfter, width, height, align, left, top, right, bottom) \
    { id, idAfter, width, height, align, 0, { left, top, right, bottom }, NULL }

#define DECL_DLGITEMPOS_CHILD(id, idAfter, width, height, align, left, top, right, bottom, children) \
    { id, idAfter, width, height, align, (sizeof(children)/sizeof(DLGITEMPOSDEF)), { left, top, right, bottom }, children }

/// <summary>
/// Represent the preference dialog.
/// </summary>
/// <seealso cref="CDialogImpl{CEyereachPrefUI, CWindow}" />
/// <seealso cref="FakeIUnknownImpl{IEyereachPrefUI}" />
class CEyereachPrefUI :
    public CDialogImpl<CEyereachPrefUI, CWindow>,
    public FakeIUnknownImpl<IEyereachPrefUI>
{
public:
    enum { IDD = IDD_DIALOG1, WIN_WIDTH = 600, WIN_HEIGHT = 420 };

    BEGIN_MSG_MAP(CEyereachPrefUI)
        MESSAGE_HANDLER(WM_CLOSE, OnWmClose)
        MESSAGE_HANDLER(WM_SIZE, OnWmSize)
        MESSAGE_HANDLER(WM_KEYDOWN, OnWmKeyUp)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnWmGetDlgCode)

        MESSAGE_HANDLER(WM_INITDIALOG, OnWmInitDialog)
        MESSAGE_HANDLER(WM_DRAWITEM, OnWmDrawItem)
        
        NOTIFY_ID_HANDLER(IDC_TAB1, OnTab1Notify)
        NOTIFY_CODE_HANDLER(NM_RELEASEDCAPTURE, OnSliderReleaseCapture)

        COMMAND_ID_HANDLER(IDC_BUTTON1, OnIdButton1)
        COMMAND_ID_HANDLER(IDOK, OnIdOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnIdCancel)
        COMMAND_ID_HANDLER(IDC_CHECK4, OnIdCheck)
        COMMAND_ID_HANDLER(IDC_BUTTON3, OnIdButton3)
        COMMAND_ID_HANDLER(IDC_BUTTON4, OnIdButton3)

        
    END_MSG_MAP()

    CEyereachPrefUI(void);
    ~CEyereachPrefUI(void);

    // IEyereachPrefUI
    virtual HRESULT STDMETHODCALLTYPE Initialize(IEyereachControl *pControl);

public:    
    LRESULT OnWmInitDialog(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled);

    LRESULT OnWmGetDlgCode(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled) {        
        return DLGC_WANTMESSAGE;
    }

    LRESULT OnWmClose(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled) {
        _doQuitOrApply(FALSE);
        return 0;
    }
    
    LRESULT OnWmSize(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled) {
        _updateLayout();
        return 0;
    }

    LRESULT OnWmDrawItem(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled);
    LRESULT OnWmKeyUp(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled);

    LRESULT OnTab1Notify(int id, LPNMHDR lp, BOOL& bHandled);
    LRESULT OnSliderReleaseCapture(int id, LPNMHDR lp, BOOL& bHandled);

    LRESULT OnIdOk(WORD code, WORD id, HWND hwnd, BOOL& bHandled) {        
        _doQuitOrApply(TRUE);
        return 0;
    }

    LRESULT OnIdCancel(WORD code, WORD id, HWND hwnd, BOOL& bHandled) {
        _doQuitOrApply(FALSE);
        return 0;
    }

    LRESULT OnIdButton1(WORD code, WORD id, HWND hwnd, BOOL& bHandled);
	LRESULT OnIdButton2(WORD code, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnIdCheck(WORD cod, WORD id, HWND hwnd, BOOL& bHandled);
    LRESULT OnIdButton3(WORD code, WORD id, HWND hwnd, BOOL& bHandled);

private:
    void _doQuitOrApply(BOOL apply);     

    	//virtual void OnFinalMessage(HWND) { _AtlModule.setPrefUI(NULL); }

    void _updateLayout(void);

    void* mp_Image;
    class CEyereachPrefMonitorPage* mp_MWin[2];

    static DLGITEMPOSDEF m_dlgItemPosDefs_File[];
    static DLGITEMPOSDEF m_dlgItemPosDefs_Preview[];
    static DLGITEMPOSDEF m_dlgItemPosDefs_Control[];
    static DLGITEMPOSDEF m_dlgItemPosDefs[];
public:
    LRESULT OnBnClickedButton2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

/*
===============================================
*/

#endif
