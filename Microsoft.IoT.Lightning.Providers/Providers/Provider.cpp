// Copyright (c) Microsoft. All rights reserved

#include "pch.h"
#include "Provider.h"

#include "GpioDeviceProvider.h"
#include "I2cDeviceProvider.h"
#include "SpiDeviceProvider.h"
#include "AdcDeviceProvider.h"
#include "PwmDeviceProvider.h"

#include "errorcodes.h"
#include "pins_arduino.h"

using namespace Microsoft::IoT::Lightning::Providers;

#pragma region Declarations

#if defined(_M_IX86) || defined(_M_X64)

// Pin numbers mappings for MinnowBoard Max

const std::map<int, int> MBM_GPIO_Pins =
{
    { 0, GPIO0 },
    { 1, GPIO1 },
    { 2, GPIO2 },
    { 3, GPIO3 },
    { 4, GPIO4 },
    { 5, GPIO5 },
    { 6, GPIO6 },
    { 7, GPIO7 },
    { 8, GPIO8 },
    { 9, GPIO9 }
};

#elif defined (_M_ARM)

// Pin numbers mappings for Raspberry Pi2

const std::map<int, int> RPI2_GPIO_Pins =
{
    { 2,  GPIO2 },
    { 3,  GPIO3 },
    { 4,  GPIO4 },
    { 5,  GPIO5 },
    { 6,  GPIO6 },
    { 7,  GPIO7 },
    { 8,  GPIO8 },
    { 9,  GPIO9 },
    { 10, GPIO10 },
    { 11, GPIO11 },
    { 12, GPIO12 },
    { 13, GPIO13 },
    { 14, GPIO14 },
    { 15, GPIO15 },
    { 16, GPIO16 },
    { 17, GPIO17 },
    { 18, GPIO18 },
    { 19, GPIO19 },
    { 20, GPIO20 },
    { 21, GPIO21 },
    { 22, GPIO22 },
    { 23, GPIO23 },
    { 24, GPIO24 },
    { 25, GPIO25 },
    { 26, GPIO26 },
    { 27, GPIO27 },
    { 47, GPIO47 }
};

#endif // defined (_M_ARM)

#pragma endregion

#pragma region LightningProvider

IAdcControllerProvider ^ LightningProvider::AdcControllerProvider::get()
{
    return ref new LightningMCP3008AdcControllerProvider();
}

IGpioControllerProvider ^ LightningProvider::GpioControllerProvider::get()
{
    return ref new LightningGpioControllerProvider();
}

II2cControllerProvider ^ LightningProvider::I2cControllerProvider::get()
{
    return ref new LightningI2cControllerProvider(EXTERNAL_I2C_BUS);
}

IPwmControllerProvider ^ LightningProvider::PwmControllerProvider::get()
{
    return ref new LightningPCA9685PwmControllerProvider();
}

ISpiControllerProvider ^ LightningProvider::SpiControllerProvider::get()
{
    return ref new LightningSpiControllerProvider();
}

bool LightningProvider::IsLightningEnabled::get()
{
    ULONG state;
    if (g_pins.getPinState(10, state) == DMAP_E_DEVICE_NOT_FOUND_ON_SYSTEM)
    {
        return false;
    }

    return true;
}

void LightningProvider::ThrowError(HRESULT hr, LPCWSTR errorMessage)
{
    Platform::String^ errMessage = ref new Platform::String(errorMessage);

    auto it = DmapErrors.find(hr);
    if (it != DmapErrors.end())
    {
        errMessage += L"Additional Information: ";
        errMessage += ref new Platform::String(it->second);
    }

    throw ref new Platform::Exception(hr, errMessage);
    
}

int LightningProvider::MapGpioPin(BoardPinsClass::BOARD_TYPE boardType, int pin)
{
    int mappedPin = -1;

#if defined(_M_IX86) || defined(_M_X64)


    // Pin numbers mappings for Raspberry Pi2

    if (boardType == BoardPinsClass::BOARD_TYPE::MBM_BARE)
    {
        auto it = MBM_GPIO_Pins.find(pin);
        if (it == MBM_GPIO_Pins.end())
        {
            throw ref new Platform::InvalidArgumentException(L"Gpio Pin could not be mapped.");
        }
        mappedPin = it->second;
    }

#elif defined (_M_ARM)

    if (boardType == BoardPinsClass::BOARD_TYPE::PI2_BARE)
    {
        auto it = RPI2_GPIO_Pins.find(pin);
        if (it == RPI2_GPIO_Pins.end())
        {
            throw ref new Platform::InvalidArgumentException(L"Gpio Pin could not be mapped.");
        }
        mappedPin = it->second;
    }

#endif
    return mappedPin;
}

ILowLevelDevicesAggregateProvider^ LightningProvider::providerSingleton = nullptr;

ILowLevelDevicesAggregateProvider ^ LightningProvider::GetAggregateProvider()
{
    if (providerSingleton == nullptr)
    {
        providerSingleton = ref new LightningProvider();
    }

    return providerSingleton;
}

#pragma endregion
