using System;
using System.Windows;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Media;
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
                g.Clear(System.Drawing.Color.Black);

                var mediaColor = System.Windows.Media.Color.FromArgb(255, 128, 128, 128);
                var drawingColor = System.Drawing.Color.FromArgb(mediaColor.A, mediaColor.R, mediaColor.G, mediaColor.B);

                g.FillRectangle(new System.Drawing.SolidBrush(drawingColor), 0, 8, 16, 8);
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

        public void UpdateIcon(float cpuUsage, float gpu1Usage, float gpu2Usage, float gpu1Temp, float gpu2Temp)
        {
            using var bitmap = new Bitmap(32, 32);
            using (var g = Graphics.FromImage(bitmap))
            {
                g.Clear(System.Drawing.Color.Black);

                // Draw CPU bar (same as before)
                var cpuHeight = (int)(32 * (cpuUsage / 100f));
                g.FillRectangle(GetUsageColor(cpuUsage), 0, 32 - cpuHeight, 16, cpuHeight);

                // Draw first GPU bar (gpu1)
                var gpu1Height = (int)(32 * (gpu1Usage / 100f));
                g.FillRectangle(GetUsageColor(gpu1Usage), 16, 32 - gpu1Height, 8, gpu1Height);

                // Draw second GPU bar (gpu2)
                var gpu2Height = (int)(32 * (gpu2Usage / 100f));
                g.FillRectangle(GetUsageColor(gpu2Usage), 24, 32 - gpu2Height, 8, gpu2Height);
            }
            _trayIcon.Icon = Icon.FromHandle(bitmap.GetHicon());
            _trayIcon.Text = $"CPU: {cpuUsage:0.0}%\n" +
                             $"GPU1: {gpu1Usage:0.0}% Temp: {gpu1Temp:0.0}°C\n" +
                             $"GPU2: {gpu2Usage:0.0}% Temp: {gpu2Temp:0.0}°C";
        }

        private void UpdateTooltip()
        {
            _trayIcon.Text = $"CPU: {_mainWindow.CpuUsage:F1}%\n" +
                             $"GPU: {_mainWindow.GpuUsage:F1}%\n" +
                             $"Temp: {_mainWindow.GpuTemp:F1}°C\n" +
                             $"RAM: --";
        }

        private System.Drawing.Brush GetUsageColor(float usage)
        {
            return usage switch
            {
                < 50 => System.Drawing.Brushes.LimeGreen,
                < 75 => System.Drawing.Brushes.Orange,
                _ => System.Drawing.Brushes.OrangeRed
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
