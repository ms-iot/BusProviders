// Copyright (c) Microsoft. All rights reserved.
#pragma once  
#include "PwmSoftwarePin.h"
namespace PwmSoftware
{
	typedef Windows::Foundation::Collections::IVectorView<Windows::Devices::Gpio::GpioPin^> IGpioPinCollection;

	ref class PwmControllerProviderSoftware sealed :
		public Windows::Devices::Pwm::Provider::IPwmControllerProvider
	{


		static const unsigned short minFrequency = 40;
		static const unsigned short maxFrequency = 1000;
		static const unsigned long clockFrequency = 25000000L;

		static const unsigned pulseResolution = 4096;


	public:
		PwmControllerProviderSoftware();
		virtual ~PwmControllerProviderSoftware();

		// Inherited via IPwmControllerProvider  		
		virtual property double ActualFrequency
		{
			double get() { return actualFrequency; }
		}
		virtual property double MaxFrequency
		{
			double get() { return double(maxFrequency); }
		}
		virtual property double MinFrequency
		{
			double get() { return double(minFrequency); }
		}
		virtual property int PinCount
		{
			int get() { return pins->Size; }
		}

		virtual double SetDesiredFrequency(double frequency) {
			actualFrequency = frequency;
			return actualFrequency;
		}
		virtual void AcquirePin(int pin);
		virtual void ReleasePin(int pin);
		virtual void EnablePin(int pin);
		virtual void DisablePin(int pin);
		virtual void SetPulseParameters(int pin, double dutyCycle, bool invertPolarity);


	

	private:
		double actualFrequency;
		Platform::Collections::Vector<PwmSoftware::PwmSoftwarePin^>^ pins;;
		Windows::Devices::Gpio::GpioController^ gpioController;
		property double Period
		{  
			double get() { return 1000.0 / ActualFrequency; }
		}
		void Loop();
		int64 TicksASecond;

		// Lock to serialize access to pinAcquired  		
		//TODO: Microsoft::WRL::Wrappers::SRWLock pinLock;  
	};
}
#pragma once
