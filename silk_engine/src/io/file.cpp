#include "file.h"

std::string File::read(const std::filesystem::path& file)
{
	std::ifstream is(file, std::ios::ate | std::ios::binary);

	SK_ASSERT(is.is_open(), "Couldn't open file: {0}", file);

	size_t size = is.tellg();
	std::string buffer;
	buffer.resize(size);
	is.seekg(0);
	is.read(buffer.data(), size);

	return buffer;
}

void File::write(const std::filesystem::path& file, const char* data, size_t size)
{
	std::ofstream os(file, std::ofstream::trunc);

	SK_ASSERT(os.is_open(), "Couldn't open file: {0}", file);

	if (size)
		os.write(data, size);
	else
		os << data;
}

bool File::exists(const std::filesystem::path& file)
{
	return std::filesystem::exists(file);
}