#include "pch.h"
#include <windows.h>
#include <pdh.h>

#pragma comment(lib, "pdh.lib")

extern "C" __declspec(dllexport) float GetCpuUsage()
{
	static PDH_HQUERY query;
	static PDH_HCOUNTER counter;
	static bool initialized = false;
	PDH_STATUS status;

	if (!initialized) {
		if ((status = PdhOpenQuery(NULL, NULL, &query)) != ERROR_SUCCESS)
			return -1.0f;

		if ((status = PdhAddEnglishCounter(query, L"\\Processor(_Total)\\% Processor Time", NULL, &counter)) != ERROR_SUCCESS)
			return -1.0f;

		PdhCollectQueryData(query);
		initialized = true;
		Sleep(1000);
		return 0.0f;
	}

	PDH_FMT_COUNTERVALUE value;
	if ((status = PdhCollectQueryData(query)) != ERROR_SUCCESS)
		return -1.0f;

	if ((status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value)) != ERROR_SUCCESS)
		return -1.0f;

	if (value.CStatus != ERROR_SUCCESS)
		return -1.0f;

	return (float)value.doubleValue;
}