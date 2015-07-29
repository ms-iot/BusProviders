// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include <chrono>  
#include <thread>  
#include <cctype>  
#include <atomic>  
#include "AdcAds1x15ControllerProvider.h"  
#include "AdcAds1x15Provider.h"

using namespace AdcAds1x15;
using namespace Platform;
using namespace Windows::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::I2c;
using namespace Windows::Devices::Adc::Provider;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace concurrency;

namespace {
	enum : BYTE {
		ADC1015_REG_CONVERSION = 0x0,
		ADC1015_REG_CONFIG = 0x01,
		ADC1015_REG_LOTHRESH = 0x02,
		ADC1015_REG_HITHRESH = 0x02,
	};

	const int CONVERSION_POLL_MAXTRIES = 3;
}

AdcAds1x15ControllerProvider::AdcAds1x15ControllerProvider(Ads1x15Type type)
{
	auto aqs = I2cDevice::GetDeviceSelector();
	auto deviceOp = DeviceInformation::FindAllAsync(aqs);
	if (type == Ads1x15Type::Ads1115) {
		minValue = -32768;
		maxValue = 32767;
		resolutionInBits = 16;
		conversionDelay = 10;
		bitShift = 0;
	}
	else {
		minValue = -4096;
		maxValue = 4095;
		resolutionInBits = 12;
		conversionDelay = 3;
		bitShift = 4;
	}

	DeviceInformationCollection^ devices = create_task(DeviceInformation::FindAllAsync(aqs)).get();

	DeviceInformation^ di = devices->GetAt(0);
	//    
	// No I2C Controllers.  	
	//  	
	if (di == nullptr)  
		throw ref new Platform::FailureException(L"No I2c Controllers detected");

	I2cConnectionSettings^ controllerSettings =
		ref new I2cConnectionSettings(
			AdcAds1x15ControllerProvider::controllerI2cSlaveAddress);
	controllerSettings->BusSpeed = I2cBusSpeed::FastMode;
	controllerSettings->SharingMode = I2cSharingMode::Shared;

	controller = create_task(I2cDevice::FromIdAsync(di->Id, controllerSettings)).get();

	//  	
	// Controller is acquired exclusively. So we may get  	
	// nullptr if controller is already acquired by a   	
	// different thread/app.  	
	//  	
	if (!controller)  
		throw ref new Platform::Exception(HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION));

	//  	
	// I2c Device object for Software reset of ADS1015.  	
	//  	
	I2cConnectionSettings^ resetControllerSettings =  
	ref new I2cConnectionSettings(
		AdcAds1x15ControllerProvider::resetControllerI2cSlaveAddress);
	resetControllerSettings->BusSpeed = I2cBusSpeed::FastMode;
	resetControllerSettings->SharingMode = I2cSharingMode::Shared;
	resetController = create_task(I2cDevice::FromIdAsync(di->Id, controllerSettings)).get();

	if (!resetController)
		throw ref new Platform::FailureException();

	InitializeController();

	channelStatus = 0;
}

AdcAds1x15ControllerProvider::~AdcAds1x15ControllerProvider()
{
	ResetController();
}

bool AdcAds1x15ControllerProvider::IsChannelModeSupported(ProviderAdcChannelMode mode)
{
	if (mode != ProviderAdcChannelMode::Differential &&
		mode != ProviderAdcChannelMode::SingleEnded)
	{
		throw ref new Platform::InvalidArgumentException();
	}

	return true;
}

void AdcAds1x15ControllerProvider::AcquireChannel(int channel)
{
	std::atomic_uchar oldVal = channelStatus.fetch_or(1 << channel);

	if ((oldVal & (1 << channel)) != 0)
		throw ref new Platform::AccessDeniedException();
}

void AdcAds1x15ControllerProvider::ReleaseChannel(int channel)
{
	channelStatus.fetch_and(~(1 << channel));
}

