#pragma once

#include <string>
#include "SimpleSerial.h"

class DeviceNotificationSender
{
public:
	DeviceNotificationSender();
	DeviceNotificationSender(const string& DeviceID, SimpleSerial *serial);
	void VolChanged(float vol);
	void MuteChanged(bool mute);

private:
	string DeviceID;
	SimpleSerial *serial;
	float prev_vol = -1;
	bool prev_mute = false;

	void SendSerial_VolChange(float vol);
	void SendSerial_MuteChange(bool mute);
};

