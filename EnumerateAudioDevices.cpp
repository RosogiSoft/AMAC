#include <stdlib.h>
#include <iostream>
#include <audioendpoints.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <Winerror.h>
#include <combaseapi.h>

#define EXIT_ON_ERROR(hres)  \
			  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
			  if ((punk) != NULL)  \
				{ (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

using namespace std;

void PrintEndpointNames()
{
	HRESULT hr = S_OK;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDeviceCollection* pCollection = NULL;
	IMMDevice* pEndpoint = NULL;
	IPropertyStore* pProps = NULL;
	LPWSTR pwszID = NULL;

	//“ребуетс¤ единожды дл¤ вызова CoCreateInstance в рамках потока (thread)
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	//ѕример использует что-то св¤зное с COM вещ¤ми, надо разобратьс¤ подробнее
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

		hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
	EXIT_ON_ERROR(hr)

		UINT  count;
	hr = pCollection->GetCount(&count);
	EXIT_ON_ERROR(hr)

		if (count == 0)
		{
			printf("No endpoints found.\n");
		}

	// Each loop prints the name of an endpoint device.
	for (ULONG i = 0; i < count; i++)
	{
		// Get pointer to endpoint number i.
		hr = pCollection->Item(i, &pEndpoint);
		EXIT_ON_ERROR(hr)

			// Get the endpoint ID string.
			hr = pEndpoint->GetId(&pwszID);	EXIT_ON_ERROR(hr)

			hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps); EXIT_ON_ERROR(hr)

			PROPVARIANT varName;
		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the endpoint's friendly-name property.
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName); EXIT_ON_ERROR(hr)

			// Print endpoint friendly name and endpoint ID.
			cout << "Endpoint #" << i << ": name = ";

		char buffer[500];
		size_t buffer_size;
		wcstombs_s(&buffer_size, buffer, (size_t)500, varName.pwszVal, (size_t)500);
		cout << buffer << ", id = ";
		wcstombs_s(&buffer_size, buffer, (size_t)500, pwszID, (size_t)500);
		cout << buffer << endl;

		CoTaskMemFree(pwszID);
		pwszID = NULL;
		PropVariantClear(&varName);
		SAFE_RELEASE(pProps)
			SAFE_RELEASE(pEndpoint)
	}

	SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pCollection)
		return;

Exit:
	printf("Error!\n");
	CoTaskMemFree(pwszID);
	SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pCollection)
		SAFE_RELEASE(pEndpoint)
		SAFE_RELEASE(pProps)
}

int main()
{
	setlocale(LC_CTYPE, "rus");
	std::cout << "Calling PrintEndpointNames()..." << endl;
	PrintEndpointNames();
	return 0;
}
