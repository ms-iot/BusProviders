#include "pch.h"
#include "PwmControllerProvider.h"
#include "PwmSoftwarePin.h"
#include <Windows.h>

using namespace PwmSoftware;
using namespace Platform;
using namespace Windows::Devices::Gpio;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;

PwmControllerProviderSoftware::PwmControllerProviderSoftware()
{
	gpioController = GpioController::GetDefault();
	pins = ref new Platform::Collections::Vector<PwmSoftwarePin^>(gpioController->PinCount);
	actualFrequency = MinFrequency;
	WorkItemHandler^ loop = ref new WorkItemHandler([&](IAsyncAction^ task2)
		{
			Loop();
		});
	ThreadPool::RunAsync(loop, WorkItemPriority::High);
}

PwmControllerProviderSoftware::~PwmControllerProviderSoftware()
{
	for (unsigned int i = 0; i < pins->Size; i++) 
	{
		PwmSoftwarePin^ pin = pins->GetAt(i);
		if (pin != nullptr)
		{
			pins->SetAt(i, nullptr);
		}
		
	}
}

void PwmControllerProviderSoftware::Loop() 
{
	LARGE_INTEGER ticks;
	QueryPerformanceFrequency(&ticks);
	TicksASecond = ticks.QuadPart;
	Platform::Collections::Vector<PwmSoftwarePin^>^ enabledPins = ref new Platform::Collections::Vector<PwmSoftwarePin^>(0);
	while(true)
	{
		enabledPins->Clear();
		for (int i = 0; i < pins->Size; i++)
		{	
			PwmSoftwarePin^ pin = pins->GetAt(i);
			if (pin != nullptr && pin->Enabled && pin->DutyCycle != 0)
			{
				bool inserted = false;
				for (int j = 0; j < enabledPins->Size; j++) 
				{
					PwmSoftwarePin^ compare = enabledPins->GetAt(j);
					if (compare->DutyCycle > pin->DutyCycle)
					{
						enabledPins->InsertAt(j, pin);
						inserted = true;
						break;
					}
				}
				if (!inserted) {
					enabledPins->Append(pin);
				}								
			}
		}
		for (int i = 0; i < enabledPins->Size; i++) 
		{
			PwmSoftwarePin^ pin = enabledPins->GetAt(i);				
			GpioPinValue value = (pin->InvertPolarity) ? GpioPinValue::Low : GpioPinValue::High;
			pin->GpioPin->Write(value);
		}
		LARGE_INTEGER startTicks;
		QueryPerformanceCounter(&startTicks);
		for (int i = 0; i < enabledPins->Size; i++) 
		{
			PwmSoftwarePin^ pin = enabledPins->GetAt(i);
			double targetTicks = startTicks.QuadPart + pin->DutyCycle*Period*TicksASecond / 1000.0;
			LARGE_INTEGER currentTicks;
			QueryPerformanceCounter(&currentTicks);
			while (currentTicks.QuadPart < targetTicks) {
				QueryPerformanceCounter(&currentTicks);
			}
			GpioPinValue pinValue = (pin->InvertPolarity) ? GpioPinValue::High : GpioPinValue::Low;
			pin->GpioPin->Write(pinValue);
		}
		double endCycleTicks = startTicks.QuadPart + Period*TicksASecond / 1000.0;
		LARGE_INTEGER currentTicks;
		QueryPerformanceCounter(&currentTicks);
		while (currentTicks.QuadPart < endCycleTicks) {
			QueryPerformanceCounter(&currentTicks);
		}
		
	}
}



void PwmControllerProviderSoftware::AcquirePin(int pin)
{
	if (pins->GetAt(pin) != nullptr)
	{
		throw ref new Platform::AccessDeniedException();
	}
	GpioPin^ gpioPin = gpioController->OpenPin(pin);
	gpioPin->SetDriveMode(GpioPinDriveMode::Output);
	pins->SetAt(pin, ref new PwmSoftwarePin(gpioPin));
}

void PwmControllerProviderSoftware::ReleasePin(int pin)
{
	if (pins->GetAt(pin) == nullptr) 
	{
		throw ref new Platform::AccessDeniedException();
	}
	GpioPin^ gpioPin = pins->GetAt(pin)->GpioPin;
	pins->SetAt(pin, nullptr);
	delete gpioPin;
}

void PwmControllerProviderSoftware::EnablePin(int pin)
{
	PwmSoftwarePin^ pwmPin = pins->GetAt(pin);
	if (pwmPin == nullptr)
	{
		throw ref new Platform::AccessDeniedException();
	}
	else 
	{
		pwmPin->Enabled = true;
	}
}

void PwmControllerProviderSoftware::DisablePin(int pin) 
{
	PwmSoftwarePin^ pwmPin = pins->GetAt(pin);
	if (pwmPin == nullptr)
	{
		throw ref new Platform::AccessDeniedException();
	}
	else
	{
		pwmPin->Enabled = false;
	}
}



void PwmControllerProviderSoftware::SetPulseParameters(int pin, double dutyCycle, bool invertPolarity) 
{
	PwmSoftwarePin^ pwmPin = pins->GetAt(pin);
	if (pwmPin == nullptr)
	{
		throw ref new Platform::AccessDeniedException();
	}
	pwmPin->DutyCycle = dutyCycle;
	pwmPin->InvertPolarity = invertPolarity;
}
