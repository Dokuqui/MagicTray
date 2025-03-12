#include "../include/system_monitor.h"
#include <windows.h>
#include <pdh.h>
#include <psapi.h>
#include <iostream>
#include <string>

double calculate_cpu_usage() {
	static ULARGE_INTEGER previousIdleTime, previousKernelTime, previousUserTime;
	ULARGE_INTEGER idleTime, kernelTime, userTime;
	FILETIME ftIdle, ftKernel, ftUser;

	if (!GetSystemTimes(&ftIdle, &ftKernel, &ftUser)) {
		std::cerr << "GetSystemTimes failed. Error: " << GetLastError() << std::endl;
		return -1.0;
	}

	idleTime.LowPart = ftIdle.dwLowDateTime;
	idleTime.HighPart = ftIdle.dwHighDateTime;
	kernelTime.LowPart = ftKernel.dwLowDateTime;
	kernelTime.HighPart = ftKernel.dwHighDateTime;
	userTime.LowPart = ftUser.dwLowDateTime;
	userTime.HighPart = ftUser.dwHighDateTime;

	ULONGLONG systemTime = (kernelTime.QuadPart - previousKernelTime.QuadPart) +
		(userTime.QuadPart - previousUserTime.QuadPart);

	ULONGLONG idleDelta = idleTime.QuadPart - previousIdleTime.QuadPart;

	previousIdleTime = idleTime;
	previousKernelTime = kernelTime;
	previousUserTime = userTime;

	if (systemTime == 0) {
		std::cerr << "System time delta is zero." << std::endl;
		return -1.0;
	}

	double cpuUsage = (1.0 - (double)idleDelta / systemTime) * 100.0;

	if (cpuUsage < 0.0) cpuUsage = 0.0;
	else if (cpuUsage > 100.0) cpuUsage = 100.0;

	return cpuUsage;
}

double get_cpu_usage() {
	static bool firstCall = true;
	double cpuUsage;

	if (firstCall) {
		firstCall = false;
		calculate_cpu_usage();
		Sleep(1000);
	}

	cpuUsage = calculate_cpu_usage();
	return cpuUsage;
}

double get_ram_usage()
{
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);

	double totalRam = memInfo.ullTotalPhys / (1024.0 * 1024 * 1024);
	double usedRam = totalRam - (memInfo.ullAvailPhys / (1024.0 * 1024 * 1024));

	return usedRam;
}
