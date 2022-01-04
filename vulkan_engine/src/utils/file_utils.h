#pragma once

class FileUtils
{
public:
	static std::vector<char> read(const std::string& file);
	static void write(const std::string& file, const char* data, size_t size);
private:
};