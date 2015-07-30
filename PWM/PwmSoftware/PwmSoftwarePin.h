// Copyright (c) Microsoft. All rights reserved.
#pragma once

namespace PwmSoftware
{
	
	ref class  PwmSoftwarePin 
	{
	internal:
		property Windows::Devices::Gpio::GpioPin^ GpioPin 
		{
			Windows::Devices::Gpio::GpioPin^ get() { return gpioPin; }
		}

		property bool Enabled
		{
			bool get() { return enabled; }
			void set(bool value) { enabled = value; }
		}

		property bool InvertPolarity
		{
			bool get() { return invertPolarity; }
			void set(bool value) { invertPolarity = value; }
		}

		property double DutyCycle
		{
			double get() { return dutyCycle; }
			void set(double value) { dutyCycle = value; }
		}

		PwmSoftwarePin(Windows::Devices::Gpio::GpioPin^ pin) 
		{
			gpioPin = pin;
			dutyCycle = 0;
			enabled = false;
		}

	private:
		
		Windows::Devices::Gpio::GpioPin^ gpioPin;
		bool enabled = false;
		bool invertPolarity = false;
		double dutyCycle = 0;
	};
}
#pragma once
