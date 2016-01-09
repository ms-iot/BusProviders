// Copyright (c) Microsoft. All rights reserved.
#pragma once

#include <string>

namespace ArduinoProviders
{
	public ref class ArduinoConnectionConfiguration sealed
	{
	public:
		ArduinoConnectionConfiguration();
		ArduinoConnectionConfiguration(Platform::String ^vid, Platform::String ^pid, unsigned int baudRate);

		property Platform::String^ Vid { Platform::String^ get() { return _vid; } }
		property Platform::String^ Pid { Platform::String^ get() { return _pid; } }
		property unsigned int BaudRate { unsigned int get() { return _baudRate; } }

	private:
		Platform::String ^_vid;
		Platform::String ^_pid;
		unsigned int _baudRate;
	};
}
