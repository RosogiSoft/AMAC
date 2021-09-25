#include "DeviceManager.h"

DeviceManager::DeviceManager(DeviceIDAndName dIDandName, DeviceNotificationSender *dev_send_upd){
	dID = dIDandName.first;
	name = dIDandName.second;
	GetDeviceByID(dID, pDevice);
	pDevice->GetState(&state);

	dev_vol_events = EndpointCallback();

	dev_vol_events.Set_DeviceNotificationSender(dev_send_upd);
	dev_vol_ctrl = DeviceVolController(pDevice);
	dev_vol_ctrl.RegisterNotifications(&dev_vol_events);
}

DeviceVolController*  DeviceManager::Get_DeviceVolController() {
	return &dev_vol_ctrl;
}

IMMDevice* DeviceManager::Get_pDevice() {
	return pDevice;
}

DWORD DeviceManager::GetDeviceState() {
	return state;
}
