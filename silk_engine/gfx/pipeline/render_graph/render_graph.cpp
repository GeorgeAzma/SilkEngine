#include "render_graph.h"
#include "silk_engine/gfx/pipeline/render_pass.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/window/swap_chain.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/sync/fence.h"
#include "silk_engine/gfx/sync/semaphore.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/allocators/query_pool.h"

RenderGraph::~RenderGraph()
{
	RenderContext::getLogicalDevice().wait();
}

void RenderGraph::build(const char* backbuffer)
{
	render_finished = makeShared<Semaphore>();
	swap_chain_image_available = makeShared<Semaphore>();

	// Deduce which ones are root passes -> passes that have all the outputs unused are root passes
	std::vector<Pass*> roots;
	for (auto& pass : passes)
	{
		for (auto& pass2 : passes)
		{
			bool should_remove = false;
			if (pass != pass2)
			{
				for (const auto& input : pass->getInputs())
				{
					if (&input->getPass() == pass2.get())
					{
						should_remove = true;
						break;
					}
				}
			}
			if (!should_remove)
				roots.emplace_back(pass2.get());
		}
	}

	// Sort passes based on execution order
	sorted_passes.reserve(passes.size());
	auto build_node = [&](this auto&& self, Pass& pass) -> void
	{
		sorted_passes.emplace_back(&pass);
		for (const auto& input : pass.getInputs())
			self(input->getPass());
	};
	for (Pass* root : roots)
		build_node(*root);
	
	// Remove duplicate passes
	for (int i = 1; i < sorted_passes.size(); ++i)
	{
		auto it = std::find(sorted_passes.begin(), sorted_passes.begin() + i, sorted_passes[i]);
		if (it != sorted_passes.begin() + i)
		{
			sorted_passes.erase(it);
			--i;
		}
	}

	std::ranges::reverse(sorted_passes);

	render_pass = makeShared<RenderPass>();
	for (Pass* pass : sorted_passes)
	{
		pass->setSubpass(render_pass->addSubpass());
		for (Resource* output : pass->getOutputs())
		{
			for (Resource* input : pass->getInputs())
			{
				render_pass->addInputAttachment(input->attachment.index);
				render_pass->addSubpassDependency(input->getPass().getSubpass(), output->getPass().getSubpass(), 
					isColorFormat(input->attachment.format) ? PipelineStage::COLOR_ATTACHMENT_OUTPUT : PipelineStage::EARLY_FRAGMENT_TESTS, PipelineStage::FRAGMENT, 
					isColorFormat(input->attachment.format) ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 
					VK_ACCESS_INPUT_ATTACHMENT_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT);
			}
			
			AttachmentProps props{};
			props.format = output->attachment.format;
			if (isDepthOnlyFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			else if (isStencilOnlyFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
			else if (isDepthStencilFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else if (backbuffer && strcmp(output->getName(), backbuffer) == 0) // If color attachment is root, then it is present src
				props.final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			else
				props.final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			props.samples = output->attachment.samples;
			props.load_operation = output->attachment.clear.has_value() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE; // TODO: LOAD_OP_LOAD for RMW
			props.store_operation = VK_ATTACHMENT_STORE_OP_STORE; // TODO: In case of depth attachments this is unnecessary, unless it's read from subsequent passes
			output->attachment.index = render_pass->addAttachment(props);

		}
	}
	render_pass->build();

	// Set render pass attachment clear values
	for (Pass* pass : sorted_passes)
	{
		for (Resource* output : pass->getOutputs())
		{
			Resource& resource = *output;
			if (resource.attachment.clear.has_value())
				if (isDepthStencilFormat(resource.attachment.format))
					render_pass->setClearDepthStencilValue(resource.attachment.index, resource.attachment.clear->depthStencil);
				else
					render_pass->setClearColorValue(resource.attachment.index, resource.attachment.clear->color);

		}
	}

	resize(Window::get().getSwapChain());

	query_pool = makeShared<QueryPool>(QueryPool::VERTEX_SHADER_INVOCATIONS | QueryPool::GEOMETRY_SHADER_PRIMITIVES | QueryPool::FRAGMENT_SHADER_INVOCATIONS | QueryPool::COMPUTE_SHADER_INVOCATIONS);
}

void RenderGraph::render(Statistics* statistics)
{
	if (command_buffer)
		command_buffer->wait();

	if (!Window::get().getSwapChain().acquireNextImage(*swap_chain_image_available))
	{
		Window::get().recreate();
		resize(Window::get().getSwapChain());
	}

	if (statistics)
		query_pool->begin();
	render_pass->begin();

	uint32_t width = render_pass->getWidth();
	uint32_t height = render_pass->getHeight();

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = float(height);
	viewport.width = float(width);
	viewport.height = -float(height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	RenderContext::getCommandBuffer().setViewport({ viewport });

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { width, height };
	RenderContext::getCommandBuffer().setScissor({ scissor });

	for (const auto& pass : sorted_passes)
	{
		pass->callRender();
		for (Resource* output : pass->getOutputs())
			output->getAttachment()->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		render_pass->nextSubpass();
	}
	render_pass->end();
	if (statistics)
		query_pool->end();

	command_buffer = RenderContext::submit({ PipelineStage::TOP }, { *swap_chain_image_available }, { *render_finished });

	if (statistics)
	{
		std::vector<uint32_t> results = query_pool->getResults(0, true);
		memcpy(statistics, results.data(), results.size() * sizeof(uint32_t));
	}

	if (!Window::get().getSwapChain().present(*render_finished))
	{
		Window::get().recreate();
		resize(Window::get().getSwapChain());
	}
}

void RenderGraph::resize(const SwapChain& swap_chain)
{
	render_pass->resize(swap_chain);
}

const shared<Image>& RenderGraph::getAttachment(std::string_view resource_name) const
{
	return resources_map.at(resource_name)->getAttachment();
}
