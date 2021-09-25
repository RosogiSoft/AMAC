#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>

#include "DeviceNotificationSender.h"

class EndpointCallback : public IAudioEndpointVolumeCallback
{
    LONG _cRef;

public:
    EndpointCallback();

    ~EndpointCallback();

    // IUnknown methods -- AddRef, Release, and QueryInterface

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface);

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

    void Set_DeviceNotificationSender(DeviceNotificationSender* dev_send_upd);

private:
    DeviceNotificationSender * dev_send_upd = NULL;

};

