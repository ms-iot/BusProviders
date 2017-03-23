// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices::Gpio::Provider;

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::RemoteWiring;

namespace ArduinoProviders
{
    public ref class ArduinoGpioPinProvider sealed : public IGpioPinProvider
    {

    public:
        // Inherited via IGpioPinProvider
        virtual event TypedEventHandler<IGpioPinProvider^, GpioPinProviderValueChangedEventArgs^>^ ValueChanged;

        virtual property TimeSpan DebounceTimeout
        {
            TimeSpan get() { throw ref new Platform::Exception(E_NOTIMPL, L"DebounceTimeout is not supported"); }
            void set(TimeSpan timeSpan) { throw ref new Platform::Exception(E_NOTIMPL, L"DebounceTimeout is not supported"); }
        }
        virtual property int PinNumber { int get() { return _PinNumber; } }
        virtual property ProviderGpioSharingMode SharingMode
        {
            ProviderGpioSharingMode get() { return ProviderGpioSharingMode::Exclusive; }
        }

        virtual bool IsDriveModeSupported(ProviderGpioPinDriveMode driveMode)
        {
            return (driveMode == ProviderGpioPinDriveMode::Input) ||
                (driveMode == ProviderGpioPinDriveMode::Output);
        }
        virtual ProviderGpioPinDriveMode GetDriveMode() { return _DriveMode; }

        virtual void SetDriveMode(ProviderGpioPinDriveMode value);
        virtual void Write(ProviderGpioPinValue value);
        virtual ProviderGpioPinValue Read();

        virtual ~ArduinoGpioPinProvider()
        {
        }

    internal:
        ArduinoGpioPinProvider(RemoteDevice^ arduino, int pinNumber, ProviderGpioSharingMode sharingMode) :
            _Arduino(arduino),
            _PinNumber(pinNumber),
            _SharingMode(sharingMode),
            _DriveMode(ProviderGpioPinDriveMode::Input)
        {
            if (sharingMode != ProviderGpioSharingMode::Exclusive)
            {
                throw ref new Platform::Exception(E_NOTIMPL, L"Unsupported Gpio Pin SharingMode");
            }

            Initialize();
        }

    private:
        void Initialize();
        void OnValueChanged(GpioPinProviderValueChangedEventArgs ^args);
        void OnDigitalPinUpdated(unsigned char pin, PinState value);

    private:
        int _PinNumber;
        ProviderGpioSharingMode _SharingMode;
        ProviderGpioPinDriveMode _DriveMode;
        RemoteDevice ^_Arduino;
    };

    ref class ArduinoGpioControllerProvider sealed : public IGpioControllerProvider
    {
    public:
        ArduinoGpioControllerProvider();

        // Inherited via IGpioControllerProvider
        virtual property int PinCount { int get() { return _ArduinoPinCount; } }
        virtual IGpioPinProvider^ OpenPinProvider(int, ProviderGpioSharingMode);

    private:
        RemoteDevice ^_Arduino;
        static const unsigned short _ArduinoPinCount = 20;
    };

    public ref class ArduinoGpioProvider sealed : public IGpioProvider
    {
    public:
        virtual IVectorView<IGpioControllerProvider^>^ GetControllers();
    };
}