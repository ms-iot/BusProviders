// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.I2c.Provider;
using Windows.Foundation;
using Windows.Foundation.Metadata;
using System.Runtime.InteropServices.WindowsRuntime;

namespace SimulatedProvider
{

    public sealed class I2cProvider : II2cProvider
    {
        public IAsyncOperation<IReadOnlyList<II2cControllerProvider>> GetControllersAsync()
        {
            
            return new Task<IReadOnlyList<II2cControllerProvider>>(()=>
            {
                List<II2cControllerProvider> controllers = new List<II2cControllerProvider>();
                controllers.Add(new I2cControllerProvider());
                return controllers;
            }).AsAsyncOperation<IReadOnlyList<II2cControllerProvider>>();
            
        }
    }

    public sealed class I2cControllerProvider : II2cControllerProvider
    {
        internal I2cControllerProvider() { }
        public II2cDeviceProvider GetDeviceProvider(ProviderI2cConnectionSettings settings)
        {
            return new I2cDeviceProvider(settings);
        }
    }

    public sealed class I2cDeviceProvider : II2cDeviceProvider
    {
        ProviderI2cConnectionSettings connectionSettings;

        internal I2cDeviceProvider(ProviderI2cConnectionSettings settings)
        {
            connectionSettings = settings;
        }

        public string DeviceId
        {
            get
            {
                if (disposedValue)
                    throw new ObjectDisposedException("I2cDevice");

                return "SimulatedDeviceId";
            }
        }


        public void Read([WriteOnlyArray]byte[] buffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("I2cDevice");

            for (int i = 0; i < buffer.Length; i++)
            {
                buffer[i] = 1;
            }
        }

        public ProviderI2cTransferResult ReadPartial([WriteOnlyArray] byte[] buffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("I2cDevice");

            for (int i = 0; i < buffer.Length; i++)
            {
                buffer[i] = 1;
            }
            var result = new ProviderI2cTransferResult();
            result.BytesTransferred = (uint)buffer.Length;
            result.Status = ProviderI2cTransferStatus.FullTransfer;
            return result;
        }

        public void Write([ReadOnlyArray]byte[] buffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("I2cDevice");


        }

        public ProviderI2cTransferResult WritePartial([ReadOnlyArray]byte[] buffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("I2cDevice");

            var result = new ProviderI2cTransferResult();
            result.BytesTransferred = (uint) buffer.Length;
            result.Status = ProviderI2cTransferStatus.FullTransfer;
            return result;
        }

        public void WriteRead([ReadOnlyArray]byte[] writeBuffer, [WriteOnlyArray]  byte[] readBuffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("I2cDevice");

            for (int i = 0; i < readBuffer.Length; i++)
            {
                readBuffer[i] = 1;
            }
        }

        public ProviderI2cTransferResult WriteReadPartial([ReadOnlyArray]byte[] writeBuffer, [WriteOnlyArray]  byte[] readBuffer)
        {
            if (disposedValue)
                throw new ObjectDisposedException("I2cDevice");

            for (int i = 0; i < readBuffer.Length; i++)
            {
                readBuffer[i] = 1;
            }
            var result = new ProviderI2cTransferResult();
            result.BytesTransferred = (uint)readBuffer.Length + (uint)writeBuffer.Length;
            result.Status = ProviderI2cTransferStatus.FullTransfer;
            return result;
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
