using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace ArduinoConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            Windows.Devices.LowLevelDevicesController.DefaultProvider = new ArduinoProviders.ArduinoProvider();
            var adcController = await Windows.Devices.Adc.AdcController.GetDefaultAsync();
            var pwmController = await Windows.Devices.Pwm.PwmController.GetDefaultAsync();
            var gpioController = await Windows.Devices.Gpio.GpioController.GetDefaultAsync();
            var i2cController = await Windows.Devices.I2c.I2cController.GetDefaultAsync();

            // Should be null as we have no firmata spi implementation yet
            // var spiController = await Windows.Devices.Spi.SpiController.GetDefaultAsync(); 

        }
    }
}
