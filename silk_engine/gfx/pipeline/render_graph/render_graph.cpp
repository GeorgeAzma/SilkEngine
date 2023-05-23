#include "render_graph.h"
#include "silk_engine/gfx/pipeline/render_pass.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/window/swap_chain.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/fence.h"
#include "silk_engine/gfx/semaphore.h"
#include "silk_engine/gfx/devices/logical_device.h"

RenderGraph::~RenderGraph()
{
	RenderContext::getLogicalDevice().wait();
}

void RenderGraph::build(const char* backbuffer)
{
	previous_frame_finished = makeShared<Fence>(true);
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
				VkSubpassDependency dep{};
				dep.srcSubpass = input->getPass().getSubpass();
				dep.dstSubpass = output->getPass().getSubpass();
				dep.srcStageMask = Image::isColorFormat(input->attachment.format) ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dep.srcAccessMask = Image::isColorFormat(input->attachment.format) ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				render_pass->addSubpassDependency(dep);
				render_pass->addInputAttachment(input->attachment.index);
			}
			
			AttachmentProps props{};
			props.format = output->attachment.format;
			if (Image::isDepthOnlyFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			else if (Image::isStencilOnlyFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
			else if (Image::isDepthStencilFormat(props.format))
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
				if (Image::isDepthStencilFormat(resource.attachment.format))
					render_pass->setClearDepthStencilValue(resource.attachment.index, resource.attachment.clear->depthStencil);
				else
					render_pass->setClearColorValue(resource.attachment.index, resource.attachment.clear->color);

		}
	}

	resize(Window::getActive().getSwapChain());
}

void RenderGraph::render()
{
	previous_frame_finished->wait();
	previous_frame_finished->reset();

	if (!Window::getActive().getSwapChain().acquireNextImage(*swap_chain_image_available))
	{
		Window::getActive().recreate();
		resize(Window::getActive().getSwapChain());
	}

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

	RenderContext::submit(previous_frame_finished.get(), { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT }, { *swap_chain_image_available }, { *render_finished });

	if (!Window::getActive().getSwapChain().present(*render_finished))
	{
		Window::getActive().recreate();
		resize(Window::getActive().getSwapChain());
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
