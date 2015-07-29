#include "pch.h"  
#include <chrono>  
#include <thread>  
#include <cctype>  
#include "PwmControllerProvider.h"  

using namespace PwmPCA9685;
using namespace Platform;
using namespace Windows::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::I2c;
using namespace Windows::Devices::Pwm::Provider;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace concurrency;

namespace
{
	// Required register Addresses  	
enum : BYTE {  
	PCA9685_REG_MODE1 = 0x0,
		PCA9685_REG_MODE2 = 0x1,
		PCA9685_REG_LED0_ON_L = 0x06,
		PCA9685_REG_LED0_ON_H = 0x07,
		PCA9685_REG_LED0_OFF_L = 0x08,
		PCA9685_REG_LED0_OFF_H = 0x09,
		PCA9685_REG_LED1_ON_L = 0x0A,
		PCA9685_REG_LED1_ON_H = 0x0B,
		PCA9685_REG_LED1_OFF_L = 0x0C,
		PCA9685_REG_LED1_OFF_H = 0x0D,
		PCA9685_REG_LED2_ON_L = 0x0E,
		PCA9685_REG_LED2_ON_H = 0x0F,
		PCA9685_REG_LED2_OFF_L = 0x10,
		PCA9685_REG_LED2_OFF_H = 0x11,
		PCA9685_REG_LED3_ON_L = 0x12,
		PCA9685_REG_LED3_ON_H = 0x13,
		PCA9685_REG_LED3_OFF_L = 0x14,
		PCA9685_REG_LED3_OFF_H = 0x15,
		PCA9685_REG_LED4_ON_L = 0x16,
		PCA9685_REG_LED4_ON_H = 0x17,
		PCA9685_REG_LED4_OFF_L = 0x18,
		PCA9685_REG_LED4_OFF_H = 0x19,
		PCA9685_REG_LED5_ON_L = 0x1A,
		PCA9685_REG_LED5_ON_H = 0x1B,
		PCA9685_REG_LED5_OFF_L = 0x1C,
		PCA9685_REG_LED5_OFF_H = 0x1D,
		PCA9685_REG_LED6_ON_L = 0x1E,
		PCA9685_REG_LED6_ON_H = 0x1F,
		PCA9685_REG_LED6_OFF_L = 0x20,
		PCA9685_REG_LED6_OFF_H = 0x21,
		PCA9685_REG_LED7_ON_L = 0x22,
		PCA9685_REG_LED7_ON_H = 0x23,
		PCA9685_REG_LED7_OFF_L = 0x24,
		PCA9685_REG_LED7_OFF_H = 0x25,
		PCA9685_REG_LED8_ON_L = 0x26,
		PCA9685_REG_LED8_ON_H = 0x27,
		PCA9685_REG_LED8_OFF_L = 0x28,
		PCA9685_REG_LED8_OFF_H = 0x29,
		PCA9685_REG_LED9_ON_L = 0x2A,
		PCA9685_REG_LED9_ON_H = 0x2B,
		PCA9685_REG_LED9_OFF_L = 0x2C,
		PCA9685_REG_LED9_OFF_H = 0x2D,
		PCA9685_REG_LED10_ON_L = 0x2E,
		PCA9685_REG_LED10_ON_H = 0x2F,
		PCA9685_REG_LED10_OFF_L = 0x30,
		PCA9685_REG_LED10_OFF_H = 0x31,
		PCA9685_REG_LED11_ON_L = 0x32,
		PCA9685_REG_LED11_ON_H = 0x33,
		PCA9685_REG_LED11_OFF_L = 0x34,
		PCA9685_REG_LED11_OFF_H = 0x35,
		PCA9685_REG_LED12_ON_L = 0x36,
		PCA9685_REG_LED12_ON_H = 0x37,
		PCA9685_REG_LED12_OFF_L = 0x38,
		PCA9685_REG_LED12_OFF_H = 0x39,
		PCA9685_REG_LED13_ON_L = 0x3A,
		PCA9685_REG_LED13_ON_H = 0x3B,
		PCA9685_REG_LED13_OFF_L = 0x3C,
		PCA9685_REG_LED13_OFF_H = 0x3D,
		PCA9685_REG_LED14_ON_L = 0x3E,
		PCA9685_REG_LED14_ON_H = 0x3F,
		PCA9685_REG_LED14_OFF_L = 0x40,
		PCA9685_REG_LED14_OFF_H = 0x41,
		PCA9685_REG_LED15_ON_L = 0x42,
		PCA9685_REG_LED15_ON_H = 0x43,
		PCA9685_REG_LED15_OFF_L = 0x44,
		PCA9685_REG_LED15_OFF_H = 0x45,
		PCA9685_REG_ALL_ON_L = 0xFA,
		PCA9685_REG_ALL_ON_H = 0xFB,
		PCA9685_REG_ALL_OFF_L = 0xFC,
		PCA9685_REG_ALL_OFF_H = 0xFD,
		PCA9685_REG_PRESCALE = 0xFE
};
struct PinControlRegister {
	BYTE OnLow;
	BYTE OnHigh;
	BYTE OffLow;
	BYTE OffHigh;
};

// Exposed pins.  	
const PinControlRegister PwmPinRegs[16] =  
{
	{ PCA9685_REG_LED0_ON_L, PCA9685_REG_LED0_ON_H, PCA9685_REG_LED0_OFF_L, PCA9685_REG_LED0_OFF_H },
	{ PCA9685_REG_LED1_ON_L, PCA9685_REG_LED1_ON_H , PCA9685_REG_LED1_OFF_L , PCA9685_REG_LED1_OFF_H },
	{ PCA9685_REG_LED2_ON_L, PCA9685_REG_LED2_ON_H , PCA9685_REG_LED2_OFF_L , PCA9685_REG_LED2_OFF_H },
	{ PCA9685_REG_LED3_ON_L, PCA9685_REG_LED3_ON_H , PCA9685_REG_LED3_OFF_L , PCA9685_REG_LED3_OFF_H },
	{ PCA9685_REG_LED4_ON_L, PCA9685_REG_LED4_ON_H , PCA9685_REG_LED4_OFF_L , PCA9685_REG_LED4_OFF_H },
	{ PCA9685_REG_LED5_ON_L, PCA9685_REG_LED5_ON_H , PCA9685_REG_LED5_OFF_L , PCA9685_REG_LED5_OFF_H },
	{ PCA9685_REG_LED6_ON_L, PCA9685_REG_LED6_ON_H, PCA9685_REG_LED6_OFF_L, PCA9685_REG_LED6_OFF_H },
	{ PCA9685_REG_LED7_ON_L, PCA9685_REG_LED7_ON_H , PCA9685_REG_LED7_OFF_L , PCA9685_REG_LED7_OFF_H },
	{ PCA9685_REG_LED8_ON_L, PCA9685_REG_LED8_ON_H , PCA9685_REG_LED8_OFF_L , PCA9685_REG_LED8_OFF_H },
	{ PCA9685_REG_LED9_ON_L, PCA9685_REG_LED9_ON_H , PCA9685_REG_LED9_OFF_L , PCA9685_REG_LED9_OFF_H },
	{ PCA9685_REG_LED10_ON_L, PCA9685_REG_LED10_ON_H , PCA9685_REG_LED10_OFF_L , PCA9685_REG_LED10_OFF_H },
	{ PCA9685_REG_LED11_ON_L, PCA9685_REG_LED11_ON_H , PCA9685_REG_LED11_OFF_L , PCA9685_REG_LED11_OFF_H },
	{ PCA9685_REG_LED12_ON_L, PCA9685_REG_LED12_ON_H , PCA9685_REG_LED12_OFF_L , PCA9685_REG_LED12_OFF_H },
	{ PCA9685_REG_LED13_ON_L, PCA9685_REG_LED13_ON_H , PCA9685_REG_LED13_OFF_L , PCA9685_REG_LED13_OFF_H },
	{ PCA9685_REG_LED14_ON_L, PCA9685_REG_LED14_ON_H , PCA9685_REG_LED14_OFF_L , PCA9685_REG_LED14_OFF_H },
	{ PCA9685_REG_LED15_ON_L, PCA9685_REG_LED15_ON_H , PCA9685_REG_LED15_OFF_L , PCA9685_REG_LED15_OFF_H },

};

}

