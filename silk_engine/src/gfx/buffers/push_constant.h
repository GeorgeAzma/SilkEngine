#pragma once

class PushContant
{
	struct Data
	{
		std::vector<uint32_t> data;
		size_t offset = 0;
	};
public:
	void addConstant(const void* data, uint32_t size, VkShaderStageFlags shader_stage);
	void push(const void* data, VkShaderStageFlags shader_stage);

private:
	std::unordered_map<uint32_t, Data> offsets;
	size_t offset = 0;
};