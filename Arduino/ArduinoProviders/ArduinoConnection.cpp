// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoConnection.h"  

using namespace ArduinoProviders;
using namespace Platform::Collections;

UsbSerial ^ArduinoConnection::_Usb = nullptr;
RemoteDevice ^ArduinoConnection::_Arduino = nullptr;
bool ArduinoConnection::_Connected = false;


bool ArduinoConnection::Connected::get()
{
    return _Connected;
}

RemoteDevice^ ArduinoConnection::Arduino::get()
{
    if (_Arduino == nullptr)
    {
        _Usb = ref new UsbSerial("VID_2341", "PID_0043");
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

        int baudRate = 57600;
        _Usb->begin(baudRate, SerialConfig::SERIAL_8N1);

        while (!_Connected)
        {
            Sleep(1000);
        }
    }
    return _Arduino;

}
