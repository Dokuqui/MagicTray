using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Forms;
using System.ComponentModel;

namespace SystemMonitor.GUI
{
    public partial class TaskbarWidget : Window
    {
        [DllImport("user32.dll")]
        private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

        [DllImport("user32.dll")]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        private struct RECT
        {
            public int Left; public int Top; public int Right; public int Bottom;
        }

        public static readonly DependencyProperty CpuUsageProperty =
            DependencyProperty.Register(
                nameof(CpuUsage),
                typeof(string),
                typeof(TaskbarWidget),
                new PropertyMetadata("0%"));

        public static readonly DependencyProperty CpuColorProperty =
            DependencyProperty.Register(
                nameof(CpuColor),
                typeof(System.Windows.Media.Brush),
                typeof(TaskbarWidget),
                new PropertyMetadata(System.Windows.Media.Brushes.LimeGreen));

        public string CpuUsage
        {
            get => (string)GetValue(CpuUsageProperty);
            set => SetValue(CpuUsageProperty, value);
        }

        public System.Windows.Media.Brush CpuColor
        {
            get => (System.Windows.Media.Brush)GetValue(CpuColorProperty);
            set => SetValue(CpuColorProperty, value);
        }

        public TaskbarWidget()
        {
            InitializeComponent();
            DataContext = this;

            Loaded += (s, e) => PositionWindow();
        }

        private void PositionWindow()
        {
            var taskbarHandle = FindWindow("Shell_TrayWnd", null);
            if (!GetWindowRect(taskbarHandle, out RECT rect)) return;

            var dpi = VisualTreeHelper.GetDpi(this);
            double scaling = dpi.DpiScaleX;

            // Get taskbar position
            if (rect.Bottom - rect.Top > rect.Right - rect.Left)
            {
                // Vertical taskbar (left or right)
                // Position the widget away from the taskbar to avoid system tray area
                Left = (rect.Left / scaling) + 10;  // Add margin from taskbar's left edge
                Top = (rect.Top / scaling) + 10;  // Place it slightly above the taskbar
            }
            else
            {
                // Horizontal taskbar (bottom or top)
                // Place the widget more towards the left side to avoid the system tray area
                Left = SystemParameters.WorkArea.Right - ActualWidth - 60;  // Move further left
                Top = SystemParameters.WorkArea.Bottom - ActualHeight - 10;  // Slightly above the taskbar
            }
        }


    }
}
