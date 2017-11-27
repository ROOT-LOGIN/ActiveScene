#include "stdafx.h"
#include "common.h"

#include "Eyereach.h"

#include "dxsupport.h"

#include "views.h"
#include "player.h"

class CEVRVideoRenderer : public CVideoRenderer
{
private:
    CComPtr<IBaseFilter>            m_EVR;
    CComPtr<IMFVideoDisplayControl> m_VideoDisplay;

public:
    CEVRVideoRenderer(void);
    ~CEVRVideoRenderer(void);

    // The HasVideo method returns TRUE if the video renderer has been created. 
    virtual bool getHasVido() const;
    // The AddToGraph method adds the video renderer to the filter graph. 
    virtual HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
    // The FinalizeGraph method completes the graph-building step. 
    virtual HRESULT FinalizeGraph(IGraphBuilder *pGraph);
    // The UpdateVideoWindow method updates the video destination rectangle. 
    virtual HRESULT UpdateVideoWindow(HWND hwnd, LPCRECT prc);
    // The Repaint method redraws the current video frame. 
    virtual HRESULT Repaint(HWND hwnd, HDC hdc);
    // The DisplayModeChanged method handles display-mode changes.
    virtual HRESULT DisplayModeChanged(void);

    virtual HRESULT UpdateClient(BYTE* pBits) = 0;
};

class CVMR7VideoRenderer : public CVideoRenderer
{
private:
    CComPtr<IVMRWindowlessControl> m_Windowless;

public:
    CVMR7VideoRenderer(void);
    ~CVMR7VideoRenderer(void);

    // The HasVideo method returns TRUE if the video renderer has been created. 
    virtual bool getHasVido() const;
    // The AddToGraph method adds the video renderer to the filter graph. 
    virtual HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd);
    // The FinalizeGraph method completes the graph-building step. 
    virtual HRESULT FinalizeGraph(IGraphBuilder *pGraph);
    // The UpdateVideoWindow method updates the video destination rectangle. 
    virtual HRESULT UpdateVideoWindow(HWND hwnd, LPCRECT prc);
    // The Repaint method redraws the current video frame. 
    virtual HRESULT Repaint(HWND hwnd, HDC hdc);
    // The DisplayModeChanged method handles display-mode changes.
    virtual HRESULT DisplayModeChanged(void);

    virtual HRESULT UpdateClient(BYTE* pBits) = 0;
};

/// <summary>
/// A video renderer that use vmr9
/// </summary>
/// <seealso cref="CVideoRenderer" />
class CVMR9VideoRenderer : public CVideoRenderer
{
private:
    CComPtr<IVMRWindowlessControl9> m_Windowless;
	CComPtr<IVMRImageCompositor9> m_Compositor;
    CComObject<CVMR9SufaceAllocator>* mp_Alloctator;

public:
    CVMR9VideoRenderer(void) {
        mp_Alloctator = NULL;
    }

    ~CVMR9VideoRenderer(void) 
#ifdef DEBUG
        throw(...)
#endif
    {
        if(mp_Alloctator) {
            try{
                mp_Alloctator->Release();
            }
            catch(...) {
#ifdef DEBUG
                throw;
#endif
            }
            mp_Alloctator = NULL;
        }
    }

    // The HasVideo method returns TRUE if the video renderer has been created. 
    virtual bool getHasVido() const { return m_Windowless != NULL; }

    // The AddToGraph method adds the video renderer to the filter graph. 
    virtual HRESULT AddToGraph(IGraphBuilder *pGraph, HWND hwnd) {
        CComPtr<IBaseFilter> vmr9;
        HRESULT hr = AddFilterByCLSID(pGraph, CLSID_VideoMixingRenderer9, &vmr9, L"VMR9");
        if(FAILED(hr)) return hr;

		CComQIPtr<IVMRFilterConfig9> config = vmr9;        
        if(!config) return E_FAIL;
		
		hr = config->SetNumberOfStreams(MAX_VIDEO_STREAMS);
		if(FAILED(hr)) return hr;

		CComQIPtr<IVMRMixerControl9> mixer = vmr9;
		if(!mixer) return E_FAIL;
		
		/*VMR9NormalizedRect rect;
		rect.left  = 0.0f;  rect.top    = 0.0f;
		rect.right = 0.5f;  rect.bottom = 0.5f;
		mixer->SetOutputRect(0, &rect);
		rect.left  = 0.5f;  rect.top    = 0.5f;
		rect.right = 1.0f;  rect.bottom = 1.0f;
		mixer->SetOutputRect(1, &rect);
		rect.left  = 0.5f;  rect.top    = 0.0f;
		rect.right = 1.0f;  rect.bottom = 0.5f;
		mixer->SetOutputRect(2, &rect);
		rect.left  = 0.0f;  rect.top    = 0.5f;
		rect.right = 0.5f;  rect.bottom = 1.0f;
		mixer->SetOutputRect(3, &rect);*/

		/*
		CComObject<CVMR9Compositor>* pCompositor;
		CComObject<CVMR9Compositor>::CreateInstance(&pCompositor);
		pCompositor->QueryInterface(IID_IVMRImageCompositor9, (void**)&m_Compositor);
		hr = config->SetImageCompositor(m_Compositor);
		if(FAILED(hr)) return hr;
		*/
        return InitializeVMR9(vmr9, hwnd, &m_Windowless);
    }

