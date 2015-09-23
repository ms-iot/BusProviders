// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include <cstdint>
#include <map>

using namespace Windows::Devices::I2c::Provider;

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::Firmata;
using namespace Windows::Storage::Streams;

namespace ArduinoProviders
{
    public ref class ArduinoI2cDeviceProvider sealed : public II2cDeviceProvider
    {

    public:
        ArduinoI2cDeviceProvider(ProviderI2cConnectionSettings ^settings);
        virtual ~ArduinoI2cDeviceProvider();

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

    private:
        void SendI2cSysex(
            const uint8_t address,
            const uint8_t rw_mask,
            const uint8_t len,
            uint8_t *data);


        ProviderI2cConnectionSettings ^_ConnectionSettings;

        UsbSerial ^_Usb;
        UwpFirmata ^_Firmata;

        std::map<unsigned char, DataReader^> _I2cData;
        std::map<unsigned char, unsigned char> _I2cRegisters;

        uint32_t _ReadTimeout;
        HANDLE _DataRead;
    };

    ref class ArduinoI2cControllerProvider sealed : public II2cControllerProvider
    {

    public:
        // Inherited via II2cControllerProvider
        virtual II2cDeviceProvider ^ GetDeviceProvider(ProviderI2cConnectionSettings ^settings);

    };

    public ref class ArduinoI2cProvider sealed : public II2cProvider
    {
    public:
        virtual IAsyncOperation<IVectorView<II2cControllerProvider^>^>^ GetControllersAsync();
    };
}