PwmControllerProviderPCA9685::PwmControllerProviderPCA9685() :
	preScale(defaultPreScale)
{
	auto aqs = I2cDevice::GetDeviceSelector();
	auto deviceOp = DeviceInformation::FindAllAsync(aqs);

	create_task(deviceOp).then([this](DeviceInformationCollection^ devices) {

		DeviceInformation^ di = devices->GetAt(0);
		I2cConnectionSettings^ controllerSettings =
			ref new I2cConnectionSettings(
				PwmControllerProviderPCA9685::controllerI2cSlaveAddress);
		controllerSettings->BusSpeed = I2cBusSpeed::FastMode;
		controllerSettings->SharingMode = I2cSharingMode::Exclusive;

		auto i2cDeviceOp = I2cDevice::FromIdAsync(di->Id, controllerSettings);
		create_task(i2cDeviceOp).then([this](I2cDevice^ device) {
			this->SetDevice(device);
		}).wait();

		//  		// I2c Device object for Software reset of PCA9685.  		//  		
		I2cConnectionSettings^ resetControllerSettings =  
		ref new I2cConnectionSettings(
			PwmControllerProviderPCA9685::controllerI2cSlaveAddress);
		resetControllerSettings->SlaveAddress = 0x0;
		i2cDeviceOp = I2cDevice::FromIdAsync(di->Id, resetControllerSettings);
		create_task(i2cDeviceOp).then([this](I2cDevice^ device) {
			this->SetResetDevice(device);
		}).wait();

	}).wait();

	//  	// Controller is acquired exclusively. So we may get  	
	// nullptr if controller is already acquired by a   	
	// different thread/app.  	//  	
	if (!controller)  
	throw ref new Platform::FailureException();

	InitializeController();

	// Initialize PinData  	
	for (int i = 0; i < pinCount; i++)  
	{
		pinAcquired[i] = false;
	}
}


