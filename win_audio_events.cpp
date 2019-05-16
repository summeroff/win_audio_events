//-----------------------------------------------------------
// This function enumerates all active (plugged in) audio
// rendering endpoint devices. It prints the friendly name
// and endpoint ID string of each endpoint device.
//-----------------------------------------------------------
#include "win_audio_events.h"

#pragma comment(lib, "uuid.lib")

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

IMMDeviceEnumerator *pEnumerator = NULL;
IMMDeviceEnumerator *pEnumeratorGlobal = NULL;


void PrintEndpointNames()
{
	HRESULT hr = S_OK;

	IMMDeviceCollection *pCollection = NULL;
	IMMDevice *pEndpoint = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR pwszID = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if (FAILED(hr))
	{
		printf("failed to get enumerator.%d \n", hr);
	}
	else
	{
		hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
		if (FAILED(hr))
		{
			printf("failed to get audio collection from enumerator.\n");
		}
		else
		{
			UINT  count = 0;
			hr = pCollection->GetCount(&count);
			if (FAILED(hr))
			{
				printf("Failed to get count for audio endpoints .\n");
			}
			else
			{
				if (count == 0)
				{
					printf("No endpoints found.\n");
				}

				// Each loop prints the name of an endpoint device.
				for (ULONG i = 0; i < count; i++)
				{
					// Get pointer to endpoint number i.
					hr = pCollection->Item(i, &pEndpoint);
					if (FAILED(hr))
					{
					}
					else
					{
						// Get the endpoint ID string.
						hr = pEndpoint->GetId(&pwszID);
						if (FAILED(hr))
						{
						}
						else
						{
							hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
							if (FAILED(hr))
							{
							}
							else
							{
								PROPVARIANT varName;
								// Initialize container for property value.
								PropVariantInit(&varName);

								hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
								if (FAILED(hr))
								{
								}
								else
								{
									// Print endpoint friendly name and endpoint ID.
									printf("Endpoint %d: \"%S\" (%S)\n", i, varName.pwszVal, pwszID);

									PropVariantClear(&varName);
								}
								pProps->Release();
							}
							CoTaskMemFree(pwszID);
							pwszID = NULL;
						}
						pEndpoint->Release();
					}
				}
			}
			pCollection->Release();
		}
		pEnumerator->Release();
	}
}

void PrintDefaultDevice()
{
	HRESULT hr = S_OK;

	IMMDeviceCollection *pCollection = NULL;
	IMMDevice *pEndpoint = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR pwszID = NULL;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		printf("failed to initialize COM \n");
		return;
	}

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if (FAILED(hr))
	{
		printf("failed to get enumerator.%d \n", hr);
	}
	else
	{
		hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pEndpoint);
		if (FAILED(hr))
		{
			printf("failed to get audio default device from enumerator.\n");
		}
		else
		{
			// Get the endpoint ID string.
			hr = pEndpoint->GetId(&pwszID);
			if (FAILED(hr))
			{
			}
			else
			{
				hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
				if (FAILED(hr))
				{
				}
				else
				{
					PROPVARIANT varName;
					// Initialize container for property value.
					PropVariantInit(&varName);

					hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
					if (FAILED(hr))
					{
					}
					else
					{
						SYSTEMTIME time;
						GetSystemTime(&time);
						// Print endpoint friendly name and endpoint ID.
						printf("%02d:%03d Endpoint Default: \"%S\" (%S)\n", time.wSecond, time.wMilliseconds, varName.pwszVal, pwszID);

						PropVariantClear(&varName);
					}
					pProps->Release();
				}
				CoTaskMemFree(pwszID);
				pwszID = NULL;
			}
			pEndpoint->Release();

		}
		pEnumerator->Release();
	}
}

class audioDeviceMonitoring : public IMMNotificationClient
{
	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId);

	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR /*pwstrDeviceId*/, DWORD /*dwNewState*/) { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR /*pwstrDeviceId*/) { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR /*pwstrDeviceId*/) { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR /*pwstrDeviceId*/, const PROPERTYKEY /*key*/) { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnDeviceQueryRemove() { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnDeviceQueryRemoveFailed() { return S_OK; }
	HRESULT STDMETHODCALLTYPE OnDeviceRemovePending() { return S_OK; }

	IFACEMETHODIMP QueryInterface(const IID& iid, void** ppUnk){ AddRef(); return S_OK; }
	IFACEMETHODIMP_(ULONG) AddRef() { return S_OK; }
	IFACEMETHODIMP_(ULONG) Release() { return S_OK; }
};

HRESULT audioDeviceMonitoring::OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR pwstrDefaultDeviceId)
{
	if (Flow == eCapture)
	{
		printf("Default device change event : eCapture %s \n", pwstrDefaultDeviceId);
	}

	if (Flow == eRender)
	{
		if (Role == eCommunications)
		{
			printf("Default device change event : eRender eCommunications %s \n", pwstrDefaultDeviceId);
		}
		if (Role == eConsole)
		{
			printf("Default device change event : eRender eConsole %s \n", pwstrDefaultDeviceId);
		}
		
	}	

	return S_OK;
}

int main()
{
	HRESULT hr = S_OK;
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		printf("failed to initialize COM \n");
		return 1;
	} else {
		audioDeviceMonitoring * audioDeviceMonitor = new audioDeviceMonitoring();

		
		hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumeratorGlobal);
		hr = pEnumeratorGlobal->RegisterEndpointNotificationCallback(audioDeviceMonitor);
		PrintEndpointNames();
		for(int i = 0 ; i < 100 ; i++  )
		{

			PrintDefaultDevice();
			Sleep(250);
		}

		pEnumeratorGlobal->UnregisterEndpointNotificationCallback(audioDeviceMonitor);
		delete audioDeviceMonitor;

		pEnumeratorGlobal->Release();
	}


	return 0;
}