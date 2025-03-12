#pragma once

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <vector>
#include <string>

struct GPUInfo {
	std::wstring name;
	double totalVRAM;
	double usedVRAM;
	double gpuUsage;
	double temperature;
};

double get_cpu_usage();

double get_ram_usage();

std::vector<GPUInfo> getGPUInfo();

#endif // !SYSTEM_MONITOR_H