    // The FinalizeGraph method completes the graph-building step. 
    virtual HRESULT FinalizeGraph(IGraphBuilder *pGraph) {
        if(!m_Windowless) return S_OK;

        CComQIPtr<IBaseFilter> filter = m_Windowless;
        if(!filter) return E_FAIL;

        bool B;
        RemoveUnconnectedRenderer(pGraph, filter, &B);
        if(B) {
            m_Windowless.Release();
        }
        return S_OK;
    }

    // The UpdateVideoWindow method updates the video destination rectangle. 
    virtual HRESULT UpdateVideoWindow(HWND hwnd, LPCRECT prc) {
        if(!m_Windowless) return S_OK;

        if(!prc) {
            MonitorWrap MI;
            if(MI.getMonitorCount() == 1)
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                prc = &rc;
            }
            else {                
                MI.Fill(1);
                prc = &MI.rcWork;
            }
        }
        
        return m_Windowless->SetVideoPosition(NULL, (LPRECT)prc);
    }

    // The Repaint method redraws the current video frame. 
    virtual HRESULT Repaint(HWND hwnd, HDC hdc) {
        if(m_Windowless)
            return m_Windowless->RepaintVideo(hwnd, hdc);
        return S_OK;
    }
    // The DisplayModeChanged method handles display-mode changes.
    virtual HRESULT DisplayModeChanged(void) {
        if(!m_Windowless) return S_OK;

        return m_Windowless->DisplayModeChanged();
    }

    virtual HRESULT UpdateSetting(DWORD sid, int mid, void* value) {
        if(!mp_Alloctator) return E_FAIL;
        return mp_Alloctator->UpdateSetting(sid, mid, value);
    }

    virtual HRESULT QuerySetting(DWORD sid, int mid, LPVOID* pvalue) {
        if(!mp_Alloctator) return E_FAIL;
        return mp_Alloctator->QuerySetting(sid, mid, pvalue);
    }

    /*virtual HRESULT UpdateClient(BYTE* pBits) {
        if(mp_Alloctator)
            return mp_Alloctator->DrawClientTexture(pBits);
        return S_OK;
    }
    */

private:
    HRESULT InitializeVMR9(IBaseFilter *pFilter, HWND hwnd, IVMRWindowlessControl9 **ppWindowless)
    {        
        CComQIPtr<IVMRFilterConfig9> config = pFilter;        
        if(!config) return E_FAIL;
#if VMR9_WINDOWLESS        
        HRESULT hr = config->SetRenderingMode(VMR9Mode_Windowless);
        if(FAILED(hr)) return hr;

        CComQIPtr<IVMRWindowlessControl9> wndless = pFilter;
        if(!wndless) return E_NOINTERFACE;

        hr = wndless->SetVideoClippingWindow(hwnd);
        if(FAILED(hr)) return hr;

        hr = wndless->SetAspectRatioMode(VMR9ARMode_LetterBox);
        if(FAILED(hr)) return hr;

        *ppWindowless = wndless.Detach();
#else		
		HRESULT hr = CVMR9SufaceAllocator::InitializeVMR9(pFilter, hwnd, &mp_Alloctator);
#endif
        return hr;
    }
};

/* ============================================
*/

DShowPlayer::~DShowPlayer(void) { 
    TearDownGraph(); 
}

