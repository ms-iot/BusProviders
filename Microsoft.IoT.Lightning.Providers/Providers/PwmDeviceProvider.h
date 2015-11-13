// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices::Pwm::Provider;
using namespace Windows::Devices::Gpio::Provider;

namespace Microsoft {
    namespace IoT {
        namespace Lightning {
            namespace Providers {

                public ref class LightningPwmProvider sealed : public IPwmProvider
                {
                public:
                    virtual IVectorView<IPwmControllerProvider^>^ GetControllers();
                    static IPwmProvider^ GetPwmProvider();

                private:
                    LightningPwmProvider() { }
                    static IPwmProvider^ providerSingleton;
                };

                ref class  LightningSoftwarePwmPin
                {
                internal:
                    property IGpioPinProvider^ GpioPin
                    {
                        IGpioPinProvider^ get() { return _gpioPin; }
                    }

                    property bool Enabled
                    {
                        bool get() { return _enabled; }
                        void set(bool value) { _enabled = value; }
                    }

                    property bool InvertPolarity
                    {
                        bool get() { return _invertPolarity; }
                        void set(bool value) { _invertPolarity = value; }
                    }

                    property double DutyCycle
                    {
                        double get() { return _dutyCycle; }
                        void set(double value) { _dutyCycle = value; }
                    }

                    LightningSoftwarePwmPin(IGpioPinProvider^ pin) :
                        _gpioPin(pin),
                        _dutyCycle(0),
                        _enabled(false),
                        _invertPolarity(false)
                    {
                    }

                private:

                    IGpioPinProvider^ _gpioPin;
                    bool _enabled;
                    bool _invertPolarity;
                    double _dutyCycle;
                };

                public ref class LightningSoftwarePwmControllerProvider sealed : public IPwmControllerProvider
                {
                private:
                    static const int MAX_FREQUENCY = 1000;
                    static const int MIN_FREQUENCY = 40;
                public:
                    // Inherited via IPwmControllerProvider
                    virtual property double ActualFrequency { double get() { { return _actualFrequency; } }}
                    virtual property double MaxFrequency { double get() { { return MAX_FREQUENCY; } }}
                    virtual property double MinFrequency { double get() { { return MIN_FREQUENCY; } }}
                    virtual property int PinCount { int get() { return _pinCount; } }

                    virtual double SetDesiredFrequency(double frequency);
                    virtual void AcquirePin(int pin);
                    virtual void ReleasePin(int pin);
                    virtual void EnablePin(int pin);
                    virtual void DisablePin(int pin);
                    virtual void SetPulseParameters(int pin, double dutyCycle, bool invertPolarity);

                internal:
                    LightningSoftwarePwmControllerProvider();

                private:
                    unsigned short _pinCount;
                    double _actualFrequency;
                    bool _started;
                    LightningGpioControllerProvider^ _gpioController;
                    Platform::Collections::Vector<LightningSoftwarePwmPin^>^ _pins;
                    BoardPinsClass::BOARD_TYPE _boardType;

                    property double Period
                    {
                        double get() { return 1000.0 / ActualFrequency; }
                    }

                    void Initialize();
                    void Loop();

                };

                ref class LightningPCA9685PwmPin
                {
                internal:
                    property bool InvertPolarity
                    {
                        bool get() { return _invertPolarity; }
                        void set(bool value) { _invertPolarity = value; }
                    }

                    property double DutyCycle
                    {
                        double get() { return _dutyCycle; }
                        void set(double value) { _dutyCycle = value; }
                    }

                    LightningPCA9685PwmPin() :
                        _dutyCycle(0),
                        _invertPolarity(false)
                    {
                    }

                private:
                    bool _invertPolarity;
                    double _dutyCycle;
                };

                public ref class LightningPCA9685PwmControllerProvider sealed : public IPwmControllerProvider
                {
                private:
                    static const int MAX_FREQUENCY = 1000;
                    static const int MIN_FREQUENCY = 24;
                    static const unsigned int PCA9685_PIN_COUNT = 16;
                    static const unsigned int RESOLUTION_BITS = 8;

                public:
                    // Inherited via IPwmControllerProvider
                    virtual property double ActualFrequency { double get(); }
                    virtual property double MaxFrequency { double get() { return MAX_FREQUENCY; }}
                    virtual property double MinFrequency { double get() { return MIN_FREQUENCY; }}
                    virtual property int PinCount { int get() { return PCA9685_PIN_COUNT; } }

                    virtual double SetDesiredFrequency(double frequency);
                    virtual void AcquirePin(int pin);
                    virtual void ReleasePin(int pin);
                    virtual void EnablePin(int pin);
                    virtual void DisablePin(int pin);
                    virtual void SetPulseParameters(int pin, double dutyCycle, bool invertPolarity);

                internal:
                    LightningPCA9685PwmControllerProvider();

                private:
                    double _desiredFrequency;
                    Platform::Collections::Vector<LightningPCA9685PwmPin^>^ _pins;

                    property double Period
                    {
                        double get() { return 1000.0 / ActualFrequency; }
                    }

                    void Initialize();
                    inline UINT GetIoPin(int pin)
                    {
                        return PWM0 + pin;
                    }

                    inline ULONGLONG scaleDutyCycle(double dutyCycle, bool invertPolarity)
                    {
                        return (((ULONGLONG)((invertPolarity ? 1- dutyCycle : dutyCycle) * 255) * (1ULL << 32)) + (1ULL << (RESOLUTION_BITS - 1))) / (1ULL << RESOLUTION_BITS);
                    }

                };
            }
        }
    }
}