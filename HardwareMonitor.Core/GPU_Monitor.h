#pragma once
#ifndef GPUMONITOR_H
#define GPUMONITOR_H

extern "C" __declspec(dllexport) int GetGpuCount();
extern "C" __declspec(dllexport) float GetGpuUsage(int gpuIndex);
extern "C" __declspec(dllexport) float GetGpuTemperature(int gpuIndex);

#endif
