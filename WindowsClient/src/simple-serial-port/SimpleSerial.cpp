#include "SimpleSerial.h"

SimpleSerial::SimpleSerial(char* com_port, DWORD COM_BAUD_RATE, int timeout)
{
	connected_ = false;

	io_handler_ = CreateFileA(static_cast<LPCSTR>(com_port),
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

	if (io_handler_ == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			printf("Warning: Handle was not attached. Reason: %s not available\n", com_port);
		}
	} else {
		DCB dcbSerialParams = { 0 };
		if (!GetCommState(io_handler_, &dcbSerialParams)) {
			printf("Warning: Failed to get current serial params");
		} else {
			dcbSerialParams.BaudRate = COM_BAUD_RATE;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;
			dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(io_handler_, &dcbSerialParams)) {
				printf("Warning: could not set serial port params\n");
			} else {
				connected_ = true;
				PurgeComm(io_handler_, PURGE_RXCLEAR | PURGE_TXCLEAR);				
			}
		}

		_COMMTIMEOUTS timeoutsettings = {
			timeoutsettings.ReadIntervalTimeout = 0,
			timeoutsettings.ReadTotalTimeoutMultiplier = 1,
			timeoutsettings.ReadTotalTimeoutConstant = timeout,
			timeoutsettings.WriteTotalTimeoutMultiplier = 1,
			timeoutsettings.WriteTotalTimeoutConstant = timeout
		};
		SetCommTimeouts(io_handler_, &timeoutsettings);
	}
}

void SimpleSerial::CustomSyntax(string syntax_type) {

	ifstream syntaxfile_exist("syntax_config.txt");

	if (!syntaxfile_exist) {		
		ofstream syntaxfile;
		syntaxfile.open("syntax_config.txt");

		if (syntaxfile) {
			syntaxfile << "json { }\n";
			syntaxfile << "greater_less_than < >\n";
			syntaxfile << "square [ ]\n";
			syntaxfile.close();
		}
	}

	syntaxfile_exist.close();
	
	ifstream syntaxfile_in;
	syntaxfile_in.open("syntax_config.txt");
	
	string line;
	bool found = false;	

	if (syntaxfile_in.is_open()) {
		while (syntaxfile_in) {			
			syntaxfile_in >> syntax_name_ >> front_delimiter_ >> end_delimiter_;
			getline(syntaxfile_in, line);			
			if (syntax_name_ == syntax_type) {
				found = true;
				break;
			}
		}

		syntaxfile_in.close();
		if (!found) {
			syntax_name_ = "";
			front_delimiter_ = ' ';
			end_delimiter_ = ' ';
			printf("Warning: Could not find delimiters, may cause problems!\n");
		}
	}
	else printf ("Warning: No file open");
}

string SimpleSerial::ReadSerialPort(string syntax_type) {

	DWORD bytes_read;
	char inc_msg[1];	
	string complete_inc_msg;
	bool began_read = false;

	CustomSyntax(syntax_type);
	ClearCommError(io_handler_, &errors_, &status_);

	while (status_.cbInQue > 0){
		if (ReadFile(io_handler_, inc_msg, 1, &bytes_read, NULL)) {
			if (began_read) {
				if (inc_msg[0] == end_delimiter_){
					return complete_inc_msg;
				} else {
					complete_inc_msg.append(inc_msg, 1);
				}
			} else if (inc_msg[0] == front_delimiter_) {
				began_read = true;
			} 
		}
	}
	return "";		
}

bool SimpleSerial::WriteSerialPort(char *data_sent)
{
	DWORD bytes_sent;	

	unsigned int data_sent_length = strlen(data_sent);

	if (!WriteFile(io_handler_, (void*)data_sent, data_sent_length, &bytes_sent, NULL)) {
		ClearCommError(io_handler_, &errors_, &status_);
		return false;
	}
	else
		return true;
}

bool SimpleSerial::CloseSerialPort()
{
	if (connected_) {
		connected_ = false;
		CloseHandle(io_handler_);
		return true;
	}	
	else
		return false;
}

SimpleSerial::~SimpleSerial()
{
	if (connected_) {
		connected_ = false;
		CloseHandle(io_handler_);		
	}
}
