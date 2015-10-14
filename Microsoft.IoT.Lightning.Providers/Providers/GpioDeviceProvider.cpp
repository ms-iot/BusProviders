// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"
#include "GpioDeviceProvider.h"
#include "Provider.h"
#include "boardpins.h"

using namespace Microsoft::IoT::Lightning::Providers;

#pragma region Declarations

const std::map<int, int> RPI2_GPIO_Pins = 
{ 
    { 2, 3 },
    { 3, 5 },
    { 4, 7 },
    { 5, 29 },
    { 6, 31 },
    { 7, 26 },
    { 8, 24 },
    { 9, 21 },
    { 10, 19 },
    { 11, 23 },
    { 12, 32 },
    { 13, 33 },
    { 14, 8 },
    { 15, 10 },
    { 16, 36 },
    { 17, 11 },
    { 18, 12 },
    { 19, 35 },
    { 20, 38 },
    { 21, 40 },
    { 22, 15 },
    { 23, 16 },
    { 24, 18 },
    { 25, 22 },
    { 26, 37 },
    { 27, 13 }
};

const std::map<int, int> MBM_GPIO_Pins =
{
    { 0, 21 },
    { 1, 23 },
    { 2, 25 },
    { 3, 14 },
    { 4, 16 },
    { 5, 18 },
    { 6, 20 },
    { 7, 22 },
    { 8, 24 },
    { 9, 26 }
};

#pragma endregion

#pragma region LightningGpioProvider

IGpioProvider^ LightningGpioProvider::providerSingleton = nullptr;

IGpioProvider ^ LightningGpioProvider::GetGpioProvider()
{
    if (providerSingleton == nullptr)
    {
        providerSingleton = ref new LightningGpioProvider();
    }

    return providerSingleton;
}

#pragma endregion

#pragma region LightningGpioControllerProvider

LightningGpioControllerProvider::LightningGpioControllerProvider()
{
    Initialize();
}

void LightningGpioControllerProvider::Initialize()
{
    HRESULT hr = g_pins.getBoardType(_boardType);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred determining board type.");
    }

    if (!(_boardType == BoardPinsClass::BOARD_TYPE::MBM_BARE || 
        _boardType == BoardPinsClass::BOARD_TYPE::PI2_BARE))
    {
        throw ref new Platform::NotImplementedException(L"This board type has not been implemented.");
    }

    ULONG pinCount = 0;
    hr = g_pins.getGpioPinCount(pinCount);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Gpio Controller Provider Init Failed.");
    }

    _pinCount = (USHORT)pinCount;
}

IGpioPinProvider^ LightningGpioControllerProvider::OpenPinProvider(
    int pin,
    ProviderGpioSharingMode sharingMode
    )
{
    int mappedPin = -1;
    if (_boardType == BoardPinsClass::BOARD_TYPE::MBM_BARE)
    {
        auto it = MBM_GPIO_Pins.find(pin);
        if (it == MBM_GPIO_Pins.end())
        {
            throw ref new Platform::InvalidArgumentException(L"Gpio Pin could not be mapped.");
        }
        mappedPin = it->second;
    }
    else if (_boardType == BoardPinsClass::BOARD_TYPE::PI2_BARE)
    {
        auto it = RPI2_GPIO_Pins.find(pin);
        if (it == RPI2_GPIO_Pins.end())
        {
            throw ref new Platform::InvalidArgumentException(L"Gpio Pin could not be mapped.");
        }
        mappedPin = it->second;
    }

    return OpenPinProviderNoMapping(pin, mappedPin, sharingMode);
}


IGpioPinProvider^ LightningGpioControllerProvider::OpenPinProviderNoMapping(int pin, int mappedPin, ProviderGpioSharingMode sharingMode)
{
    return ref new LightningGpioPinProvider(pin, mappedPin, sharingMode);
}

IVectorView<IGpioControllerProvider^>^ LightningGpioProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IGpioControllerProvider^>();
    controllerCollection->Append(ref new LightningGpioControllerProvider());
    return controllerCollection->GetView();
}

#pragma endregion

#pragma region LightningGpioPinProvider

void LightningGpioPinProvider::SetDriveMode(
    ProviderGpioPinDriveMode value
    )
{
    if (_DriveMode == value)
    {
        return; // Already set
    }

    SetDriveModeInternal(value);
}


void LightningGpioPinProvider::SetDriveModeInternal(
    ProviderGpioPinDriveMode value
    )
{
    HRESULT hr = S_OK;

    switch (value)
    {
    case ProviderGpioPinDriveMode::Input:
        hr = g_pins.setPinMode(_MappedPinNumber, DIRECTION_IN, false);
        break;
    case ProviderGpioPinDriveMode::Output:
        hr = g_pins.setPinMode(_MappedPinNumber, DIRECTION_OUT, false);
        break;
    case ProviderGpioPinDriveMode::InputPullUp:
        hr = g_pins.setPinMode(_MappedPinNumber, DIRECTION_IN, true);
        break;
    default:
        throw ref new Platform::NotImplementedException(L"Pin drive mode not implemented");
    }

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Error setting pin drive mode.");
    }

    // set the member variable
    _DriveMode = value;
}

void LightningGpioPinProvider::Write(
    ProviderGpioPinValue value
    )
{
    HRESULT hr = S_OK;

    ULONG state = (value == ProviderGpioPinValue::Low) ? LOW : HIGH;

    hr = g_pins.verifyPinFunction(_MappedPinNumber, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Invalid function for pin.");
    }

    hr = g_pins.setPinState(_MappedPinNumber, state);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not write pin value.");
    }
}

ProviderGpioPinValue LightningGpioPinProvider::Read()
{
    HRESULT hr = S_OK;
    ULONG state = 0;
    ProviderGpioPinValue returnValue = ProviderGpioPinValue::Low;

    hr = g_pins.verifyPinFunction(_MappedPinNumber, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Invalid function for pin.");
    }
    
    hr = g_pins.getPinState(_MappedPinNumber, state);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not read pin value.");
    }

    switch (state)
    {
    case LOW:
        returnValue = ProviderGpioPinValue::Low;
    case HIGH:
        returnValue = ProviderGpioPinValue::High;
    default:
        LightningProvider::ThrowError(hr, L"Invalid pin value.");
    }

    return returnValue;
}

#pragma endregion
