#pragma once

class GraphicsPipeline
{
public:
	GraphicsPipeline(const VkDevice* logical_device);
	~GraphicsPipeline();

private:
	const VkDevice* logical_device = nullptr;
};