#include "render_pass.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/framebuffer.h"
#include "gfx/window/swap_chain.h"
#include "gfx/subrender/subrender.h"

RenderPass::RenderPass(const std::vector<SubpassProps>& subpass_props)
    : subpass_count(subpass_props.size())
{
    std::vector<VkSubpassDescription> subpass_descriptions(subpass_props.size());
    std::vector<VkSubpassDependency> subpass_dependencies(subpass_props.size());
    std::vector<std::vector<VkAttachmentReference>> resolve_attachment_references(subpass_props.size());
    std::vector<std::vector<VkAttachmentReference>> color_attachment_references(subpass_props.size());
    std::vector<VkAttachmentReference> depth_attachment_references(subpass_props.size(), { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED });
    std::vector<std::vector<uint32_t>> preserve_attachment_references(subpass_props.size());
    std::vector<std::vector<VkAttachmentReference>> input_attachment_references(subpass_props.size());
    for (size_t subpass_index = 0; subpass_index < subpass_props.size(); ++subpass_index)
    {
        const auto& subpass = subpass_props[subpass_index];
        bool has_depth_output = false;
        for (const auto& output : subpass.outputs)
        {
            bool multisampled = output.samples != VK_SAMPLE_COUNT_1_BIT;
            bool depth = Image::isDepthFormat(output.format);
            has_depth_output |= depth;
            bool stencil = Image::isStencilFormat(output.format);
            bool color = !(depth || stencil);
            VkAttachmentDescription attachment_description{};
            attachment_description.format = VkFormat(output.format);
            attachment_description.samples = output.samples;
            attachment_description.storeOp = output.store_operation;
            attachment_description.loadOp = output.load_operation;
            attachment_description.stencilLoadOp = stencil ? output.stencil_load_operation : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment_description.stencilStoreOp = stencil ? output.stencil_store_operation : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment_description.initialLayout = output.initial_layout;
            attachment_description.finalLayout = output.final_layout;

            attachment_descriptions.emplace_back(attachment_description);
            // We need resolve attachments for each multisampled color attachment
            // If attachment layout is present_src then it should be set to color_attachment
            // And it's corresponding resolve attachment should get the present_src layout instead
            if (multisampled)
            {
                attachment_descriptions.back().storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                if (attachment_descriptions.back().finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                    attachment_descriptions.back().finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            if (color)
            {
                if (multisampled)
                {
                    color_attachment_references[subpass_index].emplace_back((uint32_t)attachment_descriptions.size() - 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

                    attachment_descriptions.emplace_back(attachment_description);
                    attachment_descriptions.back().samples = VK_SAMPLE_COUNT_1_BIT;
                    attachment_descriptions.back().loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    resolve_attachment_references[subpass_index].emplace_back((uint32_t)attachment_descriptions.size() - 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                }
                else
                {
                    color_attachment_references[subpass_index].emplace_back((uint32_t)attachment_descriptions.size() - 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                    resolve_attachment_references[subpass_index].emplace_back(VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED);
                }
            }
            // Handle Depth/Stencil attachments
            else if (depth && !stencil)
                depth_attachment_references[subpass_index] = { (uint32_t)attachment_descriptions.size() - 1, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL };
            else if (!depth && stencil)
                depth_attachment_references[subpass_index] = { (uint32_t)attachment_descriptions.size() - 1, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL };
            else if (depth && stencil)
                depth_attachment_references[subpass_index] = { (uint32_t)attachment_descriptions.size() - 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

            if (output.preserve) // If multisampled, only preserves resolved attachment??
                preserve_attachment_references[subpass_index].emplace_back(attachment_descriptions.size() - 1);
            
            if (output.clear_value)
                clear_values.emplace_back(*output.clear_value);
            else if (depth || stencil)
                clear_values.emplace_back(VkClearValue{ .depthStencil = { 1.0f, 0 } });
            else
                clear_values.emplace_back(VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } });
        }

        input_attachment_references[subpass_index].resize(subpass.inputs.size());
        for (size_t input_index = 0; input_index < subpass.inputs.size(); ++input_index)
        {
            const auto& input = subpass.inputs[input_index];
            input_attachment_references[subpass_index][input_index] = VkAttachmentReference(input, attachment_descriptions[input].finalLayout);
        }
        
        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = input_attachment_references[subpass_index].size();
        subpass_description.pInputAttachments = input_attachment_references[subpass_index].data();
        subpass_description.colorAttachmentCount = color_attachment_references[subpass_index].size();
        subpass_description.pColorAttachments = color_attachment_references[subpass_index].data();
        subpass_description.pResolveAttachments = resolve_attachment_references[subpass_index].data();
        subpass_description.pDepthStencilAttachment = &depth_attachment_references[subpass_index];
        subpass_description.preserveAttachmentCount = preserve_attachment_references[subpass_index].size();
        subpass_description.pPreserveAttachments = preserve_attachment_references[subpass_index].data();
        subpass_descriptions[subpass_index] = std::move(subpass_description);

        //TODO: Figure out how to do this automatically (I don't think it is possible to do it automatically though, without some performance implication) (good reference: https://www.reddit.com/r/vulkan/comments/s80reu/subpass_dependencies_what_are_those_and_why_do_i/)
        //NOTE: VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT does not support access mask VK_ACCESS_SHADER_READ_BIT
        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = subpass_index ? (subpass_index - 1) : VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = subpass_index;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        if (has_depth_output)
        {
            subpass_dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            subpass_dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            subpass_dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        subpass_dependencies[subpass_index] = std::move(subpass_dependency);
    }

    VkRenderPassCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = attachment_descriptions.size();
    ci.pAttachments = attachment_descriptions.data();
    ci.subpassCount = subpass_descriptions.size();
    ci.pSubpasses = subpass_descriptions.data();
    ci.dependencyCount = subpass_dependencies.size();
    ci.pDependencies = subpass_dependencies.data();
    render_pass = RenderContext::getLogicalDevice().createRenderPass(ci);
}

RenderPass::~RenderPass()
{
    RenderContext::getLogicalDevice().destroyRenderPass(render_pass);
}

void RenderPass::render()
{
    begin();
    uint32_t last_subpass = 0;
    for (auto&& [subpass, subrenders] : subrenders)
    {
        if (last_subpass != subpass)
        {
            nextSubpass();
            last_subpass = subpass;
        }
        for (auto& subrender : subrenders)
            if (subrender->enabled)
                subrender->render();
    }
    end();
}

void RenderPass::begin(VkSubpassContents subpass_contents)
{
    current_subpass = 0;
    RenderContext::record(
        [&](CommandBuffer& cb)
        {
            VkRenderPassBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            begin_info.renderPass = render_pass;
            begin_info.framebuffer = *framebuffer;

            begin_info.renderArea.offset = { 0, 0 };
            begin_info.renderArea.extent.width = framebuffer->getWidth();
            begin_info.renderArea.extent.height = framebuffer->getHeight();

            std::vector<VkClearValue> clear_values = this->clear_values;

            begin_info.clearValueCount = clear_values.size();
            begin_info.pClearValues = clear_values.data();

            cb.beginRenderPass(begin_info, subpass_contents);
        });
}

void RenderPass::nextSubpass(VkSubpassContents subpass_contents)
{
    if (current_subpass >= (subpass_count - 1))
        return;

    RenderContext::record([&](CommandBuffer& cb) { cb.nextSubpass(subpass_contents); });
    ++current_subpass;
}

void RenderPass::end()
{
    RenderContext::record([&](CommandBuffer& cb) { cb.endRenderPass(); });
}

void RenderPass::resize(const SwapChain& swap_chain)
{
    framebuffer = makeShared<Framebuffer>(swap_chain, *this, viewport.x ? viewport.x : swap_chain.getWidth(), viewport.y ? viewport.y : swap_chain.getHeight());
}
