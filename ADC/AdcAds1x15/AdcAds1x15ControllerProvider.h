// Copyright (c) Microsoft. All rights reserved.
#pragma once
#include "AdcAds1x15Provider.h"

#define ADS1015_REG_POINTER_MASK (0x03)
#define ADS1015_REG_POINTER_CONVERT (0x00)
#define ADS1015_REG_POINTER_CONFIG (0x01)
#define ADS1015_REG_POINTER_LOWTHRESH (0x02)
#define ADS1015_REG_POINTER_HITHRESH (0x03)
/*=========================================================================*/

/*=========================================================================
CONFIG REGISTER
-----------------------------------------------------------------------*/
#define ADS1015_REG_CONFIG_OS_MASK (0x8000)
#define ADS1015_REG_CONFIG_OS_SINGLE (0x8000) // Write: Set to start a single-conversion
#define ADS1015_REG_CONFIG_OS_BUSY (0x0000) // Read: Bit = 0 when conversion is in progress
#define ADS1015_REG_CONFIG_OS_NOTBUSY (0x8000) // Read: Bit = 1 when device is not performing a conversion

#define ADS1015_REG_CONFIG_MUX_MASK (0x7000)
#define ADS1015_REG_CONFIG_MUX_DIFF_0_1 (0x0000) // Differential P = AIN0, N = AIN1 (default)
#define ADS1015_REG_CONFIG_MUX_DIFF_0_3 (0x1000) // Differential P = AIN0, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_1_3 (0x2000) // Differential P = AIN1, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_2_3 (0x3000) // Differential P = AIN2, N = AIN3
#define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000) // Single-ended AIN0
#define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000) // Single-ended AIN1
#define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000) // Single-ended AIN2
#define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000) // Single-ended AIN3

#define ADS1015_REG_CONFIG_PGA_MASK (0x0E00)
#define ADS1015_REG_CONFIG_PGA_6_144V (0x0000) // +/-6.144V range = Gain 2/3
#define ADS1015_REG_CONFIG_PGA_4_096V (0x0200) // +/-4.096V range = Gain 1
#define ADS1015_REG_CONFIG_PGA_2_048V (0x0400) // +/-2.048V range = Gain 2 (default)
#define ADS1015_REG_CONFIG_PGA_1_024V (0x0600) // +/-1.024V range = Gain 4
#define ADS1015_REG_CONFIG_PGA_0_512V (0x0800) // +/-0.512V range = Gain 8
#define ADS1015_REG_CONFIG_PGA_0_256V (0x0A00) // +/-0.256V range = Gain 16

#define ADS1015_REG_CONFIG_MODE_MASK (0x0100)
#define ADS1015_REG_CONFIG_MODE_CONTIN (0x0000) // Continuous conversion mode
#define ADS1015_REG_CONFIG_MODE_SINGLE (0x0100) // Power-down single-shot mode (default)

#define ADS1015_REG_CONFIG_DR_MASK (0x00E0) 
#define ADS1015_REG_CONFIG_DR_128SPS (0x0000) // 128 samples per second
#define ADS1015_REG_CONFIG_DR_250SPS (0x0020) // 250 samples per second
#define ADS1015_REG_CONFIG_DR_490SPS (0x0040) // 490 samples per second
#define ADS1015_REG_CONFIG_DR_920SPS (0x0060) // 920 samples per second
#define ADS1015_REG_CONFIG_DR_1600SPS (0x0080) // 1600 samples per second (default)
#define ADS1015_REG_CONFIG_DR_2400SPS (0x00A0) // 2400 samples per second
#define ADS1015_REG_CONFIG_DR_3300SPS (0x00C0) // 3300 samples per second

#define ADS1015_REG_CONFIG_CMODE_MASK (0x0010)
#define ADS1015_REG_CONFIG_CMODE_TRAD (0x0000) // Traditional comparator with hysteresis (default)
#define ADS1015_REG_CONFIG_CMODE_WINDOW (0x0010) // Window comparator

#define ADS1015_REG_CONFIG_CPOL_MASK (0x0008)
#define ADS1015_REG_CONFIG_CPOL_ACTVLOW (0x0000) // ALERT/RDY pin is low when active (default)
#define ADS1015_REG_CONFIG_CPOL_ACTVHI (0x0008) // ALERT/RDY pin is high when active

