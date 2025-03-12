using System.Windows;
using System.Windows.Threading;
using System.Windows.Media;
using System.Diagnostics;
using System.Linq;

namespace SystemMonitor.GUI
{
    public partial class App : System.Windows.Application
    {
        private TrayIconManager? _trayManager;
        private MainWindow? _mainWindow;
        private DispatcherTimer? _updateTimer;
        private TaskbarWidget? _widgetWindow;

        protected override void OnStartup(StartupEventArgs e)
        {
            base.OnStartup(e);

            _mainWindow = new MainWindow();
            _trayManager = new TrayIconManager(_mainWindow);
            _widgetWindow = new TaskbarWidget();

            InitializeUpdateTimer();
            _widgetWindow.Show();
        }

        private void InitializeUpdateTimer()
        {
            _updateTimer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(1)
            };
            _updateTimer.Tick += UpdateMetrics;
            _updateTimer.Start();
        }

        private void UpdateMetrics(object? sender, EventArgs e)
        {
            try
            {
                var cpuUsage = NativeMonitor.GetCpuUsage();

                var gpuCount = NativeMonitor.GetGpuCount();
                var gpuUsages = new float[gpuCount];
                var gpuTemps = new float[gpuCount];

                for (int i = 0; i < gpuCount; i++)
                {
                    gpuUsages[i] = NativeMonitor.GetGpuUsage(i);
                    gpuTemps[i] = NativeMonitor.GetGpuTemperature(i);
                }

                // Update non-visual components
                _mainWindow?.UpdateCpuUsage(cpuUsage);
                _mainWindow?.UpdateGpuUsage(gpuUsages);
                _mainWindow?.UpdateGraph(cpuUsage, gpuUsages);

                _trayManager?.UpdateIcon(cpuUsage, gpuUsages[0], gpuUsages.Length > 1 ? gpuUsages[1] : 0, gpuTemps[0], gpuTemps.Length > 1 ? gpuTemps[1] : 0);

                // Update UI thread with the information
                System.Windows.Application.Current.Dispatcher.Invoke(() =>
                {
                    if (_mainWindow != null)
                    {
                        _mainWindow.CpuUsage = cpuUsage;
                        _mainWindow.GpuUsage = string.Join(", ", gpuUsages.Select(g => $"{g:0}%"));
                        _mainWindow.GpuTemp = string.Join(", ", gpuTemps.Select(t => $"{t:0}°C"));
                    }

                    if (gpuUsages.Length > 0)
                    {
                        _widgetWindow.Gpu1Usage = $"{gpuUsages[0]:0}%";
                        _widgetWindow.Gpu1Color = GetUsageBrush(gpuUsages[0]);
                    }

                    if (gpuUsages.Length > 1)
                    {
                        _widgetWindow.Gpu2Usage = $"{gpuUsages[1]:0}%";
                        _widgetWindow.Gpu2Color = GetUsageBrush(gpuUsages[1]);
                    }
                });
            }
            catch (Exception ex)
            {
                Debug.WriteLine($"Monitoring error: {ex.Message}");
            }
        }

        private SolidColorBrush GetUsageBrush(float value)
        {
            return value switch
            {
                < 0 => System.Windows.Media.Brushes.Gray,
                < 50 => System.Windows.Media.Brushes.LimeGreen,
                < 75 => System.Windows.Media.Brushes.Orange,
                _ => System.Windows.Media.Brushes.OrangeRed
            };
        }

        protected override void OnExit(ExitEventArgs e)
        {
            _trayManager?.Dispose();
            base.OnExit(e);
        }
    }
}
