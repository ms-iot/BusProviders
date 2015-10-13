#pragma once

// This file has been added as a workaround for a bug in one of Windows SDK tools, WinMDExp.
// A limitation of the tool is that it requires refrence assemblies count to be >= refernce types
// However, because the provider references both the IOT and Extensions SDK, with several
// assemblies referenced in each, that breaks the tool.
// This class references more types to overcome that issue.
// 
// Note: Future versions of the Windows SDK; i.e. > 10, will not have that limitation.
//
namespace ArduinoProviders {
    public ref class Dummy sealed
    {
    public:
        Dummy() {}
        void Func(
            Windows::Devices::Adc::AdcChannel^ a,
            Windows::Devices::Adc::AdcController^ a2,
            Windows::Devices::Adc::AdcChannelMode a3,
            Windows::Devices::Gpio::GpioController^ a4,
            Windows::Devices::Gpio::GpioOpenStatus a5,
            Windows::Devices::Gpio::GpioPin^ a6,
            Windows::Devices::Gpio::GpioPinDriveMode a7,
            Windows::Devices::Gpio::GpioPinEdge a8,
            Windows::Devices::Gpio::GpioPinValue a9,
            Windows::Devices::Gpio::GpioSharingMode a10,
            Windows::Devices::Spi::SpiBusInfo^ a11,
            Windows::Devices::Spi::SpiConnectionSettings^ a12,
            Windows::Devices::Spi::SpiController^ a13,
            Windows::Devices::Spi::SpiDevice^ a14,
            Windows::Devices::Spi::SpiMode a15,
            Windows::Devices::Spi::SpiSharingMode a16,
            Windows::Devices::I2c::I2cBusSpeed a17,
            Windows::Devices::I2c::I2cConnectionSettings^ a18,
            Windows::Devices::I2c::I2cController^ a19,
            Windows::Devices::I2c::I2cDevice^ a20,
            Windows::Devices::I2c::I2cSharingMode a21,
            Windows::Devices::I2c::I2cTransferResult a22,
            Windows::Devices::I2c::I2cTransferStatus a23,
            Windows::Devices::I2c::II2cDeviceStatics^ a24
            )
        {
        }
    };
}
