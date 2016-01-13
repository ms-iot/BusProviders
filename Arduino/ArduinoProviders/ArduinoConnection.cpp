// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoConnection.h"  

using namespace ArduinoProviders;
using namespace Platform::Collections;

UsbSerial ^ArduinoConnection::_Usb = nullptr;
RemoteDevice ^ArduinoConnection::_Arduino = nullptr;
bool ArduinoConnection::_Connected = false;
ArduinoConnectionConfiguration ^ArduinoConnection::_ArduinoConnectionConfiguration = nullptr;

bool ArduinoConnection::Connected::get()
{
    return _Connected;
}

ArduinoConnectionConfiguration^ ArduinoConnection::Configuration::get()
{
    if (nullptr == _ArduinoConnectionConfiguration)
    {
        _ArduinoConnectionConfiguration = ref new ArduinoConnectionConfiguration();
    }
    return _ArduinoConnectionConfiguration;
}

void ArduinoConnection::Configuration::set(ArduinoConnectionConfiguration^ value)
{
    if (nullptr != _Arduino)
    {
        throw ref new Platform::Exception(E_ACCESSDENIED, L"Cannot change connection properties after a connection has been established");
    }
    _ArduinoConnectionConfiguration = value;
}

RemoteDevice^ ArduinoConnection::Arduino::get()
{
    if (_Arduino == nullptr)
    {
        _Usb = ref new UsbSerial(Configuration->Vid, Configuration->Pid);
        _Arduino = ref new RemoteDevice(_Usb);

        _Arduino->DeviceReady +=
            ref new RemoteDeviceConnectionCallback([]() -> void
        {
            _Connected = true;
        });

        _Arduino->DeviceConnectionFailed +=
            ref new RemoteDeviceConnectionCallbackWithMessage([](Platform::String^ message) -> void
        {
            throw ref new Platform::Exception(E_FAIL, message);
        });

        int baudRate = Configuration->BaudRate;
        _Usb->begin(baudRate, SerialConfig::SERIAL_8N1);

        while (!_Connected)
        {
            Sleep(1000);
        }
    }
    return _Arduino;

}
