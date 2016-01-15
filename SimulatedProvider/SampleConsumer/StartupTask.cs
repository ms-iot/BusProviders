// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.Devices;
using Windows.Devices.Gpio;
using Windows.Devices.I2c;
using Windows.Devices.Adc;
using Windows.Devices.Pwm;
using Windows.Devices.Spi;
using Windows.System.Threading;
// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace SampleConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        const bool UseSimulatedBusses = true;
        private BackgroundTaskDeferral deferral;

        private GpioPin pin;
        private GpioPinValue pinValue = GpioPinValue.High;

        private I2cDevice tempSensor;
        ThreadPoolTimer timer;

        AdcChannel adcChannel;

        PwmPin pwmPin;

        SpiDevice spiDevice;

        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();


            if (UseSimulatedBusses)
            {
                LowLevelDevicesController.DefaultProvider = new SimulatedProvider.AggregateProvider();
            }

            var gpioController = await GpioController.GetDefaultAsync();
            pin = gpioController.OpenPin(5);

            var i2cController = await I2cController.GetDefaultAsync();
            tempSensor = i2cController.GetDevice(new I2cConnectionSettings(0x40));

            var adcController = await AdcController.GetDefaultAsync();
            adcChannel = adcController.OpenChannel(0);

            var pwmController = await PwmController.GetDefaultAsync();
            pwmPin = pwmController.OpenPin(6);
            pwmPin.SetActiveDutyCyclePercentage(0.5);
            pwmPin.Start();

            var spiController = await SpiController.GetDefaultAsync();
            spiDevice = spiController.GetDevice(new SpiConnectionSettings(0));

            timer = ThreadPoolTimer.CreateTimer(Timer_Tick, TimeSpan.FromMilliseconds(500));
        }

        private void Timer_Tick(ThreadPoolTimer timer)
        {
            //Flip the pins value
            pinValue = (pinValue == GpioPinValue.High) ? GpioPinValue.Low : GpioPinValue.High;
            pin.Write(pinValue);

            //Read the temperature
            byte[] tempCommand = new byte[1] { 0xE3 };
            byte[] tempData = new byte[2];
            tempSensor.WriteRead(tempCommand, tempData);
            var rawTempReading = tempData[0] << 8 | tempData[1];
            var tempRatio = rawTempReading / (float)65536;
            double temperature = (-46.85 + (175.72 * tempRatio)) * 9 / 5 + 32;
            System.Diagnostics.Debug.WriteLine("Temp: " + temperature.ToString());

            //Read the analog value
            var result = adcChannel.ReadRatio();
            System.Diagnostics.Debug.WriteLine("ADC Value:" + result.ToString());

           
            //Flip PWM duty cycle between 0.5 and 1       
            if (pwmPin.GetActiveDutyCyclePercentage() == 0.5)
            {
                pwmPin.SetActiveDutyCyclePercentage(1);
            }
            else
            {
                pwmPin.SetActiveDutyCyclePercentage(0.5);
            }

            //Send command to spi device
            byte[] spiCommand = new byte[1] { 0xE4 };
            spiDevice.Write(spiCommand);

        }
    }
}
