// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"
#include "Provider.h"
#include "I2cDeviceProvider.h"

#include "boardpins.h"
#include "BcmI2cController.h"
#include "BtI2cController.h"

using namespace Microsoft::IoT::Lightning::Providers;

#pragma region LightningI2cProvider

II2cProvider^ LightningI2cProvider::providerSingleton = nullptr;

II2cProvider^ LightningI2cProvider::GetI2cProvider()
{
    if (providerSingleton == nullptr)
    {
        providerSingleton = ref new LightningI2cProvider();
    }

    return providerSingleton;
}

IAsyncOperation<IVectorView<II2cControllerProvider^>^>^ LightningI2cProvider::GetControllersAsync(
    void
    )
{
    return concurrency::create_async([]() -> IVectorView<II2cControllerProvider^>^ {

        auto controllerCollection = ref new Vector<II2cControllerProvider^>();
        controllerCollection->Append(ref new LightningI2cControllerProvider(EXTERNAL_I2C_BUS));

        // RPI2 has a second I2C bus

        BoardPinsClass::BOARD_TYPE board;
        HRESULT hr = g_pins.getBoardType(board);

        if (SUCCEEDED(hr) && board == BoardPinsClass::BOARD_TYPE::PI2_BARE)
        {
            controllerCollection->Append(ref new LightningI2cControllerProvider(SECOND_EXTERNAL_I2C_BUS));
        }

        return controllerCollection->GetView();

    });
}

#pragma endregion

#pragma region LightningI2cControllerProvider

II2cDeviceProvider ^ LightningI2cControllerProvider::GetDeviceProvider(
    ProviderI2cConnectionSettings ^settings
    )
{
    return ref new LightningI2cDeviceProvider(settings, _busNumber);
}

#pragma endregion

#pragma region LightningI2cDeviceProvider

LightningI2cDeviceProvider::LightningI2cDeviceProvider(ProviderI2cConnectionSettings ^settings, ULONG busNumber) : _busNumber(busNumber)
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
        _I2cController.reset(new BtI2cControllerClass());
        hr = _I2cController->configurePins(BARE_MBM_PIN_I2C_DAT, BARE_MBM_PIN_I2C_CLK);
    }
    else if (board == BoardPinsClass::BOARD_TYPE::PI2_BARE)
    {
        _I2cController.reset(new BcmI2cControllerClass());
        if (_busNumber == SECOND_EXTERNAL_I2C_BUS)
        {
            hr = _I2cController->configurePins(BARE_PI2_PIN_I2C0_DAT, BARE_PI2_PIN_I2C0_CLK);
        }
        else
        {
            hr = _I2cController->configurePins(BARE_PI2_PIN_I2C1_DAT, BARE_PI2_PIN_I2C1_CLK);
        }
    }
    else
    {
        throw ref new Platform::NotImplementedException(L"This board type has not been implemented.");
    }

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred while configuring pins for I2c");
    }

    hr = _I2cController->begin(busNumber);

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"An error occurred while initializing the I2c controller");
    }

    _i2cTransaction.reset(new I2cTransactionClass());

    hr = _i2cTransaction->setAddress(_ConnectionSettings->SlaveAddress);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not set i2c salve address.");
    }

    if (_ConnectionSettings->BusSpeed == ProviderI2cBusSpeed::FastMode)
    {
        _i2cTransaction->useHighSpeed();
    }

}

ProviderI2cTransferResult LightningI2cDeviceProvider::WritePartial(const Platform::Array<unsigned char>^ buffer)
{
    ProviderI2cTransferResult result;
    result.BytesTransferred = 0;
    result.Status = ProviderI2cTransferStatus::PartialTransfer;

    _i2cTransaction->reset();

    HRESULT hr = _i2cTransaction->queueWrite(buffer->Data, buffer->Length);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->execute(_I2cController.get());

    if (SUCCEEDED(hr) && !_i2cTransaction->isIncomplete())
    {
        result.Status = ProviderI2cTransferStatus::FullTransfer;
        result.BytesTransferred = buffer->Length;
    }
    else if (FAILED(hr))
    {
        if (_i2cTransaction->getError() == I2cTransactionClass::ERROR_CODE::ADR_NACK)
        {
            result.Status = ProviderI2cTransferStatus::SlaveAddressNotAcknowledged;
        }
        else if (_i2cTransaction->getError() == I2cTransactionClass::ERROR_CODE::DATA_NACK)
        {
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
        }
        else
        {
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
        }
    }
    else if (_i2cTransaction->isIncomplete())
    {
        result.Status = ProviderI2cTransferStatus::PartialTransfer;
    }

    return result;
}

