#pragma once

class File
{
public:
	static std::string read(const std::string& file);
	static void write(const std::string& file, const char* data, size_t size);
private:
};