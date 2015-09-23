// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoPwmDeviceProvider.h"  

using namespace ArduinoProviders;
using namespace Platform::Collections;

UsbSerial ^ArduinoPwmControllerProvider::_Usb = nullptr;
RemoteDevice ^ArduinoPwmControllerProvider::_Arduino = nullptr;
bool ArduinoPwmControllerProvider::_Connected = false;

int _ArduinoPwmPins[] = { 3,5,6,9,10,11 };

ArduinoPwmControllerProvider::~ArduinoPwmControllerProvider()
{
}

void ArduinoPwmControllerProvider::AcquirePin(int /*pin*/)
{
    // No-op
}

void ArduinoPwmControllerProvider::ReleasePin(int /*pin*/)
{
    // No-op
}

void ArduinoPwmControllerProvider::EnablePin(int /*pin*/)
{
    // No-op
}

void ArduinoPwmControllerProvider::DisablePin(int /*pin*/)
{
    // No-op
}

void ArduinoPwmControllerProvider::SetPulseParameters(int pin, double dutyCycle, bool invertPolarity)
{
    if (!_Connected)
    {
        throw ref new Platform::AccessDeniedException();
    }

    if (invertPolarity)
    {
        throw ref new Platform::Exception(E_NOTIMPL, L"invertPolarity is not supported");
    }

    // TODO: watch for pwm pins only?

    unsigned short dutyCycleValue = static_cast<unsigned short>((0xff) * (dutyCycle / 100.0));
    _Arduino->analogWrite(pin, dutyCycleValue);
}

void ArduinoPwmControllerProvider::Initialize()
{
    if (_Usb == nullptr)
    {
        _Usb = ref new UsbSerial("VID_2341", "PID_0043");

        int baudRate = 115200; //TODO: correct baud rate?
        _Usb->begin(baudRate, SerialConfig::SERIAL_8N1);


        if (_Arduino == nullptr)
        {
            _Arduino = ref new RemoteDevice(_Usb);
        }

        _Arduino->DeviceReady +=
            ref new RemoteDeviceConnectionCallback([this]() -> void
        {
            _Connected = true;
        });

    }

}

IVectorView<IPwmControllerProvider^>^ ArduinoPwmProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IPwmControllerProvider^>();
    controllerCollection->Append(ref new ArduinoPwmControllerProvider());
    return controllerCollection->GetView();
}


