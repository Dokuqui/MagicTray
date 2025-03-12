using System.Windows;
using System.Windows.Threading;


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

                // Update non-visual components
                _mainWindow?.UpdateCpuUsage(cpuUsage);
                _mainWindow?.UpdateGraph(cpuUsage);
                _trayManager?.UpdateIcon(cpuUsage);

                // Update visual components on UI thread
                System.Windows.Application.Current.Dispatcher.Invoke(() =>
                {
                    if (_widgetWindow != null)
                    {
                        _widgetWindow.CpuUsage = $"{cpuUsage:0}%";
                        _widgetWindow.CpuColor = cpuUsage switch
                        {
                            < 50 => System.Windows.Media.Brushes.LimeGreen,
                            < 75 => System.Windows.Media.Brushes.Orange,
                            _ => System.Windows.Media.Brushes.OrangeRed
                        };
                    }
                });
            }
            catch
            {
                // Handle errors
            }
        }


        protected override void OnExit(ExitEventArgs e)
        {
            _trayManager?.Dispose();
            base.OnExit(e);
        }
    }
}