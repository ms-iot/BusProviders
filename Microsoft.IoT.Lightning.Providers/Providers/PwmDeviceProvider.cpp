// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"
#include "Provider.h"
#include "GpioDeviceProvider.h"
#include "PwmDeviceProvider.h"
#include "boardpins.h"

using namespace Microsoft::IoT::Lightning::Providers;
using namespace Windows::System::Threading;

#pragma region LightningPwmProvider

IPwmProvider^ LightningPwmProvider::providerSingleton = nullptr;

IPwmProvider ^ LightningPwmProvider::GetPwmProvider()
{
    if (providerSingleton == nullptr)
    {
        providerSingleton = ref new LightningPwmProvider();
    }

    return providerSingleton;
}

IVectorView<IPwmControllerProvider^>^ LightningPwmProvider::GetControllers(
    void
    )
{
    auto controllerCollection = ref new Vector<IPwmControllerProvider^>();
    controllerCollection->Append(ref new LightningPCA9685PwmControllerProvider());
    controllerCollection->Append(ref new LightningSoftwarePwmControllerProvider());
    return controllerCollection->GetView();
}

#pragma endregion

#pragma region LightningSoftwarePwmControllerProvider

LightningSoftwarePwmControllerProvider::LightningSoftwarePwmControllerProvider() :
    _started(false),
    _actualFrequency(MinFrequency)
{
    Initialize();
}

void LightningSoftwarePwmControllerProvider::Initialize()
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
        LightningProvider::ThrowError(hr, L"Pwm Controller Provider Init Failed.");
    }

    _pinCount = (USHORT)pinCount;

    _gpioController = ref new LightningGpioControllerProvider();

    _pins = ref new Vector<LightningSoftwarePwmPin^>(_gpioController->PinCount);
}

double LightningSoftwarePwmControllerProvider::SetDesiredFrequency(double frequency)
{
    _actualFrequency = frequency;
    return _actualFrequency;
}

void LightningSoftwarePwmControllerProvider::AcquirePin(int pin)
{
    if (!_started)
    {
        _started = true;

        WorkItemHandler^ loop = ref new WorkItemHandler([&](IAsyncAction^ asyncAction)
        {
            Loop();
        });
        ThreadPool::RunAsync(loop, WorkItemPriority::High);
    }

    if (_pins->GetAt(pin) != nullptr)
    {
        throw ref new Platform::AccessDeniedException();
    }

    // Open the chip select pin
    int mappedPin = LightningProvider::MapGpioPin(_boardType, pin);

    auto gpioPin = _gpioController->OpenPinProviderNoMapping(pin, mappedPin, ProviderGpioSharingMode::Exclusive);
    gpioPin->SetDriveMode(ProviderGpioPinDriveMode::Output);
    _pins->SetAt(pin, ref new LightningSoftwarePwmPin(gpioPin));
}

void LightningSoftwarePwmControllerProvider::ReleasePin(int pin)
{
    if (_pins->GetAt(pin) == nullptr)
    {
        throw ref new Platform::AccessDeniedException();
    }
    _pins->SetAt(pin, nullptr);
}

void LightningSoftwarePwmControllerProvider::EnablePin(int pin)
{
    auto pwmPin = _pins->GetAt(pin);
    if (pwmPin == nullptr)
    {
        throw ref new Platform::AccessDeniedException();
    }

    pwmPin->Enabled = true;
}

void LightningSoftwarePwmControllerProvider::DisablePin(int pin)
{
    auto pwmPin = _pins->GetAt(pin);
    if (pwmPin == nullptr)
    {
        throw ref new Platform::AccessDeniedException();
    }

    pwmPin->Enabled = false;
}

void LightningSoftwarePwmControllerProvider::SetPulseParameters(int pin, double dutyCycle, bool invertPolarity)
{
    auto pwmPin = _pins->GetAt(pin);
    if (pwmPin == nullptr)
    {
        throw ref new Platform::AccessDeniedException();
    }

    pwmPin->DutyCycle = dutyCycle;
    pwmPin->InvertPolarity = invertPolarity;
}

