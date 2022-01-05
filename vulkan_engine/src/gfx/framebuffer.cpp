#include "framebuffer.h"
#include "graphics.h"

Framebuffer::Framebuffer(VkRenderPass render_pass, const std::vector<VkImageView>& attachments)
{
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass;
    create_info.attachmentCount = attachments.size();
    create_info.pAttachments = attachments.data();
    //create_info.width = swapChainExtent.width;
    create_info.width = 1280; //HARDCODED
    //create_info.height = swapChainExtent.height;
    create_info.height = 720; //HARDCODED
    create_info.layers = 1;

    VE_CORE_ASSERT(vkCreateFramebuffer(Graphics::getLogicalDevice()->getLogicalDevice(), &create_info, nullptr, &framebuffer) == VK_SUCCESS,
        "Vulkan: Couldn't create a framebuffer");
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(Graphics::getLogicalDevice()->getLogicalDevice(), framebuffer, nullptr);
}