PwmControllerProviderPCA9685::~PwmControllerProviderPCA9685()
{
	ResetController();
}

double PwmControllerProviderPCA9685::SetDesiredFrequency(double frequency)
{
	if (frequency < minFrequency || frequency > maxFrequency)
	{
		throw ref new Platform::InvalidArgumentException();
	}

	preScale = (unsigned char)round((clockFrequency / (frequency * pulseResolution))) - 1;
	actualFrequency = clockFrequency / double((preScale + 1) * pulseResolution);

	BYTE mode1 = SleepController();

	Platform::Array<BYTE>^ buffer = ref new Platform::Array<BYTE>(2);
	// Set PRE_SCALE  	
	buffer[0] = PCA9685_REG_PRESCALE;  
	buffer[1] = preScale;
	controller->Write(buffer);

	// Restart  	
	RestartController(mode1);  

	return actualFrequency;
}

void PwmControllerProviderPCA9685::AcquirePin(int pin)
{
	//TODO: auto lock = pinLock.LockExclusive();

	if (pinAcquired[pin])
		throw ref new Platform::AccessDeniedException();

	pinAcquired[pin] = true;
}

void PwmControllerProviderPCA9685::ReleasePin(int pin)
{
	//TODO: auto lock = pinLock.LockExclusive();
	pinAcquired[pin] = false;
}

BYTE PwmControllerProviderPCA9685::SleepController()
{
	Platform::Array<BYTE>^ writeBuf = ref new Platform::Array<BYTE>(2);
	Platform::Array<BYTE>^ mode = ref new Platform::Array<BYTE>(1);
	Platform::Array<BYTE>^ modeAddr = ref new Platform::Array<BYTE>(1);

	// Read MODE1 register  	
	modeAddr[0] = BYTE(PCA9685_REG_MODE1);  
	controller->WriteRead(modeAddr, mode);

	// Disable Oscillator  	
	writeBuf[0] = PCA9685_REG_MODE1;  
	writeBuf[1] = mode[0] | (1 << 4);
	controller->Write(writeBuf);

	// Wait for more than 500us to stabilize.  	
	std::this_thread::sleep_for(std::chrono::milliseconds(1));  

	return mode[0];
}

