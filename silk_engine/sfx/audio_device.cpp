#include "audio_device.h"
#include "audio_context.h"
#include <AL/alc.h>

AudioDevice::AudioDevice(const char* device_name)
{
	device = alcOpenDevice(device_name); //nullptr is default device
	if (!device)
		return;

	context = new AudioContext(device);
	
#ifdef SK_ENABLE_DEBUG_OUTPUT // Print audio devices
    const char* devices = nullptr;
	if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT"))
		devices = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);

    if (!devices)
		devices = alcGetString(device, ALC_DEVICE_SPECIFIER);
	
	if (devices)
	{
		SK_TRACE("----Audio Devices----");
		while (size_t len = strlen(devices))
		{
			SK_TRACE(std::string(devices, len));
			devices += len;
		}
		SK_TRACE("---------------------");
	}
#endif
}

AudioDevice::~AudioDevice()
{
	if (!device)
		return;

	delete context;

    if (!alcCloseDevice(device))
		SK_ERROR("Couldn't close Audio device");
}