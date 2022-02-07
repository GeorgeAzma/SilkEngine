#include "render_pass.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/enums.h"
#include "gfx/window/swap_chain.h"

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

    if (samples != VK_SAMPLE_COUNT_1_BIT)
    {
        subpasses.back().multisampled = true;
    }
    
    subpasses.back().attachments.emplace_back(attachment_description);
    
    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = subpasses.back().attachments.size() - 1;
    if (EnumInfo::hasDepth(attachment_description.format) || EnumInfo::hasStencil(attachment_description.format))
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpasses.back().depth_stencil_attachment_reference = attachment_reference;
    }
    else if (subpasses.back().multisampled && image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpasses.back().resolve_attachment_reference = attachment_reference;
    }
    else
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpasses.back().color_attachment_references.emplace_back(attachment_reference);
    }
    
    subpasses.back().attachment_references.emplace_back(attachment_reference);
   
    return *this;
}

RenderPass& RenderPass::addSubpass()
{
    subpasses.emplace_back();

    if (subpasses.size() > 1)
    {
        subpasses.back().input_attachment_references = subpasses[subpasses.size() - 2].attachment_references;
        for (size_t i = 0; i < subpasses.back().input_attachment_references.size(); ++i)
        {
            VkAttachmentReference& input_attachment_reference = subpasses.back().input_attachment_references[i];
            input_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //bit hardcoded
        }
    }

    return *this;
}

void RenderPass::build()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpass_descriptions(subpasses.size());
    std::vector<VkSubpassDependency> subpass_dependencies(subpasses.size());
    for (size_t i = 0; i < subpasses.size(); ++i)
    {
        attachments.insert(attachments.end(), subpasses[i].attachments.begin(), subpasses[i].attachments.end());
        
        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = subpasses[i].input_attachment_references.size();
        subpass_description.pInputAttachments = subpasses[i].input_attachment_references.data();
        subpass_description.colorAttachmentCount = subpasses[i].color_attachment_references.size();
        subpass_description.pColorAttachments = subpasses[i].color_attachment_references.data();
        subpass_description.pDepthStencilAttachment = &subpasses[i].depth_stencil_attachment_reference;
        subpass_description.pResolveAttachments = &subpasses[i].resolve_attachment_reference;     
        subpass_descriptions[i] = std::move(subpass_description);

        VkSubpassDependency dependency{};
        dependency.srcSubpass = i ? (i - 1) : VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = i;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpass_dependencies[i] = std::move(dependency);
    }
    
    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.subpassCount = subpass_descriptions.size();
    create_info.pSubpasses = subpass_descriptions.data();
    create_info.dependencyCount = subpass_dependencies.size();
    create_info.pDependencies = subpass_dependencies.data();
    
    Graphics::vulkanAssert(vkCreateRenderPass(*Graphics::logical_device, &create_info, nullptr, &render_pass));
}

void RenderPass::begin(VkFramebuffer framebuffer, VkSubpassContents subpass_contents)
{
    if (Graphics::active.render_pass == render_pass)
        return;
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
            clear_value.color = { 0.15f, 0.4f, 0.6f, 1.0f }; //TEMP sky color (should be 0.0 normally)
        }
        clear_values[i] = clear_value;
    }

    render_pass_begin_info.clearValueCount = clear_values.size();
    render_pass_begin_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(Graphics::active.command_buffer, &render_pass_begin_info, subpass_contents);
    Graphics::active.render_pass = render_pass;
}

void RenderPass::nextSubpass()
{
    vkCmdNextSubpass(Graphics::active.command_buffer, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end()
{
    if (Graphics::active.render_pass != render_pass)
        return;

    vkCmdEndRenderPass(Graphics::active.command_buffer);

    Graphics::active.render_pass = VK_NULL_HANDLE;
    if (Graphics::active.bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
    {
        Graphics::active.pipeline = VK_NULL_HANDLE;
        Graphics::active.bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }
}