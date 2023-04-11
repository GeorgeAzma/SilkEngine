#include "includer.h"
#include "io/file.h"

shaderc_include_result* Includer::GetInclude(const char* requested_path, shaderc_include_type type, const char* requesting_path, size_t include_depth)
{
	std::string source = File::read(path("res/shaders") / requested_path);

	auto* container = new std::array<std::string, 2>;
	(*container)[0] = requested_path;
	(*container)[1] = source;

	auto* data = new shaderc_include_result;
	data->user_data = container;
	data->source_name = (*container)[0].data();
	data->source_name_length = (*container)[0].size();
	data->content = (*container)[1].data();
	data->content_length = (*container)[1].size();

	return data;
}

void Includer::ReleaseInclude(shaderc_include_result* data)
{
	delete static_cast<std::array<std::string, 2>*>(data->user_data);
	delete data;
}