#define ADS1015_REG_CONFIG_CLAT_MASK (0x0004) // Determines if ALERT/RDY pin latches once asserted
#define ADS1015_REG_CONFIG_CLAT_NONLAT (0x0000) // Non-latching comparator (default)
#define ADS1015_REG_CONFIG_CLAT_LATCH (0x0004) // Latching comparator

#define ADS1015_REG_CONFIG_CQUE_MASK (0x0003)
#define ADS1015_REG_CONFIG_CQUE_1CONV (0x0000) // Assert ALERT/RDY after one conversions
#define ADS1015_REG_CONFIG_CQUE_2CONV (0x0001) // Assert ALERT/RDY after two conversions
#define ADS1015_REG_CONFIG_CQUE_4CONV (0x0002) // Assert ALERT/RDY after four conversions
#define ADS1015_REG_CONFIG_CQUE_NONE (0x0003) // Disable the comparator and put ALERT/RDY in high state (default)


namespace AdcAds1x15
{
	ref class AdcAds1x15ControllerProvider sealed :
		public Windows::Devices::Adc::Provider::IAdcControllerProvider
	{
		// ADC1x15 exposes 4 channels in singleended  		
		// as well as differential mode.  		
		//  SingleEnded Mode:   		
		//       Channel 0 => AIN0  		
		//       Channel 1 => AIN1  		
		//       Channel 2 => AIN2  		
		
		//       Channel 3 => AIN3  		
		//  Differential Mode:   		
		//       Channel 0 => (AINp -> AIN0, AINn -> AIN1)  		
		//       Channel 1 => (AINp -> AIN0, AINn -> AIN3)  		
		//       Channel 2 => (AINp -> AIN1, AINn -> AIN3)  		
		//       Channel 3 => (AINp -> AIN2, AINn -> AIN3)  		
		static const unsigned char channelCount = 4;  

		//  		
		// Slave Address is hard wired as 0x48.  		
		//  		
		static const int controllerI2cSlaveAddress = 0x48;  

		//  		
		// ADS1015 provides an alternate I2c address using which the  		
		// controller can be reset(write to this address)  		
		//  		
		static const int resetControllerI2cSlaveAddress = 0x0;  

		int minValue = -32768;
		int maxValue = 32767;
		int resolutionInBits = 16;
		int conversionDelay = 10;
		int bitShift = 0;

		~AdcAds1x15ControllerProvider();

	public:
		AdcAds1x15ControllerProvider(Ads1x15Type selectedType);


		// Inherited via IAdcControllerProvider  		
		virtual property int MinValue  
		{
			int get() { return this->minValue; }
		}

		virtual property int MaxValue
		{
			int get() { return this->maxValue; }
		}

		virtual property int ResolutionInBits
		{
			int get() { return this->resolutionInBits; }
		}

		virtual property int ChannelCount
		{
			int get() { return this->channelCount; }
		}

		virtual property Windows::Devices::Adc::Provider::ProviderAdcChannelMode ChannelMode
		{
			Windows::Devices::Adc::Provider::ProviderAdcChannelMode get()
			{
				return this->channelMode;
			}
			void set(Windows::Devices::Adc::Provider::ProviderAdcChannelMode mode)
			{
				if (mode != Windows::Devices::Adc::Provider::ProviderAdcChannelMode::Differential &&
					mode != Windows::Devices::Adc::Provider::ProviderAdcChannelMode::SingleEnded)
				{
					throw ref new Platform::InvalidArgumentException();
				}
				this->channelMode = mode;
			}
		}

		virtual bool IsChannelModeSupported(Windows::Devices::Adc::Provider::ProviderAdcChannelMode mode);
		virtual void AcquireChannel(int channel);
		virtual void ReleaseChannel(int channel);

		virtual int ReadValue(int channel);

	private:
		void InitializeController();
		void ResetController();
		int ReadSingleValue(int channel);
		int ReadDifferentialValue(int channel);
		std::atomic_uchar channelStatus;
		
	
		Windows::Devices::I2c::I2cDevice^ controller;  
 		
		Windows::Devices::I2c::I2cDevice^ resetController;  

		// Adc Channel Mode  		
		Windows::Devices::Adc::Provider::ProviderAdcChannelMode channelMode;  

	};
}
