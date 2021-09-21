#include "IMMDevice.h"
#include "IMMUtility.h"
#include "SessionsControl.h"

#include <iostream>
#include <combaseapi.h>
#include <map>
#include <vector>

using namespace std;

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
	IMMDeviceMap devices = CreateMMDevicesMap();
	vector<string> devices_fnames;
	for (auto d : devices) {
		devices_fnames.push_back(d.first);
	}
	cout << "Devices: " << endl;
	PrintFriendlyNames(devices_fnames);
	string dID = devices.at(devices_fnames[3]);
	SetDeviceVolScaled(dID, 1);

	//Session simple test
	//map<friendly_name, sessionID>
	IMMDevice_SessionsMap sessions = GetSessionsMapBydID(dID);
	vector<string> sessions_fnames;
	for (auto d : sessions) {
		sessions_fnames.push_back(d.first);
	}
	cout << "Device #3 sessions: " << endl;
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
