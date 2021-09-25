#include "DeviceVolController.h"
#include <stdio.h>

const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

DeviceVolController::DeviceVolController() {
	return;
}

DeviceVolController::DeviceVolController(IMMDevice* pDevice) {
	HRESULT hresult = S_OK;
	hresult = pDevice->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void**)&dev_vol_ctrl);
	if (hresult != S_OK) printf("Error in DeviceVolController::DeviceVolController()!\n");
}

void DeviceVolController::RegisterNotifications(EndpointCallback* dev_vol_events) {
	HRESULT hresult = S_OK;
	hresult = dev_vol_ctrl->RegisterControlChangeNotify(dev_vol_events); 
	if (hresult != S_OK) printf("Error in DeviceVolController::RegisterNotifications()!\n");
	else this->dev_vol_events = dev_vol_events;
}

void DeviceVolController::SetVol(float vol) {
	HRESULT hresult = S_OK;
	hresult = dev_vol_ctrl->SetMasterVolumeLevelScalar(vol, NULL);
	if (hresult != S_OK) printf("Error in DeviceVolController::SetVol()!\n");
}
void DeviceVolController::SetMute(bool mute) {
	HRESULT hresult = S_OK;
	hresult = dev_vol_ctrl->SetMute(mute, NULL);
	if (hresult != S_OK) printf("Error in DeviceVolController::SetMute()!\n");
}

DeviceVolController::~DeviceVolController() {
	HRESULT hresult = S_OK;
	hresult = dev_vol_ctrl->UnregisterControlChangeNotify(dev_vol_events);
	if (hresult != S_OK) printf("Error in DeviceVolController::RegisterNotifications()!\n");
}