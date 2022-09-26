#include "file.h"

std::vector<uint8_t> File::read(std::string_view file, std::ios::openmode open_mode)
{
	std::ifstream is(file.data(), std::ios::ate | open_mode);
	if (!is)
		return {};
	std::vector<uint8_t> buffer(is.tellg());
	is.seekg(std::ios::beg);
	is.read((char*)buffer.data(), buffer.size());
	return buffer;
}

void File::write(std::string_view file, const void* data, size_t size, std::ios::openmode open_mode)
{
	std::ofstream os(file.data(), open_mode);
	if (!os)
		return;
	os.write((const char*)data, size);
}

bool File::exists(std::string_view file)
{
	return std::filesystem::exists(file);
}

std::filesystem::path File::directory(const std::filesystem::path& file)
{
		return file.parent_path().string();
}