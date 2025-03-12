// In SystemMonitor.GUI project
using System.Runtime.InteropServices;

public static class NativeMonitor
{
    [DllImport("HardwareMonitor.Core.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern float GetCpuUsage();
}