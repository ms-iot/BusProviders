using System.Collections.Generic;
using Windows.Devices.Adc.Provider;

namespace AdcMcp3008
{
    public sealed class AdcMcp3008Provider : IAdcProvider
    {
        AdcMcp3008Provider()
        {
        }

        static IAdcProvider providerSingleton = null;

        static public IAdcProvider GetAdcProvider()
        {
            if (providerSingleton == null)
            {
                providerSingleton = new AdcMcp3008Provider();
            }
            return providerSingleton;

        }

        public IReadOnlyList<IAdcControllerProvider> GetControllers()
        {
            AdcMcp3008ControllerProvider provider = new AdcMcp3008ControllerProvider();

            List<IAdcControllerProvider> list = new List<IAdcControllerProvider>();
            list.Add(provider);

            return list;
        }
    }
}
