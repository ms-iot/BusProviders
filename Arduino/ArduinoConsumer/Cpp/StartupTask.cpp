// Copyright (c) Microsoft. All rights reserved.

#include "pch.h"
#include "StartupTask.h"

using namespace ArduinoConsumerCpp;

using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;
using namespace Windows::Devices::Pwm;
using namespace Windows::Devices::Adc;
using namespace Windows::Devices::Gpio;
using namespace Windows::Devices::I2c;
using namespace Windows::System::Threading;
using namespace concurrency;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    _Deferral = taskInstance->GetDeferral();
    _FanOn = true;

    ArduinoProviders::ArduinoProvider::Configuration =
        ref new ArduinoProviders::ArduinoConnectionConfiguration("VID_2341", "PID_0043", 57600);
    Windows::Devices::LowLevelDevicesController::DefaultProvider = ref new ArduinoProviders::ArduinoProvider();

    TimeSpan interval;
    interval.Duration = 50 * 1000 * 10;


    //
    // Use PwmController to turn on and off a servo on PWM pin #3
    // based on whether the ambient temperature is higher than the
    // temperature set by the potentiometer attached via ADC below.
    //
    concurrency::create_task(PwmController::GetDefaultAsync()).then([this, interval](PwmController^ pwmController) {
        _PwmPin = pwmController->OpenPin(3);
        _PwmPin->SetActiveDutyCyclePercentage(0.0);
        _PwmPin->Start();

        _PwmTimer = ThreadPoolTimer::CreatePeriodicTimer(
            ref new TimerElapsedHandler([this, pwmController](ThreadPoolTimer ^timer)
        {
            std::lock_guard<std::mutex> lock(_Mutex);

            if (!_FanOn || _TemperatureThreshold >= _CurrentTemperature)
            {
                // if the fan is off or the ambient temperature is at or below the threshold, turn off the fan
                _PwmPin->SetActiveDutyCyclePercentage(0.0);
            }
            else
            {
                // if the ambient temperature is above the threshold, turn on the fan
                _PwmPin->SetActiveDutyCyclePercentage(2.0 / (1000.0 / pwmController->ActualFrequency));
            }
        }),
            interval);
    });

    //
    // Use AdcController to read the value from a potentiometer
    // on ADC pin #1.  This value will converted to a temperature
    // threshold between 50 and 100 degrees farenheit.
    //
    concurrency::create_task(AdcController::GetDefaultAsync()).then([this, interval](AdcController^ adcController) {
        _AdcChannel = adcController->OpenChannel(1);
        _AdcTimer = ThreadPoolTimer::CreatePeriodicTimer(
            ref new TimerElapsedHandler([this](ThreadPoolTimer ^timer)
        {
            if (_FanOn)
            {
                std::lock_guard<std::mutex> lock(_Mutex);

                // set new threshold between min and max based on potentiometer reading
                _TemperatureThreshold =
                    std::round(
                        double(_TemperatureRangeMin) +
                        double(_TemperatureRangeMax - _TemperatureRangeMin) * double(_AdcChannel->ReadValue()) / double(_PotentiometerMax - _PotentiometerMin));
            }
        }),
            interval);
    });

    //
    // Use I2cController to read the ambient temperature from a HTU21D
    // sensor connected to the I2c pins.
    //
    concurrency::create_task(I2cController::GetDefaultAsync()).then([this, interval](I2cController^ i2cController) {
        auto i2cConnectionSettings = ref new Windows::Devices::I2c::I2cConnectionSettings(0x40);
        _I2cDevice = i2cController->GetDevice(i2cConnectionSettings);
        _I2cTimer = ThreadPoolTimer::CreatePeriodicTimer(
            ref new TimerElapsedHandler([this](ThreadPoolTimer ^timer)
        {
            if (_FanOn)
            {
                std::lock_guard<std::mutex> lock(_Mutex);

                auto command = ref new Platform::Array<byte>(1);
                command[0] = 0xE3;

                auto data = ref new Array<byte>(2);
                _I2cDevice->WriteRead(command, data);

                auto rawReading = data[0] << 8 | data[1];
                auto ratio = rawReading / (float)65536;
                _CurrentTemperature = std::round((-46.85 + (175.72 * ratio)) * 9.0 / 5.0 + 32.0);
            }
        }),
            interval);
    });

    //
    // Use GpioController to toggle a LED on GPIO pin 7 when our
    // fan system is on/off.
    //
    concurrency::create_task(GpioController::GetDefaultAsync()).then([this, interval](GpioController^ gpioController) {
        _LedPin = gpioController->OpenPin(7);
        _LedPin->SetDriveMode(Windows::Devices::Gpio::GpioPinDriveMode::Output);
        _LedPin->Write(
            _FanOn ?
            Windows::Devices::Gpio::GpioPinValue::Low :
            Windows::Devices::Gpio::GpioPinValue::High
        );

        //
        // Use GpioController to read the on/off state of our fan
        // system from a button connected to GPIO pin 8
        //
        _ButtonPin = gpioController->OpenPin(8);
        _ButtonPin->SetDriveMode(Windows::Devices::Gpio::GpioPinDriveMode::Input);
        _ButtonPin->ValueChanged +=
            ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Gpio::GpioPin^, Windows::Devices::Gpio::GpioPinValueChangedEventArgs^>(
                [this](
                    Windows::Devices::Gpio::GpioPin ^sender,
                    Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^e)
            {
                if (e->Edge == Windows::Devices::Gpio::GpioPinEdge::FallingEdge)
                {
                    _FanOn = !_FanOn;
                    _LedPin->Write(
                        _FanOn ?
                        Windows::Devices::Gpio::GpioPinValue::Low :
                        Windows::Devices::Gpio::GpioPinValue::High
                    );
                }

            });
        });
}

StartupTask::~StartupTask()
{
    if (_PwmTimer) _PwmTimer->Cancel();
    if (_AdcTimer) _AdcTimer->Cancel();
    if (_I2cTimer) _I2cTimer->Cancel();
    if (_PwmPin) _PwmPin->Stop();
}