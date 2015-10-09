// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoAdcDeviceProvider.h"  
#include "ArduinoConnection.h"

using namespace ArduinoProviders;
using namespace Platform::Collections;

void ArduinoAdcControllerProvider::AcquireChannel(int channel)
{
    _Arduino->pinMode(14 + channel, PinMode::ANALOG);
}

int ArduinoAdcControllerProvider::ReadValue(int channel)
{
    auto value = _Arduino->analogRead(ref new Platform::String(L"A") + channel.ToString());
    return value;
}

void ArduinoAdcControllerProvider::Initialize()
{
    _Arduino = ArduinoConnection::Arduino;
}

IVectorView<IAdcControllerProvider^>^ ArduinoAdcProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IAdcControllerProvider^>();
    controllerCollection->Append(ref new ArduinoAdcControllerProvider());
    return controllerCollection->GetView();
}


