// Copyright (c) Microsoft. All rights reserved.
#pragma once

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
        static property RemoteDevice^ Arduino { RemoteDevice^ get(); }
        static property bool Connected { bool get(); }

    private:

        static UsbSerial ^_Usb;
        static RemoteDevice ^_Arduino;
        static bool _Connected;
    };
}