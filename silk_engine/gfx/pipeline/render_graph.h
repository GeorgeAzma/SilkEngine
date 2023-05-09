#pragma once

class RenderPass;

class RenderGraph
{
	struct AttachmentInfo
	{
		uint32_t width = 0;
		uint32_t height = 0;
		VkFormat format = VK_FORMAT_UNDEFINED;
	};

	struct BufferInfo
	{
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
	};

	struct Resource
	{
		const char* name;
		std::optional<AttachmentInfo> attachment_info;
		std::optional<BufferInfo> buffer_info;
	};

	struct Node
	{
		const char* name;
		std::vector<Resource> inputs;
		std::vector<Resource> outputs;
	};
	 
public:
	void addRenderPass();

private:
	std::vector<shared<RenderPass>> render_passes;
};