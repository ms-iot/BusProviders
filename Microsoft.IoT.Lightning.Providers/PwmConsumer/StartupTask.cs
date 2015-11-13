using Microsoft.IoT.Lightning.Providers;
using System;
using Windows.ApplicationModel.Background;
using Windows.Devices.Pwm;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace PwmConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        ThreadPoolTimer timer;
        double ClockwisePulseLength = 1;
        double CounterClockwisePulseLegnth = 2;
        double RestingPulseLegnth = 0;
        double currentPulseLength = 0;
        double secondPulseLength = 0;
        int iteration = 0;
        PwmPin motorPin;
        PwmPin secondMotorPin;
        PwmController pwmController;

        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            var deferral = taskInstance.GetDeferral();

            if (!LightningProvider.IsLightningEnabled)
            {
                // Lightning provider is required for this sample
                return;
            }
            pwmController = (await PwmController.GetControllersAsync(LightningPwmProvider.GetPwmProvider()))[0]; // [0]: LightningPCA9685PwmControllerProvider (selected)
                                                                                                                 // [1]: LightningSoftwarePwmControllerProvider
            pwmController.SetDesiredFrequency(50);
            motorPin = pwmController.OpenPin(0);
            motorPin.SetActiveDutyCyclePercentage(RestingPulseLegnth);
            motorPin.Start();
            secondMotorPin = pwmController.OpenPin(1);
            secondMotorPin.SetActiveDutyCyclePercentage(RestingPulseLegnth);
            secondMotorPin.Start();

            timer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(500));
        }

        private void Timer_Tick(ThreadPoolTimer timer)
        {
            iteration++;
            if (iteration % 3 == 0)
            {
                currentPulseLength = ClockwisePulseLength;
                secondPulseLength = CounterClockwisePulseLegnth;
            }
            else if (iteration % 3 == 1)
            {
                currentPulseLength = CounterClockwisePulseLegnth;
                secondPulseLength = ClockwisePulseLength;
            }
            else
            {
                currentPulseLength = 0;
                secondPulseLength = 0;
            }

            double desiredPercentage = currentPulseLength / (1000.0 / pwmController.ActualFrequency);
            motorPin.SetActiveDutyCyclePercentage(desiredPercentage);
            double secondDesiredPercentage = secondPulseLength / (1000.0 / pwmController.ActualFrequency);
            secondMotorPin.SetActiveDutyCyclePercentage(secondDesiredPercentage);
        }

        ~StartupTask()
        {
            motorPin.Stop();
        }
    }
}
