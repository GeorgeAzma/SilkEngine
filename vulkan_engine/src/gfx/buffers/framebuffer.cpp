#include "framebuffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

Framebuffer::Framebuffer(VkRenderPass render_pass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height)
{
    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass;
    create_info.attachmentCount = attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.width = width;
    create_info.height = height;
    create_info.layers = 1;

    Graphics::vulkanAssert(vkCreateFramebuffer(*Graphics::logical_device, &create_info, nullptr, &framebuffer));
}

Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(*Graphics::logical_device, framebuffer, nullptr);
}
