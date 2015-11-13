// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"
#include "AdcDeviceProvider.h"
#include "Provider.h"
#include "boardpins.h"

using namespace Microsoft::IoT::Lightning::Providers;

#pragma region LightningAdcProvider

IAdcProvider^ LightningAdcProvider::providerSingleton = nullptr;

IAdcProvider ^ LightningAdcProvider::GetAdcProvider()
{
    if (providerSingleton == nullptr)
    {
        providerSingleton = ref new LightningAdcProvider();
    }

    return providerSingleton;
}

#pragma endregion

#pragma region LightningAdcControllerProvider


void LightningAdcControllerProvider::AcquireChannel(int channel)
{
    throw ref new Platform::NotImplementedException();
}

void LightningAdcControllerProvider::ReleaseChannel(int channel)
{
    throw ref new Platform::NotImplementedException();
}

int LightningAdcControllerProvider::ReadValue(int channelNumber)
{
    return 0;
}

LightningAdcControllerProvider::LightningAdcControllerProvider()
{
    Initialize();
}

void LightningAdcControllerProvider::Initialize()
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


    _pinCount = (USHORT)pinCount;
}

IVectorView<IAdcControllerProvider^>^ LightningAdcProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IAdcControllerProvider^>();
    controllerCollection->Append(ref new LightningAdcControllerProvider());
    return controllerCollection->GetView();
}

#pragma endregion
