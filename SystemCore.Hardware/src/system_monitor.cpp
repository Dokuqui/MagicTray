#include "../include/system_monitor.h"
#include <windows.h>
#include <pdh.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <comdef.h>
#include <wbemidl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")

double calculate_cpu_usage() {
	static ULARGE_INTEGER prevIdleTime, prevKernelTime, prevUserTime;
	ULARGE_INTEGER idleTime, kernelTime, userTime;
	FILETIME ftIdle, ftKernel, ftUser;

	if (!GetSystemTimes(&ftIdle, &ftKernel, &ftUser)) {
		return -1.0;
	}

	idleTime.LowPart = ftIdle.dwLowDateTime;
	idleTime.HighPart = ftIdle.dwHighDateTime;
	kernelTime.LowPart = ftKernel.dwLowDateTime;
	kernelTime.HighPart = ftKernel.dwHighDateTime;
	userTime.LowPart = ftUser.dwLowDateTime;
	userTime.HighPart = ftUser.dwHighDateTime;

	ULONGLONG sysTime = (kernelTime.QuadPart - prevKernelTime.QuadPart) + (userTime.QuadPart - prevUserTime.QuadPart);
	ULONGLONG idleDelta = idleTime.QuadPart - prevIdleTime.QuadPart;

	prevIdleTime = idleTime;
	prevKernelTime = kernelTime;
	prevUserTime = userTime;

	if (sysTime == 0) return -1.0;

	return (1.0 - (double)idleDelta / sysTime) * 100.0;
}

double get_cpu_usage() {
	static bool firstCall = true;
	if (firstCall) {
		firstCall = false;
		calculate_cpu_usage();
		Sleep(1000);
	}
	return calculate_cpu_usage();
}

double get_ram_usage() {
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	double totalRam = memInfo.ullTotalPhys / (1024.0 * 1024 * 1024);
	double usedRam = totalRam - (memInfo.ullAvailPhys / (1024.0 * 1024 * 1024));

	return usedRam;
}

double getVRAMUsage(IDXGIAdapter1* adapter) {
	IDXGIAdapter3* adapter3 = nullptr;
	DXGI_QUERY_VIDEO_MEMORY_INFO vramInfo = {};
	double usedVRAM = -1.0;

	if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter3)))) {
		if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vramInfo))) {
			usedVRAM = static_cast<double>(vramInfo.CurrentUsage) / (1024.0 * 1024 * 1024);
		}
		adapter3->Release();
	}

	return usedVRAM > 0 ? usedVRAM : 0.0;  // Ensure no negative values
}

double getGPUUsage(const std::wstring& gpuName) {
	IWbemLocator* pLocator = nullptr;
	IWbemServices* pServices = nullptr;
	IEnumWbemClassObject* pEnumerator = nullptr;
	HRESULT hres;

	CoInitializeEx(0, COINIT_MULTITHREADED);
	hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
	if (FAILED(hres)) return -1.0;

	hres = pLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, NULL, 0, 0, &pServices);
	if (FAILED(hres)) {
		pLocator->Release();
		return -1.0;
	}

	hres = pServices->ExecQuery(bstr_t("WQL"),
		bstr_t("SELECT Name, PercentGPUUsage FROM Win32_PerfFormattedData_GPUPerformanceCounters_GPUAdapter"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);

	if (FAILED(hres)) {
		pServices->Release();
		pLocator->Release();
		return -1.0;
	}

	IWbemClassObject* pClassObject = nullptr;
	ULONG uReturn = 0;
	double gpuUsage = -1.0;

	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
		if (0 == uReturn) break;

		VARIANT vtName, vtUsage;
		pClassObject->Get(L"Name", 0, &vtName, 0, 0);
		std::wstring name = vtName.bstrVal;

		if (name.find(gpuName) != std::wstring::npos) {
			pClassObject->Get(L"PercentGPUUsage", 0, &vtUsage, 0, 0);
			gpuUsage = vtUsage.uintVal;
			VariantClear(&vtUsage);
		}

		VariantClear(&vtName);
		pClassObject->Release();
	}

	pEnumerator->Release();
	pServices->Release();
	pLocator->Release();
	CoUninitialize();

	return gpuUsage >= 0 ? gpuUsage : 0.0;
}

double getGPUTemperature() {
	IWbemLocator* pLoc = nullptr;
	IWbemServices* pSvc = nullptr;
	IEnumWbemClassObject* pEnumerator = nullptr;
	HRESULT hres;

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) return -1.0;

	hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres)) return -1.0;

	hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, NULL, 0, 0, &pSvc);
	if (FAILED(hres)) return -1.0;

	hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT CurrentTemperature FROM Win32_TemperatureProbe"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
	if (FAILED(hres)) return -1.0;

	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;
	double temperature = -1.0;

	while (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK) {
		VARIANT vtProp;
		pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
		if (vtProp.vt == VT_I4) {
			temperature = (vtProp.intVal - 2732) / 10.0;
		}
		VariantClear(&vtProp);
		pclsObj->Release();
	}

	return temperature > 0 ? temperature : -1.0;
}

std::vector<GPUInfo> getGPUInfo() {
	std::vector<GPUInfo> gpus;
	IDXGIFactory4* dxgiFactory = nullptr;

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
		return gpus;
	}

	IDXGIAdapter1* dxgiAdapter = nullptr;
	int adapterIndex = 0;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 adapterDesc;
		dxgiAdapter->GetDesc1(&adapterDesc);

		std::wstring gpuName(adapterDesc.Description);

		if (gpuName.find(L"Microsoft Basic Render Driver") != std::wstring::npos) {
			dxgiAdapter->Release();
			adapterIndex++;
			continue;
		}

		GPUInfo gpu;
		gpu.name = gpuName;
		gpu.totalVRAM = static_cast<double>(adapterDesc.DedicatedVideoMemory) / (1024.0 * 1024 * 1024);
		gpu.usedVRAM = getVRAMUsage(dxgiAdapter);
		gpu.gpuUsage = getGPUUsage(gpuName);
		gpu.temperature = getGPUTemperature();

		gpus.push_back(gpu);
		dxgiAdapter->Release();
		adapterIndex++;
	}

	dxgiFactory->Release();
	return gpus;
}