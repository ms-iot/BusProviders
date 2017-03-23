// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoGpioDeviceProvider.h"
#include "ArduinoConnection.h"
#include <ppltasks.h>

using namespace concurrency;

using namespace ArduinoProviders;
using namespace Platform::Collections;

void ArduinoGpioPinProvider::SetDriveMode(
    ProviderGpioPinDriveMode value
    )
{
    if (_DriveMode != value)
    {
        switch (value)
        {
        case ProviderGpioPinDriveMode::Input:
            _Arduino->pinMode(_PinNumber, PinMode::INPUT);
            break;
        case ProviderGpioPinDriveMode::Output:
            _Arduino->pinMode(_PinNumber, PinMode::OUTPUT);
            break;
        default:
            throw ref new Platform::Exception(E_NOTIMPL, L"Unsupported Gpio Pin DriveMode");
        }

        _DriveMode = value;
    }
}

void ArduinoGpioPinProvider::Write(
    ProviderGpioPinValue value
    )
{
    _Arduino->digitalWrite(
        _PinNumber, 
        (value == ProviderGpioPinValue::High) ? 
            PinState::HIGH :
            PinState::LOW);
}

ProviderGpioPinValue ArduinoGpioPinProvider::Read()
{
    PinState state = _Arduino->digitalRead(_PinNumber);
    return (state == PinState::HIGH) ?
        ProviderGpioPinValue::High :
        ProviderGpioPinValue::Low;
}

void ArduinoGpioPinProvider::OnValueChanged(GpioPinProviderValueChangedEventArgs ^args)
{
    ValueChanged(this, args);
}

void ArduinoGpioPinProvider::Initialize()
{
    _Arduino->DigitalPinUpdated +=
        ref new Microsoft::Maker::RemoteWiring::DigitalPinUpdatedCallback(this, &ArduinoGpioPinProvider::OnDigitalPinUpdated);

    auto mode = _Arduino->getPinMode(_PinNumber);
    SetDriveMode(
        (mode == PinMode::INPUT) ?
        ProviderGpioPinDriveMode::Input :
        ProviderGpioPinDriveMode::Output);

}

void ArduinoGpioPinProvider::OnDigitalPinUpdated(unsigned char pin, PinState value)
{
    if (pin == _PinNumber)
    {
        ProviderGpioPinEdge edge = (value == PinState::LOW) ? 
            ProviderGpioPinEdge::FallingEdge : 
            ProviderGpioPinEdge::RisingEdge;
        ValueChanged(this, ref new GpioPinProviderValueChangedEventArgs(edge));
    }
}

ArduinoGpioControllerProvider::ArduinoGpioControllerProvider()
{
    _Arduino = create_task(ArduinoConnection::GetArduinoConnectionAsync()).get();
}

IGpioPinProvider^ ArduinoGpioControllerProvider::OpenPinProvider(
    int pinNumber,
    ProviderGpioSharingMode sharingMode
    )
{
    return ref new ArduinoGpioPinProvider(_Arduino, pinNumber, sharingMode);
}

IVectorView<IGpioControllerProvider^>^ ArduinoGpioProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IGpioControllerProvider^>();
    controllerCollection->Append(ref new ArduinoGpioControllerProvider());
    return controllerCollection->GetView();
}


