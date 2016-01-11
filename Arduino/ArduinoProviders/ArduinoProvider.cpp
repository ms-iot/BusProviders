// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoProvider.h"  

#include "ArduinoAdcDeviceProvider.h"
#include "ArduinoPwmDeviceProvider.h"
#include "ArduinoGpioDeviceProvider.h"
#include "ArduinoI2cDeviceProvider.h"

using namespace ArduinoProviders;

bool ArduinoProvider::_Created = false;

IAdcControllerProvider ^ ArduinoProvider::AdcControllerProvider::get()
{
    return ref new ArduinoAdcControllerProvider();
}
IGpioControllerProvider ^ ArduinoProvider::GpioControllerProvider::get()
{
    return ref new ArduinoGpioControllerProvider();
}
II2cControllerProvider ^ ArduinoProvider::I2cControllerProvider::get()
{
    return ref new ArduinoI2cControllerProvider();
}
IPwmControllerProvider ^ ArduinoProvider::PwmControllerProvider::get()
{
    return ref new ArduinoPwmControllerProvider();
}
ISpiControllerProvider ^ ArduinoProvider::SpiControllerProvider::get()
{
    return nullptr;
}

ArduinoConnectionConfiguration^ ArduinoProvider::Configuration::get()
{
    return ArduinoConnection::Configuration;
}

void ArduinoProvider::Configuration::set(ArduinoConnectionConfiguration ^value)
{
    if (ArduinoProvider::_Created)
    {
        throw ref new Platform::Exception(E_ACCESSDENIED, L"Cannot change connection properties after a provider has been created");
    }
    ArduinoConnection::Configuration = value;
}
