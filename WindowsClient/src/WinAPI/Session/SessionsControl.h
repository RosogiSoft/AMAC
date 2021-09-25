#pragma once
#include <string>
#include <map>
#include <vector>
#include <audiopolicy.h>
#include <audioendpoints.h>

//map<friendly_name, sessionID>; pair<friendly_name, sessionID> for
typedef std::map<std::string, std::string> IMMDevice_SessionsMap; 
typedef std::pair<std::string, std::string> Session_NameAndID_Pair;

void Fill_SessionCtrlsVector_by_dID(const std::string& dID, std::vector<IAudioSessionControl*>& session_contollers);
IMMDevice_SessionsMap GetSessionsMapBydID(const std::string& dID);
void SetSessionVol(const std::string& dID, const std::string& sID, float vol);