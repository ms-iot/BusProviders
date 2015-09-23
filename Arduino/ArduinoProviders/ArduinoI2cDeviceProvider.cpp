// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoI2cDeviceProvider.h"  

using namespace ArduinoProviders;
using namespace Platform::Collections;

ArduinoI2cDeviceProvider::ArduinoI2cDeviceProvider(ProviderI2cConnectionSettings ^settings)
{
    _ConnectionSettings = settings;

    _ReadTimeout = 1000; //TODO: avoid INFINITE timeout ... give it a second to respond?
    _DataRead = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

    _Usb = ref new UsbSerial("VID_2341", "PID_0043");

    int baudRate = 115200;
    _Usb->begin(baudRate, SerialConfig::SERIAL_8N1);

    _Firmata = ref new UwpFirmata();
    _Firmata->begin(_Usb);

    _Firmata->I2cReplyReceived += 
        ref new I2cReplyCallbackFunction(
            [this](
                UwpFirmata ^caller, 
                I2cCallbackEventArgs^ args) -> void 
            {
                auto address = args->getAddress();

                _I2cData.insert_or_assign(address, DataReader::FromBuffer(args->getDataBuffer()));
                _I2cRegisters.insert_or_assign(address, args->getRegister());
                SetEvent(_DataRead);
            });
}

ArduinoI2cDeviceProvider::~ArduinoI2cDeviceProvider()
{
    CloseHandle(_DataRead);
    _Firmata->finish();
    _Usb->end();

}

Platform::String ^ ArduinoI2cDeviceProvider::DeviceId::get()
{
    return ref new Platform::String(L"Arduino");
}

ProviderI2cTransferResult ArduinoI2cDeviceProvider::WritePartial(
    const Platform::Array<unsigned char> ^buffer
    )
{
    SendI2cSysex(_ConnectionSettings->SlaveAddress, 0, buffer->Length, buffer->begin());
    ProviderI2cTransferResult result;
    result.BytesTransferred = buffer->Length;
    result.Status = ProviderI2cTransferStatus::FullTransfer;
    return result;
}

ProviderI2cTransferResult ArduinoI2cDeviceProvider::ReadPartial(
    Platform::WriteOnlyArray<unsigned char> ^buffer
    )
{
    uint8_t numBytes = buffer->Length;

    ResetEvent(_DataRead);
    SendI2cSysex(_ConnectionSettings->SlaveAddress, 0x08, 1, &numBytes);

    DWORD dwWaitResult = WaitForSingleObjectEx(_DataRead, _ReadTimeout, true);

    ProviderI2cTransferResult result;
    if (dwWaitResult == WAIT_OBJECT_0 && 
        (_I2cData.find(_ConnectionSettings->SlaveAddress) != _I2cData.end()))
    {
        // fill buffer with i2c reply data
        auto foundData = _I2cData.find(_ConnectionSettings->SlaveAddress);
        auto data = foundData->second;

        data->ReadBytes(buffer);
        
        result.BytesTransferred = buffer->Length;
        result.Status = (result.BytesTransferred == numBytes) ?
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

void ArduinoI2cDeviceProvider::SendI2cSysex(
    const uint8_t address,
    const uint8_t rw_mask,
    const uint8_t len,
    uint8_t *data
    )
{
    _Firmata->lock();
    try
    {
        _Firmata->write(static_cast<uint8_t>(Command::START_SYSEX));
        _Firmata->write(static_cast<uint8_t>(SysexCommand::I2C_REQUEST));
        _Firmata->write(address);
        _Firmata->write(rw_mask);

        if (data != nullptr)
        {
            for (size_t i = 0; i < len; ++i)
            {
                _Firmata->sendValueAsTwo7bitBytes(data[i]);
            }
        }

        _Firmata->write(static_cast<uint8_t>(Command::END_SYSEX));
        _Firmata->flush();
        _Firmata->unlock();
    }
    catch (...)
    {
        _Firmata->unlock();
    }
}

II2cDeviceProvider ^ ArduinoI2cControllerProvider::GetDeviceProvider(
    ProviderI2cConnectionSettings ^settings
    )
{
    return ref new ArduinoI2cDeviceProvider(settings);
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
