#pragma once

#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/pipeline/compute_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"

class Image;
class Buffer;

// TODO:
class Material : NonCopyable
{
public:
	Material(const shared<GraphicsPipeline>& graphics_pipeline);
	Material(const shared<ComputePipeline>& compute_pipeline);

	void set(std::string_view name, const Buffer& buffer);
	void set(std::string_view name, const shared<Buffer>& buffer) { set(name, *buffer); }
	void set(std::string_view name, const std::vector<shared<Buffer>>& buffers);
	void set(std::string_view name, const VkDescriptorBufferInfo& buffer);
	void set(std::string_view name, const std::vector<VkDescriptorBufferInfo>& buffers);

	void set(std::string_view name, const Image& image);
	void set(std::string_view name, const shared<Image>& image) { set(name, *image); }
	void set(std::string_view name, const std::vector<shared<Image>>& images);
	void set(std::string_view name, const VkDescriptorImageInfo& image);
	void set(std::string_view name, const std::vector<VkDescriptorImageInfo>& images);

	void set(std::string_view name, VkBufferView buffer_view);

	void set(std::string_view name, std::vector<VkBufferView> buffer_views);

	void bind();

	const shared<Pipeline>& getPipeline() const { return pipeline; }

private:
	shared<Pipeline> pipeline;
	std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets;
};