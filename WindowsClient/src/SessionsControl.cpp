#include "SessionsControl.h"
#include "IMMDevice.h"
#include "IMMUtility.h"

#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { goto Exit;}
#define SAFE_RELEASE(punk) if ((punk) != NULL) { (punk)->Release(); (punk) = NULL;}

const IID IID_IAudioSessionManager = __uuidof(IAudioSessionManager);
const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);

void FillSControllersVectorByDeviceID(const std::string& dID, std::vector<IAudioSessionControl*>& session_contollers) {
	std::map<std::string, std::string> sessions_list;
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

std::map<std::string, std::string> GetSessionsMapBydID(const std::string& dID) {
	std::map<std::string, std::string> sessions;
	std::pair<std::string, std::string> session_info;
	std::string temp;

	HRESULT hresult = S_OK;
	IAudioSessionControl* pSController_type1 = NULL;
	IAudioSessionControl2* pSController_type2 = NULL;
	LPWSTR display_name = NULL;
	LPWSTR session_GUID = NULL;
	DWORD processID = NULL;

	std::vector<IAudioSessionControl*> session_contollers;
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

void SetSessionVol(const std::string& dID, const std::string& sID, float vol) {
	HRESULT hresult = S_OK;
	IMMDevice* pDevice = NULL;
	IAudioSessionControl* pSControl = NULL;
	IAudioSessionControl2* pSControl2 = NULL;
	std::vector<IAudioSessionControl*> sControllers;
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