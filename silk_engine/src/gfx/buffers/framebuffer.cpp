#include "framebuffer.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"

Framebuffer::Framebuffer(VkRenderPass render_pass, uint32_t width, uint32_t height) : 
    render_pass(render_pass),
    framebuffers(Graphics::swap_chain->getImageCount()),
    attachments(Graphics::swap_chain->getImageCount()),
    width(width ? width : Window::getWidth()),
    height(height ? height : Window::getHeight())
{
}

Framebuffer::~Framebuffer()
{
    for (size_t i = 0; i < framebuffers.size(); ++i)
        Graphics::logical_device->destroyFramebuffer(framebuffers[i]);
}

Framebuffer& Framebuffer::addAttachment(const Image2DProps& image_props)
{
    if (image_props.samples != VK_SAMPLE_COUNT_1_BIT)
        multisampled = true;
    for (size_t i = 0; i < framebuffers.size(); ++i)
        attachments[i].emplace_back(makeShared<Image2D>(image_props));
    return *this;
}

Framebuffer& Framebuffer::addAttachment(ImageFormat format, VkSampleCountFlagBits samples)
{
    if (samples != VK_SAMPLE_COUNT_1_BIT)
        multisampled = true;
    Image2DProps image_props{};
    image_props.create_sampler = false;
    image_props.format = format;
    image_props.width = width;
    image_props.height = height;
    image_props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_props.mipmap = false;
    image_props.samples = samples;
    image_props.usage = ImageFormatEnum::hasDepth(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if(multisampled && samples == VK_SAMPLE_COUNT_1_BIT)
        image_props.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    addAttachment(image_props);
    return *this;
}

Framebuffer& Framebuffer::addAttachment(const std::vector<shared<Image2D>>& images)
{
    for (size_t i = 0; i < attachments.size(); ++i)
        attachments[i].emplace_back(images[i]);
    return *this;
}

void Framebuffer::build()
{
    for (size_t i = 0; i < framebuffers.size(); ++i)
    {
        std::vector<VkImageView> attachment_views(attachments.size());
        for (size_t j = 0; j < attachments[i].size(); ++j)
            attachment_views[j] = attachments[i][j]->getView();

        VkFramebufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = render_pass;
        ci.attachmentCount = attachment_views.size();
        ci.pAttachments = attachment_views.data();
        ci.width = width;
        ci.height = height;
        ci.layers = 1;
        framebuffers[i] = Graphics::logical_device->createFramebuffer(ci);
    }
}

const std::vector<shared<Image2D>>& Framebuffer::getAttachments() const
{
   return attachments[Graphics::swap_chain->getImageIndex()];
}

Framebuffer::operator const VkFramebuffer& () const
{ 
    return framebuffers[Graphics::swap_chain->getImageIndex()];
}
