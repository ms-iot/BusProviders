using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.Devices.Adc;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace SampleAdcConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        AdcChannel channelZero;
        AdcChannel channelOne;
        AdcChannel channelTwo;
        AdcChannel channelThree;

        ThreadPoolTimer timer;
        bool reading = false;
        AdcChannelMode mode;
        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
            mode = AdcChannelMode.SingleEnded;
            //mode = AdcChannelMode.Differential;
            AdcController controller = (await AdcController.GetControllersAsync(AdcAds1x15.AdcAds1x15Provider.GetAdcProvider(AdcAds1x15.Ads1x15Type.Ads1115)))[0];
            controller.ChannelMode = mode;
            channelZero = controller.OpenChannel(0);
            channelOne = controller.OpenChannel(1);
            channelTwo = controller.OpenChannel(2);
            channelThree = controller.OpenChannel(3);
            timer = ThreadPoolTimer.CreatePeriodicTimer(this.Tick, TimeSpan.FromMilliseconds(1000));

        }

        void Tick(ThreadPoolTimer sender)
        {

            if (mode == AdcChannelMode.SingleEnded)
            {
                System.Diagnostics.Debug.WriteLine("0: " + channelZero.ReadValue());
                System.Diagnostics.Debug.WriteLine("1: " + channelOne.ReadValue());
                System.Diagnostics.Debug.WriteLine("2: " + channelTwo.ReadValue());
                System.Diagnostics.Debug.WriteLine("3: " + channelThree.ReadValue());
            }
            else
            {
                System.Diagnostics.Debug.WriteLine("0 - 1: " + channelZero.ReadValue());
                System.Diagnostics.Debug.WriteLine("0 - 2: " + channelOne.ReadValue());
                System.Diagnostics.Debug.WriteLine("1 - 3: " + channelTwo.ReadValue());
                System.Diagnostics.Debug.WriteLine("2 - 3: " + channelThree.ReadValue());
            }

        }

    }
}
