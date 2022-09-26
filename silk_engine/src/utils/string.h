#pragma once


class String
{
public:
	static std::vector<std::string> split(std::string_view str, char delimeter = ' ');
	static std::string replaceFirst(std::string str, std::string_view token, std::string_view to);
};