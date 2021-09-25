#include "EndpointCallback.h"


EndpointCallback::EndpointCallback() {
    _cRef = 1;
}

EndpointCallback::~EndpointCallback() {
}

ULONG STDMETHODCALLTYPE EndpointCallback::AddRef(){
    return InterlockedIncrement(&_cRef);
}

ULONG STDMETHODCALLTYPE EndpointCallback::Release(){
    ULONG ulRef = InterlockedDecrement(&_cRef);
    if (0 == ulRef){
        delete this;
    }
    return ulRef;
}

HRESULT STDMETHODCALLTYPE EndpointCallback::QueryInterface(REFIID riid, VOID** ppvInterface){
    if (IID_IUnknown == riid){
        AddRef();
        *ppvInterface = (IUnknown*)this;
    } else if (__uuidof(IAudioEndpointVolumeCallback) == riid){
        AddRef();
        *ppvInterface = (IAudioEndpointVolumeCallback*)this;
    } else {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

// Callback method for endpoint-volume-change notifications.

HRESULT STDMETHODCALLTYPE EndpointCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify){
    if (pNotify == NULL){
        return E_INVALIDARG;
    } else {
        dev_send_upd->VolChanged(pNotify->fMasterVolume);
        dev_send_upd->MuteChanged(pNotify->bMuted);
    }
    return S_OK;
}

void EndpointCallback::Set_DeviceNotificationSender(DeviceNotificationSender* dev_send_upd) {
    this->dev_send_upd = dev_send_upd;
}