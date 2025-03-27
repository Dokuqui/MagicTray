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

// AMD and NVIDIA SDKs
#include "../include/adl_sdk.h"
#include "../include/nvapi.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")
// ========================= INIT ========================= //
// Global ADL context (initialize once)
int ADL_init() {
	if (ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1) != ADL_OK)
		return -1;
	return 0;
}

// Call this before using ADL functions
static bool nvapiInitialized = []() {
	return NvAPI_Initialize() == NVAPI_OK;
	}();

// ========================= CPU USAGE ========================= //
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

// ========================= RAM USAGE ========================= //
double get_ram_usage() {
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	double totalRam = memInfo.ullTotalPhys / (1024.0 * 1024 * 1024);
	double usedRam = totalRam - (memInfo.ullAvailPhys / (1024.0 * 1024 * 1024));

	return usedRam;
}

// ========================= GPU USAGE (DXGI) ========================= //
double getGPUUsageDXGI(IDXGIAdapter1* adapter) {
	IDXGIAdapter3* adapter3 = nullptr;
	DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};
	double usage = 0.0;

	if (SUCCEEDED(adapter->QueryInterface(IID_PPV_ARGS(&adapter3)))) {
		if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo))) {
			usage = static_cast<double>(memoryInfo.CurrentUsage) / memoryInfo.Budget * 100.0;
		}
		adapter3->Release();
	}

	return usage >= 0 ? usage : 0.0;
}

// ========================= VRAM USAGE ========================= //
SIZE_T getProcessVRAMUsage() {
	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
		return pmc.PrivateUsage / (1024 * 1024);  // Convert to MB
	}
	return 0;
}

// ========================= GPU TEMPERATURE ========================= //
double getGPUTemperature_ADL(int adapterIndex) {
	ADLTemperature adlTemp = { sizeof(ADLTemperature) };
	if (ADL_Overdrive5_Temperature_Get(adapterIndex, 0, &adlTemp) == ADL_OK) {
		return adlTemp.iTemperature / 1000.0;
	}
	return -1.0;
}

double getGPUTemperature_NVAPI() {
	NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS];
	NvU32 gpuCount = 0;

	if (NvAPI_EnumPhysicalGPUs(handles, &gpuCount) == NVAPI_OK && gpuCount > 0) {
		NV_GPU_THERMAL_SETTINGS thermalSettings = {};
		thermalSettings.version = NV_GPU_THERMAL_SETTINGS_VER;

		if (NvAPI_GPU_GetThermalSettings(handles[0], NVAPI_THERMAL_TARGET_GPU, &thermalSettings) == NVAPI_OK) {
			return static_cast<double>(thermalSettings.sensor[0].currentTemp);
		}
	}
	return -1.0;
}

// ========================= GPU INFO ========================= //
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
		gpu.usedVRAM = getProcessVRAMUsage();
		gpu.gpuUsage = getGPUUsageDXGI(dxgiAdapter);

		if (gpuName.find(L"NVIDIA") != std::wstring::npos) {
			gpu.temperature = getGPUTemperature_NVAPI();
		}
		else if (gpuName.find(L"AMD") != std::wstring::npos) {
			gpu.temperature = getGPUTemperature_ADL(adapterIndex);
		}
		else {
			gpu.temperature = -1.0;
		}

		gpus.push_back(gpu);
		dxgiAdapter->Release();
		adapterIndex++;
	}

	dxgiFactory->Release();
	return gpus;
}