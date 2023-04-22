#include "framebuffer.h"
#include "gfx/render_context.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"

Framebuffer::Framebuffer(const SwapChain& swap_chain, VkRenderPass render_pass, uint32_t width, uint32_t height) : 
    swap_chain(swap_chain),
    render_pass(render_pass),
    framebuffers(swap_chain.getImageCount()),
    attachments(swap_chain.getImageCount()),
    width(width),
    height(height)
{
}

Framebuffer::~Framebuffer()
{
    for (size_t i = 0; i < framebuffers.size(); ++i)
        RenderContext::getLogicalDevice().destroyFramebuffer(framebuffers[i]);
}

Framebuffer& Framebuffer::addAttachment(Image::Props image_props)
{
    shared<Image> image = makeShared<Image>(image_props);
    for (size_t i = 0; i < framebuffers.size(); ++i)
        attachments[i].emplace_back(image);
    return *this;
}

Framebuffer& Framebuffer::addAttachment(Image::Format format, VkSampleCountFlagBits samples)
{
    Image::Props image_props{};
    image_props.format = format;
    image_props.width = width;
    image_props.height = height;
    image_props.sampler_props.mipmap_mode = Sampler::MipmapMode::NONE;
    image_props.samples = samples;
    image_props.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_props.allocation_props.preferred_device = Allocation::Device::GPU;
    image_props.allocation_props.priority = 1.0f;
    image_props.usage = Image::isDepthStencilFormat(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (samples != VK_SAMPLE_COUNT_1_BIT)
        image_props.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    addAttachment(image_props);
    return *this;
}

Framebuffer& Framebuffer::addSwapchainAttachments()
{
    const auto& images = swap_chain.getImages();
    for (size_t i = 0; i < images.size(); ++i)
        attachments[i].emplace_back(images[i]);
    return *this;
}

void Framebuffer::build(bool imageless)
{
    for (size_t i = 0; i < framebuffers.size(); ++i)
    {
        std::vector<VkImageView> attachment_views(attachments[i].size());
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
        ci.flags = imageless * VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
        framebuffers[i] = RenderContext::getLogicalDevice().createFramebuffer(ci);
    }
}

const std::vector<shared<Image>>& Framebuffer::getAttachments() const
{
   return attachments[swap_chain.getImageIndex()];
}

Framebuffer::operator const VkFramebuffer& () const
{ 
    return framebuffers[swap_chain.getImageIndex()];
}
