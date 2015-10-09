// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices::Gpio::Provider;

namespace Microsoft {
    namespace IoT {
        namespace Lightning {
            namespace Providers {

                public ref class GpioProvider sealed : public IGpioProvider
                {
                public:
                    virtual IVectorView<IGpioControllerProvider^>^ GetControllers();
                    static IGpioProvider^ GetGpioProvider();

                private:
                    GpioProvider() { }
                    static IGpioProvider^ providerSingleton;
                };

                public ref class GpioControllerProvider sealed : public IGpioControllerProvider
                {
                public:
                    // Inherited via IGpioControllerProvider
                    virtual property int PinCount { int get() { return _pinCount; } }
                    virtual IGpioPinProvider^ OpenPinProvider(int pin, ProviderGpioSharingMode sharingMode);

                internal:
                    GpioControllerProvider();
                    IGpioPinProvider^ OpenPinProviderNoMapping(int mappedPin, ProviderGpioSharingMode sharingMode);

                private:
                    unsigned short _pinCount;
                    BoardPinsClass::BOARD_TYPE _boardType;
                    void Initialize();

                };

                public ref class GpioPinProvider sealed : public IGpioPinProvider
                {

                public:
                    // Inherited via IGpioPinProvider
                    virtual property TimeSpan DebounceTimeout
                    {
                        TimeSpan get() { throw ref new Platform::Exception(E_NOTIMPL, L"DebounceTimeout is not supported"); }
                        void set(TimeSpan timeSpan) { throw ref new Platform::Exception(E_NOTIMPL, L"DebounceTimeout is not supported"); }
                    }

                    virtual property int PinNumber { int get() { return _PinNumber; } }
                    virtual property ProviderGpioSharingMode SharingMode
                    {
                        ProviderGpioSharingMode get() { return _SharingMode; }
                    }

                    virtual bool IsDriveModeSupported(ProviderGpioPinDriveMode driveMode)
                    {
                        return (driveMode == ProviderGpioPinDriveMode::Input) ||
                            (driveMode == ProviderGpioPinDriveMode::InputPullUp) ||
                            (driveMode == ProviderGpioPinDriveMode::Output);
                    }

                    virtual ProviderGpioPinDriveMode GetDriveMode() { return _DriveMode; }

                    virtual void SetDriveMode(ProviderGpioPinDriveMode value);
                    virtual void Write(ProviderGpioPinValue value);
                    virtual ProviderGpioPinValue Read();

                    // Not implemented since Lightning does yet support interrupts yet
                    virtual event TypedEventHandler<IGpioPinProvider^, GpioPinProviderValueChangedEventArgs^>^ ValueChanged
                    {
                        EventRegistrationToken add(Windows::Foundation::TypedEventHandler<IGpioPinProvider ^, GpioPinProviderValueChangedEventArgs ^> ^)
                        {
                            // Not yet implemnted
                            // Simply return an empty registration token
                            // Do not throw an exception; add event may be called by the framework
                            return EventRegistrationToken();
                        }
                        void remove(Windows::Foundation::EventRegistrationToken)
                        {
                            // Not yet implemnted
                            // Do not throw an exception; remove event may be called by the framework
                        }
                    }

                    virtual ~GpioPinProvider() { }

                internal:
                    GpioPinProvider(int pinNumber, ProviderGpioSharingMode sharingMode) :
                        _PinNumber(pinNumber),
                        _SharingMode(sharingMode),
                        _DriveMode(ProviderGpioPinDriveMode::Input)
                    {
                        if (sharingMode != ProviderGpioSharingMode::Exclusive)
                        {
                            throw ref new Platform::NotImplementedException(L"Unsupported Gpio Pin SharingMode");
                        }
                    }

                private:
                    void Initialize();
                    void SetDriveModeInternal(ProviderGpioPinDriveMode value);

                    int _PinNumber;
                    ProviderGpioSharingMode _SharingMode;
                    ProviderGpioPinDriveMode _DriveMode;
                };

            }
        }
    }
}