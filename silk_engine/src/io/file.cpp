#include "file.h"

std::vector<char> File::read(const std::string& file)
{
	std::ifstream is(file, std::ios::ate | std::ios::binary);

	SK_ASSERT(is.is_open(), "Couldn't open file: {0}", file);

	size_t size = is.tellg();
	std::vector<char> buffer(size);

	is.seekg(0);

	is.read(buffer.data(), size);

	is.close();

	return buffer;
}

void File::write(const std::string& file, const char* data, size_t size)
{
	std::ofstream os(file, std::ofstream::trunc);

	SK_ASSERT(os.is_open(), "Couldn't open file: {0}", file);

	if (size)
	{
		os.write(data, size);
	}
	else
	{
		os << data;
	}

	os.close();
}
