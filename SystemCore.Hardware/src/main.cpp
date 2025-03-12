#include "../include/system_monitor.h"
#include <windows.h>
#include <iostream>

int main()
{
	while (true)
	{
		double cpuUsage = get_cpu_usage();
		double ramUsage = get_ram_usage();

		system("cls");
		std::cout << "CPU Usage: " << cpuUsage << "%\n";
		std::cout << "RAM Usage: " << ramUsage << "GB\n";

		Sleep(1000);
	}

	return 0;
}