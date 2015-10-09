// Copyright (c) Microsoft. All rights reserved

#include "pch.h"
#include "Provider.h"

#include "GpioDeviceProvider.h"
#include "I2cDeviceProvider.h"
#include "SpiDeviceProvider.h"

#include "boardpins.h"
#include "errorcodes.h"

using namespace Microsoft::IoT::Lightning::Providers;

IAdcControllerProvider ^ Provider::AdcControllerProvider::get()
{
    throw ref new Platform::NotImplementedException();
}

IGpioControllerProvider ^ Provider::GpioControllerProvider::get()
{
    return ref new Microsoft::IoT::Lightning::Providers::GpioControllerProvider();
}

II2cControllerProvider ^ Provider::I2cControllerProvider::get()
{
    return ref new Microsoft::IoT::Lightning::Providers::I2cControllerProvider(EXTERNAL_I2C_BUS);
}

IPwmControllerProvider ^ Provider::PwmControllerProvider::get()
{
    throw ref new Platform::NotImplementedException();
}

ISpiControllerProvider ^ Provider::SpiControllerProvider::get()
{
    return ref new Microsoft::IoT::Lightning::Providers::SpiControllerProvider();
}

bool Provider::IsLightningEnabled::get()
{
    ULONG state;
    if (g_pins.getPinState(10, state) == DMAP_E_DEVICE_NOT_FOUND_ON_SYSTEM)
    {
        return false;
    }

    return true;
}

void Provider::ThrowError(HRESULT hr, LPCWSTR errorMessage)
{
    auto it = DmapErrors.find(hr);
        
    if (it != DmapErrors.end())
    {
        throw ref new Platform::Exception(hr, ref new Platform::String(it->second));
    }
    else
    {
        throw ref new Platform::Exception(hr, ref new Platform::String(errorMessage));
    }
}
