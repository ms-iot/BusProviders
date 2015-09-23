// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoAdcDeviceProvider.h"  

using namespace ArduinoProviders;
using namespace Platform::Collections;

UsbSerial ^ArduinoAdcControllerProvider::_Usb = nullptr;
RemoteDevice ^ArduinoAdcControllerProvider::_Arduino = nullptr;
bool ArduinoAdcControllerProvider::_Connected = false;

void ArduinoAdcControllerProvider::AcquireChannel(int /*channel*/)
{
    // No-op
}

void ArduinoAdcControllerProvider::ReleaseChannel(int /*channel*/)
{
    // No-op
}

int ArduinoAdcControllerProvider::ReadValue(int channel)
{
    if (!_Connected)
    {
        throw ref new Platform::AccessDeniedException();
    }

    return _Arduino->analogRead(ref new Platform::String(L"A") + channel.ToString());
}

void ArduinoAdcControllerProvider::Initialize()
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

IVectorView<IAdcControllerProvider^>^ ArduinoAdcProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IAdcControllerProvider^>();
    controllerCollection->Append(ref new ArduinoAdcControllerProvider());
    return controllerCollection->GetView();
}


