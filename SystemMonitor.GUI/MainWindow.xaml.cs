using System.Windows;
using System.Windows.Media;
using System.ComponentModel;
using System.Windows.Shapes;

namespace SystemMonitor.GUI
{

    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private float _cpuUsage;
        private const int MaxPoints = 100;
        private readonly Queue<float> _cpuHistory = new(MaxPoints);
        private Polyline? _graphLine;

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

        public string CpuUsageText => $"{CpuUsage:0.0}%";

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            InitializeGraph();
            Hide();
        }

        private void InitializeGraph()
        {
            _graphLine = new Polyline
            {
                StrokeThickness = 2,
                Points = new PointCollection()
            };
            CpuGraphCanvas.Children.Add(_graphLine);
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (_graphLine == null) return;

            // Create a copy of the current history to avoid modification during enumeration
            var historySnapshot = new Queue<float>(_cpuHistory);

            // Recreate graph
            CpuGraphCanvas.Children.Clear();
            InitializeGraph();

            // Redraw using the snapshot instead of modifying the original collection
            foreach (var usage in historySnapshot)
            {
                Dispatcher.Invoke(() => UpdateGraph(usage));
            }
        }

        public void UpdateGraph(float cpuUsage)
        {
            Dispatcher.Invoke(() =>
            {
                if (_graphLine == null) return;

                if (!CpuGraphCanvas.IsLoaded) return;

                if (_cpuHistory.Count >= MaxPoints)
                    _cpuHistory.Dequeue();
                _cpuHistory.Enqueue(cpuUsage);

                _graphLine.Points.Clear();
                _graphLine.Stroke = GetColorForUsage(cpuUsage);

                double xStep = CpuGraphCanvas.ActualWidth / MaxPoints;
                double yScale = CpuGraphCanvas.ActualHeight / 100;

                for (int i = 0; i < _cpuHistory.Count; i++)
                {
                    double x = i * xStep;
                    double y = CpuGraphCanvas.ActualHeight - (_cpuHistory.ElementAt(i) * yScale);
                    _graphLine.Points.Add(new System.Windows.Point(x, y));
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
    }
}