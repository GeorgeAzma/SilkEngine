#include "render_pass.h"
#include "graphics.h"
#include "graphics_state.h"

RenderPass::RenderPass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format = Graphics::swap_chain->getSurfaceFormat().format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    VkAttachmentReference color_attachment_reference{};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    //The index of the attachment in this array is directly 
    //referenced from the fragment shader with the 
    //layout(location = 0) out vec4 color directive
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;

    Graphics::vulkanAssert(vkCreateRenderPass(*Graphics::logical_device, &create_info, nullptr, &render_pass));
}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(*Graphics::logical_device, render_pass, nullptr);
}

void RenderPass::begin(VkFramebuffer framebuffer)
{
    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = *Graphics::render_pass;
    render_pass_begin_info.framebuffer = framebuffer;

    render_pass_begin_info.renderArea.offset = { 0, 0 };
    render_pass_begin_info.renderArea.extent = Graphics::swap_chain->getExtent(); //TODO: I think I can set this to the maximum size from framebuffers[i].attachments[j].size;

    VkClearValue clear_value = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_value;

    VE_CORE_ASSERT(graphics_state.command_buffer, "Command buffer was nullptr");
    vkCmdBeginRenderPass(*graphics_state.command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end()
{
    VE_CORE_ASSERT(graphics_state.command_buffer, "Command buffer was nullptr");
    vkCmdEndRenderPass(*graphics_state.command_buffer);
}