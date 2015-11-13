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

#pragma region LightningMCP3008AdcControllerProvider


void LightningMCP3008AdcControllerProvider::AcquireChannel(int channel)
{
    if (_channelsAcquired[channel])
    {
        throw ref new Platform::AccessDeniedException(L"Channel already acquired");
    }

    _channelsAcquired[channel] = true;
}

void LightningMCP3008AdcControllerProvider::ReleaseChannel(int channel)
{
    if (!_channelsAcquired[channel])
    {
        throw ref new Platform::AccessDeniedException(L"Channel not acquired");
    }

    _channelsAcquired[channel] = false;
}

int LightningMCP3008AdcControllerProvider::ReadValue(int channelNumber)
{
    if (!_channelsAcquired[channelNumber])
    {
        throw ref new Platform::AccessDeniedException(L"Channel not acquired");
    }

    ULONG value = 0;;
    ULONG bits = 0;
    _addOnAdc->readValue(channelNumber, value, bits);

    // Scale the digitized analog value to the currently set analog read resolution.
    if (_resolutionInBits > (int)bits)
    {
        value = value << (_resolutionInBits - (int)bits);
    }
    else if ((int)bits > _resolutionInBits)
    {
        value = value >> ((int)bits - _resolutionInBits);
    }

    return (int)value;
}

LightningMCP3008AdcControllerProvider::LightningMCP3008AdcControllerProvider() : 
    _resolutionInBits(MCP3008_ADC_BIT_RESOLUTION)
{
    Initialize();
}

LightningMCP3008AdcControllerProvider::~LightningMCP3008AdcControllerProvider()
{
    _addOnAdc->end();
}

void LightningMCP3008AdcControllerProvider::Initialize()
{
    BoardPinsClass::BOARD_TYPE boardType;
    HRESULT hr = g_pins.getBoardType(boardType);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred determining board type.");
    }

    if (!(boardType == BoardPinsClass::BOARD_TYPE::MBM_BARE ||
        boardType == BoardPinsClass::BOARD_TYPE::PI2_BARE))
    {
        throw ref new Platform::NotImplementedException(L"This board type has not been implemented.");
    }

    _addOnAdc.reset(new MCP3008Device());

    hr = _addOnAdc->begin();
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred Initializing ADC.");
    }

    _channelsAcquired.resize(MCP3008_ADC_CHANNEL_COUNT, false);
}

IVectorView<IAdcControllerProvider^>^ LightningAdcProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IAdcControllerProvider^>();
    controllerCollection->Append(ref new LightningMCP3008AdcControllerProvider());
    return controllerCollection->GetView();
}

#pragma endregion
