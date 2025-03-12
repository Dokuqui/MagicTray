#include "pch.h"
#include "GPU_Monitor.h"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <comdef.h>
#include <wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

#pragma comment(lib, "pdh.lib")

extern "C" __declspec(dllexport) int GetGpuCount()
{
	HRESULT hres;
	IWbemLocator* pLoc = NULL;
	IWbemServices* pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;
	int gpuCount = 0;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return 0;

	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return 0;
	}

	hres = pLoc->ConnectServer(_bstr_t(L"root\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return 0;
	}

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 0;
	}

	hres = pSvc->ExecQuery(bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_VideoController"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL, &pEnumerator);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 0;
	}

	while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == 0) {
		gpuCount++;
		pclsObj->Release();
	}

	pEnumerator->Release();
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return gpuCount;
}

extern "C" __declspec(dllexport) float GetGpuUsage(int gpuIndex)
{
	HRESULT hres;
	IWbemLocator* pLoc = NULL;
	IWbemServices* pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;
	float gpuUsage = -1.0f;
	int currentGpuIndex = 0;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return -1.0f;

	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return -1.0f;
	}

	hres = pLoc->ConnectServer(_bstr_t(L"root\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return -1.0f;
	}

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -1.0f;
	}

	hres = pSvc->ExecQuery(bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_VideoController"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL, &pEnumerator);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -1.0f;
	}

	while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == 0) {
		if (currentGpuIndex == gpuIndex) {
			VARIANT vtProp;
			hres = pclsObj->Get(L"LoadPercentage", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hres) && vtProp.vt == VT_I4) {
				gpuUsage = (float)vtProp.lVal;
			}
			VariantClear(&vtProp);
			pclsObj->Release();
			break;
		}
		currentGpuIndex++;
	}

	pEnumerator->Release();
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return gpuUsage;
}

extern "C" __declspec(dllexport) float GetGpuTemperature(int gpuIndex)
{
	HRESULT hres;
	IWbemLocator* pLoc = NULL;
	IWbemServices* pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;
	float gpuTemp = -1.0f;
	int currentGpuIndex = 0;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return -1.0f;

	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return -1.0f;
	}

	hres = pLoc->ConnectServer(_bstr_t(L"root\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return -1.0f;
	}

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -1.0f;
	}

	hres = pSvc->ExecQuery(bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_TemperatureProbe WHERE Name LIKE '%GPU%'"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL, &pEnumerator);

	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -1.0f;
	}

	while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == 0) {
		if (currentGpuIndex == gpuIndex) {
			VARIANT vtProp;
			hres = pclsObj->Get(L"CurrentReading", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hres) && vtProp.vt == VT_I4) {
				gpuTemp = (float)vtProp.lVal;
			}
			VariantClear(&vtProp);
			pclsObj->Release();
			break;
		}
		currentGpuIndex++;
	}

	pEnumerator->Release();
	pSvc->Release();
	pLoc->Release();
	CoUninitialize();

	return gpuTemp;
}
