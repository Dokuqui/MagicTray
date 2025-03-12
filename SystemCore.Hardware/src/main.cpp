#include "../include/system_monitor.h"
#include <windows.h>
#include <iostream>

int main()
{
	while (true)
	{
		double cpuUsage = get_cpu_usage();
		double ramUsage = get_ram_usage();
		std::vector<GPUInfo> gpus = getGPUInfo();

		system("cls");
		std::cout << "CPU Usage: " << cpuUsage << "%\n";
		std::cout << "RAM Usage: " << ramUsage << "GB\n";

		for (size_t i = 0; i < gpus.size(); ++i) {
			std::wcout << L"GPU " << i + 1 << L": " << gpus[i].name << std::endl;
			std::cout << "  Total VRAM: " << gpus[i].totalVRAM << " GB" << std::endl;
			std::cout << "  Used VRAM: " << gpus[i].usedVRAM << " GB" << std::endl;
			std::cout << "  GPU Usage: " << gpus[i].gpuUsage << " %" << std::endl;
			std::cout << "  Temperature: " << gpus[i].temperature << " °C\n\n";
		}

		Sleep(1000);
	}

	return 0;
}