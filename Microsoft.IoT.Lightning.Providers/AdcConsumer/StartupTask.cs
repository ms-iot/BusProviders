// Copyright (c) Microsoft. All rights reserved.

using Microsoft.IoT.Lightning.Providers;
using System;
using Windows.ApplicationModel.Background;
using Windows.Devices.Adc;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace AdcConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        AdcChannel adcChannel;

        ThreadPoolTimer timer;

        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            var deferral = taskInstance.GetDeferral();

            if (!LightningProvider.IsLightningEnabled)
            {
                // Lightning provider is required for this sample
                deferral.Complete();
                return;
            }

            // Use the PAC9685 PWM provider
            AdcController controller = (await AdcController.GetControllersAsync(LightningAdcProvider.GetAdcProvider()))[0]; // select LightningMCP3008AdcControllerProvider

            adcChannel = controller.OpenChannel(0);

            timer = ThreadPoolTimer.CreatePeriodicTimer(this.Tick, TimeSpan.FromMilliseconds(1000));
        }

        void Tick(ThreadPoolTimer sender)
        {
            // Assuming there's a temp sensor at channel 0
            int reading = adcChannel.ReadValue();

            double voltage = ( reading * 5.0) / 1024.0;
            double temperatureC = 100 * (voltage - 0.5);
            double temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;     // now convert to Fahrenheit
            System.Diagnostics.Debug.WriteLine(string.Format("Channel 0: Voltage: {0}; Temp (F): {1}; Temp (C): {2}", voltage, temperatureC, temperatureF));
        }

    }
}
