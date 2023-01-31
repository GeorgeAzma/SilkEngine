#pragma once

class File
{
public:
	static std::string read(const path& file, std::ios::openmode open_mode = std::ios::binary);
	static void write(const path& file, const void* data, size_t size, std::ios::openmode open_mode = std::ios::trunc);
	static bool exists(const path& file);
	static path directory(const path& file);
};