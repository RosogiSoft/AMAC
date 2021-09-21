#pragma once
#include <string>
#include <map>
#include <vector>
#include <audiopolicy.h>
#include <audioendpoints.h>

typedef std::map<std::string, std::string> IMMDevice_SessionsMap;

void FillSControllersVectorByDeviceID(const std::string& dID, std::vector<IAudioSessionControl*>& session_contollers);
IMMDevice_SessionsMap GetSessionsMapBydID(const std::string& dID);
void SetSessionVol(const std::string& dID, const std::string& sID, float vol);