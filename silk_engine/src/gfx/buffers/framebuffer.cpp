#include "framebuffer.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/devices/logical_device.h"

Framebuffer::Framebuffer(VkRenderPass render_pass, uint32_t width, uint32_t height) : 
    render_pass(render_pass),
    width(width ? width : Window::getWidth()),
    height(height ? height : Window::getHeight())
{
}

Framebuffer::~Framebuffer()
{
    Graphics::logical_device->destroyFramebuffer(framebuffer);
}

Framebuffer& Framebuffer::addAttachment(const FramebufferAttachmentProps& props)
{
    Image2DProps image_props{};
    image_props.create_sampler = false;
    image_props.format = props.format;
    image_props.width = (!props.width) ? width : props.width;
    image_props.height = (!props.height) ? height : props.height;
    image_props.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_props.mipmap = false;
    image_props.samples = props.samples;
    image_props.usage = props.usage;
    attachments.emplace_back(makeShared<Image2D>(image_props));
    return *this;
}

Framebuffer& Framebuffer::addAttachment(const shared<Image2D>& image)
{
    attachments.emplace_back(image);
    return *this;
}

void Framebuffer::build()
{
    std::vector<VkImageView> attachment_views(attachments.size());
    for (size_t i = 0; i < attachments.size(); ++i)
        attachment_views[i] = attachments[i]->getView();

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