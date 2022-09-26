#pragma once

class File
{
public:
	static std::vector<uint8_t> read(std::string_view file, std::ios::openmode open_mode = std::ios::binary);
	static void write(std::string_view file, const void* data, size_t size, std::ios::openmode open_mode = std::ios::trunc);
	static bool exists(std::string_view file);
	static std::filesystem::path directory(const std::filesystem::path& file);
};