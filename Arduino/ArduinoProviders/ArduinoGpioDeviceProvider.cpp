// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoGpioDeviceProvider.h"  

using namespace ArduinoProviders;
using namespace Platform::Collections;

UsbSerial ^ArduinoGpioControllerProvider::_Usb = nullptr;
RemoteDevice ^ArduinoGpioControllerProvider::_Arduino = nullptr;

ArduinoGpioPinProvider::~ArduinoGpioPinProvider()
{
}

void ArduinoGpioPinProvider::SetDriveMode(
    ProviderGpioPinDriveMode value
    )
{
    if (!_Connected)
    {
        throw ref new Platform::AccessDeniedException();
    }

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
    if (!_Connected)
    {
        throw ref new Platform::AccessDeniedException();
    }

    _Arduino->digitalWrite(
        _PinNumber, 
        (value == ProviderGpioPinValue::High) ? 
            PinState::HIGH :
            PinState::LOW);
}

ProviderGpioPinValue ArduinoGpioPinProvider::Read()
{
    if (!_Connected)
    {
        throw ref new Platform::AccessDeniedException();
    }

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
    _Arduino = ArduinoGpioControllerProvider::Arduino;
    _Arduino->DigitalPinUpdated +=
        ref new Microsoft::Maker::RemoteWiring::DigitalPinUpdatedCallback(this, &ArduinoGpioPinProvider::OnDigitalPinUpdated);

    auto mode = _Arduino->getPinMode(_PinNumber);
    this->SetDriveMode(
        (mode == PinMode::INPUT) ? 
            ProviderGpioPinDriveMode::Input : 
            ProviderGpioPinDriveMode::Output);

    _Arduino->DeviceReady +=
        ref new RemoteDeviceConnectionCallback([this]() -> void
    {
        _Connected = true;
    });
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

RemoteDevice^ ArduinoGpioControllerProvider::Arduino::get()
{
    if (_Arduino == nullptr)
    {
        _Usb = ref new UsbSerial("VID_2341", "PID_0043");

        int baudRate = 115200; //TODO: correct baud rate?
        _Usb->begin(baudRate, SerialConfig::SERIAL_8N1);


        _Arduino = ref new RemoteDevice(_Usb);
    }
    return _Arduino;

}

IGpioPinProvider^ ArduinoGpioControllerProvider::OpenPinProvider(
    int pinNumber,
    ProviderGpioSharingMode sharingMode
    )
{
    return ref new ArduinoGpioPinProvider(pinNumber, sharingMode);
}

IVectorView<IGpioControllerProvider^>^ ArduinoGpioProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IGpioControllerProvider^>();
    controllerCollection->Append(ref new ArduinoGpioControllerProvider());
    return controllerCollection->GetView();
}


