#include "DeviceUtility.h"
#include "IMMUtility.h"
#include "SessionsControl.h"
#include "SimpleSerial.h"

#include "DeviceManager.h"
#include "DeviceVolController.h"

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
	string dname = "Steinberg";
	string dID = devices.at(devices_fnames[i]);
	
	DeviceIDAndName device = make_pair(dID, dname);

	char com_port[] = "\\\\.\\COM4";
	DWORD COM_BAUD_RATE = 500000;
	int timeout = 50;
	SimpleSerial serial = SimpleSerial(com_port, COM_BAUD_RATE, timeout);
	if (serial.connected_) {
		cout << "Serial is connected!" << endl;
	}
	else {
		cout << "Serial connection Error!" << endl;
	}

	DeviceNotificationSender dev_send_upd = DeviceNotificationSender("Steinberg", &serial);
	DeviceManager dev_mngr = DeviceManager(device, &dev_send_upd);
	DeviceVolController *dev_vol_ctrl = dev_mngr.Get_DeviceVolController();

	int counter = 50;
	int last_counter = 50;
	string incoming;
	int refresh_time = 500;
	while (serial.connected_) {
		incoming = serial.ReadSerialPort("json");
		if (incoming == "1") {
			if (counter < 100) counter += 5;
			cout << "Recieved: " << incoming << "; counter = " << counter << endl;
		} else if (incoming == "0") {
			if (counter > 0) counter -= 5;
			cout << "Recieved: " << incoming << "; counter = " << counter << endl;
		} else if (incoming != ""){
			cout << "Debug: " << incoming << ";" << endl;
		}

		//if (incoming != "") cout << incoming << endl;

		if (counter != last_counter) {
			float vol = float(counter) / 100;
			dev_vol_ctrl->SetVol(vol);
			last_counter = counter;
		}
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
