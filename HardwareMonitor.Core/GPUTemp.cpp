#include "pch.h"
#include <windows.h>
#include <comdef.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

extern "C" __declspec(dllexport) float GetGpuTemperature()
{
	HRESULT hres;
	IWbemLocator* pLoc = NULL;
	IWbemServices* pSvc = NULL;
	float temp = -1.0f;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return -1.0f;

	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) goto Cleanup;

	hres = pLoc->ConnectServer(_bstr_t(L"root\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) goto Cleanup;

	hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE);
	if (FAILED(hres)) goto Cleanup;

	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_TemperatureProbe WHERE Name LIKE '%GPU%'"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres)) goto Cleanup;

	IWbemClassObject* pclsObj = NULL;
	ULONG uReturn = 0;
	while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == 0) {
		VARIANT vtProp;
		hres = pclsObj->Get(L"CurrentReading", 0, &vtProp, 0, 0);
		if (SUCCEEDED(hres) && vtProp.vt == VT_I4) {
			temp = (float)vtProp.lVal;
		}
		VariantClear(&vtProp);
		pclsObj->Release();
		break;
	}

Cleanup:
	if (pSvc) pSvc->Release();
	if (pLoc) pLoc->Release();
	CoUninitialize();
	return temp;
}