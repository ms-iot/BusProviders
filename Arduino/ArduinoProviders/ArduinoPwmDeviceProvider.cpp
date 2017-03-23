// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoPwmDeviceProvider.h"
#include "ArduinoConnection.h"
#include <ppltasks.h>

using namespace concurrency;


using namespace ArduinoProviders;
using namespace Platform::Collections;

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
    _Arduino = create_task(ArduinoConnection::GetArduinoConnectionAsync()).get();
}

IVectorView<IPwmControllerProvider^>^ ArduinoPwmProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IPwmControllerProvider^>();
    controllerCollection->Append(ref new ArduinoPwmControllerProvider());
    return controllerCollection->GetView();
}


