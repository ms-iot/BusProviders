using Microsoft.IoT.Lightning.Providers;
using System;
using System.Threading.Tasks;
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
        // Use GPIO pin 5, which is available on Minnowboard Max, Raspberry Pi2 and Pi3
        // On Other boards, the pin number should be changed
        private readonly int LED_PIN = 5;
        private long currentTicks = 500; // every 500 msec
        private ThreadPoolTimer blinkyTimer;
        private bool blinkyStarted = false;
        private GpioPin pin = null;

        public MainPage()
        {
            this.InitializeComponent();
            InitGpio();
        }

        private void InitGpio()
        {

            // Set the Lightning Provider as the default if Lightning driver is enabled on the target device
            // Otherwise, the inbox provider will continue to be the default
            if (LightningProvider.IsLightningEnabled)
            {
                // Set Lightning as the default provider
                LowLevelDevicesController.DefaultProvider = LightningProvider.GetAggregateProvider();
                GpioStatus.Text = "GPIO Using Lightning Provider";
            }
            else
            {
                GpioStatus.Text = "GPIO Using Default Provider";
            }

            var gpioController = GpioController.GetDefault(); /* Get the default GPIO controller on the system */
            if (gpioController == null)
            {
                GpioStatus.Text += "\nNo GPIO Controller found!";
                BlinkyStartStop.IsEnabled = false;
            }

            pin = gpioController.OpenPin(LED_PIN, GpioSharingMode.Exclusive);
            pin.SetDriveMode(GpioPinDriveMode.Output);
        }

        private void Timer_Tick(ThreadPoolTimer timer)
        {
            if (pin == null)
            {
                return;
            }

            if (pin.Read() == GpioPinValue.High)
            {
                pin.Write(GpioPinValue.Low);
            }
            else
            {
                pin.Write(GpioPinValue.High);
            }
        }

        private async void Start()
        {
            blinkyTimer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(currentTicks));
            blinkyStarted = true;

            await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                BlinkyStartStop.Content = "Stop Blinky";
            });
        }

        private async Task Stop()
        {
            blinkyTimer.Cancel();
            blinkyStarted = false;

            if (pin != null)
            {
                pin.Write(GpioPinValue.Low);

                await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    BlinkyStartStop.Content = "Start Blinky";
                });
            }
        }


        private async void BlinkyStartStop_Click(object sender, RoutedEventArgs e)
        {
            if (blinkyStarted)
            {
                await Stop();
            }
            else
            {
                Start();
            }
        }

        private async void Delay_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            await CoreWindow.GetForCurrentThread().Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                if (e.NewValue <= Delay.Minimum)
                {
                    DelayText.Text = "BlinkyStopped";
                    await Stop();
                }
                else
                {
                    DelayText.Text = e.NewValue + "ms";
                    currentTicks = (long)e.NewValue;
                    if (blinkyStarted)
                    {
                        await Stop();
                        Start();
                    }
                }
            });
        }


    }
}
