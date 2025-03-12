using System.Windows;
using System.Windows.Media;
using System.ComponentModel;
using System.Windows.Shapes;
using System.Windows.Controls;
using System.Diagnostics;

namespace SystemMonitor.GUI
{
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private float _cpuUsage;
        private string _gpuUsage;
        private string _gpuTemp;

        private const int MaxPoints = 100;
        private readonly Queue<float> _cpuHistory = new(MaxPoints);
        private Polyline? _graphLine;
        private readonly Queue<float> _gpuHistory = new(MaxPoints);
        private Polyline? _gpuGraphLine;

        public float CpuUsage
        {
            get => _cpuUsage;
            set
            {
                _cpuUsage = value;
                OnPropertyChanged(nameof(CpuUsage));
                OnPropertyChanged(nameof(CpuUsageText));
            }
        }

        public string GpuUsage
        {
            get => _gpuUsage;
            set
            {
                _gpuUsage = value;
                OnPropertyChanged(nameof(GpuUsage));
                OnPropertyChanged(nameof(GpuUsageText));
            }
        }

        public string GpuTemp
        {
            get => _gpuTemp;
            set
            {
                _gpuTemp = value;
                OnPropertyChanged(nameof(GpuTemp));
                OnPropertyChanged(nameof(GpuTempText));
            }
        }

        public string GpuTempText => $"{GpuTemp}";
        public string GpuUsageText => $"{GpuUsage}";
        public string CpuUsageText => $"{CpuUsage:0.0}%";

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            InitializeGraph();
            InitializeGpuGraph();
            Hide();

            _gpuUsage = "0%";
            _gpuTemp = "0°C";
        }

        private void InitializeGraph()
        {
            _graphLine = new Polyline
            {
                StrokeThickness = 2,
                Stroke = System.Windows.Media.Brushes.LimeGreen,
                StrokeDashCap = PenLineCap.Round,
                StrokeLineJoin = PenLineJoin.Round,
                StrokeStartLineCap = PenLineCap.Round,
                StrokeEndLineCap = PenLineCap.Round
            };

            var gradient = new LinearGradientBrush(
                Colors.LimeGreen,
                Colors.Transparent,
                new System.Windows.Point(0.5, 1),
                new System.Windows.Point(0.5, 0));

            _graphLine.Stroke = gradient;
            CpuGraphCanvas.Children.Add(_graphLine);
        }

        private void InitializeGpuGraph()
        {
            _gpuGraphLine = new Polyline
            {
                StrokeThickness = 2,
                Stroke = System.Windows.Media.Brushes.DodgerBlue,
                StrokeDashCap = PenLineCap.Round,
                StrokeLineJoin = PenLineJoin.Round,
                StrokeStartLineCap = PenLineCap.Round,
                StrokeEndLineCap = PenLineCap.Round
            };

            var gradient = new LinearGradientBrush(
                Colors.DodgerBlue,
                Colors.Transparent,
                new System.Windows.Point(0.5, 1),
                new System.Windows.Point(0.5, 0));

            _gpuGraphLine.Stroke = gradient;
            GpuGraphCanvas.Children.Add(_gpuGraphLine);
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            UpdateGraphLayout(CpuGraphCanvas, _graphLine, _cpuHistory);
            UpdateGraphLayout(GpuGraphCanvas, _gpuGraphLine, _gpuHistory);
        }

        private void UpdateGraphLayout(Canvas canvas, Polyline? graphLine, Queue<float> history)
        {
            if (graphLine == null || !canvas.IsLoaded) return;

            var historySnapshot = new Queue<float>(history);
            canvas.Children.Clear();
            graphLine.Points.Clear();

            if (canvas == CpuGraphCanvas)
                InitializeGraph();
            else
                InitializeGpuGraph();

            foreach (var usage in historySnapshot)
            {
                Dispatcher.Invoke(() => UpdateGraphInternal(usage, canvas, graphLine, history));
            }
        }

        public void UpdateGraph(float cpuUsage, float[] gpuUsages)
        {
            UpdateCpuGraph(cpuUsage);
            foreach (var gpuUsage in gpuUsages)
            {
                UpdateGpuGraph(gpuUsage);
            }
        }

        private void UpdateCpuGraph(float cpuUsage)
        {
            UpdateGraphInternal(cpuUsage, CpuGraphCanvas, _graphLine, _cpuHistory);
        }

        private void UpdateGpuGraph(float gpuUsage)
        {
            UpdateGraphInternal(gpuUsage, GpuGraphCanvas, _gpuGraphLine, _gpuHistory);
        }

        private void UpdateGraphInternal(float usage, Canvas canvas, Polyline? graphLine, Queue<float> history)
        {
            Dispatcher.Invoke(() =>
            {
                if (graphLine == null || !canvas.IsLoaded) return;

                if (history.Count >= MaxPoints)
                    history.Dequeue();
                history.Enqueue(usage);

                graphLine.Stroke = GetColorForUsage(usage);
                graphLine.Points.Clear();

                double xStep = canvas.ActualWidth / MaxPoints;
                double yScale = canvas.ActualHeight / 100;

                for (int i = 0; i < history.Count; i++)
                {
                    double x = i * xStep;
                    double y = canvas.ActualHeight - (history.ElementAt(i) * yScale);
                    graphLine.Points.Add(new System.Windows.Point(x, y));
                }
            });
        }

        private SolidColorBrush GetColorForUsage(float usage)
        {
            return usage switch
            {
                < 50 => System.Windows.Media.Brushes.LimeGreen,
                < 75 => System.Windows.Media.Brushes.Orange,
                _ => System.Windows.Media.Brushes.OrangeRed
            };
        }

        public event PropertyChangedEventHandler? PropertyChanged;

        protected virtual void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public void OpenSettings(object sender, RoutedEventArgs e)
        {
            System.Windows.MessageBox.Show("Settings coming soon!");
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);
            e.Cancel = true;
            this.Hide();
        }

        public void ShowWindowAgain()
        {
            if (!this.IsVisible)
            {
                this.Show();
            }
        }

        public void UpdateCpuUsage(float cpuUsage)
        {
            CpuUsage = cpuUsage;
        }

        public void UpdateGpuUsage(float[] gpuUsages)
        {
            GpuUsage = string.Join(", ", gpuUsages.Select(g => $"{g:0}%"));
        }
    }
}
