# Microsoft.Iot.Lightning.Providers
Repository for Windows IoT Core Bus Provider Libraries for 

## Using the Microsoft.Iot.Lightning.Providers library and sample code
The Microsoft.Iot.Lightning.Providers library provides a set of Windows.Devices.*Providers WinRT APIs enabling apps to use the new Lightning driver to communicate with and control GPIO, I2C and SPI devices.
The current version includes this set of classes:
1. Microsoft.IoT.Lightning.Providers.Provider: Implements ILowLevelDevicesAggregateProvider and is the top level API to enable retrieving controller providers below.
2. Microsoft.IoT.Lightning.Providers.Gpio*: Lightning GPIO provider implementation
3. Microsoft.IoT.Lightning.Providers I2c*:  Lightning I2c provider implementation
4. Microsoft.IoT.Lightning.Providers Spi*:  Lightning Spi provider implementation

## API Usage
The general pattern of the Lightning provider (or any other WinRT component implementing the Windows.Devices.*Provider interfaces) is to retrieve the top level controller from the provider and then use the controller the same way as the default controllers; e.g.
using Microsoft.Iot.Lightning.Providers;

The easiest way to use these providers is to set the Lightning Provider as the default provider for an application. The code below will, if the Lightning Provider is available, set it as the default provider. If no default provider is explicitly set, the various busses will fall back to the normal driver/controllers.
```C#
if (Microsoft.Iot.Lightning.Providers.Provider.IsLightningEnabled)
{
   Windows.Devices.LowLevelDevicesController.DefaultProvider =  new Microsoft.Iot.Lightning.Providers.Provider();
}

gpioController = await GpioController.GetDefaultAsync();
i2cController = await I2cController.GetDefaultAsync();
spiController = await SpiController.GetDefaultAsync();
```

After you have a controller for the desired bus, you can use it as you normally would. If you want to use a different default provider, the sections below show how you can use the Lightning providers for individual busses. 


### For GPIO
```C#
GpioController gpioController = 
        (await GpioController.GetControllersAsync(GpioProvider.GetGpioProvider()))[0];
GpioPin pin = gpioController.OpenPin(LED_PIN, GpioSharingMode.Exclusive);
```

### For I2C
```C#
I2cController controller = 
         (await I2cController.GetControllersAsync(I2cProvider.GetI2cProvider()))[0];
I2cDevice sensor = controller.GetDevice(new I2cConnectionSettings(0x40));
```

### For SPI
```C#
SpiController controller = 
         (await SpiController.GetControllersAsync(SpiProvider.GetSpiProvider()))[0];
SpiDevice SpiDisplay = controller.GetDevice(settings); 
```
Checking for the Lightning (DMAP) driver
It’s also a good practice to verify if the required Lightning provider is installed before calling the provider APIs. That way dual-mode code could be written; e.g. here’s the code for GPIO. The same pattern can be used for all other providers:
```C#
GpioController gpioController;
bool isLightningEnabled = Provider.IsLightningEnabled;
if (isLightningEnabled)
{
    gpioController = 
        (await GpioController.GetControllersAsync(GpioProvider.GetGpioProvider()))[0];
}
else
{
    gpioController = GpioController.GetDefault();
}

// All following code can be used for both default and provider GPIO calls
GpioPin pin = gpioController.OpenPin(LED_PIN, GpioSharingMode.Exclusive);
Each of the samples includes with the library show the same pattern.
```

## Build Requirements
The Lightning Provider library depends on the Microsoft.IoT.Lightning Nuget package (https://www.nuget.org/packages/Microsoft.IoT.Lightning), which in turn requires the Arduino SDK Nuget (https://www.nuget.org/packages/Microsoft.IoT.SDKFromArduino). Both Nuget packages are included in the library solution, and are available from Nuget.org. Source code for each is also on GitHub.
Note, currently Microsoft.IoT.Lightning Nuget is still pre-release, so should be updated from Nuget.org, when newer versions are available.
### SDK version
Additionally, the following version of the Windows SDK should be available for building and using the library: 10.0.10556.0 

## Runtime Requirements
The APIs in the Lightning Provider library require the Lightning (DMAP) driver to be enabled on the target device. Both Raspberry Pi2 and MinnowBoard Max have the driver available, but not enabled by default. The driver can be enabled using the Windows Devices Web Portal.
Refer to the “Enabling Lightning Driver” <link tbd> guide for additional information on how to enable the DMAP driver.
