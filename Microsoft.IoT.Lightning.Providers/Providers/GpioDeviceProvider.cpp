// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"
#include "GpioDeviceProvider.h"
#include "Provider.h"
#include "boardpins.h"

using namespace Microsoft::IoT::Lightning::Providers;

#define MBM_PIN_COUNT 26
#define RPI_PIN_COUNT 53

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

    if (_boardType == BoardPinsClass::BOARD_TYPE::MBM_BARE)
    {
        _pinCount = MBM_PIN_COUNT;
    }
    else if (_boardType == BoardPinsClass::BOARD_TYPE::PI2_BARE)
    {
        _pinCount = RPI_PIN_COUNT;
    }
    else
    {
        throw ref new Platform::NotImplementedException(L"This board type has not been implemented.");
    }
}

IGpioPinProvider^ LightningGpioControllerProvider::OpenPinProvider(
    int pin,
    ProviderGpioSharingMode sharingMode
    )
{
    int mappedPin = LightningProvider::MapGpioPin(_boardType, pin);

    return OpenPinProviderNoMapping(pin, mappedPin, sharingMode);
}


IGpioPinProvider^ LightningGpioControllerProvider::OpenPinProviderNoMapping(int pin, int mappedPin, ProviderGpioSharingMode sharingMode)
{
    HRESULT hr = g_pins.verifyPinFunction(mappedPin, FUNC_DIO, BoardPinsClass::NO_LOCK_CHANGE);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Invalid function for pin.");
    }

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
    if (_DriveMode != value)
    {
        SetDriveModeInternal(value);
    }
}


void LightningGpioPinProvider::SetDriveModeInternal(
    ProviderGpioPinDriveMode value
    )
{
    HRESULT hr = S_OK;
    ULONG mode = 0;
    BOOL pullUp = FALSE;
    switch (value)
    {
    case ProviderGpioPinDriveMode::Input:
        mode = DIRECTION_IN;
        pullUp = FALSE;
        break;
    case ProviderGpioPinDriveMode::Output:
        mode = DIRECTION_OUT;
        pullUp = FALSE;
        break;
    case ProviderGpioPinDriveMode::InputPullUp:
        mode = DIRECTION_IN;
        pullUp = TRUE;
        break;
    default:
        throw ref new Platform::NotImplementedException(L"Pin drive mode not implemented");
    }

    hr = g_pins.setPinMode(_MappedPinNumber, mode, pullUp);

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
    // It's safe to cast these value to a state expected by setPinState
    //   ProviderGpioPinValue::Low == 0 == LOW
    //   ProviderGpioPinValue::High == 1 == HIGH
    ULONG state = safe_cast<ULONG>(value);

    HRESULT  hr = g_pins.setPinState(_MappedPinNumber, state);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not write pin value.");
    }
}

ProviderGpioPinValue LightningGpioPinProvider::Read()
{
    ULONG state = 0;

    HRESULT hr = g_pins.getPinState(_MappedPinNumber, state);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not read pin value.");
    }

    if (state == HIGH)
    {
        return ProviderGpioPinValue::High;
    }

    return ProviderGpioPinValue::Low;
}

#pragma endregion
