#include "file.h"

std::string File::read(const std::filesystem::path& file, std::ios::openmode open_mode)
{
	std::ifstream is(file, std::ios::ate | open_mode);

	SK_ASSERT(is.is_open(), "Couldn't open file: {0}", file);

	size_t size = is.tellg();
	std::string buffer;
	buffer.resize(size);
	is.seekg(0);
	is.read(buffer.data(), size);

	return buffer;
}

void File::write(const std::filesystem::path& file, const void* data, size_t size, std::ios::openmode open_mode)
{
	auto directory = File::directory(file);
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directory(directory);

	std::ofstream os(file, open_mode);
	
	SK_ASSERT(os.is_open(), "Couldn't open file: {0}", file);

	if (size)
		os.write((const char*)data, size);
	else
		os << data;
}

bool File::exists(const std::filesystem::path& file)
{
	return std::filesystem::exists(file);
}

std::filesystem::path File::directory(const std::filesystem::path& file)
{
		return file.parent_path().string();
}