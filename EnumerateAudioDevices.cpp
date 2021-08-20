#include <stdlib.h>
#include <iostream>
#include <audioendpoints.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <Winerror.h>
#include <combaseapi.h>
#include <map>
#include <vector>


#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { goto Exit;}
#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL;}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);

using namespace std;

IMMDeviceCollection* GetMMDevicesCollection(){
	HRESULT hresult = S_OK;
	IMMDeviceEnumerator* pMMDeviceEnumerator = NULL;
	IMMDeviceCollection* pMMDeviceCollection = NULL;

	hresult = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**) &pMMDeviceEnumerator);
	EXIT_ON_ERROR(hresult)

	hresult = pMMDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection);
	EXIT_ON_ERROR(hresult)

	SAFE_RELEASE(pMMDeviceEnumerator)
	return pMMDeviceCollection;

Exit:
	printf("Error in GetMMDevicesCollection()!\n");
	SAFE_RELEASE(pMMDeviceCollection)
	SAFE_RELEASE(pMMDeviceEnumerator)
	return pMMDeviceCollection;
}

pair<string, string> GetDeviceIDandFriendlyName(IMMDevice* pMMDevice) {
	HRESULT hresult = S_OK;
	IPropertyStore* pMMDeviceProps = NULL;
	LPWSTR pwszID = NULL;
	string fname, dID;

	// Get the endpoint ID string.
	hresult = pMMDevice->GetId(&pwszID);	EXIT_ON_ERROR(hresult)
	hresult = pMMDevice->OpenPropertyStore(STGM_READ, &pMMDeviceProps); EXIT_ON_ERROR(hresult)

	// Initialize container for property value.
	PROPVARIANT varName;
	PropVariantInit(&varName);

	// Get the endpoint's friendly-name property.
	hresult = pMMDeviceProps->GetValue(PKEY_Device_FriendlyName, &varName); EXIT_ON_ERROR(hresult)
	
	//Translate LPWSTR to string;
	char buffer[500];
	size_t buffer_size;
	wcstombs_s(&buffer_size, buffer, (size_t)500, varName.pwszVal, (size_t)500);
	fname = buffer;
	wcstombs_s(&buffer_size, buffer, (size_t)500, pwszID, (size_t)500);
	dID = buffer;

	CoTaskMemFree(pwszID);
	PropVariantClear(&varName);
	SAFE_RELEASE(pMMDeviceProps)
		return { dID , fname };

Exit:
	printf("Error in GetDiveceIDandFriendlyName()!\n");
	CoTaskMemFree(pwszID);
	PropVariantClear(&varName);
	SAFE_RELEASE(pMMDeviceProps)
		return { dID , fname };
}

map<string, string> GetMMDevicesMap() {
	map<string, string> devices;
	pair<string, string> temp;
	HRESULT hresult = S_OK;
	IMMDevice* pMMDevice = NULL;
	IMMDeviceCollection* pMMDeviceCollection = GetMMDevicesCollection();

	UINT count;
	hresult = pMMDeviceCollection->GetCount(&count); EXIT_ON_ERROR(hresult)
	
	if (count == 0) {
		cout << "No devices found" << endl;
		return devices;
	}

	for (ULONG i = 0; i < count; i++) {
		hresult = pMMDeviceCollection->Item(i, &pMMDevice); EXIT_ON_ERROR(hresult)
		devices.insert(GetDeviceIDandFriendlyName(pMMDevice));
	}

	SAFE_RELEASE(pMMDevice)
	SAFE_RELEASE(pMMDeviceCollection)

	return devices;

	Exit:
	printf("Error in GetMMDevicesMap()!\n");
	SAFE_RELEASE(pMMDevice)
	SAFE_RELEASE(pMMDeviceCollection)
	return devices;
}

void PrintDevicesMap(const map<string, string>& devices) {
	int counter = 0;
	for (auto i : devices) {
		counter++;
		cout << "#" << counter << " - Friendly Name: " << i.second << "; id = " << i.first << endl;
	}
}

void ChangeDeviceVol(const string& dID, float vol) {
	HRESULT hresult = S_OK;
	IMMDeviceEnumerator* pMMDeviceEnumerator = NULL;
	IMMDeviceCollection* pMMDeviceCollection = NULL;
	IMMDevice* pMMDevice = NULL;
	IAudioEndpointVolume* pVolControl = NULL;
	wstring temp = wstring(dID.begin(), dID.end());
	LPCWSTR lpwcwdID = temp.c_str();
	
	hresult = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pMMDeviceEnumerator);
	EXIT_ON_ERROR(hresult)	

	hresult = pMMDeviceEnumerator->GetDevice(lpwcwdID, &pMMDevice);
	EXIT_ON_ERROR(hresult)

	hresult = pMMDevice->Activate(IID_IAudioEndpointVolume, CLSCTX_ALL, NULL, (void**)&pVolControl);
	EXIT_ON_ERROR(hresult)

	hresult = pVolControl->SetMasterVolumeLevelScalar(vol, NULL);
	EXIT_ON_ERROR(hresult)

	SAFE_RELEASE(pVolControl)
	SAFE_RELEASE(pMMDevice)
	SAFE_RELEASE(pMMDeviceEnumerator)
	return;

Exit:
	printf("Error in ChangeDeviceVol()!\n");
	SAFE_RELEASE(pVolControl)
	SAFE_RELEASE(pMMDevice)
	SAFE_RELEASE(pMMDeviceEnumerator)
	return;
}

int main()
{
	setlocale(LC_CTYPE, "rus");

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	map<string, string> devices = GetMMDevicesMap();
	PrintDevicesMap(devices);

	//For console usability will use vector<pair<LPWSTR, string>>, instead of map so I can adress by count
	vector<pair<string, string>> devices_console;
	for (auto i : devices) devices_console.push_back(i);

	int i = 0;
	float vol = 0;
	cout << "Choose endpoint device: ";
	cin >> i;
	cout << "Input volume value 0.0 - 1.0: ";
	cin >> vol;

	ChangeDeviceVol(devices_console[i-1].first, vol);
	
	return 0;
}
