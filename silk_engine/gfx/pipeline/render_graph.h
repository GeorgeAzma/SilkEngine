#pragma once

#include "gfx/images/image.h"
#include "gfx/buffers/buffer.h"

class RenderPass;
class SwapChain;
class Fence;
class Semaphore;

struct AttachmentInfo
{
	Image::Format format = Image::Format::BGRA;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
};

struct BufferInfo
{
	VkDeviceSize size = 0;
	Buffer::Usage usage = 0;
	Allocation::Props allocation_props{};
};

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
		Resource(const char* name, Pass* pass, size_t index, const AttachmentInfo& info)
			: name(name), pass(pass), index(index), attachment_info(info) {}
		Resource(const char* name, Pass* pass, size_t index, const BufferInfo& info)
			: name(name), pass(pass), index(index), buffer_info(info) {}

		const shared<Image>& getAttachment() const;

	private:
		const char* name = nullptr;
		Pass* pass = nullptr;
		size_t index; 
		Type type = Type::ATTACHMENT; 
		AttachmentInfo attachment_info{};
		BufferInfo buffer_info{};
		size_t render_pass_attachment_index = 0;
	};

	friend class Pass;
	class Pass
	{
		friend class RenderGraph;
	public:
		Pass(const char* name, RenderGraph* render_graph, size_t index)
			: name(name), render_graph(render_graph), index(index) {}

		void setRenderCallback(std::function<void(const RenderGraph&)>&& render_callback) { this->render_callback = std::move(render_callback); }

		Resource& addAttachment(const char* name = nullptr, const AttachmentInfo& info = {}, const std::vector<const Resource*>& inputs = {});
		
		const shared<RenderPass>& getRenderPass() const { return render_pass; }

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

	const shared<RenderPass>& getRenderPass(std::string_view name) const { return render_pass_map.at(name); }
	const Resource& getResource(std::string_view name) const { return *resource_map.at(name); }

private:
	void buildNode(size_t resource_index);

private:
	std::vector<unique<Pass>> passes;	
	std::vector<Pass*> sorted_passes;
	std::vector<const Resource*> roots;
	std::vector<unique<Resource>> resources;
	std::unordered_map<std::string_view, shared<RenderPass>> render_pass_map;
	std::unordered_map<std::string_view, const Resource*> resource_map;
	unique<Fence> previous_frame_finished = nullptr;
	unique<Semaphore> swap_chain_image_available = nullptr;
	unique<Semaphore> render_finished = nullptr;
};