int AdcAds1x15ControllerProvider::ReadSingleValue(int channel) {
	BYTE address;
	BYTE readBuf[2];
	BYTE writeBuf[3];
	Platform::Array<BYTE>^ addressBuffer = Platform::ArrayReference<BYTE>(&address, 1);
	Platform::Array<BYTE>^ readBuffer = Platform::ArrayReference<BYTE>(readBuf, 2);
	Platform::Array<BYTE>^ writeBuffer = Platform::ArrayReference<BYTE>(writeBuf, 3);
	unsigned short config =
		ADS1015_REG_CONFIG_CQUE_NONE |
		ADS1015_REG_CONFIG_CLAT_NONLAT |
		ADS1015_REG_CONFIG_CPOL_ACTVLOW |
		ADS1015_REG_CONFIG_CMODE_TRAD |
		ADS1015_REG_CONFIG_DR_1600SPS |
		ADS1015_REG_CONFIG_MODE_SINGLE;
	
	config |= ADS1015_REG_CONFIG_PGA_6_144V;
	switch (channel) {
	case (0) :
		config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
		break;
	case (1) :
		config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
		break;
	case (2) :
		config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
		break;
	case (3) :
		config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
		break;
	}

	config |= ADS1015_REG_CONFIG_OS_SINGLE;

	writeBuffer[0] = ADS1015_REG_POINTER_CONFIG;
	writeBuffer[1] = (config >> 8);
	writeBuffer[2] = config & 0xFF;
	controller->Write(writeBuffer);
	std::this_thread::sleep_for(std::chrono::milliseconds(conversionDelay ));

	addressBuffer[0] = ADS1015_REG_POINTER_CONVERT;
	controller->WriteRead(addressBuffer, readBuffer);
	int16 value = readBuffer[0] << 8 | readBuffer[1];
	value = value >> bitShift;
	return value;

}

int AdcAds1x15ControllerProvider::ReadDifferentialValue(int channel) {
	BYTE address;
	BYTE readBuf[2];
	BYTE writeBuf[3];
	Platform::Array<BYTE>^ addressBuffer = Platform::ArrayReference<BYTE>(&address, 1);
	Platform::Array<BYTE>^ readBuffer = Platform::ArrayReference<BYTE>(readBuf, 2);
	Platform::Array<BYTE>^ writeBuffer = Platform::ArrayReference<BYTE>(writeBuf, 3);
	unsigned short config =
		ADS1015_REG_CONFIG_CQUE_NONE |
		ADS1015_REG_CONFIG_CLAT_NONLAT |
		ADS1015_REG_CONFIG_CPOL_ACTVLOW |
		ADS1015_REG_CONFIG_CMODE_TRAD |
		ADS1015_REG_CONFIG_DR_1600SPS |
		ADS1015_REG_CONFIG_MODE_SINGLE;

	config |= ADS1015_REG_CONFIG_PGA_6_144V;
	switch (channel) {
	case (0) :
		config |= ADS1015_REG_CONFIG_MUX_DIFF_0_1;
		break;
	case (1) :
		config |= ADS1015_REG_CONFIG_MUX_DIFF_0_3;
		break;
	case (2) :
		config |= ADS1015_REG_CONFIG_MUX_DIFF_1_3;
		break;
	case (3) :
		config |= ADS1015_REG_CONFIG_MUX_DIFF_2_3;
		break;
	}

	config |= ADS1015_REG_CONFIG_OS_SINGLE;

	writeBuffer[0] = ADS1015_REG_POINTER_CONFIG;
	writeBuffer[1] = (config >> 8);
	writeBuffer[2] = config & 0xFF;
	controller->Write(writeBuffer);
	std::this_thread::sleep_for(std::chrono::milliseconds(conversionDelay));

	addressBuffer[0] = ADS1015_REG_POINTER_CONVERT;
	controller->WriteRead(addressBuffer, readBuffer);
	uint16 value = readBuffer[0] << 8 | readBuffer[1];
	value = value >> bitShift;
	if (bitShift == 4)
	{
		//Negative number, need to sign extend
		if (value > 0x07FF) {
			value |= 0xF000;
		}
	}
	return value;

}

int AdcAds1x15ControllerProvider::ReadValue(int channel)
{
	if (channelMode == ProviderAdcChannelMode::SingleEnded)
	{
		return ReadSingleValue(channel);
	}
	else {
		return ReadDifferentialValue(channel);
	}
	
}

void AdcAds1x15ControllerProvider::InitializeController()
{
	if (!controller) return;

	ResetController();

	//   	
	// This sample supports only singleshot mode.  	
	// More Initialization code can go in here if we need  	
	// to support other needed of this controller.  	
	//  
}  

	void AdcAds1x15ControllerProvider::ResetController()
	{
		Platform::Array<BYTE>^ resetCommand = ref new Platform::Array<BYTE>(1);
		resetCommand[0] = 0x06;
		resetController->Write(resetCommand);
	}
