#include "pch.h"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>

#pragma comment(lib, "pdh.lib")

extern "C" __declspec(dllexport) float GetGpuUsage()
{
	static PDH_HQUERY query;
	static PDH_HCOUNTER counter;
	static bool initialized = false;
	PDH_STATUS status;

	if (!initialized) {
		if (PdhOpenQuery(NULL, NULL, &query) != ERROR_SUCCESS)
			return -1.0f;

		// Try different GPU counters
		const wchar_t* counterPaths[] = {
			L"\\GPU Engine(*)\\Utilization Percentage",
			L"\\GPU Engine(*engtype_3D)\\Utilization Percentage",
			L"\\GPU Engine(*engtype_VideoDecode)\\Utilization Percentage"
		};

		for (const wchar_t* path : counterPaths) {
			if (PdhAddCounter(query, path, NULL, &counter) == ERROR_SUCCESS) {
				initialized = true;
				break;
			}
		}

		if (!initialized) return -1.0f;
		PdhCollectQueryData(query);
		Sleep(1000);
		return 0.0f;
	}

	PDH_FMT_COUNTERVALUE value;
	if (PdhCollectQueryData(query) != ERROR_SUCCESS)
		return -1.0f;

	if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value) != ERROR_SUCCESS)
		return -1.0f;

	return (float)value.doubleValue;
}