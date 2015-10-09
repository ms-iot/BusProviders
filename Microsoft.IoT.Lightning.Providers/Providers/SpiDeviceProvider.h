// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include <SpiController.h>
#include <GpioDeviceProvider.h>

using namespace Windows::Devices::Spi::Provider;

namespace Microsoft {
    namespace IoT {
        namespace Lightning {
            namespace Providers {

                public ref class SpiProvider sealed : public ISpiProvider
                {
                public:
                    virtual IAsyncOperation<IVectorView<ISpiControllerProvider^>^>^ GetControllersAsync();
                    static ISpiProvider^ GetSpiProvider();
                private:
                    SpiProvider() { }
                    static ISpiProvider^ providerSingleton;
                };

                public ref class SpiControllerProvider sealed : public ISpiControllerProvider
                {

                public:
                    // Inherited via ISpiControllerProvider
                    virtual ISpiDeviceProvider ^ GetDeviceProvider(ProviderSpiConnectionSettings ^settings);
                };

                public ref class SpiDeviceProvider sealed : public ISpiDeviceProvider
                {

                public:
                    // Inherited via ISpiDeviceProvider
                    virtual property Platform::String ^ DeviceId
                    {
                        Platform::String^ get();
                    }

                    virtual property ProviderSpiConnectionSettings ^ ConnectionSettings
                    {
                        ProviderSpiConnectionSettings^ get();
                    }

                    virtual void Read(Platform::WriteOnlyArray<unsigned char>^ buffer);
                    virtual void TransferFullDuplex(const Platform::Array<unsigned char> ^writeBuffer, Platform::WriteOnlyArray<unsigned char> ^readBuffer);
                    virtual void TransferSequential(const Platform::Array<unsigned char> ^writeBuffer, Platform::WriteOnlyArray<unsigned char> ^readBuffer);
                    virtual void Write(const Platform::Array<unsigned char> ^buffer);


                    virtual ~SpiDeviceProvider();

                internal:
                    SpiDeviceProvider(ProviderSpiConnectionSettings ^settings);

                private:
                    SpiDeviceProvider() { }
                    ProviderSpiConnectionSettings ^_ConnectionSettings;
                    std::unique_ptr<SpiControllerClass> _SpiController;
                    IGpioPinProvider^ _chipSelectPin;

                    HRESULT TransferFullDuplexInternal(const Platform::Array<unsigned char> ^writeBuffer, Platform::WriteOnlyArray<unsigned char> ^readBuffer);
                };

            }
        }
    }
}