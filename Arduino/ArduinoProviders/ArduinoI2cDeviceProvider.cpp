// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoI2cDeviceProvider.h"
#include "ArduinoConnection.h"
#include <ppltasks.h>

using namespace concurrency;


using namespace ArduinoProviders;
using namespace Platform::Collections;

ArduinoI2cDeviceProvider::ArduinoI2cDeviceProvider(RemoteDevice^ arduino, ProviderI2cConnectionSettings ^settings)
{
    _ConnectionSettings = settings;

    _DataRead = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

    _Arduino = arduino;

    _Arduino->I2c->I2cReplyEvent +=
            ref new I2cReplyCallback(
                [this](
                    uint8_t address_, 
                    uint8_t reg_, 
                    Windows::Storage::Streams::DataReader ^response
                    ) -> void 
                {
                    std::lock_guard<std::mutex> lock(_DataReaderMutex);

                    auto byteArrayCopy = ref new Platform::Array<unsigned char>(response->UnconsumedBufferLength);
                    response->ReadBytes(byteArrayCopy);
                    _I2cData.insert_or_assign(address_, byteArrayCopy);
                    _I2cRegisters.insert_or_assign(address_, reg_);
                    SetEvent(_DataRead);
                });

    _Arduino->I2c->enable(500);
}

ArduinoI2cDeviceProvider::~ArduinoI2cDeviceProvider()
{
    CloseHandle(_DataRead);
}

Platform::String ^ ArduinoI2cDeviceProvider::DeviceId::get()
{
    return ref new Platform::String(L"Arduino");
}

ProviderI2cTransferResult ArduinoI2cDeviceProvider::WritePartial(
    const Platform::Array<unsigned char> ^buffer
    )
{
    _Arduino->I2c->beginTransmission(_ConnectionSettings->SlaveAddress);
    for (size_t i = 0; i < buffer->Length; ++i)
    {
        _Arduino->I2c->write(buffer->begin()[i]);
    }
    _Arduino->I2c->endTransmission();

    ProviderI2cTransferResult result;
    result.BytesTransferred = buffer->Length;
    result.Status = ProviderI2cTransferStatus::FullTransfer;
    return result;
}

ProviderI2cTransferResult ArduinoI2cDeviceProvider::ReadPartial(
    Platform::WriteOnlyArray<unsigned char> ^buffer
    )
{
    ResetEvent(_DataRead);
    _Arduino->I2c->requestFrom(_ConnectionSettings->SlaveAddress, buffer->Length);

    DWORD dwWaitResult = WaitForSingleObjectEx(_DataRead, INFINITE, true);

    ProviderI2cTransferResult result;
    if (dwWaitResult == WAIT_OBJECT_0 && 
        (_I2cData.find(_ConnectionSettings->SlaveAddress) != _I2cData.end()))
    {
        std::lock_guard<std::mutex> lock(_DataReaderMutex);

        // fill buffer with i2c reply data
        auto foundData = _I2cData.find(_ConnectionSettings->SlaveAddress);
        auto data = foundData->second;
        auto dataLength = data->Length;
        auto expectedLength = buffer->Length;
 
        for (unsigned int i = 0; i < ((dataLength < expectedLength) ? dataLength : expectedLength); i++)
        {
            buffer[i] = data[i];
        }
        
        result.BytesTransferred = dataLength;
        result.Status = (dataLength == expectedLength) ?
            result.Status = ProviderI2cTransferStatus::FullTransfer :
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
    }
    else
    {
        result.Status = ProviderI2cTransferStatus::SlaveAddressNotAcknowledged;
    }

    return result;
}

ProviderI2cTransferResult ArduinoI2cDeviceProvider::WriteReadPartial(
    const Platform::Array<unsigned char> ^writeBuffer,
    Platform::WriteOnlyArray<unsigned char> ^readBuffer
    )
{
    WritePartial(writeBuffer);
    
    auto expectedReadLength = readBuffer->Length;
    ProviderI2cTransferResult readResult = ReadPartial(readBuffer);

    ProviderI2cTransferResult result;
    if (readResult.Status == ProviderI2cTransferStatus::SlaveAddressNotAcknowledged)
    {
        result.Status = ProviderI2cTransferStatus::SlaveAddressNotAcknowledged;
    }
    else
    {
        result.BytesTransferred = writeBuffer->Length + readResult.BytesTransferred;
        result.Status = (readResult.BytesTransferred == expectedReadLength) ?
            result.Status = ProviderI2cTransferStatus::FullTransfer :
            result.Status = ProviderI2cTransferStatus::PartialTransfer;
    }
    return result;
}

ArduinoI2cControllerProvider::ArduinoI2cControllerProvider()
{
    _Arduino = create_task(ArduinoConnection::GetArduinoConnectionAsync()).get();
}

II2cDeviceProvider ^ ArduinoI2cControllerProvider::GetDeviceProvider(
    ProviderI2cConnectionSettings ^settings
    )
{
    return ref new ArduinoI2cDeviceProvider(_Arduino, settings);
}

IAsyncOperation<IVectorView<II2cControllerProvider^>^>^ ArduinoI2cProvider::GetControllersAsync(
    void
    )
{
    return concurrency::create_async([]() -> IVectorView<II2cControllerProvider^>^ {

        auto controllerCollection = ref new Vector<II2cControllerProvider^>();
        controllerCollection->Append(ref new ArduinoI2cControllerProvider());
        return controllerCollection->GetView();

    });
}
