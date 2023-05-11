#include "render_pass.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"
#include "gfx/buffers/framebuffer.h"
#include "gfx/window/swap_chain.h"

// TODO:
// figure stuff out with VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT
// Because of resolve attachments getting automatically added to attachment_descriptions, SubpassProps::inputs may not function as user would think, make it simpler
// Support subpass input depth attachments
RenderPass::RenderPass(const std::vector<SubpassProps>& subpass_props, const std::vector<VkSubpassDependency>& dependencies)
    : subpass_count(subpass_props.size())
{
    std::vector<VkSubpassDescription> subpass_descriptions(subpass_props.size());
    std::vector<VkSubpassDependency> subpass_dependencies = dependencies;
    std::vector<std::vector<VkAttachmentReference>> resolve_attachment_references(subpass_props.size());
    std::vector<std::vector<VkAttachmentReference>> color_attachment_references(subpass_props.size());
    std::vector<VkAttachmentReference> depth_attachment_references(subpass_props.size(), { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED });
    std::vector<std::vector<uint32_t>> preserve_attachment_references(subpass_props.size());
    std::vector<std::vector<VkAttachmentReference>> input_attachment_references(subpass_props.size());
    for (size_t subpass_index = 0; subpass_index < subpass_props.size(); ++subpass_index)
    {
        const auto& subpass = subpass_props[subpass_index];
        for (const auto& output : subpass.outputs)
        {
            size_t attachment_index = attachment_descriptions.size();

            VkAttachmentDescription attachment_description{};
            attachment_description.format = VkFormat(output.format);
            attachment_description.samples = output.samples;
            attachment_description.initialLayout = output.initial_layout;
            attachment_description.storeOp = output.store_operation;
            attachment_description.loadOp = output.load_operation;
            attachment_description.stencilLoadOp = output.stencil_load_operation;
            attachment_description.stencilStoreOp = output.stencil_store_operation;

            // RULE: Preserved subpass attachments always have LOAD_OP_LOAD and STORE_OP_STORE
            if (output.preserve)
            {
                attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                preserve_attachment_references[subpass_index].emplace_back(attachment_index);
            }

            // RULE: If not using stencil, stencil load and store ops should be set to DONT_CARE
            bool stencil = Image::isStencilFormat(output.format);
            if (!stencil)
            {
                attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }

            bool multisampled = output.samples != VK_SAMPLE_COUNT_1_BIT;
            if (multisampled)
            {
                if (attachment_description.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
                    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }

            if (Image::isColorFormat(output.format))
            {
                if (multisampled)
                {
                    attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    size_t resolve_attachment_index = attachment_descriptions.size();
                    VkAttachmentDescription resolve_attachment_description = attachment_description;
                    resolve_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
                    resolve_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    resolve_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    resolve_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    resolve_attachment_description.finalLayout = output.final_layout;
                    resolve_attachment_references[subpass_index].emplace_back(resolve_attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                    attachment_descriptions.emplace_back(std::move(resolve_attachment_description));
                    ++attachment_index;
                }
                else
                {
                    attachment_description.finalLayout = output.final_layout;
                    resolve_attachment_references[subpass_index].emplace_back(VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED);
                }
                color_attachment_references[subpass_index].emplace_back(attachment_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                clear_values.emplace_back(output.clear_value ? *output.clear_value : VkClearValue{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } });
            }
            else // Depth | Stencil
            {
                if (Image::isDepthOnlyFormat(output.format))
                    attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                else if (Image::isStencilOnlyFormat(output.format))
                    attachment_description.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                else if (Image::isDepthStencilFormat(output.format))
                    attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                else SK_ERROR("Render pass attachment is in incompatible format");
                depth_attachment_references[subpass_index].attachment = attachment_index;
                depth_attachment_references[subpass_index].layout = attachment_description.finalLayout;
                clear_values.emplace_back(output.clear_value ? *output.clear_value : VkClearValue{ .depthStencil = { 1.0f, 0 } });            
            }
            attachment_descriptions.emplace_back(std::move(attachment_description));
        }

        // RULE: Subpass input attachments are always either VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL or VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL or 
        // VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL or VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, depending on final layout of the input attachment
        input_attachment_references[subpass_index].resize(subpass.inputs.size());
        for (size_t i = 0; i < subpass.inputs.size(); ++i)
        {
            auto& subpass_input_attachment = input_attachment_references[subpass_index][i];
            subpass_input_attachment.attachment = subpass.inputs[i];

            auto& previous_subpass_output_attachment = attachment_descriptions[subpass.inputs[i]];
            // RULE: Subpasses that are used in a subsequent subpass as an input attachment always have STORE_OP_STORE
            previous_subpass_output_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            switch (previous_subpass_output_attachment.finalLayout)
            {
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                subpass_input_attachment.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                subpass_input_attachment.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
                break;
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                subpass_input_attachment.layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                subpass_input_attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            default:
                SK_ERROR("Previous subpass output attachment had an incompatible final layout to be used as an input attachment to subsequent subpasses");
            }
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
    }

    //TODO: (good reference: https://www.reddit.com/r/vulkan/comments/s80reu/subpass_dependencies_what_are_those_and_why_do_i/)
    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = VK_ACCESS_NONE;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependencies.emplace_back(std::move(subpass_dependency));

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
