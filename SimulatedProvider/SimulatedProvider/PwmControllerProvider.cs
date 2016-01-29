// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Pwm.Provider;

namespace SimulatedProvider
{
    public sealed class PwmProvider : IPwmProvider
    {
        public IReadOnlyList<IPwmControllerProvider> GetControllers()
        {
            List<IPwmControllerProvider> providers = new List<IPwmControllerProvider>();
            providers.Add(new PwmControllerProvider());
            return providers;
        }
    }

    public sealed class PwmControllerProvider : IPwmControllerProvider
    {
        private double actualFrequency;
        private double maxFrequency;
        private double minFrequency;
        private int pinCount;
        private List<bool> pins;

        internal PwmControllerProvider()
        {
            actualFrequency = 0;
            maxFrequency = 50;
            minFrequency = 0;
            pinCount = 16;
            pins = new List<bool>(0);
            for (int i = 0; i < pinCount; i++)
            {
                pins.Add(false);
            }
        }

        public double ActualFrequency
        {
            get
            {
                return actualFrequency;
            }
        }

        public double MaxFrequency
        {
            get
            {
                return maxFrequency;
            }
        }

        public double MinFrequency
        {
            get
            {
                return minFrequency;
            }
        }

        public int PinCount
        {
            get
            {
                return pinCount;
            }
        }

        public void AcquirePin(int pin)
        {
            if (pin < 0 || pin >= pinCount)
            {
                throw new InvalidOperationException("Invalid pin specified");
            }
            if (pins[pin])
            {
                throw new InvalidOperationException("Pin is already acquired");
            }
            else
            {
                pins[pin] = true;
            }
        }

        public void DisablePin(int pin)
        {
            if (pin < 0 || pin >= pinCount)
            {
                throw new InvalidOperationException("Invalid pin specified");
            }
            if (!pins[pin])
            {
                throw new InvalidOperationException("Pin is not acquired");
            }
        }

        public void EnablePin(int pin)
        {
            if (pin < 0 || pin >= pinCount)
            {
                throw new InvalidOperationException("Invalid pin specified");
            }
            if (!pins[pin])
            {
                throw new InvalidOperationException("Pin is not acquired");
            }
        }

        public void ReleasePin(int pin)
        {
            if (pin < 0 || pin >= pinCount)
            {
                throw new InvalidOperationException("Invalid pin specified");
            }
            if (!pins[pin])
            {
                throw new InvalidOperationException("Pin is not acquired");
            }
            else
            {
                pins[pin] = false;
            }
        }

        public double SetDesiredFrequency(double frequency)
        {
            if (frequency < minFrequency || frequency > maxFrequency)
            {
                throw new InvalidOperationException("Frequency out of supported range");
            }
            else
            {
                actualFrequency = frequency;
               
            }
            return actualFrequency;
        }

        public void SetPulseParameters(int pin, double dutyCycle, bool invertPolarity)
        {
            if (pin < 0 || pin >= pinCount)
            {
                throw new InvalidOperationException("Invalid pin specified");
            }
            else if (!pins[pin])
            {
                throw new InvalidOperationException("Pin is not acquired");
            }

        }
    }
}
