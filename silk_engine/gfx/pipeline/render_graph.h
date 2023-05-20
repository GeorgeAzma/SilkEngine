#pragma once

#include "gfx/images/image.h"
#include "gfx/buffers/buffer.h"

class RenderPass;
class SwapChain;
class Fence;
class Semaphore;

class RenderGraph
{
public:
	friend class Resource;
	class Pass;
	class Resource
	{
		friend class RenderGraph;
	public:
		enum class Type
		{
			ATTACHMENT,
			BUFFER
		};

	public:
		Resource(const char* name, Pass* pass, size_t index)
			: name(name), pass(pass), index(index) {}
		Resource(const char* name, Pass* pass, size_t index, Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, const std::optional<VkClearValue>& clear_value = std::nullopt)
			: name(name), pass(pass), index(index), attachment_format(format), attachment_samples(samples), attachment_clear_value(clear_value) {}
		Resource(const char* name, Pass* pass, size_t index, VkDeviceSize size, Buffer::Usage usage, Allocation::Props allocation_props = {})
			: name(name), pass(pass), index(index), buffer_size(size), buffer_usage(usage), buffer_allocation_props(allocation_props) {}

		const shared<Image>& getAttachment() const;

	private:
		const char* name = nullptr;
		Pass* pass = nullptr;
		size_t index; 
		Type type = Type::ATTACHMENT; 
		
		Image::Format attachment_format = Image::Format::BGRA;
		VkSampleCountFlagBits attachment_samples = VK_SAMPLE_COUNT_1_BIT;
		std::optional<VkClearValue> attachment_clear_value = std::nullopt;
		size_t render_pass_attachment_index = 0;
		
		VkDeviceSize buffer_size = 0;
		Buffer::Usage buffer_usage = 0;
		Allocation::Props buffer_allocation_props = {};

	};

	friend class Pass;
	class Pass
	{
		friend class RenderGraph;
	public:
		Pass(const char* name, RenderGraph* render_graph, size_t index)
			: name(name), render_graph(render_graph), index(index) {}

		void setRenderCallback(std::function<void(const RenderGraph&)>&& render_callback) { this->render_callback = std::move(render_callback); }

		Resource& addAttachment(const char* name, Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, const std::vector<const Resource*>& inputs = {});
		Resource& addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearColorValue& color_clear_value, const std::vector<const Resource*>& inputs = {});
		Resource& addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearDepthStencilValue& depth_stencil_clear_value, const std::vector<const Resource*>& inputs = {});
		
		const shared<RenderPass>& getRenderPass() const { return render_pass; }

	private:
		Resource& addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const std::optional<VkClearValue>& clear_value, const std::vector<const Resource*>& inputs = {});

	private:
		const char* name = nullptr;
		RenderGraph* render_graph = nullptr;
		size_t index;
		std::vector<size_t> reads;
		std::vector<size_t> writes;
		shared<RenderPass> render_pass = nullptr;
		std::function<void(const RenderGraph&)> render_callback = nullptr;
	};

public:
	RenderGraph();

	Pass& addPass(const char* name = nullptr);
	void addRoot(Resource& resource) { roots.emplace_back(&resource); }

	void build();
	void print() const;
	void resize(const SwapChain& swap_chain);
	void render();

	void setClearColorValue(const char* attachment_name, const VkClearColorValue& color_clear_value);
	void setClearDepthStencilValue(const char* attachment_name, const VkClearDepthStencilValue& depth_stencil_clear_value);

	const Pass& getPass(std::string_view name) const { return *pass_map.at(name); }
	const shared<RenderPass>& getRenderPass(std::string_view name) const { return pass_map.at(name)->render_pass; }
	const Resource& getResource(std::string_view name) const { return *resource_map.at(name); }
	const shared<Image>& getAttachment(std::string_view name) const { return resource_map.at(name)->getAttachment(); }

private:
	void buildNode(size_t resource_index);

private:
	std::vector<unique<Pass>> passes;	
	std::vector<Pass*> sorted_passes;
	std::vector<const Resource*> roots;
	std::vector<unique<Resource>> resources;
	std::unordered_map<std::string_view, const Pass*> pass_map;
	std::unordered_map<std::string_view, const Resource*> resource_map;
	unique<Fence> previous_frame_finished = nullptr;
	unique<Semaphore> swap_chain_image_available = nullptr;
	unique<Semaphore> render_finished = nullptr;
};