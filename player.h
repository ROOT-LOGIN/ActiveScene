#ifndef ___PLAYER_H_INCLUDE___
#define ___PLAYER_H_INCLUDE___

// GraphEventFN is a pointer to a callback function 
// for handling filter graph events. The application 
// implements this callback function. 
typedef void (CALLBACK* GraphEventFN)(HWND, long, LONG_PTR, LONG_PTR);

/// <summary>
/// The player
/// </summary>
class DShowPlayer
{
public:
    DShowPlayer(HWND hwndDefView, HWND hwndWorker);
    ~DShowPlayer(void);

    PlaybackState getState() const { return m_state; }

    HRESULT OpenFile(LPCWSTR pszFile);
    
    HRESULT Play(void);
    HRESULT Pause(void);
    HRESULT Stop(bool isFinal);
	HRESULT Loop(void);
    HRESULT Volume(LONG volume);
    HRESULT Duration(float start, float end);

    bool getHasVideo(void) const { return mp_Video && mp_Video->getHasVido(); }

    //HRESULT UpdateClient(BYTE* pBits);
    HRESULT UpdateVideoWindow(const LPRECT prc);
    HRESULT Repaint(HDC hdc);
    HRESULT DisplayModeChanged(void);
    
    HRESULT HandleGraphEvent(GraphEventFN pfnOnGraphEvent);
    HRESULT HandleGraphEvent(void);
    
    HRESULT UpdateSetting(DWORD sid, int mid, void* value);
    HRESULT QuerySetting(DWORD sid, int mid, LPVOID* pvalue);

private:
    //static void CALLBACK OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2);
    void OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2);

    HRESULT InitializeGraph(void);

    void TearDownGraph(void);

    HRESULT CreateVideoRenderer(void);        

    HRESULT RenderStreams(IBaseFilter *pSource);

    HWND getHwnd(void) const { 
        return m_hwndWorkerW;
	}

    static LRESULT CALLBACK WorkerGetMsgProc(int code, WPARAM wp, LPARAM lp);

private:
    PlaybackState m_state;
    CComPtr<IGraphBuilder> m_Graph;
    CComPtr<IMediaControl> m_Control;
	CComPtr<IMediaSeeking> m_Seeking;
    CComPtr<IMediaEventEx> m_Event;
    CComPtr<IBasicAudio> m_Audio;
    LONG m_volume;

    // The mp_Video member variable provides a wrapper for 
    // the various DirectShow video renderers. 
    CVideoRenderer*  mp_Video;

    HHOOK m_hhookWorker;

public:
    SUBCLASS_WINDOW m_ListView;
    SUBCLASS_WINDOW m_DefView;
    HWND m_hwndWorkerW;
};

#endif
