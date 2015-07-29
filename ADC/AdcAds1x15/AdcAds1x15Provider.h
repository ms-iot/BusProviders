// Copyright (c) Microsoft. All rights reserved.
#pragma once

namespace AdcAds1x15
{
	typedef Windows::Foundation::Collections::IVectorView<Windows::Devices::Adc::Provider::IAdcControllerProvider^> IAdcControllerCollection;

	public enum class Ads1x15Type { Ads1115, Ads1015 };

	public ref class AdcAds1x15Provider sealed : public Windows::Devices::Adc::Provider::IAdcProvider
	{
	public:
		virtual IAdcControllerCollection^ GetControllers(void);

		static Windows::Devices::Adc::Provider::IAdcProvider^ GetAdcProvider(Ads1x15Type type);

	private:
		AdcAds1x15Provider(Ads1x15Type type);
		Ads1x15Type type;

		static Windows::Devices::Adc::Provider::IAdcProvider^ providerSingleton;
	};


}