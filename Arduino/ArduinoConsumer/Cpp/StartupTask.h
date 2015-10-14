// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include "pch.h"

namespace ArduinoConsumerCpp
{
	[Windows::Foundation::Metadata::WebHostHidden]
    public ref class StartupTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);

    private:
        ~StartupTask();
    
    private:
        Windows::ApplicationModel::Background::BackgroundTaskDeferral ^_Deferral;

        bool _FanOn;
        const double _PotentiometerMin = 0;
        const double _PotentiometerMax = 700;
        const double _TemperatureRangeMin = 50;
        const double _TemperatureRangeMax = 100;
        double _TemperatureThreshold;
        double _CurrentTemperature;

        Windows::Devices::Pwm::PwmPin ^ _PwmPin;
        Windows::Devices::Adc::AdcChannel ^ _AdcChannel;
        Windows::Devices::Gpio::GpioPin ^ _LedPin;
        Windows::Devices::Gpio::GpioPin ^ _ButtonPin;
        Windows::Devices::I2c::I2cDevice ^ _I2cDevice;

        Windows::System::Threading::ThreadPoolTimer ^_PwmTimer;
        Windows::System::Threading::ThreadPoolTimer ^_AdcTimer;
        Windows::System::Threading::ThreadPoolTimer ^_I2cTimer;

        std::mutex _Mutex;
    };
}
