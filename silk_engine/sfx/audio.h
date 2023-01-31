#pragma once

#include "audio_format.h"

class RawAudio;

class Audio : NonCopyable
{
public:
    Audio(const RawAudio& audio);
    Audio(const path& file);
    ~Audio();

    uint32_t getChannelCount() const;
    uint32_t getBitsPerSample() const;
    uint32_t getSamples() const;
    float getDuration() const;
    AudioFormat getFormat() const { return format; }
    uint32_t getSampleRate() const { return sample_rate; }
    uint32_t getSize() const { return size; }
    uint32_t getID() const { return id; }

    void setData(const void* data, size_t size);

private:
    AudioFormat format = AudioFormat::MONO16;
    uint32_t sample_rate = 0;
    size_t size = 0;
    uint32_t id = 0;
};