// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices;
using Windows.Devices.Gpio.Provider;
using Windows.Devices.Spi.Provider;
using Windows.Devices.I2c.Provider;
using Windows.Devices.Adc.Provider;
using Windows.Devices.Pwm.Provider;

namespace SimulatedProvider
{
    public sealed class AggregateProvider : ILowLevelDevicesAggregateProvider
    {
        public IAdcControllerProvider AdcControllerProvider
        {
            get
            {
                return new AdcControllerProvider();
            }
        }

        public IGpioControllerProvider GpioControllerProvider
        {
            get
            {
                return new GpioControllerProvider();
            }
        }

        public II2cControllerProvider I2cControllerProvider
        {
            get
            {
                return new I2cControllerProvider();
            }
        }

        public IPwmControllerProvider PwmControllerProvider
        {
            get
            {
                return new PwmControllerProvider();
            }
        }

        public ISpiControllerProvider SpiControllerProvider
        {
            get
            {
                return new SpiControllerProvider();
            }
        }
    }
}
