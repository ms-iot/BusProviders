#pragma once

namespace PwmSoftware
{
	typedef Windows::Foundation::Collections::IVectorView<Windows::Devices::Pwm::Provider::IPwmControllerProvider^> IPwmControllerCollection;

	public ref class PwmProviderSoftware sealed : public Windows::Devices::Pwm::Provider::IPwmProvider
	{
	public:
		virtual IPwmControllerCollection^ GetControllers(void);

		static Windows::Devices::Pwm::Provider::IPwmProvider^ GetPwmProvider()
		{
			if (!providerSingleton)
			{
				providerSingleton = ref new PwmProviderSoftware();
			}
			return providerSingleton;
		}
	private:
		PwmProviderSoftware();
		static Windows::Devices::Pwm::Provider::IPwmProvider^ providerSingleton;
	};
}
