#include "pch.h"  
#include "AdcAds1x15Provider.h"  
#include "AdcAds1x15ControllerProvider.h"  

using namespace AdcAds1x15;
using namespace Platform;
using namespace Windows::Devices::Adc::Provider;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace concurrency;


AdcAds1x15Provider::AdcAds1x15Provider(Ads1x15Type selectedType)
{
	type = selectedType;
}

IAdcProvider^ AdcAds1x15Provider::providerSingleton = nullptr;

IAdcProvider^ AdcAds1x15Provider::GetAdcProvider(Ads1x15Type type)
{
	if (!providerSingleton)
	{
		providerSingleton = ref new AdcAds1x15Provider(type);
	}
	return providerSingleton;
}

IAdcControllerCollection^ AdcAds1x15Provider::GetControllers()
{	
	AdcAds1x15ControllerProvider^ provider = ref new AdcAds1x15ControllerProvider(type);  

	Platform::Collections::Vector<IAdcControllerProvider^>^ controllerCollection =
		ref new Platform::Collections::Vector<IAdcControllerProvider^>();

	controllerCollection->Append(provider);

	return controllerCollection->GetView();
}
