// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "Provider.h"
#include "SpiDeviceProvider.h"
#include "GpioDeviceProvider.h"
#include "boardpins.h"
#include <BcmSpiController.h>
#include <BtSpiController.h>

using namespace Microsoft::IoT::Lightning::Providers;

#pragma region LightningSpiProvider

ISpiProvider^ LightningSpiProvider::providerSingleton = nullptr;

ISpiProvider ^ LightningSpiProvider::GetSpiProvider()
{
    if (providerSingleton == nullptr)
    {
        providerSingleton = ref new LightningSpiProvider();
    }

    return providerSingleton;
}

IAsyncOperation<IVectorView<ISpiControllerProvider^>^>^ LightningSpiProvider::GetControllersAsync(
    void
    )
{
    return concurrency::create_async([]() -> IVectorView<ISpiControllerProvider^>^ {

        auto controllerCollection = ref new Vector<ISpiControllerProvider^>();
        controllerCollection->Append(ref new LightningSpiControllerProvider());
        return controllerCollection->GetView();
    });
}

#pragma endregion

#pragma region LightningSpiControllerProvider

ISpiDeviceProvider ^ LightningSpiControllerProvider::GetDeviceProvider(
    ProviderSpiConnectionSettings ^settings
    )
{
    return ref new LightningSpiDeviceProvider(settings);
}

#pragma endregion

#pragma region LightningSpiDeviceProvider

LightningSpiDeviceProvider::LightningSpiDeviceProvider(ProviderSpiConnectionSettings ^settings)
{
    _ConnectionSettings = settings;

    BoardPinsClass::BOARD_TYPE board;
    HRESULT hr = g_pins.getBoardType(board);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred determining board type.");
    }

    if (board == BoardPinsClass::BOARD_TYPE::MBM_BARE)
    {
        _SpiController.reset(new BtSpiControllerClass());
        hr = _SpiController->configurePins(MBM_PIN_MISO, MBM_PIN_MOSI, MBM_PIN_SCK);
    }
    else if (board == BoardPinsClass::BOARD_TYPE::PI2_BARE)
    {
        _SpiController.reset(new BcmSpiControllerClass());
        hr = _SpiController->configurePins(PI2_PIN_SPI0_MISO, PI2_PIN_SPI0_MOSI, PI2_PIN_SPI0_SCK);
    }
    else
    {
        throw ref new Platform::NotImplementedException(L"This board type has not been implemented.");
    }

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred while configuring pins for SPI");
    }

    // Set the SPI bit shifting order to MSB
    // TODO: Should this be the default?
    _SpiController->setMsbFirstBitOrder();

    hr = _SpiController->begin(EXTERNAL_SPI_BUS, (ULONG)settings->Mode, (ULONG)settings->ClockFrequency, (ULONG)settings->DataBitLength);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred while initializing the SPI controller");
    }

    // Try to find and open the SPI CS pin
    int spiChipSelectPinMapped = 0;
    if (board == BoardPinsClass::BOARD_TYPE::MBM_BARE)
    {
        if (settings->ChipSelectLine == 0)
        {
            spiChipSelectPinMapped = MBM_PIN_CS0;
        }
        else
        {
            throw ref new Platform::InvalidArgumentException("Invalid chip select line.");
        }
    }
    else if (board == BoardPinsClass::BOARD_TYPE::PI2_BARE)
    {
        if (settings->ChipSelectLine == 0)
        {
            spiChipSelectPinMapped = PI2_PIN_SPI0_CS0;
        }
        else if (settings->ChipSelectLine == 1)
        {
            spiChipSelectPinMapped = PI2_PIN_SPI0_CS1;
        }
        else
        {
            throw ref new Platform::InvalidArgumentException("Invalid chip select line.");
        }
    }

    // Open the chip select pin
    auto gpioControllerProvider = ref new LightningGpioControllerProvider();
    _chipSelectPin = gpioControllerProvider->OpenPinProviderNoMapping(settings->ChipSelectLine, spiChipSelectPinMapped, ProviderGpioSharingMode::Exclusive);
    _chipSelectPin->SetDriveMode(ProviderGpioPinDriveMode::Output);
}

