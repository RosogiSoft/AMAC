#pragma once
#include <string>
#include <map>
#include <vector>
#include <combaseapi.h>

void PrintFullMap(const std::map<std::string, std::string>& map);
void PrintFriendlyNames(const std::vector<std::string>& v);
std::string LPWSTRtoString(const LPWSTR& in);
void RemovePathFromName(const std::string& s);
std::string GetNameFromProccesID(const DWORD& processID);