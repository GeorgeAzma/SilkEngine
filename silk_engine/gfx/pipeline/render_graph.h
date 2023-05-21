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
	class AttachmentNode
	{
		friend class RenderGraph;

	public:
		AttachmentNode(const char* name, Pass* pass, size_t index, Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, const std::optional<VkClearValue>& clear_value = std::nullopt)
			: name(name), pass(pass), index(index), format(format), samples(samples), clear_value(clear_value) {}

		const shared<Image>& getAttachment() const;

	private:
		const char* name = nullptr;
		Pass* pass = nullptr;
		size_t index; 
		
		Image::Format format = Image::Format::BGRA;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		std::optional<VkClearValue> clear_value = std::nullopt;
		size_t attachment_index = 0;
	};

	friend class Pass;
	class Pass
	{
		friend class RenderGraph;
	public:
		Pass(const char* name, RenderGraph* render_graph, size_t index)
			: name(name), render_graph(render_graph), index(index) {}

		void setRenderCallback(std::function<void(const RenderGraph&)>&& render_callback) { this->render_callback = std::move(render_callback); }

		AttachmentNode& addAttachment(const char* name, Image::Format format = Image::Format::BGRA, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, const std::vector<const AttachmentNode*>& inputs = {});
		AttachmentNode& addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearColorValue& color_clear_value, const std::vector<const AttachmentNode*>& inputs = {});
		AttachmentNode& addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearDepthStencilValue& depth_stencil_clear_value, const std::vector<const AttachmentNode*>& inputs = {});
		
		const shared<RenderPass>& getRenderPass() const { return render_graph->render_pass; }
		uint32_t getSubpass() const { return subpass; }

	private:
		AttachmentNode& addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const std::optional<VkClearValue>& clear_value, const std::vector<const AttachmentNode*>& inputs = {});

	private:
		const char* name = nullptr;
		RenderGraph* render_graph = nullptr;
		size_t index;
		std::vector<size_t> reads;
		std::vector<size_t> writes;
		std::function<void(const RenderGraph&)> render_callback = nullptr;
		uint32_t subpass = 0;
	};

public:
	RenderGraph();

	Pass& addPass(const char* name = nullptr);
	void addRoot(const AttachmentNode& resource) { roots.emplace_back(&resource); }

	void build();
	void print() const;
	void resize(const SwapChain& swap_chain);
	void render();

	void setClearColorValue(const char* attachment_name, const VkClearColorValue& color_clear_value);
	void setClearDepthStencilValue(const char* attachment_name, const VkClearDepthStencilValue& depth_stencil_clear_value);

	const Pass& getPass(std::string_view name) const { return *pass_map.at(name); }
	const shared<RenderPass>& getRenderPass(std::string_view name) const { return pass_map.at(name)->getRenderPass(); }
	const AttachmentNode& getResource(std::string_view name) const { return *resource_map.at(name); }
	const shared<Image>& getAttachment(std::string_view name) const { return resource_map.at(name)->getAttachment(); }

private:
	void buildNode(size_t resource_index);

private:
	std::vector<unique<Pass>> passes;	
	std::vector<Pass*> sorted_passes;
	shared<RenderPass> render_pass = nullptr;
	std::vector<const AttachmentNode*> roots;
	std::vector<unique<AttachmentNode>> resources;
	std::unordered_map<std::string_view, const Pass*> pass_map;
	std::unordered_map<std::string_view, const AttachmentNode*> resource_map;
	unique<Fence> previous_frame_finished = nullptr;
	unique<Semaphore> swap_chain_image_available = nullptr;
	unique<Semaphore> render_finished = nullptr;
};