// Copyright (c) Microsoft. All rights reserved.
#pragma once  

namespace PwmPCA9685
{
	ref class PwmControllerProviderPCA9685 sealed :
		public Windows::Devices::Pwm::Provider::IPwmControllerProvider
	{
		static const unsigned char pinCount = 16;  

		//  		// Slave Address is hard wired as 0x40.  		//  		
		static const int controllerI2cSlaveAddress = 0x40;  

		//  		// PCA9685 provides an alternate I2c address using which the  		
		// controller can be reset(write to this address)  		//  		
		static const int resetControllerI2cSlaveAddress = 0x0;  

		static const unsigned short minFrequency = 40;
		static const unsigned short maxFrequency = 1000;
		static const unsigned long clockFrequency = 25000000L;

		static const unsigned pulseResolution = 4096;
		static const BYTE defaultPreScale = 0x1E;


	public:
		PwmControllerProviderPCA9685();
		virtual ~PwmControllerProviderPCA9685();

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
			int get() { return pinCount; }
		}

		virtual double SetDesiredFrequency(double frequency);
		virtual void AcquirePin(int pin);
		virtual void ReleasePin(int pin);
		virtual void EnablePin(int pin);
		virtual void DisablePin(int pin);
		virtual void SetPulseParameters(int pin, double dutyCycle, bool invertPolarity);

		void SetDevice(Windows::Devices::I2c::I2cDevice^ device) { controller = device; }
		void SetResetDevice(Windows::Devices::I2c::I2cDevice^ device) { resetController = device; }

	private:
		void InitializeController();
		void ResetController();
		BYTE SleepController();
		void RestartController(BYTE mode1);

		double actualFrequency;
		BYTE preScale;
		bool pinAcquired[pinCount];

		// PCA9685 controller  		
		Windows::Devices::I2c::I2cDevice^ controller;  

		// PCA9685 reset controller  		
		Windows::Devices::I2c::I2cDevice^ resetController;  

		// Lock to serialize access to pinAcquired  		
		//TODO: Microsoft::WRL::Wrappers::SRWLock pinLock;  
	};
}
