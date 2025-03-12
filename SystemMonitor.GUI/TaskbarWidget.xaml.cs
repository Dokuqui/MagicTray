using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using Forms = System.Windows.Forms;
using System.ComponentModel;

namespace SystemMonitor.GUI
{
    public partial class TaskbarWidget : Window
    {
        [DllImport("user32.dll")]
        private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

        [DllImport("user32.dll")]
        private static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

        [StructLayout(LayoutKind.Sequential)]
        private struct RECT
        {
            public int Left;
            public int Top;
            public int Right;
            public int Bottom;
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

        public static readonly DependencyProperty Gpu1UsageProperty =
            DependencyProperty.Register(
                nameof(Gpu1Usage),
                typeof(string),
                typeof(TaskbarWidget),
                new PropertyMetadata("0%"));

        public static readonly DependencyProperty Gpu1ColorProperty =
            DependencyProperty.Register(
                nameof(Gpu1Color),
                typeof(System.Windows.Media.Brush),
                typeof(TaskbarWidget),
                new PropertyMetadata(System.Windows.Media.Brushes.LimeGreen));

        public static readonly DependencyProperty Gpu2UsageProperty =
            DependencyProperty.Register(
                nameof(Gpu2Usage),
                typeof(string),
                typeof(TaskbarWidget),
                new PropertyMetadata("0%"));

        public static readonly DependencyProperty Gpu2ColorProperty =
            DependencyProperty.Register(
                nameof(Gpu2Color),
                typeof(System.Windows.Media.Brush),
                typeof(TaskbarWidget),
                new PropertyMetadata(System.Windows.Media.Brushes.LimeGreen));

        public string Gpu1Usage
        {
            get => (string)GetValue(Gpu1UsageProperty);
            set => SetValue(Gpu1UsageProperty, value);
        }

        public System.Windows.Media.Brush Gpu1Color
        {
            get => (System.Windows.Media.Brush)GetValue(Gpu1ColorProperty);
            set => SetValue(Gpu1ColorProperty, value);
        }

        public string Gpu2Usage
        {
            get => (string)GetValue(Gpu2UsageProperty);
            set => SetValue(Gpu2UsageProperty, value);
        }

        public System.Windows.Media.Brush Gpu2Color
        {
            get => (System.Windows.Media.Brush)GetValue(Gpu2ColorProperty);
            set => SetValue(Gpu2ColorProperty, value);
        }

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

            Loaded += OnLoaded;
        }

        private void OnLoaded(object? sender, RoutedEventArgs e)
        {
            PositionWindow();
        }

        private void PositionWindow()
        {
            var taskbarHandle = FindWindow("Shell_TrayWnd", null);
            if (taskbarHandle == IntPtr.Zero) return;

            if (!GetWindowRect(taskbarHandle, out var rect)) return;

            var screenWidth = SystemParameters.PrimaryScreenWidth;
            var screenHeight = SystemParameters.PrimaryScreenHeight;

            if (rect.Bottom - rect.Top > rect.Right - rect.Left)
            {
                // Vertical taskbar
                Left = rect.Left + 12;
                Top = rect.Top + 12;
            }
            else
            {
                // Horizontal taskbar
                Left = screenWidth - ActualWidth - 12;
                Top = screenHeight - ActualHeight - 12;
            }
        }
    }
}
