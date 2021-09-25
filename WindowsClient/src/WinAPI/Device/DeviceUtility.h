#pragma once
#include <string>
#include <map>
#include <mmdeviceapi.h>

typedef std::pair<std::string, std::string> DeviceIDAndName;
typedef std::map<std::string, std::string> IMMDeviceMap;

void GetMMDevicesCollection(IMMDeviceCollection*& pDeviceCollection);
DeviceIDAndName GetDeviceIDandName(IMMDevice* pDevice);
IMMDeviceMap CreateMMDevicesMap();
void GetDeviceByID(const std::string& dID, IMMDevice*& pDevice);
void SetDeviceVolScaled(const std::string& dID, float vol);