HRESULT DShowPlayer::OpenFile(LPCWSTR pszFile) {
    HRESULT hr = InitializeGraph();
    if(FAILED(hr)) return hr;
		
	hr = CreateVideoRenderer();
    if(FAILED(hr)) return hr;

    CComPtr<IBaseFilter> source;
    hr = m_Graph->AddSourceFilter(pszFile, NULL, &source);
    if(SUCCEEDED(hr)) {
        hr = RenderStreams(source);
    }

    if(FAILED(hr)) TearDownGraph();
    else wmemcpy(g_SceneData->s_path, pszFile, 1024);
    return hr;
}
    
HRESULT DShowPlayer::Play(void) {
   if(m_state != STATE_PAUSED && m_state != STATE_STOPPED)
       return VFW_E_WRONG_STATE;

   HRESULT hr = m_Control->Run();
   if(SUCCEEDED(hr)) m_state = STATE_RUNNING;
   return hr;
}

HRESULT DShowPlayer::Pause(void) {
    if(m_state != STATE_RUNNING)
        return VFW_E_WRONG_STATE;
    
    HRESULT hr = m_Control->Pause();
    if(SUCCEEDED(hr)) m_state = STATE_PAUSED;
    return hr;
}

HRESULT DShowPlayer::Loop(void) {
	const GUID* pFormat = NULL;
	if(m_Seeking->IsFormatSupported(&TIME_FORMAT_SAMPLE) == S_OK)
	{
		pFormat = &TIME_FORMAT_SAMPLE;
	}
	else if(m_Seeking->IsFormatSupported(&TIME_FORMAT_FRAME) == S_OK)
	{
		pFormat = &TIME_FORMAT_FRAME;
	}
	else if(m_Seeking->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME) == S_OK)
	{
		pFormat = &TIME_FORMAT_MEDIA_TIME;
	}
	else
		return E_FAIL;
	
	LONGLONG lFrame,lStart = 0;
	m_Seeking->SetTimeFormat(pFormat);		
	HRESULT hr = m_Seeking->GetDuration(&lFrame);
	if(FAILED(hr)) return hr;
	hr = m_Seeking->SetPositions(&lStart, AM_SEEKING_AbsolutePositioning | AM_SEEKING_Segment,
		&lFrame, AM_SEEKING_AbsolutePositioning);
	return hr;
}

HRESULT DShowPlayer::Volume(LONG volume)
{
    if(!m_Audio) return S_FALSE;
    if (volume <= 0)
    {
        m_Audio->put_Volume(volume);
    }
    else
    {
        m_Audio->get_Volume(&volume);
        volume = volume ? 0 : -10000;
        m_Audio->put_Volume(volume);
    }
    
    return S_OK;
}

HRESULT DShowPlayer::Duration(float start, float end)
{
    if (start > end || start < 0 || end <= 0) 
        return E_INVALIDARG;

    if (!m_Seeking) return S_FALSE;

    /*    TIME_FORMAT_NONE No format.
    TIME_FORMAT_FRAME Video frames.
    TIME_FORMAT_SAMPLE Samples in the stream.
    TIME_FORMAT_FIELD Interlaced video fields.
    TIME_FORMAT_BYTE Byte offset within the stream.
    TIME_FORMAT_MEDIA_TIME Reference time (100-nanosecond units).
    */

    LONGLONG duration; // duration
    HRESULT hr = m_Seeking->GetDuration(&duration);
    if (FAILED(hr)) return hr;

    // calc new start pos
    LONGLONG lstart = start * (double)duration;
    if (start == end) // just start position
    {        
        hr = m_Seeking->SetPositions(&lstart, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
    }
    else // start and stop
    {
        // calc new stop pos
        LONGLONG lend = end * (double)duration;
        hr = m_Seeking->SetPositions(&lstart, AM_SEEKING_AbsolutePositioning, &lend, AM_SEEKING_AbsolutePositioning);
    }
    return hr;
}
/*
HRESULT DShowPlayer::UpdateClient(BYTE* pBits)
{
    if(mp_Video)
        return mp_Video->UpdateClient(pBits);
    return S_OK;
}
*/
HRESULT DShowPlayer::UpdateVideoWindow(const LPRECT prc) {
    if(mp_Video)
        return mp_Video->UpdateVideoWindow(getHwnd(), prc);
    return S_OK;
}

HRESULT DShowPlayer::Repaint(HDC hdc) {
    if(mp_Video)
        return mp_Video->Repaint(getHwnd(), hdc);
    return S_OK;
}

HRESULT DShowPlayer::DisplayModeChanged(void) {
    if(mp_Video)
        return mp_Video->DisplayModeChanged();
    return S_OK;
}

void DShowPlayer::OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2)
{
    switch(evCode) {
    case EC_COMPLETE:
		Stop(false); Loop(); Play();
        break;
    case EC_USERABORT:
		Stop(false);
        break;
    case EC_ERRORABORT:
        MessageBeep(MB_ICONEXCLAMATION);
        Stop(true);
        break;
    }
}
/*
HRESULT DShowPlayer::HandleGraphEvent(GraphEventFN pfnOnGraphEvent) {
    if(!m_Event) return E_UNEXPECTED;

    HRESULT hr = S_OK;
    long evCode = 0;
    LONG_PTR param1 = 0, param2 = 0;
    while(SUCCEEDED(m_Event->GetEvent(&evCode, &param1, &param2, 0))) {
        pfnOnGraphEvent(getHwnd(), evCode, param1, param2);

        hr = m_Event->FreeEventParams(evCode, param1, param2);
        if(FAILED(hr)) break;
    }
    return hr;
}
*/
HRESULT DShowPlayer::HandleGraphEvent(void) {
    if(!m_Event) return E_UNEXPECTED;

    HRESULT hr = S_OK;
    long evCode = 0;
    LONG_PTR param1 = 0, param2 = 0;
    while(SUCCEEDED(m_Event->GetEvent(&evCode, &param1, &param2, 0))) {
        OnGraphEvent(getHwnd(), evCode, param1, param2);

        hr = m_Event->FreeEventParams(evCode, param1, param2);
        if(FAILED(hr)) break;
    }
    return hr;
}

