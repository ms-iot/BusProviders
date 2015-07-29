#include "pch.h"
#include "PwmProvider.h"  
#include "PwmControllerProvider.h"  

using namespace PwmPCA9685;
using namespace Platform;
using namespace Windows::Devices::I2c;
using namespace Windows::Devices::Pwm::Provider;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace concurrency;

Windows::Devices::Pwm::Provider::IPwmProvider^ PwmProviderPCA9685::providerSingleton = nullptr;

PwmProviderPCA9685::PwmProviderPCA9685()
{
}




IPwmControllerCollection^ PwmProviderPCA9685::GetControllers()
{
	PwmControllerProviderPCA9685^ provider = ref new PwmControllerProviderPCA9685();  

	Platform::Collections::Vector<IPwmControllerProvider^>^ controllerCollection =
		ref new Platform::Collections::Vector<IPwmControllerProvider^>();

	controllerCollection->Append(provider);

	return controllerCollection->GetView();

}