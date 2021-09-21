#include "IMMUtility.h"
#include <iostream>
#include <psapi.h>


void PrintFullMap(const std::map<std::string, std::string>& map) {
	int counter = 0;
	for (auto i : map) {
		std::cout << "#" << counter << " - Name: " << i.second << "; id = " << i.first << std::endl;
		counter++;
	}
}

void PrintFriendlyNames(const std::vector<std::string>& v) {
	int counter = 0;
	for (auto n : v) {
		std::cout << "#" << counter << ": " << n << std::endl;
		counter++;
	}
}

std::string LPWSTRtoString(const LPWSTR& in) {
	char buffer[500];
	size_t buffer_size;
	wcstombs_s(&buffer_size, buffer, (size_t)500, in, (size_t)500);
	return buffer;
}

void RemovePathFromName(std::string& s) {
	int start, end;
	start = s.find_last_of('/\\');
	end = s.length() - start - (s.length() - s.find_last_of('.') + 1);
	s = s.substr(start + 1, end);
}

std::string GetNameFromProccesID(const DWORD& processID) {
	std::string ret;
	HANDLE handle;
	handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processID);
	if (handle == NULL) {
		std::cout << "Error, during OpenProcess(): " << GetLastError() << std::endl;
		return ret;
	}
	else {
		DWORD buffSize = 1024;
		CHAR buffer[1024];
		if (GetProcessImageFileNameA(handle, buffer, buffSize)) {
			ret = buffer;
		}
		else std::cout << "Error, during GetProcessImageFileNameA(): " << GetLastError() << std::endl;
		CloseHandle(handle);
	}

	RemovePathFromName(ret);

	return ret;
}