ProviderI2cTransferResult LightningI2cDeviceProvider::ReadPartial(Platform::WriteOnlyArray<unsigned char>^ buffer)
{
    ProviderI2cTransferResult result;
    result.BytesTransferred = 0;
    result.Status = ProviderI2cTransferStatus::PartialTransfer;

    _i2cTransaction->reset();
    HRESULT hr = _i2cTransaction->queueRead(buffer->Data, buffer->Length);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->execute(_I2cController.get());

    if (SUCCEEDED(hr) && !_i2cTransaction->isIncomplete())
    {
        result.Status = ProviderI2cTransferStatus::FullTransfer;
        result.BytesTransferred = buffer->Length;
    }
    else if (FAILED(hr))
    {
        if (_i2cTransaction->getError() == I2cTransactionClass::ERROR_CODE::ADR_NACK)
        {
            result.Status = ProviderI2cTransferStatus::SlaveAddressNotAcknowledged;
        }
        else if (_i2cTransaction->getError() == I2cTransactionClass::ERROR_CODE::DATA_NACK)
        {
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
        }
        else
        {
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
        }
    }
    else if (_i2cTransaction->isIncomplete())
    {
        result.Status = ProviderI2cTransferStatus::PartialTransfer;
    }

    return result;
}

ProviderI2cTransferResult LightningI2cDeviceProvider::WriteReadPartial(const Platform::Array<unsigned char>^ writeBuffer, Platform::WriteOnlyArray<unsigned char>^ readBuffer)
{
    ProviderI2cTransferResult result;
    result.BytesTransferred = 0;
    result.Status = ProviderI2cTransferStatus::PartialTransfer;

    _i2cTransaction->reset();

    HRESULT hr = _i2cTransaction->queueWrite(writeBuffer->Data, writeBuffer->Length);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->queueRead(readBuffer->Data, readBuffer->Length);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->execute(_I2cController.get());

    if (SUCCEEDED(hr) && !_i2cTransaction->isIncomplete())
    {
        result.Status = ProviderI2cTransferStatus::FullTransfer;
        result.BytesTransferred = writeBuffer->Length + readBuffer->Length;
    }
    else if (FAILED(hr))
    {
        if (_i2cTransaction->getError() == I2cTransactionClass::ERROR_CODE::ADR_NACK)
        {
            result.Status = ProviderI2cTransferStatus::SlaveAddressNotAcknowledged;
        }
        else if (_i2cTransaction->getError() == I2cTransactionClass::ERROR_CODE::DATA_NACK)
        {
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
        }
        else
        {
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
        }
    }
    else if (_i2cTransaction->isIncomplete())
    {
        result.Status = ProviderI2cTransferStatus::PartialTransfer;
    }

    return result;
}

void LightningI2cDeviceProvider::Write(const Platform::Array<unsigned char>^ buffer)
{
    _i2cTransaction->reset();

    HRESULT hr = _i2cTransaction->queueWrite(buffer->Data, buffer->Length);
    if (FAILED(hr))
    {
        // Clean out the transaction so it can be used again in the future.
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->execute(_I2cController.get());

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Transfer failed.");
    }
    else if (_i2cTransaction->isIncomplete())
    {
        LightningProvider::ThrowError(E_ABORT, L"Transfer incomplete.");
    }
}

void LightningI2cDeviceProvider::Read(Platform::WriteOnlyArray<unsigned char>^ buffer)
{
    _i2cTransaction->reset();
    HRESULT hr = _i2cTransaction->queueRead(buffer->Data, buffer->Length);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->execute(_I2cController.get());

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Transfer failed.");
    }
    else if (_i2cTransaction->isIncomplete())
    {
        LightningProvider::ThrowError(E_ABORT, L"Transfer incomplete.");
    }
}

void LightningI2cDeviceProvider::WriteRead(const Platform::Array<unsigned char>^ writeBuffer, Platform::WriteOnlyArray<unsigned char>^ readBuffer)
{
    _i2cTransaction->reset();

    HRESULT hr = _i2cTransaction->queueWrite(writeBuffer->Data, writeBuffer->Length);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->queueRead(readBuffer->Data, readBuffer->Length);
    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Could not queue I2c transaction.");
    }

    hr = _i2cTransaction->execute(_I2cController.get());

    if (FAILED(hr))
    {
        LightningProvider::ThrowError(hr, L"Transfer failed.");
    }
    else if (_i2cTransaction->isIncomplete())
    {
        LightningProvider::ThrowError(E_ABORT, L"Transfer incomplete.");
    }

}

LightningI2cDeviceProvider::~LightningI2cDeviceProvider()
{
    if (_I2cController != nullptr)
    {
        // Set all I2C pins as digital I/O.
        // This frees up the external SPI bus so its pins can be used for other functions
        _I2cController->end();
    }
}

Platform::String ^ LightningI2cDeviceProvider::DeviceId::get()
{
    LPCWSTR deviceName = L"Lightning I2c Device";

    BoardPinsClass::BOARD_TYPE board;
    HRESULT hr = g_pins.getBoardType(board);

    if (SUCCEEDED(hr))
    {
        if (board == BoardPinsClass::BOARD_TYPE::MBM_BARE)
        {
            deviceName = mbmI2cDeviceName;
        }
        else if (board == BoardPinsClass::BOARD_TYPE::PI2_BARE)
        {
            switch (_busNumber)
            {
            case EXTERNAL_I2C_BUS:
                deviceName = pi2I2c1DeviceName;
                break;
            case SECOND_EXTERNAL_I2C_BUS:
                deviceName = pi2I2c0DeviceName;
                break;
            default:
                hr = DMAP_E_I2C_INVALID_BUS_NUMBER_SPECIFIED;
            }
        }
    }

    return ref new Platform::String(deviceName);
}

#pragma endregion
