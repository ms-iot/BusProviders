// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoPwmDeviceProvider.h"
#include "ArduinoConnection.h"

using namespace ArduinoProviders;
using namespace Platform::Collections;

int _ArduinoPwmPins[] = { 3,5,6,9,10,11 };

void ArduinoPwmControllerProvider::SetPulseParameters(int pin, double dutyCycle, bool invertPolarity)
{
    if (invertPolarity)
    {
        throw ref new Platform::Exception(E_NOTIMPL, L"invertPolarity is not supported");
    }

    // TODO: watch for pwm pins only?

    unsigned short dutyCycleValue = static_cast<unsigned short>((0xff) * (dutyCycle / 100.0));
    _Arduino->analogWrite(pin, dutyCycleValue);
}

void ArduinoPwmControllerProvider::Initialize()
{
    _Arduino = ArduinoConnection::Arduino;
}

IVectorView<IPwmControllerProvider^>^ ArduinoPwmProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IPwmControllerProvider^>();
    controllerCollection->Append(ref new ArduinoPwmControllerProvider());
    return controllerCollection->GetView();
}


