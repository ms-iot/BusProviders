// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include <MCP3008support.h>

#define MCP3008_ADC_CHANNEL_COUNT 8
#define MCP3008_ADC_MIN 0
#define MCP3008_ADC_MAX 1023
#define MCP3008_ADC_BIT_RESOLUTION 10

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

                public ref class LightningMCP3008AdcControllerProvider sealed : public IAdcControllerProvider
                {
                public:
                    // Inherited via IAdcControllerProvider
                    virtual property int ChannelCount
                    {
                        int get() { return MCP3008_ADC_CHANNEL_COUNT; }
                    }

                    virtual property int MaxValue
                    {
                        int get() { return MCP3008_ADC_MAX; }
                    }
                    virtual property int MinValue
                    {
                        int get() { return MCP3008_ADC_MIN; }
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

                    virtual ~LightningMCP3008AdcControllerProvider();

                internal:
                    LightningMCP3008AdcControllerProvider();

                private:
                    std::shared_ptr<MCP3008Device> _addOnAdc;
                    std::vector<bool> _channelsAcquired;
                    int _resolutionInBits;
                    ProviderAdcChannelMode _channelMode;

                    void Initialize();

                };
            }
        }
    }
}