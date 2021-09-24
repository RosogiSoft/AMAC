#include "WinAPI/IMMDevice.h"
#include "WinAPI/IMMUtility.h"
#include "WinAPI/SessionsControl.h"
#include "simple-serial-port/SimpleSerial.h"

#include <iostream>
#include <chrono>
#include <thread>
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

	int i;
	cout << "Select device to change volume of: ";
	cin >> i;
	string dID = devices.at(devices_fnames[i]);

	char com_port[] = "\\\\.\\COM3";
	DWORD COM_BAUD_RATE = CBR_9600;
	SimpleSerial serial = SimpleSerial(com_port, COM_BAUD_RATE);
	int counter = 100;
	if (serial.connected_) {
		cout << "Serial is connected!" << endl;
	}
	else {
		cout << "Serial connection Error!" << endl;
	}
	string incoming;
	while (serial.connected_) {
		int refresh_time = 100;
		incoming = serial.ReadSerialPort(refresh_time, "json");
		if (incoming == "CW") {
			if (counter < 100) counter += 5;
		}
		else if (incoming == "CCW") {
			if (counter > 0) counter -= 5;
		}
		cout << "Recieved: " << incoming << "; counter = " << counter << endl;
		SetDeviceVolScaled(dID, ((float) counter) /100);
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	/*
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
	*/
	return 0;
}
