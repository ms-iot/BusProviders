// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices::Adc::Provider;

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::RemoteWiring;

namespace ArduinoProviders
{
    public ref class ArduinoAdcControllerProvider sealed : public IAdcControllerProvider
    {
    public:
        // Inherited via IAdcControllerProvider
        virtual property int ChannelCount { int get() { return _ArduinoChannelCount; }}
        virtual property int MaxValue { int get() { return 1 << _ArduinoResolutionBits; } }
        virtual property int MinValue { int get() { return 0; } }
        virtual property int ResolutionInBits { int get() { return _ArduinoResolutionBits; } }
        virtual property ProviderAdcChannelMode ChannelMode
        {
            ProviderAdcChannelMode get() { return ProviderAdcChannelMode::SingleEnded; }
            void set(ProviderAdcChannelMode mode)
            {
                if (mode != ProviderAdcChannelMode::SingleEnded)
                    throw ref new Platform::Exception(E_NOTIMPL, L"Only SingleEnded is supported");
            }
        }

        virtual bool IsChannelModeSupported(ProviderAdcChannelMode channelMode)
        {
            return (channelMode == ProviderAdcChannelMode::SingleEnded);
        }

        virtual void AcquireChannel(int channel) { /*no op*/ }
        virtual void ReleaseChannel(int channel) { /*no op*/ }
        virtual int ReadValue(int channelNumber);

        virtual ~ArduinoAdcControllerProvider()
        {
        }

    internal:
        ArduinoAdcControllerProvider()
        {
            Initialize();
        }

    private:
        void Initialize();

    private:
        static UsbSerial ^_Usb;
        static RemoteDevice ^_Arduino;
        static bool _Connected;

        static const unsigned short _ArduinoResolutionBits = 10;
        static const unsigned short _ArduinoChannelCount = 6;
    };

    public ref class ArduinoAdcProvider sealed : public IAdcProvider
    {
    public:
        virtual IVectorView<IAdcControllerProvider^>^ GetControllers();
    };
}