#include "file.h"

std::string File::read(const path& file, std::ios::openmode open_mode)
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

void File::write(const path& file, const void* data, size_t size, std::ios::openmode open_mode)
{
	std::ofstream os(file, open_mode);
	if (!os)
		return;
	os.write((const char*)data, size);
}

bool File::exists(const path& file)
{
	return std::filesystem::exists(file);
}

path File::directory(const path& file)
{
	return file.parent_path();
}