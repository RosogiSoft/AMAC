#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include "EndpointCallback.h"

class DeviceVolController
{
public:
	DeviceVolController();
	DeviceVolController(IMMDevice* pDevice);
	void RegisterNotifications(EndpointCallback* dev_vol_events);
	void SetVol(float vol);
	void SetMute(bool mute);

	~DeviceVolController();

private:

	IAudioEndpointVolume* dev_vol_ctrl = NULL;
	EndpointCallback* dev_vol_events = NULL;
};

