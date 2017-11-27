#include "stdafx.h"
#include "common.h"

HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, REFGUID clsid, IBaseFilter **ppF, LPCWSTR wszName)
{
    *ppF = 0;
    CComPtr<IBaseFilter> filter;
    HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&filter);
    if (FAILED(hr)) return hr;
    
    hr = pGraph->AddFilter(filter, wszName);
    if (FAILED(hr)) return hr;

    *ppF = filter.Detach();
    return hr;
}

HRESULT IsPinConnected(IPin *pPin, bool *pResult)
{
    CComPtr<IPin> tmp_pin;
    HRESULT hr = pPin->ConnectedTo(&tmp_pin);
    if (SUCCEEDED(hr))
    {
        *pResult = true;
    }
    else if (hr == VFW_E_NOT_CONNECTED)
    {
        // The pin is not connected. This is not an error for our purposes.
        *pResult = false;
        hr = S_OK;
    }
    return hr;
}

HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, bool *pResult)
{
    PIN_DIRECTION pinDir;
    HRESULT hr = pPin->QueryDirection(&pinDir);
    if (SUCCEEDED(hr))
    {
        *pResult = (pinDir == dir);
    }
    return hr;
}

HRESULT MatchPin(IPin *pPin, PIN_DIRECTION direction, bool bShouldBeConnected, bool *pResult)
{
    bool bMatch = FALSE;
    bool bIsConnected = FALSE;

    HRESULT hr = IsPinConnected(pPin, &bIsConnected);
    if (SUCCEEDED(hr))
    {
        if (bIsConnected == bShouldBeConnected)
        {
            hr = IsPinDirection(pPin, direction, &bMatch);
        }
    }

    if (SUCCEEDED(hr))
    {
        *pResult = bMatch;
    }
    return hr;
}

HRESULT FindFirstPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, bool Connected, IPin **ppPin)
{
    CComPtr<IEnumPins> enumpin;
    CComPtr<IPin> pin;
    bool bFound = FALSE;
    *ppPin = NULL;

    HRESULT hr = pFilter->EnumPins(&enumpin);
    if (FAILED(hr)) return hr;

    while (S_OK == enumpin->Next(1, &pin, NULL))
    {
        hr = MatchPin(pin, PinDir, Connected, &bFound);
        if (FAILED(hr)) break;
        if (bFound)
        {
            *ppPin = pin.Detach();
            break;
        }
        pin.Release();
    }

    if (!bFound)
    {
        hr = VFW_E_NOT_FOUND;
    }
    return hr;
}

HRESULT RemoveUnconnectedRenderer(IGraphBuilder *pGraph, IBaseFilter *pRenderer, bool *pbRemoved)
{
    CComPtr<IPin> pin;
    *pbRemoved = false;
    // Look for a connected input pin on the renderer.
    HRESULT hr = FindFirstPin(pRenderer, PINDIR_INPUT, TRUE, &pin);
    // If this function succeeds, the renderer is connected, so we don't remove it.
    // If it fails, it means the renderer is not connected to anything, so
    // we remove it.
    if (FAILED(hr))
    {
        hr = pGraph->RemoveFilter(pRenderer);
        *pbRemoved = true;
    }
    return hr;
}

HRESULT ConnectFilter(IGraphBuilder *pGraph, PIN_DIRECTION direction, IPin *pOut, IBaseFilter *pDest)
{
    CComPtr<IPin> pIn;
    // Find an input pin on the downstream filter.
    HRESULT hr = FindFirstPin(pDest, direction, FALSE, &pIn);
    if (SUCCEEDED(hr))
    {
        // Try to connect them.
        hr = pGraph->Connect(pOut, pIn);
    }
    return hr;
}

HRESULT ConnectFilter(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest)
{
    CComPtr<IPin> pOut;
    // Find an output pin on the first filter.
    HRESULT hr = FindFirstPin(pSrc, PINDIR_OUTPUT, FALSE, &pOut);
    if (FAILED(hr)) return hr;
    
    hr = ConnectOutputFilter(pGraph, pOut, pDest);
    return hr;
}

