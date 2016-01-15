// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Adc.Provider;

namespace SimulatedProvider
{
    public sealed class AdcProvider : IAdcProvider
    {
        public IReadOnlyList<IAdcControllerProvider> GetControllers()
        {
            List<IAdcControllerProvider> controllers = new List<IAdcControllerProvider>();
            controllers.Add(new AdcControllerProvider());
            return controllers;
        }
    }

    public sealed class AdcControllerProvider : IAdcControllerProvider
    {

        private ProviderAdcChannelMode channelMode;
        private int channelCount;
        private int maxValue;
        private int minValue;
        private int resolutionInBits;
        private List<bool> channelsAcquired;
        private const int defaultReading = 37;
        internal AdcControllerProvider()
        {
            ChannelMode = ProviderAdcChannelMode.SingleEnded;
            channelCount = 4;
            maxValue = 1024;
            minValue = 0;
            resolutionInBits = 10;
            channelsAcquired = new List<bool>(0);
            for (int i = 0; i < channelCount; i++)
            {
                channelsAcquired.Add(false);
            }
        }

        public int ChannelCount
        {
            get
            {
                return channelCount;
            }
        }

        public ProviderAdcChannelMode ChannelMode
        {
            get
            {
                return channelMode;
            }

            set
            {
                channelMode = value;
            }
        }

        public int MaxValue
        {
            get
            {
                return maxValue;
            }
        }

        public int MinValue
        {
            get
            {
                return minValue;
            }
        }

        public int ResolutionInBits
        {
            get
            {
                return resolutionInBits;
            }
        }

        public void AcquireChannel(int channel)
        {
            if (channel < 0 || channel >= channelCount)
            {
                throw new InvalidOperationException(String.Format("Channel %0 not supported", channel));
            }
            if (channelsAcquired[channel])
            {
                throw new InvalidOperationException("Channel is already acquired");
            }
            else
            {
                channelsAcquired[channel] = true;
            }
        }

        public bool IsChannelModeSupported(ProviderAdcChannelMode channelMode)
        {
            return true;
        }

        public int ReadValue(int channelNumber)
        {
            return defaultReading;
        }

        public void ReleaseChannel(int channel)
        {
            if (channel < 0 || channel >= channelCount)
            {
                throw new InvalidOperationException(String.Format("Channel %0 not supported", channel));
            }
            if (!channelsAcquired[channel])
            {
                throw new InvalidOperationException("Channel is not acquired");
            }
            else
            {
                channelsAcquired[channel] = false;
            }
        }
    }
}