HRESULT DShowPlayer::UpdateSetting(DWORD sid, int mid, void* value)
{
    if(!mp_Video) return E_FAIL;
     
    if(sid == Pref_RenderFile) {
        return OpenFile((LPCWSTR)value);
    }
    return mp_Video->UpdateSetting(sid, mid, value);
}

HRESULT DShowPlayer::QuerySetting(DWORD sid, int mid, LPVOID* pvalue)
{
    if(!mp_Video) return E_FAIL;

    return mp_Video->QuerySetting(sid, mid, pvalue);
}

HRESULT DShowPlayer::InitializeGraph(void) {
    TearDownGraph();    

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_Graph);
    if(FAILED(hr)) return hr;

    hr = m_Graph->QueryInterface(IID_IMediaControl, (void**)&m_Control);
    if(FAILED(hr)) return hr;
	hr = m_Graph->QueryInterface(IID_IMediaSeeking, (void**)&m_Seeking);
    if(FAILED(hr)) return hr;
    hr = m_Graph->QueryInterface(IID_IMediaEvent, (void**)&m_Event);
    if(FAILED(hr)) return hr;

    hr = m_Event->SetNotifyWindow((OAHWND)getHwnd(), WM_GRAPH_EVENT, NULL);
    if(FAILED(hr)) return hr;

    m_state = STATE_STOPPED;
    return hr;
}

void DShowPlayer::TearDownGraph(void) {
    if(m_Event) {
        m_Event->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
    }
    
    m_Audio.Release();
    m_Control.Release();
	m_Seeking.Release();
    m_Event.Release();
    
    delete mp_Video;
    mp_Video = NULL;

    m_Graph.Release();
    
    m_state = STATE_NO_GRAPH;
}

HRESULT DShowPlayer::RenderStreams(IBaseFilter *pSource) {
    bool anyPin = false;
    CComPtr<IFilterGraph2> graph2;
    CComPtr<IEnumPins> enumpin;
    CComPtr<IBaseFilter> audio;

    HRESULT hr = m_Graph->QueryInterface(IID_IFilterGraph2, (void**)&graph2);
    if(FAILED(hr)) return hr;
   
    hr = AddFilterByCLSID(m_Graph, CLSID_DSoundRender, &audio, L"Audio");
    if(FAILED(hr)) return hr;

    hr = pSource->EnumPins(&enumpin);
    if(FAILED(hr)) return hr;

    CComPtr<IPin> pin;
    while (S_OK == enumpin->Next(1, &pin, NULL))
    {
        // Try to render this pin. 
        // It's OK if we fail some pins, if at least one pin renders.
        hr = graph2->RenderEx(pin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL);
        pin.Release();
        if(SUCCEEDED(hr)) { anyPin = true; /*break; */}        
    }

    hr = mp_Video->FinalizeGraph(m_Graph);
    if(FAILED(hr)) return hr;

    bool removed;
    hr = RemoveUnconnectedRenderer(m_Graph, audio, &removed);
    if(SUCCEEDED(hr)) {
        if(anyPin) {
            if(!removed) {
                if(SUCCEEDED(audio->QueryInterface(IID_IBasicAudio, (LPVOID*)&m_Audio)))
                    m_Audio->get_Volume(&m_volume);
            }
        }
        else {
            hr = VFW_E_CANNOT_RENDER;
        }
    }
    return hr;
}

