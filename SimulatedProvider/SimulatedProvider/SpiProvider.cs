// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Spi.Provider;
using Windows.Foundation;
using Windows.Foundation.Metadata;
using System.Runtime.InteropServices.WindowsRuntime;

namespace SimulatedProvider
{
    public sealed class SpiProvider : ISpiProvider
    {
        public IAsyncOperation<IReadOnlyList<ISpiControllerProvider>> GetControllersAsync()
        {
            return new Task<IReadOnlyList<ISpiControllerProvider>>(() =>
            {
                List<ISpiControllerProvider> controllers = new List<ISpiControllerProvider>();
                controllers.Add(new SpiControllerProvider());
                return controllers;
            }).AsAsyncOperation<IReadOnlyList<ISpiControllerProvider>>();
        }
    }

    public sealed class SpiControllerProvider : ISpiControllerProvider
    {
        internal SpiControllerProvider() { }
        public ISpiDeviceProvider GetDeviceProvider(ProviderSpiConnectionSettings settings)
        {
            return new SpiDeviceProvider(settings);
        }
    }

    public sealed class SpiDeviceProvider : ISpiDeviceProvider
    {
        ProviderSpiConnectionSettings connectionSettings;

        internal SpiDeviceProvider(ProviderSpiConnectionSettings settings)
        {
            connectionSettings = settings;
        }

        public ProviderSpiConnectionSettings ConnectionSettings
        {
            get
            {
                if (disposedValue)
                    throw new ObjectDisposedException("SpiDevice");

                return connectionSettings;
            }
        }

        public string DeviceId
        {
            get
            {
                if (disposedValue)
                    throw new ObjectDisposedException("SpiDevice");

                return "SimulatedDeviceId";
            }
        }

        public void Read([WriteOnlyArray] byte[] buffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("SpiDevice");

            for (int i = 0; i < buffer.Length; i++)
            {
                buffer[i] = 1;
            }   
        }

        public void TransferFullDuplex([ReadOnlyArray] byte[] writeBuffer, [WriteOnlyArray] byte[] readBuffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("SpiDevice");

            for (int i = 0; i < readBuffer.Length; i++)
            {
                readBuffer[i] = 1;
            }
        }

        public void TransferSequential([ReadOnlyArray]byte[] writeBuffer, [WriteOnlyArray] byte[] readBuffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("SpiDevice");

            for (int i = 0; i < readBuffer.Length; i++)
            {
                readBuffer[i] = 1;
            }
        }

        public void Write([ReadOnlyArray] byte[] buffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("SpiDevice");
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    
                }


                disposedValue = true;
            }
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            Dispose(true);

        }
        #endregion

    }
}
