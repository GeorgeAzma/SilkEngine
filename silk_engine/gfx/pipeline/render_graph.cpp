#include "render_graph.h"
#include "render_pass.h"
#include "gfx/render_context.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "gfx/buffers/framebuffer.h"
#include "gfx/fence.h"
#include "gfx/semaphore.h"
#include "gfx/devices/logical_device.h"
#include "gfx/allocators/query_pool.h"

// TODO:
// Handle preserved attachments
// Handle different stencil load/store operations from load/store operations
// Handle subpasses
// Handle subpass dependencies
// Handle buffers
// Handle compute pases
// Handle pipeline barriers well
// Handle multiple outputs(roots) including buffer outputs
// Handle RMW which should have LOAD_OP_LOAD and STORE_OP_STORE
// Handle different attachment sizes and blits

RenderGraph::AttachmentNode& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const std::vector<const AttachmentNode*>& inputs)
{
	return addAttachment(name, format, samples, std::nullopt, inputs);
}
RenderGraph::AttachmentNode& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearColorValue& color_clear_value, const std::vector<const AttachmentNode*>& inputs)
{
	return addAttachment(name, format, samples, VkClearValue{ .color = color_clear_value }, inputs);
}

RenderGraph::AttachmentNode& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearDepthStencilValue& depth_stencil_clear_value, const std::vector<const AttachmentNode*>& inputs)
{
	return addAttachment(name, format, samples, VkClearValue{ .depthStencil = depth_stencil_clear_value }, inputs);
}

RenderGraph::AttachmentNode& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const std::optional<VkClearValue>& clear_value, const std::vector<const AttachmentNode*>& inputs)
{
	size_t resource_index = render_graph->resources.size();
	writes.emplace_back(resource_index);
	render_graph->resources.emplace_back(makeUnique<AttachmentNode>(name, this, resource_index, format, samples, clear_value));
	for (const AttachmentNode* input : inputs)
		reads.emplace_back(input->index);
	return *render_graph->resources.back();
}

const shared<Image>& RenderGraph::AttachmentNode::getAttachment() const
{
	return pass->render_graph->render_pass->getFramebuffer()->getAttachments()[attachment_index];
}

RenderGraph::RenderGraph()
{
	previous_frame_finished = makeUnique<Fence>(true);
	swap_chain_image_available = makeUnique<Semaphore>();
	render_finished = makeUnique<Semaphore>();
}

RenderGraph::Pass& RenderGraph::addPass(const char* name)
{
	passes.emplace_back(makeUnique<Pass>(name, this, passes.size()));
	return *passes.back();
}

