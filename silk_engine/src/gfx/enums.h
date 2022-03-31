#pragma once

enum class APIVersion
{
	VULKAN_1_0,
	VULKAN_1_1,
	VULKAN_1_2,
};

class EnumInfo
{
public:
	static uint32_t apiVersion(APIVersion api_version);
	static bool needsStaging(VmaMemoryUsage usage);
	static std::string stringifyResult(VkResult result);
};