HRESULT FindFilterInterface(IGraphBuilder *pGraph, REFGUID iid, void **ppUnk)
{
    if (!pGraph || !ppUnk) return E_POINTER;
    
    CComPtr<IEnumFilters> enumflt;
    CComPtr<IBaseFilter> pF;
    HRESULT hr = pGraph->EnumFilters(&enumflt);
    if (FAILED(hr)) return hr;

    // Query every filter for the interface.
    while (S_OK == enumflt->Next(1, &pF, 0))
    {
        if (SUCCEEDED(pF->QueryInterface(iid, ppUnk)))
        {
            return S_OK;
        }
        pF.Release();
    }
    return E_NOINTERFACE;
}

HRESULT FindPinInterface(IBaseFilter *pFilter, REFGUID iid, void **ppUnk)
{
    if (!pFilter || !ppUnk) return E_POINTER;
    
    CComPtr<IEnumPins> enumpin;
    HRESULT hr = pFilter->EnumPins(&enumpin);
    if (FAILED(hr)) return hr;

    // Query every pin for the interface.
    CComPtr<IPin> pin;
    while (S_OK == enumpin->Next(1, &pin, 0))
    {
        if(SUCCEEDED(pin->QueryInterface(iid, ppUnk)))
        {
            return S_OK;
        }
        pin.Release();
    }
    return E_NOINTERFACE;
}

HRESULT FindInterfaceAnywhere(IGraphBuilder *pGraph, REFGUID iid, void **ppUnk)
{
    if (!pGraph || !ppUnk) return E_POINTER;
    
    CComPtr<IEnumFilters> enumflt;
    if (FAILED(pGraph->EnumFilters(&enumflt)))
    {
        return E_FAIL;
    }
    // Loop through every filter in the graph.
    CComPtr<IBaseFilter> pF;
    while (S_OK == enumflt->Next(1, &pF, 0))
    {
        if (SUCCEEDED(pF->QueryInterface(iid, ppUnk)))
        {
            return S_OK;
        }
        if(SUCCEEDED(FindPinInterface(pF, iid, ppUnk)))
        {
            return S_OK;
        }
        pF.Release();
    }
    return E_FAIL;
}

HRESULT GetNextFilter(IBaseFilter *pFilter, PIN_DIRECTION Dir, IBaseFilter **ppNext)
{
    if (!pFilter || !ppNext) return E_POINTER;

    CComPtr<IEnumPins> enumpin;
    CComPtr<IPin> pin;
    HRESULT hr = pFilter->EnumPins(&enumpin);
    if (FAILED(hr)) return hr;
    while (S_OK == enumpin->Next(1, &pin, 0))
    {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pin->QueryDirection(&ThisPinDir);
        if (FAILED(hr))
        {
            // Something strange happened.
            return E_UNEXPECTED;
        }
        if (ThisPinDir == Dir)
        {
            // Check if the pin is connected to another pin.
            CComPtr<IPin> nextpin;
            hr = pin->ConnectedTo(&nextpin);
            if (SUCCEEDED(hr))
            {
                // Get the filter that owns that pin.
                PIN_INFO PinInfo;
                hr = nextpin->QueryPinInfo(&PinInfo);
                if (FAILED(hr) || (PinInfo.pFilter == NULL))
                {
                    // Something strange happened.
                    return E_UNEXPECTED;
                }
                // This is the filter we're looking for.
                *ppNext = PinInfo.pFilter; // Client must release.
                return S_OK;
            }
        }
        pin.Release();
    }

    // Did not find a matching filter.
    return E_FAIL;
}


Array<DISPLAY_DEVICE>* GetAllDisplayDevices(void) {
    Array<DISPLAY_DEVICE>* dispary = NULL;

    DISPLAY_DEVICE disp;
    disp.cb = sizeof(DISPLAY_DEVICE);    
    int I = 0;
    for( ; I<16; I++) {        
        if(!EnumDisplayDevices(NULL, I, &disp, NULL))
            break;
    }

    dispary = Array<DISPLAY_DEVICE>::Create(I);
    dispary->size = I;
    for(int i=0; i<I; i++)
    {
        dispary->at(i)->cb = sizeof(DISPLAY_DEVICE);
        EnumDisplayDevices(NULL, i, dispary->at(i), EDD_GET_DEVICE_INTERFACE_NAME);
    }

    return dispary;
}
