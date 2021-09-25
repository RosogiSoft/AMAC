#pragma once

#include <combaseapi.h>
#include <string>
#include <mmdeviceapi.h>

#include "DeviceUtility.h"
#include "DeviceVolController.h"
#include "DeviceNotificationSender.h"

class DeviceManager{
public:
	DeviceManager(DeviceIDAndName dIDandName, DeviceNotificationSender *dev_send_upd);
	IMMDevice* Get_pDevice();
	DWORD GetDeviceState();
	DeviceVolController* Get_DeviceVolController();

private:
	std::string dID;
	std::string name;
	DWORD state;
	DeviceVolController dev_vol_ctrl;
	EndpointCallback dev_vol_events;
	IMMDevice* pDevice = NULL;
};