void LightningSpiDeviceProvider::Read(Platform::WriteOnlyArray<unsigned char>^ buffer)
{
    if (buffer == nullptr)
    {
        throw ref new Platform::InvalidArgumentException(L"buffer cannot be null or empty.");
    }

    HRESULT hr = TransferFullDuplexInternal(nullptr, buffer);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not read data from SPI device.");
    }
}

void LightningSpiDeviceProvider::TransferFullDuplex(const Platform::Array<unsigned char>^ writeBuffer, Platform::WriteOnlyArray<unsigned char>^ readBuffer)
{
    if (writeBuffer == nullptr || readBuffer == nullptr)
    {
        throw ref new Platform::InvalidArgumentException(L"read or write buffer cannot be null.");
    }

    if (writeBuffer->Length != readBuffer->Length)
    {
        throw ref new Platform::NotImplementedException(L"SPI transfer buffer sizes must be equal.");
    }

    HRESULT hr = TransferFullDuplexInternal(writeBuffer, readBuffer);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not transfer data from SPI device.");
    }
}

void LightningSpiDeviceProvider::TransferSequential(const Platform::Array<unsigned char>^ writeBuffer, Platform::WriteOnlyArray<unsigned char>^ readBuffer)
{
    if (writeBuffer == nullptr || readBuffer == nullptr)
    {
        throw ref new Platform::InvalidArgumentException(L"read or write buffer cannot be null.");
    }

    HRESULT hr = TransferFullDuplexInternal(writeBuffer, nullptr);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not write data to SPI device.");
    }

    hr = TransferFullDuplexInternal(nullptr, readBuffer);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not read data from SPI device.");
    }
}

void LightningSpiDeviceProvider::Write(const Platform::Array<unsigned char>^ buffer)
{
    if (buffer == nullptr)
    {
        throw ref new Platform::InvalidArgumentException(L"buffer cannot be null or empty.");
    }

    HRESULT hr = TransferFullDuplexInternal(buffer, nullptr);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not write data to SPI device.");
    }
}


HRESULT LightningSpiDeviceProvider::TransferFullDuplexInternal(const Platform::Array<unsigned char> ^writeBuffer, Platform::WriteOnlyArray<unsigned char> ^readBuffer)
{
    // Prerequisites:
    //  1) At least one of writeBuffer or ReadBuffer is valid
    //  2) If both read and write buffers are provided, they must have equal sizes

    // Take the chip select low to select the device
    _chipSelectPin->Write(ProviderGpioPinValue::Low);

    UINT bufferLength = 0;
    if (writeBuffer != nullptr)
    {
        bufferLength = writeBuffer->Length;
    }
    else
    {
        bufferLength = readBuffer->Length;
    }

    ULONG readValue;
    HRESULT hr = S_OK;
    for (UINT i = 0; i < bufferLength; i++)
    {
        hr = _SpiController->transfer8(writeBuffer ? writeBuffer[i] : 0, readValue);
        if (FAILED(hr))
        {
            break;
        } 
        
        if (readBuffer)
        {
            readBuffer[i] = (BYTE)readValue;
        }
    }

    // Regardless of the return result, take the chip select high to de-select the device
    _chipSelectPin->Write(ProviderGpioPinValue::High);

    return hr;
}

LightningSpiDeviceProvider::~LightningSpiDeviceProvider()
{
    if (_SpiController != nullptr)
    {
        // Set all SPI pins as digital I/O.
        // This frees up the external SPI bus so its pins can be used for other functions
        _SpiController->revertPinsToGpio();
    }
}

Platform::String ^ LightningSpiDeviceProvider::DeviceId::get()
{
    LPCWSTR deviceName = L"Lightning SPI Device";

    BoardPinsClass::BOARD_TYPE board;
    HRESULT hr = g_pins.getBoardType(board);

    if (SUCCEEDED(hr))
    {
        if (board == BoardPinsClass::BOARD_TYPE::MBM_BARE)
        {
            deviceName = mbmSpiDeviceName;;
        }
        else if (board == BoardPinsClass::BOARD_TYPE::PI2_BARE)
        {
            deviceName = pi2Spi0DeviceName;
        }
    }

    return ref new Platform::String(deviceName);
}

ProviderSpiConnectionSettings ^ LightningSpiDeviceProvider::ConnectionSettings::get()
{
    return _ConnectionSettings;
}

#pragma endregion
