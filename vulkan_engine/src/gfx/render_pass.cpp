#include "render_pass.h"
#include "graphics.h"

RenderPass::RenderPass()
{
    VkAttachmentDescription color_attachment{};
    //color_attachment.format = swap_chain_image_format;
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

    VE_CORE_ASSERT(vkCreateRenderPass(Graphics::getLogicalDevice()->getLogicalDevice(), &create_info, nullptr, &render_pass) == VK_SUCCESS,
        "Vulkan: Couldn't create a render pass");
    

}

RenderPass::~RenderPass()
{
    vkDestroyRenderPass(Graphics::getLogicalDevice()->getLogicalDevice(), render_pass, nullptr);
}
