#pragma once

class RawAudio;

class Audio : NonCopyable
{
public:
    Audio(const RawAudio& audio);
    Audio(std::string_view file);
    ~Audio();

    uint32_t getChannelCount() const;
    uint32_t getSize() const;
    uint32_t getBitsPerSample() const;
    uint32_t getSampleRate() const;
    uint32_t getSamples() const;
    float getDuration() const;
    uint32_t getID() const { return id; }

private:
    uint32_t id = 0;
};