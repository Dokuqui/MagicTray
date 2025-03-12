// In SystemMonitor.GUI project
using System.Runtime.InteropServices;

public static class NativeMonitor
{
    // Existing methods for single GPU
    [DllImport("HardwareMonitor.Core.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern float GetCpuUsage();

    // New methods for multiple GPUs (assuming your native code supports them)
    [DllImport("HardwareMonitor.Core.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetGpuCount();  // Returns the number of available GPUs

    [DllImport("HardwareMonitor.Core.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern float GetGpuUsage(int gpuIndex);  // Get usage for a specific GPU

    [DllImport("HardwareMonitor.Core.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern float GetGpuTemperature(int gpuIndex);  // Get temperature for a specific GPU
}
