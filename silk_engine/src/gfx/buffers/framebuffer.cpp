#include "framebuffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

Framebuffer::Framebuffer(VkRenderPass render_pass, const std::vector<shared<Image2D>>& attachments, uint32_t width, uint32_t height)
    : width(width), height(height), attachments(attachments)
{
    std::vector<VkImageView> attachment_views(this->attachments.size());
    for (size_t i = 0; i < this->attachments.size(); ++i)
        attachment_views[i] = this->attachments[i]->getDescriptorInfo().imageView;

    VkFramebufferCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    ci.renderPass = render_pass;
    ci.attachmentCount = attachment_views.size();
    ci.pAttachments = attachment_views.data();
    ci.width = width;
    ci.height = height;
    ci.layers = 1;
    framebuffer = Graphics::logical_device->createFramebuffer(ci);
}

Framebuffer::~Framebuffer()
{
    Graphics::logical_device->destroyFramebuffer(framebuffer);
}
