#include "framebuffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

Framebuffer::Framebuffer(vk::RenderPass render_pass, const std::vector<shared<Image2D>>& attachments, uint32_t width, uint32_t height)
    : width(width), height(height)
{
    std::vector<vk::ImageView> attachment_views(attachments.size());
    for (size_t i = 0; i < attachments.size(); ++i)
        attachment_views[i] = attachments[i]->getDescriptorInfo().imageView;

    vk::FramebufferCreateInfo ci{};
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