void LightningSoftwarePwmControllerProvider::Loop()
{
    LARGE_INTEGER ticks;
    QueryPerformanceFrequency(&ticks);
    auto TicksASecond = ticks.QuadPart;
    Vector<LightningSoftwarePwmPin^>^ enabledPins = ref new Vector<LightningSoftwarePwmPin^>(0);
    while (true)
    {
        enabledPins->Clear();
        for (UINT i = 0; i < _pins->Size; i++)
        {
            auto pin = _pins->GetAt(i);
            if (pin != nullptr && pin->Enabled && pin->DutyCycle != 0)
            {
                bool inserted = false;
                for (UINT j = 0; j < enabledPins->Size; j++)
                {
                    auto pinCompare = enabledPins->GetAt(j);
                    if (pinCompare->DutyCycle > pin->DutyCycle)
                    {
                        enabledPins->InsertAt(j, pin);
                        inserted = true;
                        break;
                    }
                }

                if (!inserted) 
                {
                    enabledPins->Append(pin);
                }
            }
        }

        for (UINT i = 0; i < enabledPins->Size; i++)
        {
            LightningSoftwarePwmPin^ pin = enabledPins->GetAt(i);
            auto pinValue = (pin->InvertPolarity) ? ProviderGpioPinValue::Low : ProviderGpioPinValue::High;
            pin->GpioPin->Write(pinValue);
        }
        LARGE_INTEGER startTicks;
        QueryPerformanceCounter(&startTicks);
        for (UINT i = 0; i < enabledPins->Size; i++)
        {
            auto pin = enabledPins->GetAt(i);
            double targetTicks = startTicks.QuadPart + pin->DutyCycle*Period*TicksASecond / 1000.0;
            LARGE_INTEGER currentTicks;
            QueryPerformanceCounter(&currentTicks);
            while (currentTicks.QuadPart < targetTicks) {
                QueryPerformanceCounter(&currentTicks);
            }
            auto pinValue = (pin->InvertPolarity) ? ProviderGpioPinValue::High : ProviderGpioPinValue::Low;
            pin->GpioPin->Write(pinValue);
        }
        double endCycleTicks = startTicks.QuadPart + Period*TicksASecond / 1000.0;
        LARGE_INTEGER currentTicks;
        QueryPerformanceCounter(&currentTicks);
        while (currentTicks.QuadPart < endCycleTicks) {
            QueryPerformanceCounter(&currentTicks);
        }

    }
}

#pragma endregion

#pragma region LightningPCA9685PwmControllerProvider

double LightningPCA9685PwmControllerProvider::ActualFrequency::get()
{
    return (double)g_pins.getActualPwmFrequency(0);
}

double LightningPCA9685PwmControllerProvider::SetDesiredFrequency(double frequency)
{
    _desiredFrequency = frequency;
    HRESULT hr = g_pins.setPwmFrequency(0, (ULONG)frequency);
    if (FAILED(hr)) {
        LightningProvider::ThrowError(hr, L"Could not set desired frequency.");
    }

    return (double)g_pins.getActualPwmFrequency(0);
}

void LightningPCA9685PwmControllerProvider::AcquirePin(int pin)
{
    if (_pins->GetAt(pin) != nullptr)
    {
        throw ref new Platform::AccessDeniedException();
    }

    _pins->SetAt(pin, ref new LightningPCA9685PwmPin());
}

void LightningPCA9685PwmControllerProvider::ReleasePin(int pin)
{
    if (_pins->GetAt(pin) == nullptr)
    {
        throw ref new Platform::AccessDeniedException();
    }
    _pins->SetAt(pin, nullptr);
}

void LightningPCA9685PwmControllerProvider::EnablePin(int pin)
{
    auto pwmPin = _pins->GetAt(pin);

    // Set the PWM duty cycle.
    ULONGLONG scaledDutyCycle = scaleDutyCycle(pwmPin->DutyCycle, pwmPin->InvertPolarity);
    HRESULT hr = g_pins.setPwmDutyCycle(GetIoPin(pin), (ULONG)scaledDutyCycle);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not enable PWM pin.");
    }
}

void LightningPCA9685PwmControllerProvider::DisablePin(int pin)
{
    HRESULT hr = g_pins.setPwmDutyCycle(GetIoPin(pin), 0);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not disable PWM pin.");
    }
}

void LightningPCA9685PwmControllerProvider::SetPulseParameters(int pin, double dutyCycle, bool invertPolarity)
{
    // Set the PWM duty cycle.
    ULONGLONG scaledDutyCycle = scaleDutyCycle(dutyCycle, invertPolarity);
    HRESULT hr = g_pins.setPwmDutyCycle(GetIoPin(pin), (ULONG)scaledDutyCycle);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not set PWM pulse parameters.");
    }

    auto pwmPin = _pins->GetAt(pin);
    pwmPin->DutyCycle = dutyCycle;
    pwmPin->InvertPolarity = invertPolarity;
}

LightningPCA9685PwmControllerProvider::LightningPCA9685PwmControllerProvider() :
    _desiredFrequency(MinFrequency)
{
    Initialize();
}

void LightningPCA9685PwmControllerProvider::Initialize()
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

    _pins = ref new Vector<LightningPCA9685PwmPin^>(PCA9685_PIN_COUNT);

    g_pins.setPwmFrequency(0, (ULONG)_desiredFrequency);
}

#pragma endregion
