// Copyright (c) Microsoft. All rights reserved

#include "pch.h"
#include "Provider.h"

#include "GpioDeviceProvider.h"
#include "I2cDeviceProvider.h"
#include "SpiDeviceProvider.h"

#include "boardpins.h"
#include "errorcodes.h"

using namespace Microsoft::IoT::Lightning::Providers;

IAdcControllerProvider ^ LightningProvider::AdcControllerProvider::get()
{
    throw ref new Platform::NotImplementedException();
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
    throw ref new Platform::NotImplementedException();
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

ILowLevelDevicesAggregateProvider^ LightningProvider::providerSingleton = nullptr;

ILowLevelDevicesAggregateProvider ^ LightningProvider::GetAggregateProvider()
{
    if (providerSingleton == nullptr)
    {
        providerSingleton = ref new LightningProvider();
    }

    return providerSingleton;
}
