// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoConnection.h"  
#include <ppltasks.h>

using namespace concurrency;

using namespace ArduinoProviders;
using namespace Platform::Collections;

HANDLE ArduinoConnection::_ConnectedEvent = nullptr;
std::mutex ArduinoConnection::_ConnectionMutex;

UsbSerial ^ArduinoConnection::_Usb = nullptr;
RemoteDevice ^ArduinoConnection::_Arduino = nullptr;
ArduinoConnectionConfiguration ^ArduinoConnection::_ArduinoConnectionConfiguration = nullptr;

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

bool ArduinoConnection::WaitForConnection(unsigned int timeout)
{

    DWORD dwWaitResult = WaitForSingleObjectEx(_ConnectedEvent, timeout, true);
    return (dwWaitResult == WAIT_OBJECT_0);
}

IAsyncOperation<RemoteDevice^>^ ArduinoConnection::GetArduinoConnectionAsync()
{
    auto config = ArduinoConnection::Configuration;
    auto vid = config->Vid;
    auto pid = config->Pid;
    auto baudRate = config->BaudRate;

    return create_async([vid, pid, baudRate]() -> RemoteDevice^ {

        std::lock_guard<std::mutex> lock(_ConnectionMutex);

        if (_Arduino == nullptr)
        {
            _ConnectedEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

            _Usb = ref new UsbSerial(vid, pid);
            _Arduino = ref new RemoteDevice(_Usb);

            _Arduino->DeviceReady +=
                ref new RemoteDeviceConnectionCallback([]() -> void
            {
                SetEvent(_ConnectedEvent);
            });

            _Arduino->DeviceConnectionFailed +=
                ref new RemoteDeviceConnectionCallbackWithMessage([](Platform::String^ message) -> void
            {
                throw ref new Platform::Exception(E_FAIL, message);
            });

            _Usb->begin(baudRate, SerialConfig::SERIAL_8N1);

            if (!WaitForConnection(INFINITE))
            {
                throw ref new Platform::Exception(E_FAIL);
            }
        }
        return _Arduino;
    });
}