HRESULT DShowPlayer::CreateVideoRenderer(void) {
    HRESULT hr = S_OK;
    // VMR9 only for now
    mp_Video = new CVMR9VideoRenderer();
    hr = mp_Video->AddToGraph(m_Graph, getHwnd());
    if(FAILED(hr)) {
        delete mp_Video; mp_Video = NULL;
    }
    return hr;
}

#if 0
CDShowTargetWindow::CDShowTargetWindow(void)
{
}

#pragma push_macro("SubclassWindow")

#undef SubclassWindow
CDShowTargetWindow* CDShowTargetWindow::FromHWND(DShowPlayer* player, HWND hwnd) 
{
    CDShowTargetWindow* pret = new CDShowTargetWindow();
    
    pret->SubclassWindow(hwnd);
    pret->mp_Player = player;

    return pret;
}

void CDShowTargetWindow::WithHWND(DShowPlayer* player, HWND hwnd)
{
	// LVS_EX_TRANSPARENTSHADOWTEXT | 
#define DWORD_style (LVS_EX_TRANSPARENTBKGND)
	ListView_SetExtendedListViewStyle(hwnd,
			ListView_GetExtendedListViewStyle(hwnd) & (~DWORD_style)); 
    SubclassWindow(hwnd);
    mp_Player = player;
}

void CDShowTargetWindow::GiveUpHWND( )
{
	ListView_SetExtendedListViewStyle(m_hWnd,
			ListView_GetExtendedListViewStyle(m_hWnd) | DWORD_style); 
    m_dwState |= WINSTATE_DESTROYED;
    UnsubclassWindow(FALSE);  
}

void CDShowTargetWindowParent::WithHWND(DShowPlayer* player, HWND hwnd)
{
    SubclassWindow(hwnd);
    //mp_Player = player;
}

void CDShowTargetWindowParent::GiveUpHWND( )
{
    m_dwState |= WINSTATE_DESTROYED;
    UnsubclassWindow(FALSE);  
}

#pragma pop_macro("SubclassWindow")

void CDShowTargetWindow::OnFinalMessage(HWND hwnd)
{
    if(_AtlModule.GetLockCount())
        _AtlModule.Unlock();
     
    delete mp_Player;
}

IMPL_MSG_HANDLER(CDShowTargetWindow, OnGraphEvent)
{
    bHandled = TRUE;
    //mp_Player->HandleGraphEvent(DShowPlayer::OnGraphEvent);
    mp_Player->HandleGraphEvent();
    return 0;
}    

IMPL_MSG_HANDLER(CDShowTargetWindow, OnDisplayChange)
{
    bHandled = TRUE;
    mp_Player->DisplayModeChanged();
    return 0;
}

IMPL_MSG_HANDLER(CDShowTargetWindow, OnEraseBkgnd)
{
    return 0;
}

