#pragma once
#include "DeviceUtility.h"
#include "IMMUtility.h"

#include <Functiondiscoverykeys_devpkey.h>
#include <endpointvolume.h>

#include <iostream>

#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { goto Exit;}
#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL;}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
//const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

void GetMMDevicesCollection(IMMDeviceCollection*& pDeviceCollection) {
	HRESULT hresult = S_OK;
	IMMDeviceEnumerator* pDeviceEnumerator = NULL;

	hresult = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pDeviceEnumerator);
	EXIT_ON_ERROR(hresult)
	hresult = pDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDeviceCollection);
	EXIT_ON_ERROR(hresult)

	SAFE_RELEASE(pDeviceEnumerator)
	return;

Exit:
	printf("Error in GetMMDevicesCollection()!\n");
	SAFE_RELEASE(pDeviceEnumerator)
}

DeviceIDAndName GetDeviceIDandName(IMMDevice* pDevice) {
	HRESULT hresult = S_OK;
	IPropertyStore* pDeviceProps = NULL;
	LPWSTR lpwdID = NULL;
	std::string fname, dID;

	// Get the endpoint ID string.
	hresult = pDevice->GetId(&lpwdID); EXIT_ON_ERROR(hresult)

	// Initialize container for property value.
	hresult = pDevice->OpenPropertyStore(STGM_READ, &pDeviceProps); EXIT_ON_ERROR(hresult)
	PROPVARIANT varName;
	PropVariantInit(&varName);

	// Get the endpoint's friendly-name property.
	hresult = pDeviceProps->GetValue(PKEY_Device_FriendlyName, &varName); EXIT_ON_ERROR(hresult)

	fname = LPWSTRtoString(varName.pwszVal);
	dID = LPWSTRtoString(lpwdID);

	CoTaskMemFree(lpwdID);
	PropVariantClear(&varName);
	SAFE_RELEASE(pDeviceProps)
	return { fname, dID };

Exit:
	printf("Error in GetDiveceIDandFriendlyName()!\n");
	CoTaskMemFree(lpwdID);
	//PropVariantClear(&varName);
	SAFE_RELEASE(pDeviceProps)
	return { fname, dID };
}

IMMDeviceMap CreateMMDevicesMap() {
	std::map<std::string, std::string> devices;
	std::pair<std::string, std::string> singe_device;
	HRESULT hresult = S_OK;
	IMMDevice* pDevice = NULL;
	IMMDeviceCollection* pDeviceCollection = NULL;
	GetMMDevicesCollection(pDeviceCollection);

	UINT count;
	hresult = pDeviceCollection->GetCount(&count); EXIT_ON_ERROR(hresult)

	if (count == 0) {
		std::cout << "No devices found" << std::endl;
		return devices;
	}

	for (ULONG i = 0; i < count; i++) {
		hresult = pDeviceCollection->Item(i, &pDevice); EXIT_ON_ERROR(hresult)
		devices.insert(GetDeviceIDandName(pDevice));
	}

	SAFE_RELEASE(pDevice)
	SAFE_RELEASE(pDeviceCollection)

	return devices;

Exit:
	printf("Error in GetMMDevicesMap()!\n");
	SAFE_RELEASE(pDevice)
	SAFE_RELEASE(pDeviceCollection)
	return devices;
}

void GetDeviceByID(const std::string& dID, IMMDevice*& pDevice) {
	HRESULT hresult = S_OK;
	IMMDeviceEnumerator* pDeviceEnumerator = NULL;
	LPCWSTR lpcwdID = NULL;
	std::wstring wstr = std::wstring(dID.begin(), dID.end());
	lpcwdID = wstr.c_str();
	hresult = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pDeviceEnumerator);
	EXIT_ON_ERROR(hresult)
	hresult = pDeviceEnumerator->GetDevice(lpcwdID, &pDevice);
	EXIT_ON_ERROR(hresult)

	SAFE_RELEASE(pDeviceEnumerator)
	lpcwdID = NULL;
	return;

Exit:
	printf("Error in GetDeviceByID()!\n");
	SAFE_RELEASE(pDeviceEnumerator)
	lpcwdID = NULL;
}

void SetDeviceVolScaled(const std::string& dID, float vol) {
	HRESULT hresult = S_OK;

	IMMDevice* pDevice = NULL;
	GetDeviceByID(dID, pDevice);

	IAudioEndpointVolume* pVolControl = NULL;
	hresult = pDevice->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void**)&pVolControl); EXIT_ON_ERROR(hresult)
	hresult = pVolControl->SetMasterVolumeLevelScalar(vol, NULL); EXIT_ON_ERROR(hresult)

	SAFE_RELEASE(pVolControl)
	SAFE_RELEASE(pDevice)
	return;

Exit:
	printf("Error in ChangeDeviceVol()!\n");
	SAFE_RELEASE(pVolControl)
	SAFE_RELEASE(pDevice)
}
