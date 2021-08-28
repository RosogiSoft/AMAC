#include <stdlib.h>
#include <iostream>

#include <audioendpoints.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

#include <Winerror.h>
#include <psapi.h>
#include <tchar.h>
#include <atlconv.h>

#include <combaseapi.h>
#include <map>
#include <vector>


#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { goto Exit;}
#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL;}

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioEndpointVolume = __uuidof(IAudioEndpointVolume);
const IID IID_IAudioSessionManager = __uuidof(IAudioSessionManager);
const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);

using namespace std;

//UTILITY FUNCTIONS
void PrintFullMap(const map<string, string>& map) {
	int counter = 0;
	for (auto i : map) {
		cout << "#" << counter << " - Name: " << i.second << "; id = " << i.first << endl;
		counter++;
	}
}

void PrintFriendlyNames(const vector<string>& v) {
	int counter = 0;
	for (auto n : v) {
		cout << "#" << counter <<": " << n << endl;
		counter++;
	}
}

string LPWSTRtoString(const LPWSTR& in) {
	char buffer[500];
	size_t buffer_size;
	wcstombs_s(&buffer_size, buffer, (size_t)500, in, (size_t)500);
	return buffer;
}

void RemovePathFromName(string& s) {
	int start, end;
	start = s.find_last_of('/\\');
	end = s.length() - start - (s.length() - s.find_last_of('.') + 1);
	s = s.substr(start+1, end);
}

string GetNameFromProccesID(const DWORD& processID) {
	string ret;
	HANDLE handle;
	handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processID);
	if (handle == NULL) {
		cout << "Error, during OpenProcess(): " << GetLastError() << endl;
		return ret;
	} else {
		DWORD buffSize = 1024;
		CHAR buffer[1024];
		if (GetProcessImageFileNameA(handle, buffer, buffSize)) {
			ret = buffer;
		}
		else cout << "Error, during GetProcessImageFileNameA(): " << GetLastError() << endl;
		CloseHandle(handle);
	} 

	RemovePathFromName(ret);
	
	return ret;
}

//DEVICES PART

void GetMMDevicesCollection(IMMDeviceCollection*& pDeviceCollection){
	HRESULT hresult = S_OK;
	IMMDeviceEnumerator* pDeviceEnumerator = NULL;

	hresult = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**) &pDeviceEnumerator);
	EXIT_ON_ERROR(hresult)
	hresult = pDeviceEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDeviceCollection);
	EXIT_ON_ERROR(hresult)

	SAFE_RELEASE(pDeviceEnumerator)
	return;

Exit:
	printf("Error in GetMMDevicesCollection()!\n");
	SAFE_RELEASE(pDeviceEnumerator)
}

pair<string, string> GetDeviceIDandFriendlyName(IMMDevice* pDevice) {
	HRESULT hresult = S_OK;
	IPropertyStore* pDeviceProps = NULL;
	LPWSTR lpwdID = NULL;
	string fname, dID;

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
	return {fname, dID};

Exit:
	printf("Error in GetDiveceIDandFriendlyName()!\n");
	CoTaskMemFree(lpwdID);
	PropVariantClear(&varName);
	SAFE_RELEASE(pDeviceProps)
	return {fname, dID };
}

