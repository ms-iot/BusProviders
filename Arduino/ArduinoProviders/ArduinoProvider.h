// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include "ArduinoConnectionConfiguration.h"
#include "ArduinoConnection.h"

using namespace Windows::Devices;
using namespace Windows::Devices::Adc::Provider;
using namespace Windows::Devices::Pwm::Provider;
using namespace Windows::Devices::Gpio::Provider;
using namespace Windows::Devices::I2c::Provider;
using namespace Windows::Devices::Spi::Provider;

namespace ArduinoProviders
{
    public ref class ArduinoProvider sealed : public ILowLevelDevicesAggregateProvider
    {
    public:

        // Inherited via ILowLevelDevicesAggregateProvider
        virtual property IAdcControllerProvider ^ AdcControllerProvider
        {
            IAdcControllerProvider ^get();
        }
        virtual property IGpioControllerProvider ^ GpioControllerProvider
        {
            IGpioControllerProvider ^get();
        }
        virtual property II2cControllerProvider ^ I2cControllerProvider
        {
            II2cControllerProvider ^get();
        }
        virtual property IPwmControllerProvider ^ PwmControllerProvider
        {
            IPwmControllerProvider ^get();
        }
        virtual property ISpiControllerProvider ^ SpiControllerProvider
        {
            ISpiControllerProvider ^get();
        }

	public:
		ArduinoProvider()
		{
			_Created = true;
		}

		static property ArduinoConnectionConfiguration^ Configuration
		{
			ArduinoConnectionConfiguration^ get();
			void set(ArduinoConnectionConfiguration^ value);
		}

	private:
		static bool _Created;
	};
}