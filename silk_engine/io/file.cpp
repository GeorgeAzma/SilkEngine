#include "file.h"

std::string File::read(const fs::path& file, std::ios::openmode open_mode)
{
	std::ifstream is(file, std::ios::ate | open_mode);
	if (!is)
		return {};
	std::string buffer;
	buffer.resize(is.tellg());
	is.seekg(std::ios::beg);
	is.read(buffer.data(), buffer.size());
	return buffer;
}

void File::write(const fs::path& file, const void* data, size_t size, std::ios::openmode open_mode)
{
	std::ofstream os(file, open_mode);
	if (!os)
		return;
	os.write((const char*)data, size);
}

bool File::exists(const fs::path& file)
{
	return std::filesystem::exists(file);
}

fs::path File::directory(const fs::path& file)
{
	return file.parent_path();
}