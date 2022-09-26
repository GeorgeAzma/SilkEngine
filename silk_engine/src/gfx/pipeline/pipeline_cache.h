#pragma once

class PipelineCache : NonCopyable
{
private:
	struct Header
	{
		uint32_t length = 0;
		uint32_t version = 0;
		uint32_t vendor = 0;
		uint32_t device = 0;
		uint8_t uuid[VK_UUID_SIZE] = {};

		bool isValid() const;
		operator bool() const { return isValid(); }
	};
public:
	PipelineCache();
	~PipelineCache();

	operator const VkPipelineCache& () const { return pipeline_cache; }

private:
	VkPipelineCache pipeline_cache = nullptr;
};