map<string, string> CreateMMDevicesMap() {
	map<string, string> devices;
	pair<string, string> singe_device;
	HRESULT hresult = S_OK;
	IMMDevice* pDevice = NULL;
	IMMDeviceCollection* pDeviceCollection = NULL;
	GetMMDevicesCollection(pDeviceCollection);

	UINT count;
	hresult = pDeviceCollection->GetCount(&count); EXIT_ON_ERROR(hresult)

		if (count == 0) {
			cout << "No devices found" << endl;
			return devices;
		}

	for (ULONG i = 0; i < count; i++) {
		hresult = pDeviceCollection->Item(i, &pDevice); EXIT_ON_ERROR(hresult)
			devices.insert(GetDeviceIDandFriendlyName(pDevice));
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

void GetDeviceByID(const string& dID, IMMDevice*& pDevice) {
	HRESULT hresult = S_OK;
	IMMDeviceEnumerator* pDeviceEnumerator = NULL;
	LPCWSTR lpcwdID = NULL;
	wstring wstr = wstring(dID.begin(), dID.end());
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

void SetDeviceVolScaled(const string& dID, float vol) {
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

//SESSIONS PART

void FillSControllersVectorByDeviceID(const string& dID, vector<IAudioSessionControl*>& session_contollers) {
	map<string, string> sessions_list;
	int sessions_count = 0;
	
	HRESULT hresult = S_OK;
	IMMDevice* pDevice = NULL;
	IAudioSessionManager2* pSessionMgmt2 = NULL;
	IAudioSessionEnumerator* pSessionEnum = NULL;
	IAudioSessionControl* pTempSessionControl = NULL;

	GetDeviceByID(dID, pDevice);

	hresult = pDevice->Activate(IID_IAudioSessionManager2, CLSCTX_ALL, NULL, (void**)&pSessionMgmt2); EXIT_ON_ERROR(hresult)
	hresult = pSessionMgmt2->GetSessionEnumerator(&pSessionEnum); EXIT_ON_ERROR(hresult)
	hresult = pSessionEnum->GetCount(&sessions_count); EXIT_ON_ERROR(hresult)

	for (int i = 0; i < sessions_count; i++) {
		pSessionEnum->GetSession(i, &pTempSessionControl);
		session_contollers.push_back(pTempSessionControl);
	}

	//SAFE_RELEASE(pTempSessionControl) - if released, last vector item is NULL
	SAFE_RELEASE(pSessionEnum)
	SAFE_RELEASE(pSessionMgmt2)
	SAFE_RELEASE(pDevice)
	return;

Exit:
	printf("Error in GetSessionsMapByDeviceID()!\n");
	//SAFE_RELEASE(pTempSessionControl) - if released, last vector item is NULL
	SAFE_RELEASE(pSessionEnum)
	SAFE_RELEASE(pSessionMgmt2)
	SAFE_RELEASE(pDevice)
}

map<string, string> GetSessionsMapBydID(const string& dID) {
	map<string, string> sessions;
	pair<string, string> session_info;
	string temp;

	HRESULT hresult = S_OK;
	IAudioSessionControl* pSController_type1 = NULL;
	IAudioSessionControl2* pSController_type2 = NULL;
	LPWSTR display_name = NULL;
	LPWSTR session_GUID = NULL;
	DWORD processID = NULL;

	vector<IAudioSessionControl*> session_contollers;
	FillSControllersVectorByDeviceID(dID, session_contollers);

	for (size_t i = 0; i < session_contollers.size(); i++) {
		pSController_type1 = session_contollers[i];
		hresult = pSController_type1->QueryInterface(&pSController_type2); EXIT_ON_ERROR(hresult)
		hresult = pSController_type2->GetDisplayName(&display_name); EXIT_ON_ERROR(hresult)
		hresult = pSController_type2->GetSessionIdentifier(&session_GUID); EXIT_ON_ERROR(hresult)
		session_info.second = LPWSTRtoString(session_GUID);

		temp = LPWSTRtoString(display_name);
		if (temp == "") {
			hresult = pSController_type2->GetProcessId(&processID); EXIT_ON_ERROR(hresult)
			temp = GetNameFromProccesID(processID);
		}
		session_info.first = temp;
		sessions.insert(session_info);

		SAFE_RELEASE(pSController_type2);
		SAFE_RELEASE(pSController_type1);
	}

	CoTaskMemFree(display_name);
	CoTaskMemFree(session_GUID);

	return sessions;

Exit:
	printf("Error in ChangeDeviceVol()!\n");
	SAFE_RELEASE(pSController_type2)
	SAFE_RELEASE(pSController_type1)
	CoTaskMemFree(display_name);
	CoTaskMemFree(session_GUID);
	return sessions;
}

void SetSessionVol(const string& dID, const string& sID, float vol) {
	HRESULT hresult = S_OK;
	IMMDevice* pDevice = NULL;
	IAudioSessionControl* pSControl = NULL;
	IAudioSessionControl2* pSControl2 = NULL;
	vector<IAudioSessionControl*> sControllers;
	FillSControllersVectorByDeviceID(dID, sControllers);
	LPWSTR found_sID = NULL;
	ISimpleAudioVolume* pSvol = NULL;
	

	for (auto sCtr : sControllers) {
		hresult = sCtr->QueryInterface(&pSControl2); EXIT_ON_ERROR(hresult)
		hresult = pSControl2->GetSessionIdentifier(&found_sID); EXIT_ON_ERROR(hresult)
		if (sID == LPWSTRtoString(found_sID)) {
			pSControl = sCtr;
			SAFE_RELEASE(pSControl2);
			break;
		}
		SAFE_RELEASE(pSControl2);
	}

	pSControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSvol); EXIT_ON_ERROR(hresult)
	pSvol->SetMasterVolume(vol, NULL); EXIT_ON_ERROR(hresult)

	CoTaskMemFree(found_sID);
	SAFE_RELEASE(pSvol);
	for (auto s : sControllers) SAFE_RELEASE(s);
	SAFE_RELEASE(pDevice);
	return;

Exit:
	printf("Error in SetSessionVol()!\n");
	CoTaskMemFree(found_sID);
	SAFE_RELEASE(pSvol);
	for (auto s : sControllers) SAFE_RELEASE(s);
	SAFE_RELEASE(pDevice);
	return;
}


int main()
{
	setlocale(LC_CTYPE, "rus");
	HRESULT hr = S_OK;
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hr != S_OK) {
		cout << "Cant CoInitializeEx(): " << hr << endl;
		return 0;
	}

	//Device simple test
	//<friendly_name, deviceID>
	map<string, string> devices = CreateMMDevicesMap();
	vector<string> devices_fnames;
	for (auto d : devices) {
		devices_fnames.push_back(d.first);
	}
	cout << "Devices: " << endl;
	PrintFriendlyNames(devices_fnames);
	string dID = devices.at(devices_fnames[2]);
	SetDeviceVolScaled(dID, 1);

	//Session simple test
	//<friendly_name, sessionID>
	map<string, string> sessions = GetSessionsMapBydID(dID);
	vector<string> sessions_fnames;
	for (auto d : sessions) {
		sessions_fnames.push_back(d.first);
	}
	cout << "Device #2 sessions: " << endl;
	PrintFriendlyNames(sessions_fnames);

	//Select session and set_vol
	cout << "Select session by number: ";
	int n = 0;
	cin >> n;
	float svol = 1;
	cout << "Set vol in range 0.00 - 1.00: ";
	cin >> svol;
	string sID = sessions.at(sessions_fnames[n]);
	SetSessionVol(dID, sID, svol);

	return 0;
}
