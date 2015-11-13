// Copyright (c) Microsoft. All rights reserved.
#pragma once

using namespace Windows::Devices;
using namespace Windows::Devices::Adc::Provider;
using namespace Windows::Devices::Pwm::Provider;
using namespace Windows::Devices::Gpio::Provider;
using namespace Windows::Devices::I2c::Provider;
using namespace Windows::Devices::Spi::Provider;

namespace Microsoft {
    namespace IoT {
        namespace Lightning {
            namespace Providers {

                public ref class LightningProvider sealed : public ILowLevelDevicesAggregateProvider
                {
                public:

                    // Inherited via ILowLevelDevicesAggregateProvider
                    virtual property IAdcControllerProvider ^ AdcControllerProvider
                    {
                        IAdcControllerProvider ^get();
                    }

                    virtual property IGpioControllerProvider ^ GpioControllerProvider
                    {
                        IGpioControllerProvider ^get();
                    }

                    virtual property II2cControllerProvider ^ I2cControllerProvider
                    {
                        II2cControllerProvider ^get();
                    }

                    virtual property IPwmControllerProvider ^ PwmControllerProvider
                    {
                        IPwmControllerProvider ^get();
                    }

                    virtual property ISpiControllerProvider ^ SpiControllerProvider
                    {
                        ISpiControllerProvider ^get();
                    }

                    static property bool IsLightningEnabled
                    {
                        bool get();
                    }

                    static ILowLevelDevicesAggregateProvider^ GetAggregateProvider();

                internal:
                    static void ThrowError(HRESULT hr, LPCWSTR errorMessage);
                    static int MapGpioPin(BoardPinsClass::BOARD_TYPE boardType, int pin);
               
                private:
                    LightningProvider() { }
                    static ILowLevelDevicesAggregateProvider^ providerSingleton;

                };
            }
        }
    }
}