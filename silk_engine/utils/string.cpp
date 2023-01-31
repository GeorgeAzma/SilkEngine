#include "string.h"

std::vector<std::string> String::split(std::string_view str, char delimeter)
{
	std::vector<std::string> sub_strings;

	size_t start = 0; 
	size_t end = str.find(delimeter);
	while (end != std::string::npos)
	{
		sub_strings.emplace_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(delimeter, start);
	}
	sub_strings.emplace_back(str.substr(start, end));

	return sub_strings;
}

std::string String::replaceFirst(std::string str, std::string_view token, std::string_view to) 
{
	const auto start_pos = str.find(token);
	if (start_pos == std::string::npos)
		return str;

	str.replace(start_pos, token.length(), to);
	return str;
}