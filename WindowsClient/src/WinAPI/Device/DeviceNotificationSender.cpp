#include "DeviceNotificationSender.h"

#include <iostream>;

DeviceNotificationSender::DeviceNotificationSender() {

}

DeviceNotificationSender::DeviceNotificationSender(const string& DeviceID, SimpleSerial *serial) {
	this->DeviceID = DeviceID;
	this->serial = serial;
}

void DeviceNotificationSender::VolChanged(float vol) {
	if (vol != prev_vol) {
		prev_vol = vol;
		SendSerial_VolChange(vol);
	}
}

void DeviceNotificationSender::MuteChanged(bool mute) {
	if (mute != prev_mute) {
		prev_mute = mute;
		SendSerial_MuteChange(mute);
	}
}

void DeviceNotificationSender::SendSerial_VolChange(float vol) {
	int i_vol = vol * 100;
	string send_str = "{" + DeviceID + ":vol=" + std::to_string(i_vol) + "}";
	cout << "WriteToSerial: " << send_str << endl;

	char* to_send = &send_str[0];
	serial->WriteSerialPort(to_send);
		
}

void DeviceNotificationSender::SendSerial_MuteChange(bool mute) {
	string send_str = "{" + DeviceID + ":mute=" + std::to_string(mute) + "}";
	cout << "WriteToSerial: " << send_str << endl;

	char* to_send = &send_str[0];
	serial->WriteSerialPort(to_send);
}