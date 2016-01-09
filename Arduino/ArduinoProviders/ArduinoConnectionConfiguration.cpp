// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"  
#include "ArduinoConnectionConfiguration.h"

using namespace ArduinoProviders;

ArduinoConnectionConfiguration::ArduinoConnectionConfiguration()
{
	// default to
	_vid = ref new Platform::String(L"VID_2341");
	_pid = ref new Platform::String(L"PID_0043");
	_baudRate = 57600;
}

ArduinoConnectionConfiguration::ArduinoConnectionConfiguration(Platform::String ^vid, Platform::String ^pid, unsigned int baudRate)
{
	_vid = vid;
	_pid = pid;
	_baudRate = baudRate;
}

