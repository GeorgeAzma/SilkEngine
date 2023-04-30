#pragma once

enum class AudioFormat;

class RawAudio : NoCopy
{
private:
    struct RiffChunk
    {
        uint32_t        size;      // RIFF Chunk Size
        uint8_t         type[4];   // WAVE Header     
    };

    struct FormatChunk
    {
        uint32_t        size;               // Size of the fmt chunk
        uint16_t        audio_format;       // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
        uint16_t        channels;           // Number of channels 1=Mono 2=Sterio
        uint32_t        sample_rate;        // Sampling Frequency in Hz
        uint32_t        bytes_per_second;   // bytes per second
        uint16_t        block_align;        // 2=16-bit mono, 4=16-bit stereo
        uint16_t        bits_per_sample;    // Number of bits per sample
    };

    struct ListChunk
    {
        uint32_t size;
        uint8_t type[4];
    };

    struct DataChunk
    {
        uint32_t        size;    // Sampled data length
    };

public:
    RawAudio() = default;
    RawAudio(const path& file);

    static AudioFormat getFormat(uint8_t num_channels, uint8_t bit_depth);
    void load(const path& file);

public:
    std::vector<uint8_t> data{};
    AudioFormat format{};
    uint16_t sample_rate = 0;
};