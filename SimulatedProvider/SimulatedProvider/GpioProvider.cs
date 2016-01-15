// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Gpio.Provider;
using Windows.Foundation;

namespace SimulatedProvider
{
    public sealed class GpioProvider : IGpioProvider
    {
        public IReadOnlyList<IGpioControllerProvider> GetControllers()
        {
            List<IGpioControllerProvider> providers = new List<IGpioControllerProvider>();
            providers.Add(new GpioControllerProvider());
            return providers;
        }
    }

    public sealed class GpioControllerProvider : IGpioControllerProvider
    {
        private int pinCount;
        private List<GpioPinProvider> pinProviders;

        internal GpioControllerProvider()
        {
            pinCount = 42;
            pinProviders = new List<GpioPinProvider>(0);
            for (int i = 0; i < 42; i++)
            {
                pinProviders.Add(null);
            }
        }

        public int PinCount
        {
            get
            {
                return pinCount;
            }
        }

        public IGpioPinProvider OpenPinProvider(int pin, ProviderGpioSharingMode sharingMode)
        {
            if (pin >= pinCount)
            {
                throw new InvalidOperationException("Pin is not valid");
            }
            else
            {
                if (pinProviders[pin] == null)
                {                 
                    pinProviders[pin] = new GpioPinProvider(pin, sharingMode,this);
                    return pinProviders[pin];
                }else if (sharingMode == ProviderGpioSharingMode.Exclusive)
                {
                    throw new InvalidOperationException("Pin is already opened and cannot be opened in exclusive mode");
                }else if (pinProviders[pin].SharingMode == ProviderGpioSharingMode.Exclusive)
                {
                    throw new InvalidOperationException("Pin al already opened in exclusive mode");
                }
                else
                {
                    return pinProviders[pin];
                }
            }
        }

        internal void ClosepinProvider(GpioPinProvider pin)
        {
            pinProviders[pin.PinNumber] = null;
        }
    }

    public sealed class GpioPinProvider : IGpioPinProvider
    {
        private int pinNumber;
        private ProviderGpioSharingMode sharingMode;
        private TimeSpan debounceTimeout;
        private ProviderGpioPinDriveMode driveMode;
        GpioControllerProvider controllerProvider;
        internal GpioPinProvider(int pin, ProviderGpioSharingMode desiredSharingMode, GpioControllerProvider controller)
        {
            pinNumber = pin;
            sharingMode = desiredSharingMode;
            debounceTimeout = TimeSpan.FromMilliseconds(20);
            driveMode = ProviderGpioPinDriveMode.Output;
        }

        public TimeSpan DebounceTimeout
        {
            get
            {
                return debounceTimeout;
            }

            set
            {
                debounceTimeout = value;
            }
        }

        public int PinNumber
        {
            get
            {
                return pinNumber;
            }
        }

        public ProviderGpioSharingMode SharingMode
        {
            get
            {
                return sharingMode;
            }
        }

        public event TypedEventHandler<IGpioPinProvider, GpioPinProviderValueChangedEventArgs> ValueChanged;

        public ProviderGpioPinDriveMode GetDriveMode()
        {
            return driveMode;
        }

        public bool IsDriveModeSupported(ProviderGpioPinDriveMode driveMode)
        {
            return true;
        }

        public ProviderGpioPinValue Read()
        {
            return ProviderGpioPinValue.High;
        }

        public void SetDriveMode(ProviderGpioPinDriveMode value)
        {
            driveMode = value;
        }

        public void Write(ProviderGpioPinValue value)
        {
           
        }

        ~GpioPinProvider()
        {
            controllerProvider.ClosepinProvider(this);
        }
    }
}
