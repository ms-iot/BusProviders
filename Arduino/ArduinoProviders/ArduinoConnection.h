// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include "ArduinoConnectionConfiguration.h"

using namespace Windows::Devices::Gpio::Provider;

using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::RemoteWiring;

namespace ArduinoProviders
{
    ref class ArduinoConnection sealed
    {

    public:
        static IAsyncOperation<RemoteDevice^>^ GetArduinoConnectionAsync();

        static property ArduinoConnectionConfiguration^ Configuration
        {
            ArduinoConnectionConfiguration^ get();
            void set(ArduinoConnectionConfiguration^ value);
        }

    private:
        static bool WaitForConnection(unsigned int timeout = INFINITE);

        static ArduinoConnectionConfiguration ^_ArduinoConnectionConfiguration;

        static HANDLE _ConnectedEvent;
        static std::mutex _ConnectionMutex;

        static UsbSerial ^_Usb;
        static RemoteDevice ^_Arduino;
        static bool _Connected;
    };
}