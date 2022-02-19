#pragma once

class File
{
public:
	static std::string read(const std::filesystem::path& file, std::ios::openmode open_mode = std::ios::binary);
	static void write(const std::filesystem::path& file, const void* data, size_t size, std::ios::openmode open_mode = std::ios::trunc);
	static bool exists(const std::filesystem::path& file);
};