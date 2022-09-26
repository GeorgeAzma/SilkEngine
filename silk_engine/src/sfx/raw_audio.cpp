#include "raw_audio.h"
#include "audio_format.h"

RawAudio::RawAudio(std::string_view file)
{
	load(file);
}

AudioFormat RawAudio::getFormat(uint8_t num_channels, uint8_t bit_depth)
{
	AudioFormat format = AudioFormat::MONO16;
	if (num_channels == 1)
	{
		if (bit_depth == 8)
			format = AudioFormat::MONO8;
		else if (bit_depth == 16)
			format = AudioFormat::MONO16;
		return format;
	}
	else if (num_channels == 2)
	{
		if (bit_depth == 8)
			format = AudioFormat::STEREO8;
		else if (bit_depth == 16)
			format = AudioFormat::STEREO16;
		return format;
	}

	SK_ERROR("No WAV exists with {} channels and {} bit depth:", num_channels, bit_depth);
	return format;
}

void RawAudio::load(std::string_view file)
{
	std::string path = std::string("res/audio/") + file.data();

	std::ifstream wav_file(path.data(), std::ios::binary);
	SK_ASSERT(wav_file.is_open(), "Couldn't open WAV file: {}", path.data());

	char id[4];
	uint32_t block_align = 1;
	while (wav_file.read(id, sizeof(id)))
	{
		if (strncmp(id, "RIFF", 4) == 0)
		{
			RiffChunk riff_chunk{};
			wav_file.read((char*)&riff_chunk, sizeof(riff_chunk));
			SK_ASSERT((riff_chunk.type[0] == 'W' &&
					   riff_chunk.type[1] == 'A' &&
					   riff_chunk.type[2] == 'V' &&
					   riff_chunk.type[3] == 'E'), "Invalid riff chunk");
		}
		else if (strncmp(id, "LIST", 4) == 0)
		{
			ListChunk list_chunk{};
			wav_file.read((char*)&list_chunk, sizeof(list_chunk));
			std::vector<uint8_t> data(list_chunk.size - sizeof(ListChunk::type));
			wav_file.read((char*)data.data(), data.size());
			//You can do something with data here
		}
		else if (strncmp(id, "fmt ", 4) == 0)
		{
			FormatChunk format_chunk{};
			wav_file.read((char*)&format_chunk, sizeof(format_chunk));

			sample_rate = format_chunk.sample_rate;
			block_align = format_chunk.block_align;
			format = getFormat(format_chunk.channels, format_chunk.bits_per_sample);
		}
		else if (strncmp(id, "data", 4) == 0)
		{
			DataChunk data_chunk{};
			wav_file.read((char*)&data_chunk, sizeof(data_chunk));

			data.resize((data_chunk.size % block_align ? data_chunk.size + (block_align - data_chunk.size % block_align) : data_chunk.size));
			wav_file.read((char*)data.data(), data_chunk.size);
		}
		else
		{
			break;
		}
	}
}