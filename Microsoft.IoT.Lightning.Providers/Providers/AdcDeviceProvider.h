// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices::Adc::Provider;

namespace Microsoft {
    namespace IoT {
        namespace Lightning {
            namespace Providers {

                public ref class LightningAdcProvider sealed : public IAdcProvider
                {
                public:
                    virtual IVectorView<IAdcControllerProvider^>^ GetControllers();
                    static IAdcProvider^ GetAdcProvider();

                private:
                    LightningAdcProvider() { }
                    static IAdcProvider^ providerSingleton;
                };

                public ref class LightningAdcControllerProvider sealed : public IAdcControllerProvider
                {
                public:
                    // Inherited via IAdcControllerProvider
                    virtual property int ChannelCount
                    {
                        int get() { return _channelCount; }
                    }

                    virtual property int MaxValue
                    {
                        int get() { return _maxValue; }
                    }
                    virtual property int MinValue
                    {
                        int get() { return _minValue; }
                    }

                    virtual property int ResolutionInBits
                    {
                        int get() { return _resolutionInBits; }
                    }

                    virtual property ProviderAdcChannelMode ChannelMode
                    {
                        ProviderAdcChannelMode get() { return _channelMode; }
                        void set(ProviderAdcChannelMode value)
                        {
                            if (value != ProviderAdcChannelMode::SingleEnded)
                                throw ref new Platform::Exception(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED));
                            _channelMode = value;
                        }
                    }

                    virtual bool IsChannelModeSupported(ProviderAdcChannelMode channelMode)
                    {
                        if (channelMode != ProviderAdcChannelMode::SingleEnded)
                            return false;

                        return true;
                    }

                    virtual void AcquireChannel(int channel);
                    virtual void ReleaseChannel(int channel);

                    virtual int ReadValue(int channelNumber);

                internal:
                    LightningAdcControllerProvider();

                private:
                    unsigned short _pinCount;
                    int _channelCount;
                    int _maxValue;
                    int _minValue;
                    int _resolutionInBits;
                    ProviderAdcChannelMode _channelMode;
                    BoardPinsClass::BOARD_TYPE _boardType;
                    void Initialize();
                };
            }
        }
    }
}