// Copyright (c) Microsoft. All rights reserved.
#pragma once  

namespace PwmPCA9685
{
	typedef Windows::Foundation::Collections::IVectorView<Windows::Devices::Pwm::Provider::IPwmControllerProvider^> IPwmControllerCollection;

	public ref class PwmProviderPCA9685 sealed : public Windows::Devices::Pwm::Provider::IPwmProvider
	{
	public:
		virtual IPwmControllerCollection^ GetControllers(void);

		static Windows::Devices::Pwm::Provider::IPwmProvider^ GetPwmProvider()
		{
			if (!providerSingleton)
			{
				providerSingleton = ref new PwmProviderPCA9685();
			}
			return providerSingleton;
		}
	private:
		PwmProviderPCA9685();
		static Windows::Devices::Pwm::Provider::IPwmProvider^ providerSingleton;
	};
}
