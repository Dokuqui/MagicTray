using System;
using System.Windows;
using System.Drawing;
using System.Windows.Forms;
using Forms = System.Windows.Forms;

namespace SystemMonitor.GUI
{
    public class TrayIconManager : IDisposable
    {
        private readonly Forms.NotifyIcon _trayIcon;
        private readonly MainWindow _mainWindow;

        public TrayIconManager(MainWindow mainWindow)
        {
            _mainWindow = mainWindow;

            _trayIcon = new Forms.NotifyIcon
            {
                Icon = CreateDefaultIcon(),
                Visible = true
            };

            CreateContextMenu();
            SetEventHandlers();
        }

        private Icon CreateDefaultIcon()
        {
            using var bitmap = new Bitmap(16, 16);
            using (var g = Graphics.FromImage(bitmap))
            {
                g.Clear(Color.Black);
                g.FillRectangle(Brushes.Gray, 0, 8, 16, 8);
            }
            return Icon.FromHandle(bitmap.GetHicon());
        }

        private void CreateContextMenu()
        {
            _trayIcon.ContextMenuStrip = new Forms.ContextMenuStrip();
            _trayIcon.ContextMenuStrip.Items.Add("Open", null, (s, e) => ShowMainWindow());
            _trayIcon.ContextMenuStrip.Items.Add("Exit", null, (s, e) => System.Windows.Application.Current.Shutdown());
        }

        private void SetEventHandlers()
        {
            _trayIcon.DoubleClick += (s, e) => ShowMainWindow();
            _trayIcon.MouseMove += (s, e) => UpdateTooltip();
        }

        public void UpdateIcon(float cpuUsage)
        {
            using var bitmap = new Bitmap(16, 16);
            using (var g = Graphics.FromImage(bitmap))
            {
                g.Clear(Color.Black);
                var fillHeight = (int)(16 * (cpuUsage / 100f));
                g.FillRectangle(GetUsageColor(cpuUsage), 0, 16 - fillHeight, 16, fillHeight);
            }
            _trayIcon.Icon = Icon.FromHandle(bitmap.GetHicon());
            _trayIcon.Text = $"CPU: {cpuUsage:0.0}%";
        }

        private void UpdateTooltip()
        {
            _trayIcon.Text = $"CPU: {_mainWindow.CpuUsage:0.0}%\n" +
                             $"Temp: --\n" +
                             $"RAM: --";
        }

        private Brush GetUsageColor(float usage)
        {
            return usage switch
            {
                < 50 => Brushes.LimeGreen,
                < 75 => Brushes.Orange,
                _ => Brushes.OrangeRed
            };
        }

        private void ShowMainWindow()
        {
            _mainWindow.Show();
            _mainWindow.WindowState = WindowState.Normal;
            _mainWindow.Activate();
        }

        public void Dispose()
        {
            _trayIcon?.Dispose();
        }
    }
}