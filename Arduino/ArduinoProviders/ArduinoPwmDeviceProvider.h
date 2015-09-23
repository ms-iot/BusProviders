// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices::Pwm::Provider;

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::RemoteWiring;

namespace ArduinoProviders
{
    public ref class ArduinoPwmControllerProvider sealed : public IPwmControllerProvider
    {
    public:
        // Inherited via IPwmControllerProvider
        virtual property double ActualFrequency
        {
            double get() { throw ref new Platform::Exception(E_NOTIMPL, L"Modifying the frequency is not supported"); }
        }
        virtual property double MaxFrequency
        {
            double get() { throw ref new Platform::Exception(E_NOTIMPL, L"Modifying the frequency is not supported"); }
        }
        virtual property double MinFrequency
        {
            double get() { throw ref new Platform::Exception(E_NOTIMPL, L"Modifying the frequency is not supported"); }
        }
        virtual property int PinCount
        {
            int get() { throw ref new Platform::Exception(E_NOTIMPL, L"Requesting the PinCount is not supported"); }
        }

        virtual double SetDesiredFrequency(double frequency)
        {
            throw ref new Platform::Exception(E_NOTIMPL, L"Modifying the frequency is not supported");
        }

        virtual void AcquirePin(int pin) { /*no op*/ }
        virtual void ReleasePin(int pin) { /*no op*/ }
        virtual void EnablePin(int pin) { /*no op*/ }
        virtual void DisablePin(int pin) { /*no op*/ }

        virtual void SetPulseParameters(int pin, double dutyCycle, bool invertPolarity);

        virtual ~ArduinoPwmControllerProvider()
        {
        }

    internal:
        ArduinoPwmControllerProvider() 
        {
            Initialize();
        }

    private:
        void Initialize();

    private:
        static UsbSerial ^_Usb;
        static RemoteDevice ^_Arduino;
        static bool _Connected;
    };

    public ref class ArduinoPwmProvider sealed : public IPwmProvider
    {
    public:
        virtual IVectorView<IPwmControllerProvider^>^ GetControllers();
    };
}