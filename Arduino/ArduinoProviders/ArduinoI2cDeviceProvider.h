// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include <cstdint>
#include <map>

using namespace Windows::Devices::I2c::Provider;

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::Firmata;
using namespace Microsoft::Maker::RemoteWiring;
using namespace Microsoft::Maker::RemoteWiring::I2c;
using namespace Windows::Storage::Streams;

namespace ArduinoProviders
{
    public ref class ArduinoI2cDeviceProvider sealed : public II2cDeviceProvider
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

        virtual ~ArduinoI2cDeviceProvider();

    internal:
        ArduinoI2cDeviceProvider(RemoteDevice^ arduino, ProviderI2cConnectionSettings ^settings);

    private:
        ProviderI2cConnectionSettings ^_ConnectionSettings;

        RemoteDevice ^_Arduino;

        std::map<unsigned char, Platform::Array<unsigned char>^> _I2cData;
        std::map<unsigned char, unsigned char> _I2cRegisters;

        HANDLE _DataRead;
        std::mutex _DataReaderMutex;
    };

    ref class ArduinoI2cControllerProvider sealed : public II2cControllerProvider
    {
    public:
        ArduinoI2cControllerProvider();
        // Inherited via II2cControllerProvider
        virtual II2cDeviceProvider ^ GetDeviceProvider(ProviderI2cConnectionSettings ^settings);

    private:
        RemoteDevice ^_Arduino;
    };

    public ref class ArduinoI2cProvider sealed : public II2cProvider
    {
    public:
        virtual IAsyncOperation<IVectorView<II2cControllerProvider^>^>^ GetControllersAsync();
    };
}