#pragma once

class Pipeline;
class ComputePipeline;
class GraphicsPipeline;
class DescriptorSet;
class Image;
class Buffer;

class Material : NoCopy
{
public:
	Material(const shared<Pipeline>& pipeline)
		: pipeline(pipeline) {}
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

	void set(std::string_view name, const std::vector<VkBufferView>& buffer_views);

	void bind();
	void dispatch(uint32_t global_invocation_count_x, uint32_t global_invocation_count_y = 1, uint32_t global_invocation_count_z = 1);

	const shared<Pipeline>& getPipeline() const { return pipeline; }
	const std::unordered_map<uint32_t, shared<DescriptorSet>>& getDescriptorSets() const { return descriptor_sets; }

private:
	const shared<DescriptorSet>& getDescriptorSet(uint32_t set);

private:
	shared<Pipeline> pipeline = nullptr;
	std::unordered_map<uint32_t, shared<DescriptorSet>> descriptor_sets = {};
};