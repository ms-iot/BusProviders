// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices::Gpio::Provider;

namespace Microsoft {
    namespace IoT {
        namespace Lightning {
            namespace Providers {

                public ref class LightningGpioProvider sealed : public IGpioProvider
                {
                public:
                    virtual IVectorView<IGpioControllerProvider^>^ GetControllers();
                    static IGpioProvider^ GetGpioProvider();

                private:
                    LightningGpioProvider() { }
                    static IGpioProvider^ providerSingleton;
                };

                public ref class LightningGpioControllerProvider sealed : public IGpioControllerProvider
                {
                public:
                    // Inherited via IGpioControllerProvider
                    virtual property int PinCount { int get() { return _pinCount; } }
                    virtual IGpioPinProvider^ OpenPinProvider(int pin, ProviderGpioSharingMode sharingMode);

                internal:
                    LightningGpioControllerProvider();
                    IGpioPinProvider^ OpenPinProviderNoMapping(int pin, int mappedPin, ProviderGpioSharingMode sharingMode);

                private:
                    unsigned short _pinCount;
                    BoardPinsClass::BOARD_TYPE _boardType;
                    void Initialize();

                };

                public ref class LightningGpioPinProvider sealed : public IGpioPinProvider
                {

                public:
                    // Inherited via IGpioPinProvider
                    virtual property TimeSpan DebounceTimeout
                    {
                        TimeSpan get() { return _DebounceTimeout; }
                        void set(TimeSpan value) { _DebounceTimeout = value; }
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

                    virtual event TypedEventHandler<IGpioPinProvider^, GpioPinProviderValueChangedEventArgs^>^ ValueChanged
                    {
                        EventRegistrationToken add(Windows::Foundation::TypedEventHandler<IGpioPinProvider ^, GpioPinProviderValueChangedEventArgs ^>^ handler)
                        {
                            Platform::Object^ o = reinterpret_cast<Object^>(this);
                            HRESULT hr = g_pins.attachInterruptContext(_MappedPinNumber, &s_interruptCallback, (void*)reinterpret_cast<IInspectable*>(o), DMAP_INTERRUPT_MODE_EITHER);
                            return _ValueChangedInternal += handler;
                        }
                        void remove(Windows::Foundation::EventRegistrationToken token)
                        {
                            _ValueChangedInternal -= token;
                        }
                    }

                    virtual ~LightningGpioPinProvider() { }

                internal:
                    LightningGpioPinProvider(int pin, int mappedPin, ProviderGpioSharingMode sharingMode) :
                        _MappedPinNumber(mappedPin),
                        _PinNumber(pin),
                        _SharingMode(sharingMode),
                        _DriveMode(ProviderGpioPinDriveMode::Input),
                        _lastEventTime(0),
                        _lastEventState(0)
                    {

                        if (sharingMode != ProviderGpioSharingMode::Exclusive)
                        {
                            throw ref new Platform::NotImplementedException(L"Unsupported Gpio Pin SharingMode");
                        }

                        // Default to output
                        SetDriveModeInternal(ProviderGpioPinDriveMode::Output);

                        _DebounceTimeout.Duration = 0;

                        LARGE_INTEGER li;
                        QueryPerformanceFrequency(&li);
                        _clockFrequency = double(li.QuadPart) / 100000.0; // Calaculate device clock freq in 100ns, same resolution as debounce
                    }

                private:
                    LightningGpioPinProvider () {}
                    void SetDriveModeInternal(ProviderGpioPinDriveMode value);

                    event TypedEventHandler<IGpioPinProvider^, GpioPinProviderValueChangedEventArgs^>^ _ValueChangedInternal;
                    static void s_interruptCallback(PDMAP_WAIT_INTERRUPT_NOTIFY_BUFFER InfoPtr, PVOID context) {
                        {
                            if (InfoPtr && context)
                            {
                                Platform::Object^ o = reinterpret_cast<Platform::Object^>((IInspectable*)context);
                                LightningGpioPinProvider^ pin = reinterpret_cast<LightningGpioPinProvider^>(o);

                                if (pin == nullptr || InfoPtr->IntNo != pin->_PinNumber)
                                {
                                    return;
                                }

                                auto eventTimeDiff = (InfoPtr->EventTime - pin->_lastEventTime) / pin->_clockFrequency;
                                if (eventTimeDiff >= pin->_DebounceTimeout.Duration || (eventTimeDiff > 0 && InfoPtr->NewState != pin->_lastEventState))
                                {

                                    pin->_ValueChangedInternal(pin, ref new GpioPinProviderValueChangedEventArgs((InfoPtr->NewState == 0) ?
                                        ProviderGpioPinEdge::FallingEdge :
                                        ProviderGpioPinEdge::RisingEdge));
                                }

                                // Save the last state
                                pin->_lastEventTime = InfoPtr->EventTime;
                                pin->_lastEventState = InfoPtr->NewState;
                            }
                        }
                    }

                    int _PinNumber;
                    int _MappedPinNumber;
                    ProviderGpioSharingMode _SharingMode;
                    ProviderGpioPinDriveMode _DriveMode;
                    TimeSpan _DebounceTimeout;

                    // Used to keep track of interrupts
                    long long _lastEventTime;
                    unsigned short _lastEventState;
                    double _clockFrequency;
                };

            }
        }
    }
}