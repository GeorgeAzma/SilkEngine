#pragma once

class File
{
public:
	static std::string read(const std::filesystem::path& file);
	static void write(const std::filesystem::path& file, const char* data, size_t size);
	static bool exists(const std::filesystem::path& file);
};