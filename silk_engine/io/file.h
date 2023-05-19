#pragma once

class File
{
public:
	template <typename T> requires IsContainer<T>
	static void read(const fs::path& file, T& buffer, std::ios::openmode open_mode = 0)
	{
		std::ifstream is(file, std::ios::ate | open_mode);
		if (!is)
		{
			SK_ERROR("Couldn't read file: {}", file);
			return;
		}
		size_t size = is.tellg();
		buffer.resize(size / sizeof(typename T::value_type));
		SK_VERIFY(size % sizeof(typename T::value_type) == 0, "");
		is.seekg(std::ios::beg);
		is.read((char*)buffer.data(), size);
	}
	static std::string read(const fs::path& file, std::ios::openmode open_mode = 0);
	static void write(const fs::path& file, const void* data, size_t size, std::ios::openmode open_mode = std::ios::trunc);
	static bool exists(const fs::path& file);
	static fs::path directory(const fs::path& file);
};