void RenderGraph::build()
{
	const AttachmentNode* root = roots[0]; // TODO: Support multiple roots
	sorted_passes.reserve(passes.size());
	buildNode(root->index);
	// Remove duplicates starting from root to leaves
	for (int i = 1; i < sorted_passes.size(); ++i)
	{
		auto it = std::find(sorted_passes.begin(), sorted_passes.begin() + i, sorted_passes[i]);
		if (it != sorted_passes.begin() + i)
		{
			sorted_passes.erase(it);
			--i;
		}
	}
	// Reverse to get submission ordered passes
	std::ranges::reverse(sorted_passes);

	render_pass = makeShared<RenderPass>();
	for (Pass* pass : sorted_passes)
	{
		pass->subpass = render_pass->addSubpass();

		for (size_t write : pass->writes)
		{
			AttachmentNode& write_resource = *resources[write];
			for (size_t read : pass->reads)
			{
				AttachmentNode& read_resource = *resources[read];

				VkSubpassDependency dep{};
				dep.srcSubpass = read_resource.pass->subpass;
				dep.dstSubpass = write_resource.pass->subpass;
				dep.srcStageMask = Image::isColorFormat(read_resource.format) ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dep.srcAccessMask = Image::isColorFormat(read_resource.format) ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				dep.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				dep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				render_pass->addSubpassDependency(dep);
				render_pass->addInputAttachment(read_resource.attachment_index);
			}


			AttachmentProps props{};
			props.format = write_resource.format;
			if (Image::isDepthOnlyFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			else if (Image::isStencilOnlyFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
			else if (Image::isDepthStencilFormat(props.format))
				props.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else if (write_resource.index == root->index) // If color attachment is root, then it is present src
				props.final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			else
				props.final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			props.samples = write_resource.samples;
			props.load_operation = write_resource.clear_value.has_value() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE; // TODO: LOAD_OP_LOAD for RMW
			props.store_operation = VK_ATTACHMENT_STORE_OP_STORE; // TODO: In case of depth attachments this is unnecessary, unless it's read from subsequent passes
			write_resource.attachment_index = render_pass->addAttachment(props);
	
		}
		render_pass->build(); 
		
		for (size_t write : pass->writes)
		{
			AttachmentNode& resource = *resources[write];
			if (resource.clear_value.has_value())
				if (Image::isDepthStencilFormat(resource.format))
					render_pass->setClearDepthStencilValue(resource.attachment_index, resource.clear_value->depthStencil);
				else
					render_pass->setClearColorValue(resource.attachment_index, resource.clear_value->color);

		}
	}

	resize(Window::getActive().getSwapChain());

	for (const Pass* pass : sorted_passes)
		pass_map[pass->name] = pass;

	for (const auto& resource : resources)
	{
		RenderContext::getLogicalDevice().setObjectName(VK_OBJECT_TYPE_IMAGE, VkImage(*resource->getAttachment()), resource->name);
		resource_map[resource->name] = resource.get();
	}
}

void RenderGraph::print() const
{
	SK_TRACE("|-------Render-Graph-------|");
	for (const Pass* pass : sorted_passes)
	{
		std::string reads = "";
		for (const size_t& read : pass->reads)
		{
			reads += std::to_string(read);
			if (&read != &pass->reads.back())
				reads += ", ";
		}
		SK_TRACE("\t{}({}){}:", pass->name, pass->index, reads.size() ? std::format("[{}]", reads) : "");
		for (size_t write : pass->writes)
		{
			const AttachmentNode& resource = *resources[write];
			SK_TRACE("\t\t{}({})", resource.name, resource.index);
		}
	}
	SK_TRACE("|--------------------------|");
}

void RenderGraph::resize(const SwapChain& swap_chain)
{
	render_pass->resize(swap_chain);
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

	//QueryPool query(VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT);
	//query.begin();

	render_pass->begin();

	uint32_t width = render_pass->getFramebuffer()->getWidth();
	uint32_t height = render_pass->getFramebuffer()->getHeight();

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

	for (uint32_t pass_index = 0; pass_index < sorted_passes.size(); ++pass_index)
	{
		auto& pass = *sorted_passes[pass_index];
		pass.render_callback(*this); 
		for (size_t write : pass.writes)
			resources[write]->getAttachment()->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		render_pass->nextSubpass();
	}
	render_pass->end();

	//query.end();

	RenderContext::submit(previous_frame_finished.get(), { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT }, { *swap_chain_image_available }, { *render_finished });

	//std::vector<uint32_t> results = query.getResults(0, true);
	//static Cooldown c = Cooldown(1.0f);
	//if (c())
	//{
	//	SK_INFO("----------------------------------");
	//	SK_INFO("Vertex Invocations: {}", results[0]);
	//	SK_INFO("Fragment Invocations: {}", results[1]);
	//	SK_INFO("Compute Invocations: {}", results[2]);
	//}

	if (!Window::getActive().getSwapChain().present(*render_finished))
	{
		Window::getActive().recreate();
		resize(Window::getActive().getSwapChain());
	}
}

void RenderGraph::setClearColorValue(const char* attachment_name, const VkClearColorValue& color_clear_value)
{
	const AttachmentNode& attachment = *resource_map.at(attachment_name);
	attachment.pass->getRenderPass()->setClearColorValue(attachment.attachment_index, color_clear_value);
}

void RenderGraph::setClearDepthStencilValue(const char* attachment_name, const VkClearDepthStencilValue& depth_stencil_clear_value)
{
	const AttachmentNode& attachment = *resource_map.at(attachment_name);
	attachment.pass->getRenderPass()->setClearDepthStencilValue(attachment.attachment_index, depth_stencil_clear_value);
}

void RenderGraph::buildNode(size_t resource_index)
{
	const AttachmentNode& resource = *resources[resource_index];
	sorted_passes.push_back(resource.pass);
	for (size_t read : resource.pass->reads)
		buildNode(read);
}