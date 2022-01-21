#include "render_pass.h"
#include "gfx/graphics.h"

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(*Graphics::logical_device, render_pass, nullptr);
}

RenderPass& RenderPass::addAttachment(VkFormat format, VkImageLayout image_layout, VkSampleCountFlagBits samples)
{
    VkAttachmentDescription attachment_description{};
    attachment_description.format = format;
    attachment_description.samples = samples;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = image_layout;
    
    subpasses.back().attachments.emplace_back(attachment_description);
    
    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = subpasses.back().attachments.size() - 1;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (EnumInfo::hasDepth(attachment_description.format))
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    
    if (attachment_reference.layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        subpasses.back().color_attachment_references.emplace_back(attachment_reference);
    }
    else if(attachment_reference.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        || attachment_reference.layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
        || attachment_reference.layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL)
    {
        subpasses.back().depth_attachment_reference = attachment_reference;
    }
    subpasses.back().attachment_references.emplace_back(attachment_reference);
   
    return *this;
}

RenderPass& RenderPass::addResolveAttachment(VkFormat format)
{
    VkAttachmentDescription attachment_description{};
    attachment_description.format = format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    subpasses.back().attachments.emplace_back(attachment_description);

    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = subpasses.back().attachments.size() - 1;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpasses.back().resolve_attachment_reference = attachment_reference;

    return *this;
}

RenderPass& RenderPass::beginSubpass()
{
    subpasses.emplace_back();

    if (subpasses.size() > 1)
    {
        subpasses.back().input_attachments = subpasses[subpasses.size() - 2].attachment_references;
    }

    return *this;
}

void RenderPass::build()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpasses;
    subpasses.reserve(this->subpasses.size());
    for (const auto& subpass : this->subpasses)
    {
        attachments.insert(attachments.end(), subpass.attachments.begin(), subpass.attachments.end());
        
        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = subpass.input_attachments.size();
        subpass_description.pInputAttachments = subpass.input_attachments.data();
        subpass_description.colorAttachmentCount = subpass.color_attachment_references.size();
        subpass_description.pColorAttachments = subpass.color_attachment_references.data();
        subpass_description.pDepthStencilAttachment = &subpass.depth_attachment_reference;
        subpass_description.pResolveAttachments = &subpass.resolve_attachment_reference;

        subpasses.emplace_back(subpass_description);
    }
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.subpassCount = subpasses.size();
    create_info.pSubpasses = subpasses.data();
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;
    
    Graphics::vulkanAssert(vkCreateRenderPass(*Graphics::logical_device, &create_info, nullptr, &render_pass));
}

void RenderPass::begin(VkFramebuffer framebuffer, VkSubpassContents subpass_contents)
{
    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = framebuffer;

    render_pass_begin_info.renderArea.offset = { 0, 0 };
    render_pass_begin_info.renderArea.extent = Graphics::swap_chain->getExtent();

    std::vector<VkClearValue> clear_values(subpasses.back().attachments.size());
    for (size_t i = 0; i < clear_values.size(); ++i)
    {
        VkClearValue clear_value{};
        if (EnumInfo::hasDepth(subpasses.back().attachments[i].format))
        {
            clear_value.depthStencil = { 1.0f, 0 };
        }
        else
        {
            clear_value.color = { 0.0f, 0.0f, 0.0f, 1.0f };
        }
        clear_values[i] = clear_value;
    }

    render_pass_begin_info.clearValueCount = clear_values.size();
    render_pass_begin_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(Graphics::active.command_buffer, &render_pass_begin_info, subpass_contents);

    Graphics::active.render_pass = render_pass;
    Graphics::active.subpass = 0;
}

void RenderPass::end()
{
    vkCmdEndRenderPass(Graphics::active.command_buffer);

    Graphics::active.render_pass = VK_NULL_HANDLE;
}