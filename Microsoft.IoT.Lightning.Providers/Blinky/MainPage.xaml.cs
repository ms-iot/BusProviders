using Microsoft.IoT.Lightning.Providers;
using System;
using Windows.Devices;
using Windows.Devices.Gpio;
using Windows.System.Threading;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;

namespace Blinky
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private readonly int LED_PIN = 5;
        private long currentTicks = 500; // every 500 msec
        private ThreadPoolTimer blinkyTimer;
        private int LEDStatus = 0;
        private bool blinkyStarted = false;
        private GpioPin pin = null;
        private GpioController gpioController = null;
        public MainPage()
        {
            this.InitializeComponent();
            InitGpio();
        }

        private async void InitGpio()
        {

            // Set the Lightning Provider as the default if Lightning driver is enabled on the target device
            // Otherwise, the inbox provider will continue to be the default
            if (LightningProvider.IsLightningEnabled)
            {
                LowLevelDevicesController.DefaultProvider = LightningProvider.GetAggregateProvider();
            }

            gpioController = await GpioController.GetDefaultAsync(); /* Get the default GPIO controller on the system */
            if (gpioController == null)
            {
                await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    BlinkyStartStop.Content = "No GPIO!";
                    BlinkyStartStop.IsEnabled = false;
                });
            }
        }

        private void Timer_Tick(ThreadPoolTimer timer)
        {
            if (pin == null)
                return;

            if (LEDStatus == 0)
            {
                LEDStatus = 1;
                pin.Write(GpioPinValue.High);
            }
            else
            {
                LEDStatus = 0;
                pin.Write(GpioPinValue.Low);
            }
        }

        private async void Start()
        {
            if (gpioController == null)
            {
                return;
            }

            pin = gpioController.OpenPin(LED_PIN, GpioSharingMode.Exclusive);
            pin.SetDriveMode(GpioPinDriveMode.Output);
            pin.Write(GpioPinValue.High);

            blinkyTimer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(currentTicks));

            await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                BlinkyStartStop.Content = "Stop Blinky";
            });

            blinkyStarted = true;

        }

        private async void Stop()
        {
            blinkyTimer.Cancel();
            blinkyStarted = false;

            if (pin != null)
            {
                pin.Write(GpioPinValue.Low);
                pin = null;
                await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    BlinkyStartStop.Content = "Start Blinky";
                });
            }
        }


        private void BlinkyStartStop_Click(object sender, RoutedEventArgs e)
        {
            if (pin == null)
            {
                Start();
            }
            else
            {
                Stop();
            }
        }

        private async void Delay_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (e.NewValue == Delay.Minimum)
                {
                    DelayText.Text = "BlinkyStopped";
                    Stop();
                }
                else
                {
                    DelayText.Text = e.NewValue + "ms";
                    currentTicks = (long)e.NewValue;
                    if (blinkyStarted)
                    {
                        Stop();
                        Start();
                    }
                }
            });
        }


    }
}
