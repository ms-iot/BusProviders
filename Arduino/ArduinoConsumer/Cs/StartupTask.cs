// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.ApplicationModel.Background;
using Windows.Devices;
using Windows.Devices.I2c;
using Windows.Devices.Gpio;
using Windows.Devices.Pwm;
using Windows.Devices.Adc;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace ArduinoConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
            _FanOn = true;

            ArduinoProviders.ArduinoProvider.Configuration = 
                new ArduinoProviders.ArduinoConnectionConfiguration("VID_2341", "PID_0043", 57600);
            LowLevelDevicesController.DefaultProvider = new ArduinoProviders.ArduinoProvider();

            //
            // Use PwmController to turn on and off a servo on PWM pin #3
            // based on whether the ambient temperature is higher than the
            // temperature set by the potentiometer attached via ADC below.
            //
            var pwmController = await PwmController.GetDefaultAsync();
            _PwmPin = pwmController.OpenPin(3);
            _PwmPin.SetActiveDutyCyclePercentage(0);
            _PwmPin.Start();
            _PwmTimer = ThreadPoolTimer.CreatePeriodicTimer(
                (timer) =>
                {
                    if (!_FanOn || _TemperatureThreshold >= _CurrentTemperature)
                    {
                        // if the fan is off or the ambient temperature is at or below the threshold, turn off the fan
                        _PwmPin.SetActiveDutyCyclePercentage(0.0);
                    }
                    else
                    {
                        // if the ambient temperature is above the threshold, turn on the fan
                        _PwmPin.SetActiveDutyCyclePercentage(2.0 / (1000.0 / pwmController.ActualFrequency));
                    }
                },
                TimeSpan.FromMilliseconds(50));

            //
            // Use AdcController to read the value from a potentiometer
            // on ADC pin #1.  This value will converted to a temperature
            // threshold between 50 and 100 degrees farenheit.
            //
            var adcController = await AdcController.GetDefaultAsync();
            _AdcChannel = adcController.OpenChannel(1);
            _AdcTimer = ThreadPoolTimer.CreatePeriodicTimer(
                (timer) =>
                {
                    if (_FanOn)
                    {
                        // set new threshold between min and max based on potentiometer reading
                        _TemperatureThreshold =
                            Math.Round(
                                _TemperatureRangeMin +
                                (_TemperatureRangeMax - _TemperatureRangeMin) * (double)_AdcChannel.ReadValue() / (double)(_PotentiometerMax - _PotentiometerMin));
                    }
                },
                TimeSpan.FromMilliseconds(50));

            //
            // Use I2cController to read the ambient temperature from a HTU21D
            // sensor connected to the I2c pins.
            //
            var i2cController = await I2cController.GetDefaultAsync();
            var i2cConnectionSettings = new I2cConnectionSettings(0x40);
            _I2cDevice = i2cController.GetDevice(i2cConnectionSettings);
            _I2cTimer = ThreadPoolTimer.CreatePeriodicTimer(
                (timer) =>
                {
                    if (_FanOn)
                    {
                        lock (this)
                        {
                            var command = new byte[1] { 0xE3 };
                            var data = new byte[2];
                            var status = _I2cDevice.WriteReadPartial(command, data);

                            var rawReading = data[0] << 8 | data[1];
                            var ratio = rawReading / (float)65536;
                            _CurrentTemperature = Math.Round((-46.85 + (175.72 * ratio)) * 9.0 / 5.0 + 32.0);
                        }
                    }
                },
                TimeSpan.FromMilliseconds(50));

            //
            // Use GpioController to toggle a LED on GPIO pin 7 when our
            // fan system is on/off.
            //
            var gpioController = await GpioController.GetDefaultAsync();
            _LedPin = gpioController.OpenPin(7);
            _LedPin.SetDriveMode(GpioPinDriveMode.Output);
            _LedPin.Write(_FanOn ? GpioPinValue.Low : GpioPinValue.High);

            //
            // Use GpioController to read the on/off state of our fan
            // system from a button connected to GPIO pin 8
            //
            _ButtonPin = gpioController.OpenPin(8);
            _ButtonPin.SetDriveMode(GpioPinDriveMode.Input);
            _ButtonPin.ValueChanged +=
                (sender, e) =>
                {
                    if (e.Edge == GpioPinEdge.FallingEdge)
                    {
                        _FanOn = !_FanOn;
                        _LedPin.Write(_FanOn ? GpioPinValue.Low : GpioPinValue.High);
                    }
                };
        }

        ~StartupTask()
        {
            if (_PwmTimer != null) _PwmTimer.Cancel();
            if (_AdcTimer != null) _AdcTimer.Cancel();
            if (_I2cTimer != null) _I2cTimer.Cancel();
            if (_PwmPin != null) _PwmPin.Stop();
        }


        BackgroundTaskDeferral deferral;

        bool _FanOn = true;
        const double _PotentiometerMin = 0;
        const double _PotentiometerMax = 700;
        const double _TemperatureRangeMin = 50;
        const double _TemperatureRangeMax = 100;
        double _TemperatureThreshold = 72.0;
        double _CurrentTemperature = 0.0;

        GpioPin _LedPin;
        GpioPin _ButtonPin;
        PwmPin _PwmPin;
        AdcChannel _AdcChannel;
        I2cDevice _I2cDevice;

        ThreadPoolTimer _PwmTimer;
        ThreadPoolTimer _AdcTimer;
        ThreadPoolTimer _I2cTimer;
    }
}
