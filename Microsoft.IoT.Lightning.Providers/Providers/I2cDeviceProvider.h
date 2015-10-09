// Copyright (c) Microsoft. All rights reserved.
#pragma once
#include "I2cController.h"

namespace Microsoft {
    namespace IoT {
        namespace Lightning {
            namespace Providers {
                public ref class I2cProvider sealed : public II2cProvider
                {
                public:
                    virtual IAsyncOperation<IVectorView<II2cControllerProvider^>^>^ GetControllersAsync();
                    static II2cProvider^ GetI2cProvider();

                private:
                    I2cProvider() { }
                    static II2cProvider^ providerSingleton;
                };

                public ref class I2cControllerProvider sealed : public II2cControllerProvider
                {

                public:
                    // Inherited via II2cControllerProvider
                    virtual II2cDeviceProvider ^ GetDeviceProvider(ProviderI2cConnectionSettings ^settings);

                internal:
                    explicit I2cControllerProvider(ULONG busNumber) : _busNumber(busNumber) { }

                private:
                    I2cControllerProvider() { }
                    /// The bus number for the I2C Controller associated with this object.
                    ULONG _busNumber;
                };

                public ref class I2cDeviceProvider sealed : public II2cDeviceProvider
                {

                public:
                    // Inherited via II2cDeviceProvider
                    virtual property Platform::String ^ DeviceId
                    {
                        Platform::String^ get();
                    }

                    virtual ProviderI2cTransferResult WritePartial(
                        const Platform::Array<unsigned char> ^buffer);

                    virtual ProviderI2cTransferResult ReadPartial(
                        Platform::WriteOnlyArray<unsigned char> ^buffer);

                    virtual ProviderI2cTransferResult WriteReadPartial(
                        const Platform::Array<unsigned char> ^writeBuffer,
                        Platform::WriteOnlyArray<unsigned char> ^readBuffer);

                    virtual void Write(
                        const Platform::Array<unsigned char> ^buffer)
                    {
                        throw ref new Platform::NotImplementedException();
                    }

                    virtual void Read(
                        Platform::WriteOnlyArray<unsigned char> ^buffer)
                    {
                        throw ref new Platform::NotImplementedException();
                    }

                    virtual void WriteRead(
                        const Platform::Array<unsigned char> ^writeBuffer,
                        Platform::WriteOnlyArray<unsigned char> ^readBuffer)
                    {
                        throw ref new Platform::NotImplementedException();
                    }

                    virtual ~I2cDeviceProvider();

                internal:
                    I2cDeviceProvider(ProviderI2cConnectionSettings ^settings, ULONG busNumber);

                private:
                    ProviderI2cConnectionSettings ^_ConnectionSettings;
                    std::unique_ptr<I2cControllerClass> _I2cController;
                    ULONG _busNumber;
                    std::unique_ptr< I2cTransactionClass> _i2cTransaction;
                };

            }
        }
    }
}