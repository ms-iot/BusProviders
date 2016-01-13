# Microsoft.Iot.Lightning.Providers
 Windows IoT Core WinRT Bus Provider Library for Lightning. 

## Using the Microsoft.Iot.Lightning.Providers library and sample code
The Microsoft.Iot.Lightning.Providers library provides a set of Windows.Devices.*Providers WinRT APIs enabling apps to use the new Lightning driver to communicate with and control controller devices.

The current version includes this set of classes:

1. Microsoft.IoT.Lightning.Providers.Provider: Implements ILowLevelDevicesAggregateProvider and is the top level API to enable retrieving controller providers below.

1. Microsoft.IoT.Lightning.Providers.Gpio.*: Lightning GPIO provider implementation

1. Microsoft.IoT.Lightning.Providers I2c.*:  Lightning I2c provider implementation

1. Microsoft.IoT.Lightning.Providers Spi.*:  Lightning Spi provider implementation

1. Microsoft.IoT.Lightning.Providers Adc.*:  Lightning Adc provider implementation

1. Microsoft.IoT.Lightning.Providers Pwm.*:  Lightning Pwm provider implementation

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

You also need to update your manifest to include the required capabilities to use the Lightning driver. You will need to view the source of the "Package.appxmanifest" file in your projects and add the following lines to the end of the <Capabilities> section of the manifest:

    <iot:Capability Name="lowLevelDevices" />
    <DeviceCapability Name="109b86ad-f53d-4b76-aa5f-821e2ddf2141"/>

To see a complete manifest that includes these capabilities, you can look at this file in the BlinkyBackground sample. 

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
### Visual Studio 2015 Update 1 and Windows SDK 10.0.10586 required
(Visual Studio 2015 with Update 1)[https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx] is required for building and deploying projects.
Additionally, the minimum version of the Windows SDK required for building and using the library is Windows 10 SDK Version 1511 (build 10.0.10586), included in Universal Windows Apps v1.2.
Both Visual Studio Update 1 and Tools for Universal Windows Apps v1.2 are available by running setup using the following link: http://go.microsoft.com/fwlink/?LinkID=534599 .


## Runtime Requirements
The APIs in the Lightning Provider library require the Lightning (DMAP) driver to be enabled on the target device. Both Raspberry Pi2 and MinnowBoard Max have the driver available, but not enabled by default. The driver can be enabled using the Windows Devices Web Portal.

Refer to [Enabling Lightning Driver](https://ms-iot.github.io/content/en-US/win10/LightningSetup.htm) guide for additional information on how to enable the DMAP driver.
