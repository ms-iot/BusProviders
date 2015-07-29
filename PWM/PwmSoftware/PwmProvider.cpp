#include "pch.h"
#include "PwmProvider.h"
#include "PwmControllerProvider.h"

using namespace PwmSoftware;
using namespace Platform;
using namespace Windows::Devices::Pwm;
using namespace Windows::Devices::Pwm::Provider;

Windows::Devices::Pwm::Provider::IPwmProvider^ PwmProviderSoftware::providerSingleton = nullptr;

PwmProviderSoftware::PwmProviderSoftware()
{

}

IPwmControllerCollection^ PwmProviderSoftware::GetControllers()
{
	PwmControllerProviderSoftware^ provider = ref new PwmControllerProviderSoftware();
	
	Platform::Collections::Vector<IPwmControllerProvider^>^ controllerCollection =
		ref new Platform::Collections::Vector<IPwmControllerProvider^>();

	controllerCollection->Append(provider);

	return controllerCollection->GetView();
}