IMPL_MSG_HANDLER(CDShowTargetWindow, OnPaint)
{
	if (!mp_Player || mp_Player->getState() == STATE_NO_GRAPH)
    {
		// we are not playing, let do nothing.
		bHandled = false;
		return 0;
    }

    bHandled = TRUE;
    PAINTSTRUCT ps;
        
    CRect crc;
    GetClientRect(crc);
    
    HDC hdc = BeginPaint(&ps);
    m_Bitmap.Create(hdc, crc.Width(), crc.Height());
    HDC hdcMem = CreateCompatibleDC(hdc);
    HGDIOBJ holdbmp = SelectObject(hdcMem, m_Bitmap.hBmp);    
   /* if(mp_Player->m_TargetWinParent.m_hWnd) {
        m_pfnSuperWindowProc(mp_Player->m_TargetWinParent.m_hWnd, WM_PRINTCLIENT, (WPARAM)hdcMem, (LPARAM)PRF_CLIENT);
    }*/
    SelectObject(hdcMem, holdbmp);
    EndPaint(&ps);
/*
	BITMAPINFO bi = {0};    
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);    
    bi.bmiHeader.biWidth = crc.Width();
    bi.bmiHeader.biHeight = crc.Height();
    bi.bmiHeader.biPlanes = 1;    
    bi.bmiHeader.biBitCount = 32;    
    bi.bmiHeader.biCompression = BI_RGB;    
    bi.bmiHeader.biSizeImage = 0;  
    bi.bmiHeader.biXPelsPerMeter = 0;    
    bi.bmiHeader.biYPelsPerMeter = 0;    
    bi.bmiHeader.biClrUsed = 0;    
    bi.bmiHeader.biClrImportant = 0;	
    GetDIBits(hdcMem, m_Bitmap.hBmp, 0, crc.Height(), NULL, &bi, DIB_RGB_COLORS);
    BYTE* pv = new BYTE[bi.bmiHeader.biSizeImage];
    memset(pv, 0, bi.bmiHeader.biSizeImage);
    
    int l = GetDIBits(hdcMem, m_Bitmap.hBmp, 0, crc.Height(), pv, &bi, DIB_RGB_COLORS);
	if(l != crc.Height())
		DebugBreak();
    
    HANDLE hFile = CreateFile(L"A:\\captureqwsx.bmp", GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile && hFile != INVALID_HANDLE_VALUE) {
        // Add the size of the headers to the size of the bitmap to get the total file size
        DWORD dwSizeofDIB = bi.bmiHeader.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  
        BITMAPFILEHEADER   bmfHeader;
        //Offset to where the actual bitmap bits start.
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);     
        //Size of the file 
        bmfHeader.bfSize = dwSizeofDIB;
        //bfType must always be BM for Bitmaps
        bmfHeader.bfType = 0x4D42; //BM    
        DWORD dwBytesWritten = 0;
        WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
        WriteFile(hFile, (LPSTR)pv, bi.bmiHeader.biSizeImage, &dwBytesWritten, NULL);

        //Close the handle for the file that was created
        CloseHandle(hFile);
    }    
	delete pv;
*/

	BYTE* pv = m_Bitmap.pBits;
	int alp = 0xFF;
	DWORD dw = 0;
	DWORD dw_ = 0;
	for(int i=0; i<crc.Width()*crc.Height()*4; i+=4)
	{
		if(pv[i+3] != 0) { dw++;
			alp = min(pv[i+3], alp);
		}		
	}
	for(int i=0; i<crc.Width()*crc.Height()*4; i+=4)
	{
		if(pv[i] > 0x01 && pv[i+1] > 0x1 && pv[i+2] >= 0x1) {
			pv[i+3] += 0xFF;
		}
	}

	mp_Player->UpdateClient(m_Bitmap.pBits);

    return 0;
}

IMPL_MSG_HANDLER(CDShowTargetWindow, OnSize)
{
    bHandled = TRUE;
    RECT rc;
    GetClientRect(&rc);
    mp_Player->UpdateVideoWindow(&rc);
    return 0;
}

void CDShowTargetWindow::OpenFile(HWND hwnd) {
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    WCHAR szFileName[MAX_PATH];
    szFileName[0] = L'\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = _AtlModule.m_hInst;
    ofn.lpstrFilter = L"All (*.*)\0*.*\0\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
    HRESULT hr;
    if (GetOpenFileName(&ofn))
    {
        hr = mp_Player->OpenFile(szFileName);            
        if (SUCCEEDED(hr))
        {
            ::InvalidateRect(hwnd, NULL, FALSE);
            mp_Player->Play();
            // If this file has a video stream, notify the video renderer 
            // about the size of the destination rectangle.
            /*RECT rc;
            ::GetClientRect(hwnd, &rc);*/
            mp_Player->UpdateVideoWindow(NULL);
        }
        else
        {
            delete mp_Player;
            mp_Player = NULL;
            ::MessageBox(hwnd, TEXT("Cannot open this file."), L"Error", MB_ICONERROR | MB_OK);
        }
    }
}

IMPL_MSG_HANDLER(CDShowTargetWindow, OnKeyUp)
{
    if(wp != VK_ESCAPE) return 0;
    bHandled = TRUE;
    OpenFile(m_hWnd);
    return 0;
}
#endif