void PwmControllerProviderPCA9685::RestartController(BYTE mode1)
{
	Platform::Array<BYTE>^ writeBuf = ref new Platform::Array<BYTE>(2);

	writeBuf[0] = PCA9685_REG_MODE1;
	writeBuf[1] = mode1;
	controller->Write(writeBuf);

	// Wait for more than 500us to stabilize.  	
	std::this_thread::sleep_for(std::chrono::milliseconds(1));  
}

void PwmControllerProviderPCA9685::InitializeController()
{
	if (!controller) return;

	ResetController();

	Platform::Array<BYTE>^ writeBuf = ref new Platform::Array<BYTE>(2);

	SleepController();

	// Set PRE_SCALE to default  	
	writeBuf[0] = PCA9685_REG_PRESCALE;  
	writeBuf[1] = defaultPreScale;
	controller->Write(writeBuf);

	// Set ActualFrequency to default(200Hz)  	
	actualFrequency = (unsigned short)round(float(clockFrequency) / float((preScale + 1) * pulseResolution));  


	writeBuf[0] = PCA9685_REG_ALL_OFF_H;
	writeBuf[1] = 0;
	controller->Write(writeBuf);

	writeBuf[0] = PCA9685_REG_ALL_ON_H;
	writeBuf[1] = (1 << 4);
	controller->Write(writeBuf);

	RestartController(0xA1);
}

void PwmPCA9685::PwmControllerProviderPCA9685::ResetController()
{
	Platform::Array<BYTE>^ resetCommand = ref new Platform::Array<BYTE>(1);
	resetCommand[0] = 0x06;
	resetController->Write(resetCommand);
}

void PwmControllerProviderPCA9685::EnablePin(int pin)
{
	//  	
	// Since we are using the totem-pole mode, we just need to  	
	// make sure that that pin is not fully OFF(bit 4 of LEDn_OFF_H should be zero).  	
	// We set the OFF and ON counter to zero so that the pin is held Low.  	
	// Subsequent calls to SetPulseParameters should set the pulse width.  	//  	
	Platform::Array<BYTE>^ buffer = ref new Platform::Array<BYTE>(5);  
	buffer[0] = PwmPinRegs[pin].OnLow;
	buffer[1] = buffer[2] = buffer[3] = buffer[4] = 0x0;
	controller->Write(buffer);
}

void PwmControllerProviderPCA9685::DisablePin(int pin)
{
	//  	// Since we are using the totem-pole mode, we just need to  	
	// make sure that that pin is fully OFF.  	//  	
	Platform::Array<BYTE>^ buffer = ref new Platform::Array<BYTE>(2);  
	buffer[0] = PwmPinRegs[pin].OffHigh;
	buffer[1] = 0x1 << 4;
	controller->Write(buffer);
}

void PwmControllerProviderPCA9685::SetPulseParameters(int pin, double dutyCycle, bool invertPolarity)
{
	Platform::Array<BYTE>^ buffer = ref new Platform::Array<BYTE>(5);
	unsigned short onRatio = (unsigned short)round(dutyCycle * (pulseResolution - 1));

	//  	// Set the initial Address. AI flag is ON and hence  	
	// address will auto-increment after each byte.  	//  	
	buffer[0] = PwmPinRegs[pin].OnLow;  

	if (invertPolarity)
	{
		onRatio = pulseResolution - onRatio;
		buffer[1] = onRatio & 0xFF;
		buffer[2] = (onRatio & 0xFF00) >> 8;
		buffer[3] = buffer[4] = 0;
	}
	else
	{
		buffer[1] = buffer[2] = 0;
		buffer[3] = onRatio & 0xFF;
		buffer[4] = (onRatio & 0xFF00) >> 8;
	}
	controller->Write(buffer);
}
