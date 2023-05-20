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
// Handle clear values and sets load operation to be always CLEAR
// Handle subpasses
// Handle subpass dependencies
// Handle buffers
// Handle compute pases
// Handle pipeline barriers well
// Handle multiple outputs(roots) including buffer outputs
// Handle RMW which should have LOAD_OP_LOAD and STORE_OP_STORE

RenderGraph::Resource& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const std::vector<const Resource*>& inputs)
{
	return addAttachment(name, format, samples, std::nullopt, inputs);
}
RenderGraph::Resource& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearColorValue& color_clear_value, const std::vector<const Resource*>& inputs)
{
	return addAttachment(name, format, samples, VkClearValue{ .color = color_clear_value }, inputs);
}

RenderGraph::Resource& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const VkClearDepthStencilValue& depth_stencil_clear_value, const std::vector<const Resource*>& inputs)
{
	return addAttachment(name, format, samples, VkClearValue{ .depthStencil = depth_stencil_clear_value }, inputs);
}

RenderGraph::Resource& RenderGraph::Pass::addAttachment(const char* name, Image::Format format, VkSampleCountFlagBits samples, const std::optional<VkClearValue>& clear_value, const std::vector<const Resource*>& inputs)
{
	size_t resource_index = render_graph->resources.size();
	writes.emplace_back(resource_index);
	render_graph->resources.emplace_back(makeUnique<Resource>(name, this, resource_index, format, samples, clear_value));
	for (const Resource* input : inputs)
		reads.emplace_back(input->index);
	return *render_graph->resources.back();
}

const shared<Image>& RenderGraph::Resource::getAttachment() const
{
	return pass->render_pass->getFramebuffer()->getAttachments()[render_pass_attachment_index];
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
	const Resource* root = roots[0]; // TODO: Support multiple roots
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
	/*
	
	Render Graph:
	Resources:
		Attachment:
			LOAD_OP:
				DONT_CARE
				LOAD
				CLEAR
			STORE_OP:
				DONT_CARE
				STORE
			FINAL_LAYOUT:
				DEPTH
				STENCIL
				DEPTH_STENCIL
				COLOR
				PRESENT_SRC
			STENCIL_LOAD_OP
			STENCIL_STORE_OP
		Buffer:
	Passes:
		Render Pass:
			Subpasses:
		Compute Pass:
	*/
	for (Pass* pass : sorted_passes)
	{
		pass->render_pass = makeShared<RenderPass>();
		VkSubpassDependency dep{};
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dep.srcAccessMask = 0;
		dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dep.dependencyFlags = 0;
		pass->render_pass->addSubpassDependency(dep);
		for (size_t write : pass->writes)
		{
			Resource& resource = *resources[write];
			switch (resource.type)
			{
			case Resource::Type::ATTACHMENT:
			{
				AttachmentProps props{};
				props.format = resource.attachment_format;
				if (Image::isDepthOnlyFormat(props.format))
					props.final_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				else if (Image::isStencilOnlyFormat(props.format))
					props.final_layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
				else if (Image::isDepthStencilFormat(props.format))
					props.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				else if (resource.index == root->index) // If color attachment is root, then it is present src
					props.final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				else
					props.final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				props.samples = resource.attachment_samples;
				props.load_operation = resource.attachment_clear_value.has_value() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE; // TODO:
				props.store_operation = VK_ATTACHMENT_STORE_OP_STORE; // TODO:
				resource.render_pass_attachment_index = pass->render_pass->addAttachment(props);
			}
				break;
			case Resource::Type::BUFFER:
				break;
			}
		}
		pass->render_pass->build(); 
		
		for (size_t write : pass->writes)
		{
			Resource& resource = *resources[write];
			if (resource.type == Resource::Type::ATTACHMENT && resource.attachment_clear_value.has_value())
			{
				if (Image::isDepthStencilFormat(resource.attachment_format))
					pass->render_pass->setClearDepthStencilValue(resource.render_pass_attachment_index, resource.attachment_clear_value->depthStencil);
				else
					pass->render_pass->setClearColorValue(resource.render_pass_attachment_index, resource.attachment_clear_value->color);
			}
		}
	}

	resize(Window::getActive().getSwapChain());

	for (Pass* pass : sorted_passes)
	{
		RenderContext::getLogicalDevice().setObjectName(VK_OBJECT_TYPE_RENDER_PASS, VkRenderPass(*pass->render_pass), pass->name);
		pass_map[pass->name] = pass;
	}

	for (auto& resource : resources)
	{
		switch (resource->type)
		{
		case Resource::Type::ATTACHMENT:
		RenderContext::getLogicalDevice().setObjectName(VK_OBJECT_TYPE_IMAGE, VkImage(*resource->getAttachment()), resource->name);
			break;
		case Resource::Type::BUFFER:
			// RenderContext::getLogicalDevice().setObjectName(VK_OBJECT_TYPE_BUFFER, VkBuffer(*resource->getAttachment()), resource->name); // TODO:
			break;
		}
		resource_map[resource->name] = resource.get();
	}
}

void RenderGraph::print() const
{
	SK_TRACE("|-------Render-Graph-------|");
	for (const Pass* pass : sorted_passes)
	{
		SK_TRACE("\t{}({}):", pass->name, pass->index);
		for (size_t write : pass->writes)
		{
			SK_TRACE("\t\t{}({})", resources[write]->name, resources[write]->index);
		}
	}
	SK_TRACE("|--------------------------|");
}

void RenderGraph::resize(const SwapChain& swap_chain)
{
	for (Pass* pass : sorted_passes)
		pass->render_pass->resize(swap_chain);
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

	for (uint32_t pass_index = 0; pass_index < sorted_passes.size(); ++pass_index)
	{
		
		auto& pass = *sorted_passes[pass_index];
		auto& render_pass = *pass.render_pass;
		uint32_t width = render_pass.getFramebuffer()->getWidth();
		uint32_t height = render_pass.getFramebuffer()->getHeight();
		
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

		render_pass.begin();
		pass.render_callback(*this);
		render_pass.end();
		
		if (pass_index < sorted_passes.size() - 1)
			for (size_t read : passes[pass_index + 1]->reads)
			{
				const Resource& resource = *resources[read];
				switch (resource.type)
				{
				case Resource::Type::ATTACHMENT:
					resource.getAttachment()->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					break;
				case Resource::Type::BUFFER:
					// TODO:
					break;
				}
			}
	}

	//query.end();

	RenderContext::submit(previous_frame_finished.get(), { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { *swap_chain_image_available }, { *render_finished });

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
	const Resource& attachment = *resource_map.at(attachment_name);
	attachment.pass->getRenderPass()->setClearColorValue(attachment.render_pass_attachment_index, color_clear_value);
}

void RenderGraph::setClearDepthStencilValue(const char* attachment_name, const VkClearDepthStencilValue& depth_stencil_clear_value)
{
	const Resource& attachment = *resource_map.at(attachment_name);
	attachment.pass->getRenderPass()->setClearDepthStencilValue(attachment.render_pass_attachment_index, depth_stencil_clear_value);
}

void RenderGraph::buildNode(size_t resource_index)
{
	const Resource& resource = *resources[resource_index];
	sorted_passes.push_back(resource.pass);
	for (size_t read : resource.pass->reads)
		buildNode(read);
}