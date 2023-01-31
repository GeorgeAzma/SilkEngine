#include "microphone.h"
#include <AL/alc.h>

Microphone::Microphone(const char* device_name, uint32_t sample_rate, AudioFormat format, size_t buffer_size)
	: sample_rate(sample_rate), format(format), buffer_size(buffer_size)
{
	device = alcCaptureOpenDevice(device_name, sample_rate, ALenum(format), buffer_size); //nullptr is default device

#ifdef SK_ENABLE_DEBUG_OUTPUT // Print audio devices
	if (!exists())
		return;

	const char* devices = alcGetString(nullptr, ALC_CAPTURE_DEVICE_SPECIFIER);

	if (!devices)
		devices = alcGetString(device, ALC_CAPTURE_DEVICE_SPECIFIER);

	if (devices)
	{
		SK_TRACE("----Microphones----");
		while (size_t len = strlen(devices))
		{
			SK_TRACE(std::string(devices, len));
			devices += len;
		}
		SK_TRACE("-------------------");
	}
#endif
}

Microphone::~Microphone()
{
	if (!exists())
		return;
	
	stop();
	if (!alcCaptureCloseDevice(device))
		SK_ERROR("Couldn't close Audio device");
}

void Microphone::start()
{
	if (recording)
		return;

	alcCaptureStart(device);
	recording = true;
}

void Microphone::getSamples(void* buffer, uint32_t samples) const
{
	alcCaptureSamples(device, buffer, samples);
}

uint32_t Microphone::getAvailableSampleCount() const
{
	ALCint samples = 0;
	alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples);
	return samples;
}

uint32_t Microphone::getAvailableSamples(void* buffer) const
{
	uint32_t samples = getAvailableSampleCount();
	getSamples(buffer, samples);
	return samples;
}

void Microphone::stop()
{
	if (!recording)
		return;

	alcCaptureStop(device);
	recording = false;
}