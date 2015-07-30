// Copyright (c) Microsoft. All rights reserved.
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.System.Threading;
using Windows.Devices.Pwm;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace SamplePwmConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        ThreadPoolTimer timer;
        BackgroundTaskDeferral deferral;
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
            deferral = taskInstance.GetDeferral();
            // pwmController =  (await PwmController.GetControllersAsync(PwmPCA9685.PwmProviderPCA9685.GetPwmProvider()))[0];
            pwmController = (await PwmController.GetControllersAsync(PwmSoftware.PwmProviderSoftware.GetPwmProvider()))[0];
            pwmController.SetDesiredFrequency(50);
            motorPin = pwmController.OpenPin(13);
            motorPin.SetActiveDutyCyclePercentage(RestingPulseLegnth);
            motorPin.Start();
            secondMotorPin = pwmController.OpenPin(6);
            secondMotorPin.SetActiveDutyCyclePercentage(RestingPulseLegnth);
            secondMotorPin.Start();

            timer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromSeconds(2));

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
