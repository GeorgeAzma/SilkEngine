#include "render_pass.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/enums.h"
#include "gfx/window/swap_chain.h"

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(*Graphics::logical_device, render_pass, nullptr);
}

RenderPass& RenderPass::addAttachment(VkFormat format, VkImageLayout image_layout, std::optional<VkClearValue> clear_value, VkSampleCountFlagBits samples)
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

    Attachment attachment{};
    Attachment::Type type;
    if (clear_value)
        attachment.clear_value = *clear_value;
    else
        attachment.clear_value.color = { 0.0f, 0.0f, 0.0f, 1.0f };
    attachment.description = attachment_description;
    
    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = subpasses.back().attachments.size();
    if (Image::hasDepth(attachment_description.format) || Image::hasStencil(attachment_description.format))
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        type = Attachment::Type::DEPTH_STENCIL;
    }
    else if (subpasses.back().multisampled && image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Attachment::Type::RESOLVE;
    }
    else
    {
        attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        type = Attachment::Type::COLOR;
    }
    attachment.reference = attachment_reference;
    attachment.type = type;

    if (type == Attachment::Type::DEPTH_STENCIL)
        attachment.clear_value.depthStencil = { 1.0f, 0 };
    else
        attachment.clear_value.color = { 0.0f, 0.0f, 0.0f, 1.0f };
    
    subpasses.back().attachments.emplace_back(std::move(attachment));
   
    return *this;
}

RenderPass& RenderPass::addSubpass()
{
    subpasses.emplace_back();

    if (subpasses.size() > 1)
    {
        auto& last_subpass = subpasses[subpasses.size() - 2];
        std::vector<VkAttachmentReference> input_attachment_references(last_subpass.attachments.size());
        for (size_t i = 0; i < last_subpass.attachments.size(); ++i)
            input_attachment_references[i] = last_subpass.attachments[i].reference;
        subpasses.back().input_attachment_references = std::move(input_attachment_references);
    }

    return *this;
}

void RenderPass::build()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpass_descriptions(subpasses.size());
    std::vector<VkSubpassDependency> subpass_dependencies(subpasses.size());

    //We need these alive for till this function finishes
    std::vector<std::vector<VkAttachmentReference>> color_attachment_references(subpasses.size());
    std::vector<VkAttachmentReference> depth_stencil_attachment_references(subpasses.size());
    std::vector<VkAttachmentReference> resolve_attachment_references(subpasses.size());

    for (size_t i = 0; i < subpasses.size(); ++i)
    {
        //Sort attachments in 3 categories
        for (auto& attachment : subpasses[i].attachments)
        {
            switch (attachment.type)
            {
            case Attachment::Type::COLOR:
                color_attachment_references[i].emplace_back(attachment.reference);
                break;
            case Attachment::Type::DEPTH_STENCIL:
                depth_stencil_attachment_references[i] = attachment.reference;
                break;
            case Attachment::Type::RESOLVE:
                resolve_attachment_references[i] = attachment.reference;
                break;
            }
            attachments.emplace_back(attachment.description);
        }

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.inputAttachmentCount = subpasses[i].input_attachment_references.size();
        subpass_description.pInputAttachments = subpasses[i].input_attachment_references.data();
        subpass_description.colorAttachmentCount = color_attachment_references[i].size();
        subpass_description.pColorAttachments = color_attachment_references[i].data();
        subpass_description.pDepthStencilAttachment = &depth_stencil_attachment_references[i];
        subpass_description.pResolveAttachments = &resolve_attachment_references[i];
        subpass_descriptions[i] = std::move(subpass_description);

        //TODO:
        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = i ? (i - 1) : VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = i;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpass_dependencies[i] = std::move(subpass_dependency);
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
        clear_values[i] = subpasses.back().attachments[i].clear_value;

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